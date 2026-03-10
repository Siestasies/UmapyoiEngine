/*!
\file   PlayFabAdminManager.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Provides Admin-tier PlayFab API operations for Uma Engine.

Owned by PlayFabManager. Receives a title-entity PFEntityHandle from
PlayFabManager after it authenticates with the Developer Secret Key via
PFAuthenticationGetEntityWithSecretKeyAsync. All API calls in this class
require that handle and must NEVER be used in release/public builds.

Each async operation heap-allocates its own XAsyncBlock (plus any context
it needs) and frees it inside the completion callback, so callers do not
manage async lifetimes manually.

Current scope:
  - TitleData : GetTitleData, SetTitleData, SetTitleDataBatch, DeleteTitleData

Planned scope:
  - Player management : GetPlayerProfile, BanPlayer
  - Economy / Catalog : GetCatalogItems, UpdateCatalogItems

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "PlayFabAdminManager.h"

#include <utility>
#include <algorithm>
#include <iterator>
#include "PlayFabManager.h"

void Uma_Engine::PlayFabAdminManager::SetTitleEntityHandle(PFEntityHandle titleEntityHandle)
{
    m_titleEntityHandle = titleEntityHandle;
}

bool Uma_Engine::PlayFabAdminManager::IsReady() const
{
    return m_titleEntityHandle != nullptr;
}

void Uma_Engine::PlayFabAdminManager::GetTitleData(const std::vector<std::string>& keys, OnAdminDataSuccess onSuccess, OnAdminFailure onFailure)
{
    std::vector<const char*> c_keys;
    c_keys.reserve(keys.size());
    std::transform(
        keys.begin(), 
        keys.end(), 
        std::back_inserter(c_keys), 
        [](std::string const& key) 
        {
            return key.c_str(); 
        });

    PFTitleDataManagementGetTitleDataRequest request;
    request.keys = (c_keys.empty()) ? nullptr : c_keys.data();
    request.keysCount = (c_keys.empty()) ? 0 : c_keys.size();

    auto* ctx = new GetTitleDataContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnGetTitleDataComplete;

    HRESULT hr = PFTitleDataManagementServerGetTitleDataAsync(
        m_titleEntityHandle,
        &request,
        &ctx->async
    );

    if (FAILED(hr))
    {
        DispatchError(hr, "GetTitleData (Launch)", ctx->onFailure);
        delete ctx;
    }

}

void Uma_Engine::PlayFabAdminManager::SetTitleData(const std::string& key, const std::string& value, OnAdminSuccess onSuccess, OnAdminFailure onFailure)
{
    // setting the request
    PFTitleDataManagementSetTitleDataRequest request;
    request.key = key.c_str();
    request.value = value.c_str();

    auto* ctx = new SetTitleDataContext{ std::move(onSuccess), std::move(onFailure) };

    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnSetTitleDataComplete;

    HRESULT hr = PFTitleDataManagementServerSetTitleDataAsync(
        m_titleEntityHandle,
        &request,
        &ctx->async
    );

    if (FAILED(hr))
    {
        DispatchError(hr, "SetTitleData (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::SetTitleDataBatch(const std::unordered_map<std::string, std::string>& kvPairs, OnAdminSuccess onSuccess, OnAdminFailure onFailure)
{
    for (auto const& kvPair : kvPairs)
    {
        SetTitleData(kvPair.first, kvPair.second, onSuccess, onFailure);
    }
}

void Uma_Engine::PlayFabAdminManager::DeleteTitleData(const std::string& key, OnAdminSuccess onSuccess, OnAdminFailure onFailure)
{
    SetTitleData(key, "", onSuccess, onFailure);
}

void Uma_Engine::PlayFabAdminManager::OnGetTitleDataComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<GetTitleDataContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);

    if (FAILED(hr))
    {
        DispatchError(hr, "GetTitleData (Recieving side)", ctx->onFailure);
        delete ctx;
        return;
    }

    size_t buffer_size = 0;
    hr = PFTitleDataManagementServerGetTitleDataGetResultSize(
        async,
        &buffer_size
    );

    if (FAILED(hr))
    {
        DispatchError(hr, "GetTitleData (Getting result size)", ctx->onFailure);
        delete ctx;
        return;
    }

    std::vector<uint8_t> buffer(buffer_size);
    PFTitleDataManagementGetTitleDataResult* result = nullptr;
    size_t buffer_used = 0;

    hr = PFTitleDataManagementServerGetTitleDataGetResult(
        async,
        buffer_size,
        &buffer,
        &result,
        &buffer_used
    );

    if (FAILED(hr))
    {
        DispatchError(hr, "GetTitleData (Getting result)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess)
    {
        std::unordered_map<std::string, std::string> out;

        for (size_t i = 0; i < result->dataCount; i++)
        {
            const PFStringDictionaryEntry& entry = result->data[i];

            out.emplace(entry.key, entry.value);
        }

        ctx->onSuccess(out);
    }
}

void Uma_Engine::PlayFabAdminManager::OnSetTitleDataComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<SetTitleDataContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, /*wait=*/false);

    if (SUCCEEDED(hr))
    {
        if (ctx->onSuccess)
            ctx->onSuccess();
    }
    else
    {
        DispatchError(hr, "SetTitleData", ctx->onFailure);
    }

    delete ctx;
}

void Uma_Engine::PlayFabAdminManager::DispatchError(HRESULT hr, const std::string& context, const OnAdminFailure& onFailure)
{
    if (!onFailure)
        return;

    std::string msg = "[PlayFabAdminManager] " + context + " failed: " + std::to_string(static_cast<double>(hr));
    onFailure(hr, msg);
}
