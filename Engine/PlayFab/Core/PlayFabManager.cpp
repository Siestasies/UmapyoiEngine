#include "PlayFabManager.h"

#include <sstream>
#include <utility>

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
	
	HRESULT hr = PFInitialize(nullptr);

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

	m_adminManager = std::make_unique<PlayFabAdminManager>();

	m_ready = true;

	if (!m_devSecretKey.empty())
	{
		AuthenticateAsTitle();
	}
}

void Uma_Engine::PlayFabManager::Update(float dt)
{
    (void)dt;
}

void Uma_Engine::PlayFabManager::Shutdown()
{
	m_adminManager.reset();
	
	if (m_titleEntityHandle)
	{
		PFEntityCloseHandle(m_titleEntityHandle);
		m_titleEntityHandle = nullptr;
	}

	if (m_serviceConfig)
	{
		PFServiceConfigCloseHandle(m_serviceConfig);
		m_serviceConfig = nullptr;
	}

	if (m_ready)
	{
		XAsyncBlock servicesUninitAsync{};
		HRESULT hr = PFServicesUninitializeAsync(&servicesUninitAsync);
		if (SUCCEEDED(hr))
			XAsyncGetStatus(&servicesUninitAsync, /*wait=*/true);

		XAsyncBlock uninitAsync{};
		hr = PFUninitializeAsync(&uninitAsync);
		if (SUCCEEDED(hr))
			XAsyncGetStatus(&uninitAsync, /*wait=*/true);
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

Uma_Engine::PlayFabAdminManager& Uma_Engine::PlayFabManager::Admin()
{
	return *(m_adminManager.get());
}

const Uma_Engine::PlayFabAdminManager& Uma_Engine::PlayFabManager::Admin() const
{
    return *(m_adminManager.get());
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
	
	std::string msg = "[PlayFabManager] " + context + " failed: " + std::to_string(static_cast<double>(hr));
	onFailure(hr, msg);
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

