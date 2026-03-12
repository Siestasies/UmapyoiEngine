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

	// Create an explicit task queue so we can dispatch completions in Update()
	// and drain them during Shutdown(). Without this the process default queue
	// may not pump completions at shutdown, causing an infinite hang.
	HRESULT hr = XTaskQueueCreate(
		XTaskQueueDispatchMode::ThreadPool,   // work runs on threadpool
		XTaskQueueDispatchMode::ThreadPool,   // completions also on threadpool
		&m_taskQueue
	);

	if (FAILED(hr))
	{
		DispatchError(hr, "XTaskQueueCreate", m_onAdminFailed);
		return;
	}

	// Make it the process default so all nullptr-queue XAsyncBlocks use it.
	XTaskQueueSetCurrentProcessTaskQueue(m_taskQueue);

	hr = PFInitialize(nullptr);

	if (FAILED(hr))
	{
		DispatchError(hr, "PFInitialize", m_onAdminFailed);
		return;
	}

	hr = PFServicesInitialize(nullptr);

	if (FAILED(hr))
	{
		DispatchError(hr, "PFServicesInitialize", m_onAdminFailed);
		return;
	}

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
	// 1. Close entity handles — SDK won't uninitialise while handles are open.
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

	// 2. Terminate the task queue — cancels every pending async callback so
	//    the SDK uninitialise calls below can complete immediately.
	if (m_taskQueue)
	{
		XTaskQueueTerminate(m_taskQueue, /*wait=*/true, nullptr, nullptr);
	}

	// 3. Uninitialise services, then close the service config, then core.
	if (m_ready)
	{
		XAsyncBlock servicesUninitAsync{};
		HRESULT hr = PFServicesUninitializeAsync(&servicesUninitAsync);
		if (SUCCEEDED(hr))
			XAsyncGetStatus(&servicesUninitAsync, /*wait=*/true);
	}

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

	// 4. Release the task queue.
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
	
	// Allocate context — deleted in the callback
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
	auto* ctx = reinterpret_cast<LoginWithCustomIDContext*>(async->context);

	// entityHandle is always returned — we only need that.
	// buffer/result are optional (LoginResult payload); pass 0/nullptr to skip.
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


