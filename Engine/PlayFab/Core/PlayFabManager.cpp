/*!
\file   PlayFabManager.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implementation of PlayFabManager, the engine-level facade for PlayFab integration.

Handles SDK initialization, task queue creation, service configuration, title
entity authentication via secret key, player login with custom ID, and orderly
shutdown of all PlayFab resources and async operations.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "PlayFabManager.h"

#include <sstream>
#include <utility>
#include <XTaskQueue.h>
#include <random>
#include <iomanip>

void Uma_Engine::PlayFabManager::SetCredentials(const std::string& titleId, const std::string& devSecretKey, OnAdminSuccess onAdminReady, OnAdminFailure onAdminFailed)
{
	m_titleId = titleId;
	m_devSecretKey = devSecretKey;
	m_onAdminReady = std::move(onAdminReady);
	m_onAdminFailed = std::move(onAdminFailed);
}

void Uma_Engine::PlayFabManager::Init()
{
	if (m_titleId.empty())
	{
		return;
	}

	// Guard against double initialization — SDK handles (task queue,
	// PFInitialize, service config) must only be created once.
	if (m_ready)
	{
		// Already initialized — just re-authenticate admin if credentials changed
		if (!m_devSecretKey.empty() && m_adminManager && !m_adminManager->IsReady())
		{
			AuthenticateAsTitle();
		}
		return;
	}

	// We create our own task queue instead of using the process default so
	// that we have an explicit handle we can terminate during Shutdown().
	// Terminating it cancels every pending async callback, which lets the
	// SDK uninitialise calls finish immediately instead of hanging forever.
	// Both work and completion ports use ThreadPool mode so the SDK fires
	// callbacks on OS thread-pool threads without us having to pump manually.
	HRESULT hr = XTaskQueueCreate(
		XTaskQueueDispatchMode::ThreadPool,
		XTaskQueueDispatchMode::ThreadPool,
		&m_taskQueue
	);

	if (FAILED(hr))
	{
		DispatchError(hr, "XTaskQueueCreate", m_onAdminFailed);
		return;
	}

	// Promote to process default so every XAsyncBlock created with a
	// nullptr queue (including ones inside the PlayFab SDK itself) routes
	// through our queue and can be drained on shutdown.
	XTaskQueueSetCurrentProcessTaskQueue(m_taskQueue);

	// Bootstraps the low-level PlayFab Core SDK (HTTP layer, retry
	// policies, etc.). Must be called exactly once and paired with
	// PFUninitializeAsync on teardown.
	hr = PFInitialize(nullptr);

	if (FAILED(hr))
	{
		DispatchError(hr, "PFInitialize", m_onAdminFailed);
		return;
	}

	// Layers the high-level service APIs (auth, leaderboards, data,
	// cloud-script) on top of Core. Must be uninitialised before Core
	// during shutdown (reverse init order).
	hr = PFServicesInitialize(nullptr);

	if (FAILED(hr))
	{
		DispatchError(hr, "PFServicesInitialize", m_onAdminFailed);
		return;
	}

	// A service-config handle binds a Title ID to its API endpoint.
	// All auth and entity calls reference this handle so the SDK knows
	// which title to talk to.
	std::string end_point = "https://" + m_titleId + ".playfabapi.com";

	hr = PFServiceConfigCreateHandle(
		end_point.c_str(),
		m_titleId.c_str(),
		&m_serviceConfig
	);

	if (FAILED(hr))
	{
		DispatchError(hr, "PFServiceConfigCreateHandle", m_onAdminFailed);
		return;
	}

	m_adminManager  = std::make_unique<PlayFabAdminManager>();
	m_playerManager = std::make_unique<PlayFabPlayerManager>();

	m_ready = true;

	if (!m_devSecretKey.empty())
	{
		AuthenticateAsTitle();
	}
}

void Uma_Engine::PlayFabManager::Update(float dt)
{
    (void)dt;

	if (m_adminManager)
	{
		m_adminManager->Update();
	}
}

void Uma_Engine::PlayFabManager::Shutdown()
{
	// Close entity handles first — the SDK refuses to uninitialise while
	// any entity handles are still open, so these must go before anything else.
	if (m_playerManager)
	{
		m_playerManager->ClearPlayerEntityHandle();
	}

	m_playerManager.reset();
	m_adminManager.reset();

	if (m_titleEntityHandle)
	{
		PFEntityCloseHandle(m_titleEntityHandle);
		m_titleEntityHandle = nullptr;
	}

	// Terminate the task queue with wait=true. This blocks until every
	// pending async callback has been cancelled, which prevents the SDK
	// uninitialise calls below from waiting on callbacks that will never run.
	if (m_taskQueue)
	{
		XTaskQueueTerminate(m_taskQueue, /*wait=*/true, nullptr, nullptr);
	}

	// Teardown order matters: Services before Core (reverse of init order).
	// Each uninitialise is async, so we block-wait on the XAsyncBlock to
	// ensure each layer is fully torn down before proceeding to the next.
	if (m_ready)
	{
		XAsyncBlock servicesUninitAsync{};
		HRESULT hr = PFServicesUninitializeAsync(&servicesUninitAsync);
		if (SUCCEEDED(hr))
			XAsyncGetStatus(&servicesUninitAsync, /*wait=*/true);
	}

	// Service config handle must be closed after Services uninit but
	// before Core uninit — Services may still reference it internally.
	if (m_serviceConfig)
	{
		PFServiceConfigCloseHandle(m_serviceConfig);
		m_serviceConfig = nullptr;
	}

	if (m_ready)
	{
		XAsyncBlock uninitAsync{};
		HRESULT hr = PFUninitializeAsync(&uninitAsync);
		if (SUCCEEDED(hr))
			XAsyncGetStatus(&uninitAsync, /*wait=*/true);
	}

	// Finally release the task queue handle itself now that nothing is using it.
	if (m_taskQueue)
	{
		XTaskQueueCloseHandle(m_taskQueue);
		m_taskQueue = nullptr;
	}

	m_ready = false;
	m_titleId.clear();
	m_devSecretKey.clear();
}

bool Uma_Engine::PlayFabManager::IsReady() const
{
    return m_ready;
}

bool Uma_Engine::PlayFabManager::HasAdminAccess() const
{
    return !m_devSecretKey.empty();
}

void Uma_Engine::PlayFabManager::LoginWithCustomID(const std::string& customId, bool createAccount, OnPlayerLoginSuccess onSuccess, OnPlayerLoginFailure onFailure)
{
	if (!m_ready)
	{
		DispatchError(E_FAIL, "LoginWithCustomID — PlayFabManager not ready.", onFailure);
		return;
	}

	PFAuthenticationLoginWithCustomIDRequest request{};
	request.createAccount = createAccount;
	request.customId = customId.c_str();

	// Heap-allocate a context that bundles the XAsyncBlock, the callbacks,
	// and a back-pointer to this manager. The XAsyncBlock::context field
	// stores a void* to the context so the static callback can recover it
	// via reinterpret_cast. Ownership transfers to the callback — it is
	// responsible for deleting the context when done (success or failure).
	auto* ctx = new LoginWithCustomIDContext{};
	ctx->manager = this;
	ctx->async.context = ctx;
	ctx->onSuccess = std::move(onSuccess);
	ctx->onFailure = std::move(onFailure);
	ctx->async.callback = OnLoginWithCustomIDComplete;

	HRESULT hr = PFAuthenticationLoginWithCustomIDAsync(
		m_serviceConfig,
		&request,
		&ctx->async
	);

	// If the async launch itself fails, the callback will never fire,
	// so we must delete the context here to avoid leaking it.
	if (FAILED(hr))
	{
		DispatchError(hr, "LoginWithCustomID — PFAuthenticationLoginWithCustomIDAsync failed.", ctx->onFailure);
		delete ctx;
	}
}

void Uma_Engine::PlayFabManager::Logout()
{
	if (m_playerManager)
		m_playerManager->ClearPlayerEntityHandle();
}

Uma_Engine::PlayFabAdminManager& Uma_Engine::PlayFabManager::Admin()
{
	return *(m_adminManager.get());
}

Uma_Engine::PlayFabPlayerManager& Uma_Engine::PlayFabManager::Player()
{
	return *(m_playerManager.get());
}

const Uma_Engine::PlayFabAdminManager& Uma_Engine::PlayFabManager::Admin() const
{
    return *(m_adminManager.get());
}

const Uma_Engine::PlayFabPlayerManager& Uma_Engine::PlayFabManager::Player() const
{
	return *(m_playerManager.get());
}

std::string Uma_Engine::PlayFabManager::GenerateUUID4()
{
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<uint64_t> dist;

	// Generate two random 64-bit values and splice them into the UUID
	// layout: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
	// where '4' is the version nibble and 'y' is the variant (8/9/a/b).
	uint64_t hi = dist(gen);
	uint64_t lo = dist(gen);

	// Clear bits 12-15 of hi then force them to 0100 (version 4).
	// In the final UUID string these bits land in the '4xxx' group.
	hi = (hi & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
	// Clear the top two bits of lo then force them to 10 (variant 1 / RFC 4122).
	// These bits land in the 'yxxx' group.
	lo = (lo & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

	// Format as 8-4-4-4-12 hex groups. The hi value covers the first three
	// groups (32+16+16 = 64 bits), lo covers the last two (16+48 = 64 bits).
	std::ostringstream ss;
	ss << std::hex << std::setfill('0')
		<< std::setw(8) << (hi >> 32) << '-'
		<< std::setw(4) << ((hi >> 16) & 0xFFFF) << '-'
		<< std::setw(4) << (hi & 0xFFFF) << '-'
		<< std::setw(4) << (lo >> 48) << '-'
		<< std::setw(12) << (lo & 0x0000FFFFFFFFFFFFULL);
	return ss.str();
}

void Uma_Engine::PlayFabManager::AuthenticateAsTitle()
{
	// build the request first
	PFAuthenticationGetEntityRequest request{};

	auto* ctx = new TitleAuthContext{};
	ctx->manager = this;

	ctx->async.context = ctx;
	ctx->async.queue = nullptr;
	ctx->async.callback = OnGetEntityWithSecretKeyComplete;

	HRESULT hr = PFAuthenticationGetEntityWithSecretKeyAsync(
		m_serviceConfig,
		m_devSecretKey.c_str(),
		&request,
		&ctx->async
	);

	// if failed trigger the callback
	if (FAILED(hr))
	{
		if (m_onAdminFailed)
		{
			std::ostringstream oss;
			oss << "[PlayFabManager] GetEntityWithSecretKey launch failed: HRESULT 0x"
				<< std::hex << std::uppercase << hr;
			m_onAdminFailed(hr, oss.str());
		}

		delete ctx;
	}
}

void Uma_Engine::PlayFabManager::DispatchError(HRESULT hr, const std::string& context, const OnAdminFailure& onFailure)
{
	if (!onFailure)
		return;
	
	std::ostringstream oss;
	oss << "[PlayFabManager] " << context << " failed: HRESULT 0x"
		<< std::hex << std::uppercase << hr;

	onFailure(hr, oss.str());
}


void Uma_Engine::PlayFabManager::OnGetEntityWithSecretKeyComplete(XAsyncBlock* async)
{
	// able to get the entity using the secret key

	auto* ctx = reinterpret_cast<TitleAuthContext*>(async->context);
	auto* manager = ctx->manager;

	PFEntityHandle entityHandle = nullptr;
	HRESULT hr = PFAuthenticationGetEntityWithSecretKeyGetResult(
		async,
		&entityHandle
	);

	if (SUCCEEDED(hr) && entityHandle != nullptr)
	{
		manager->m_titleEntityHandle = entityHandle;

		manager->m_adminManager->SetTitleEntityHandle(entityHandle);

		if (manager->m_onAdminReady)
		{
			manager->m_onAdminReady();
		}
	}
	else
	{
		if (manager->m_onAdminFailed)
		{
			std::ostringstream oss;
			oss << "[PlayFabManager] Title-entity auth failed: HRESULT 0x"
				<< std::hex << std::uppercase << hr;
			manager->m_onAdminFailed(hr, oss.str());
		}
	}

	delete ctx;
}

void Uma_Engine::PlayFabManager::OnLoginWithCustomIDComplete(XAsyncBlock* async)
{
	// Recover our context from the void* we stashed in XAsyncBlock::context.
	// This is the standard pattern for Xbox async callbacks: the static
	// callback casts async->context back to the original context struct.
	auto* ctx = reinterpret_cast<LoginWithCustomIDContext*>(async->context);

	// GetResult can optionally return a LoginResult payload (player profile,
	// newly-created flag, etc.) in a caller-supplied buffer. We only care
	// about the entity handle, so we pass 0/nullptr for the buffer params
	// to skip deserialising the payload entirely.
	PFEntityHandle entityHandle{ nullptr };
	HRESULT hr = PFAuthenticationLoginWithCustomIDGetResult(
		async,
		&entityHandle,
		0,          // bufferSize  — 0 = skip LoginResult payload
		nullptr,    // buffer
		nullptr,    // result
		nullptr     // bufferUsed
	);

	if (FAILED(hr))
	{
		DispatchError(hr, "OnLoginWithCustomIDComplete — GetResult failed.", ctx->onFailure);
		delete ctx;
		return;
	}

	ctx->manager->m_playerManager->SetPlayerEntityHandle(entityHandle);

	if (ctx->onSuccess)
		ctx->onSuccess();

	delete ctx;
}


