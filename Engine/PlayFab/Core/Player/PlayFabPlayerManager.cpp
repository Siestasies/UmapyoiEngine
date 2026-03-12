/*!
\file   PlayFabPlayerManager.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements PlayFabPlayerManager — all player-facing PlayFab operations.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "PlayFabPlayerManager.h"

#include <sstream>
#include <utility>
#include <algorithm>
#include <iterator>

// =============================================================================
//  Internal helpers
// =============================================================================

namespace
{
    std::vector<Uma_Engine::LeaderboardPlayerEntry> ParsePlayerLeaderboard(
        const PFLeaderboardsGetEntityLeaderboardResponse* response)
    {
        std::vector<Uma_Engine::LeaderboardPlayerEntry> out;
        if (!response) return out;

        out.reserve(response->rankingsCount);
        for (uint32_t i = 0; i < response->rankingsCount; ++i)
        {
            const PFLeaderboardsEntityLeaderboardEntry* raw = response->rankings[i];

            Uma_Engine::LeaderboardPlayerEntry e;
            e.rank        = raw->rank;
            e.displayName = raw->displayName ? raw->displayName : "";
            if (raw->entity)
                e.entityId = raw->entity->id ? raw->entity->id : "";
            if (raw->scoresCount > 0 && raw->scores[0])
                e.score = std::stod(raw->scores[0]);

            out.push_back(std::move(e));
        }
        return out;
    }
}

// =============================================================================
//  Handle management
// =============================================================================

void Uma_Engine::PlayFabPlayerManager::SetPlayerEntityHandle(PFEntityHandle handle)
{
    m_playerEntityHandle = handle;
}

void Uma_Engine::PlayFabPlayerManager::ClearPlayerEntityHandle()
{
    if (m_playerEntityHandle)
    {
        PFEntityCloseHandle(m_playerEntityHandle);
        m_playerEntityHandle = nullptr;
    }
}

bool Uma_Engine::PlayFabPlayerManager::IsLoggedIn() const
{
    return m_playerEntityHandle != nullptr;
}

// =============================================================================
//  Private helpers
// =============================================================================

bool Uma_Engine::PlayFabPlayerManager::GuardLoggedIn(const char* /*callerName*/) const
{
    return m_playerEntityHandle == nullptr;
}

void Uma_Engine::PlayFabPlayerManager::DispatchError(
    HRESULT               hr,
    const std::string&    context,
    const OnPlayerFailure& onFailure)
{
    if (!onFailure) return;

    char buf[12];
    snprintf(buf, sizeof(buf), "%08X", static_cast<unsigned>(hr));
    std::string msg = "[PlayFabPlayerManager] " + context + " failed: 0x" + buf;
    onFailure(hr, msg);
}

// =============================================================================
//  Account
// =============================================================================

void Uma_Engine::PlayFabPlayerManager::GetAccountInfo(
    OnKVResult     onSuccess,
    OnPlayerFailure onFailure)
{
    if (GuardLoggedIn("GetAccountInfo"))
    {
        DispatchError(E_FAIL, "GetAccountInfo — not logged in", onFailure);
        return;
    }

    PFAccountManagementGetAccountInfoRequest request{};
    // playFabId = nullptr → service returns the calling player's own account

    auto* ctx = new GetAccountInfoContext{};
    ctx->manager        = this;
    ctx->async.context  = ctx;
    ctx->async.callback = OnGetAccountInfoComplete;
    ctx->onSuccess      = std::move(onSuccess);
    ctx->onFailure      = std::move(onFailure);

    HRESULT hr = PFAccountManagementClientGetAccountInfoAsync(
        m_playerEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "GetAccountInfo (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabPlayerManager::SetDisplayName(
    const std::string& name,
    OnPlayerSuccess    onSuccess,
    OnPlayerFailure    onFailure)
{
    if (GuardLoggedIn("SetDisplayName"))
    {
        DispatchError(E_FAIL, "SetDisplayName — not logged in", onFailure);
        return;
    }

    PFAccountManagementUpdateUserTitleDisplayNameRequest request{};
    request.displayName = name.c_str();

    auto* ctx = new SetDisplayNameContext{};
    ctx->manager        = this;
    ctx->async.context  = ctx;
    ctx->async.callback = OnSetDisplayNameComplete;
    ctx->onSuccess      = std::move(onSuccess);
    ctx->onFailure      = std::move(onFailure);

    HRESULT hr = PFAccountManagementClientUpdateUserTitleDisplayNameAsync(
        m_playerEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "SetDisplayName (Launch)", ctx->onFailure);
        delete ctx;
    }
}

// =============================================================================
//  Player Data
// =============================================================================

void Uma_Engine::PlayFabPlayerManager::ReadData(
    const std::string& key,
    OnStringResult     onSuccess,
    OnPlayerFailure    onFailure)
{
    if (GuardLoggedIn("ReadData"))
    {
        DispatchError(E_FAIL, "ReadData — not logged in", onFailure);
        return;
    }

    const char* keyPtr = key.c_str();

    PFPlayerDataManagementGetUserDataRequest request{};
    request.keys      = &keyPtr;
    request.keysCount = 1;

    auto* ctx = new ReadDataContext{};
    ctx->manager        = this;
    ctx->async.context  = ctx;
    ctx->async.callback = OnReadDataComplete;
    ctx->key            = key;
    ctx->onSuccess      = std::move(onSuccess);
    ctx->onFailure      = std::move(onFailure);

    HRESULT hr = PFPlayerDataManagementClientGetUserDataAsync(
        m_playerEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "ReadData (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabPlayerManager::ReadMultipleData(
    const std::vector<std::string>& keys,
    OnKVResult                      onSuccess,
    OnPlayerFailure                 onFailure)
{
    if (GuardLoggedIn("ReadMultipleData"))
    {
        DispatchError(E_FAIL, "ReadMultipleData — not logged in", onFailure);
        return;
    }

    std::vector<const char*> c_keys;
    c_keys.reserve(keys.size());
    std::transform(keys.begin(), keys.end(), std::back_inserter(c_keys),
        [](const std::string& k) { return k.c_str(); });

    PFPlayerDataManagementGetUserDataRequest request{};
    request.keys      = c_keys.empty() ? nullptr : c_keys.data();
    request.keysCount = static_cast<uint32_t>(c_keys.size());

    auto* ctx = new ReadMultipleDataContext{};
    ctx->manager        = this;
    ctx->async.context  = ctx;
    ctx->async.callback = OnReadMultipleDataComplete;
    ctx->onSuccess      = std::move(onSuccess);
    ctx->onFailure      = std::move(onFailure);

    HRESULT hr = PFPlayerDataManagementClientGetUserDataAsync(
        m_playerEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "ReadMultipleData (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabPlayerManager::WriteData(
    const std::string& key,
    const std::string& value,
    OnPlayerSuccess    onSuccess,
    OnPlayerFailure    onFailure)
{
    if (GuardLoggedIn("WriteData"))
    {
        DispatchError(E_FAIL, "WriteData — not logged in", onFailure);
        return;
    }

    PFStringDictionaryEntry entry{};
    entry.key   = key.c_str();
    entry.value = value.c_str();

    PFPlayerDataManagementClientUpdateUserDataRequest request{};
    request.data      = &entry;
    request.dataCount = 1;

    auto* ctx = new WriteDataContext{};
    ctx->manager        = this;
    ctx->async.context  = ctx;
    ctx->async.callback = OnWriteDataComplete;
    ctx->onSuccess      = std::move(onSuccess);
    ctx->onFailure      = std::move(onFailure);

    HRESULT hr = PFPlayerDataManagementClientUpdateUserDataAsync(
        m_playerEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "WriteData (Launch)", ctx->onFailure);
        delete ctx;
    }
}

// =============================================================================
//  Title Data
// =============================================================================

void Uma_Engine::PlayFabPlayerManager::GetTitleData(
    const std::string& key,
    OnStringResult     onSuccess,
    OnPlayerFailure    onFailure)
{
    if (GuardLoggedIn("GetTitleData"))
    {
        DispatchError(E_FAIL, "GetTitleData — not logged in", onFailure);
        return;
    }

    const char* keyPtr = key.c_str();

    PFTitleDataManagementGetTitleDataRequest request{};
    request.keys      = &keyPtr;
    request.keysCount = 1;

    auto* ctx = new GetTitleDataContext{};
    ctx->manager        = this;
    ctx->async.context  = ctx;
    ctx->async.callback = OnGetTitleDataComplete;
    ctx->key            = key;
    ctx->onSuccess      = std::move(onSuccess);
    ctx->onFailure      = std::move(onFailure);

    HRESULT hr = PFTitleDataManagementClientGetTitleDataAsync(
        m_playerEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "GetTitleData (Launch)", ctx->onFailure);
        delete ctx;
    }
}

// =============================================================================
//  Statistics
// =============================================================================

void Uma_Engine::PlayFabPlayerManager::SubmitScore(
    const std::string& statName,
    double             value,
    OnPlayerSuccess    onSuccess,
    OnPlayerFailure    onFailure)
{
    if (GuardLoggedIn("SubmitScore"))
    {
        DispatchError(E_FAIL, "SubmitScore — not logged in", onFailure);
        return;
    }

    std::string scoreStr = std::to_string(static_cast<int64_t>(value));
    const char* scorePtr = scoreStr.c_str();

    PFStatisticsStatisticUpdate statUpdate{};
    statUpdate.name        = statName.c_str();
    statUpdate.scores      = &scorePtr;
    statUpdate.scoresCount = 1;

    const PFStatisticsStatisticUpdate* statUpdatePtr = &statUpdate;

    PFStatisticsUpdateStatisticsRequest request{};
    request.entity          = nullptr;    // uses the entity from the handle
    request.statistics      = &statUpdatePtr;
    request.statisticsCount = 1;

    auto* ctx = new SubmitScoreContext{};
    ctx->manager        = this;
    ctx->async.context  = ctx;
    ctx->async.callback = OnSubmitScoreComplete;
    ctx->onSuccess      = std::move(onSuccess);
    ctx->onFailure      = std::move(onFailure);

    HRESULT hr = PFStatisticsUpdateStatisticsAsync(
        m_playerEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "SubmitScore (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabPlayerManager::GetMyStats(
    const std::string& statName,
    OnStatResult       onSuccess,
    OnPlayerFailure    onFailure)
{
    if (GuardLoggedIn("GetMyStats"))
    {
        DispatchError(E_FAIL, "GetMyStats — not logged in", onFailure);
        return;
    }

    PFStatisticsGetStatisticsRequest request{};
    request.entity = nullptr;    // uses the entity from the handle

    auto* ctx = new GetMyStatsContext{};
    ctx->manager        = this;
    ctx->async.context  = ctx;
    ctx->async.callback = OnGetMyStatsComplete;
    ctx->statName       = statName;
    ctx->onSuccess      = std::move(onSuccess);
    ctx->onFailure      = std::move(onFailure);

    HRESULT hr = PFStatisticsGetStatisticsAsync(
        m_playerEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "GetMyStats (Launch)", ctx->onFailure);
        delete ctx;
    }
}

// =============================================================================
//  Leaderboards
// =============================================================================

void Uma_Engine::PlayFabPlayerManager::GetLeaderboard(
    const std::string&        name,
    uint32_t                  pageSize,
    uint32_t                  startPos,
    OnLeaderboardPlayerResult onSuccess,
    OnPlayerFailure           onFailure)
{
    if (GuardLoggedIn("GetLeaderboard"))
    {
        DispatchError(E_FAIL, "GetLeaderboard — not logged in", onFailure);
        return;
    }

    PFLeaderboardsGetEntityLeaderboardRequest request{};
    request.leaderboardName  = name.c_str();
    request.pageSize         = pageSize;
    request.startingPosition = &startPos;

    auto* ctx = new GetLeaderboardContext{};
    ctx->manager        = this;
    ctx->async.context  = ctx;
    ctx->async.callback = OnGetLeaderboardComplete;
    ctx->onSuccess      = std::move(onSuccess);
    ctx->onFailure      = std::move(onFailure);

    HRESULT hr = PFLeaderboardsGetLeaderboardAsync(
        m_playerEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "GetLeaderboard (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabPlayerManager::GetLeaderboardAroundMe(
    const std::string&        name,
    uint32_t                  pageSize,
    OnLeaderboardPlayerResult onSuccess,
    OnPlayerFailure           onFailure)
{
    if (GuardLoggedIn("GetLeaderboardAroundMe"))
    {
        DispatchError(E_FAIL, "GetLeaderboardAroundMe — not logged in", onFailure);
        return;
    }

    // Retrieve the player's entity key from the handle so the request knows "who is me".
    size_t entityKeyBufSize = 0;
    PFEntityGetEntityKeySize(m_playerEntityHandle, &entityKeyBufSize);
    std::vector<uint8_t> entityKeyBuf(entityKeyBufSize);
    const PFEntityKey* entityKey = nullptr;
    PFEntityGetEntityKey(
        m_playerEntityHandle, entityKeyBufSize, entityKeyBuf.data(), &entityKey, nullptr);

    PFLeaderboardsGetLeaderboardAroundEntityRequest request{};
    request.leaderboardName        = name.c_str();
    request.entity                 = entityKey;
    request.maxSurroundingEntries  = pageSize;

    auto* ctx = new GetLeaderboardAroundMeContext{};
    ctx->manager        = this;
    ctx->async.context  = ctx;
    ctx->async.callback = OnGetLeaderboardAroundMeComplete;
    ctx->onSuccess      = std::move(onSuccess);
    ctx->onFailure      = std::move(onFailure);

    HRESULT hr = PFLeaderboardsGetLeaderboardAroundEntityAsync(
        m_playerEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "GetLeaderboardAroundMe (Launch)", ctx->onFailure);
        delete ctx;
    }
}

// =============================================================================
//  Static callbacks
// =============================================================================

void Uma_Engine::PlayFabPlayerManager::OnGetAccountInfoComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<GetAccountInfoContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetAccountInfo (Receiving)", ctx->onFailure);
        delete ctx;
        return;
    }

    size_t bufferSize = 0;
    hr = PFAccountManagementClientGetAccountInfoGetResultSize(async, &bufferSize);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetAccountInfo (ResultSize)", ctx->onFailure);
        delete ctx;
        return;
    }

    std::vector<uint8_t> buffer(bufferSize);
    PFAccountManagementGetAccountInfoResult* result = nullptr;
    size_t bufferUsed = 0;

    hr = PFAccountManagementClientGetAccountInfoGetResult(
        async, bufferSize, buffer.data(), &result, &bufferUsed);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetAccountInfo (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess && result->accountInfo)
    {
        auto* info = result->accountInfo;
        std::unordered_map<std::string, std::string> out;
        out["playFabId"]    = info->playFabId ? info->playFabId : "";
        out["username"]     = info->username  ? info->username  : "";
        out["created"]      = std::to_string(static_cast<long long>(info->created));
        out["displayName"]  = (info->titleInfo && info->titleInfo->displayName)
                              ? info->titleInfo->displayName : "";
        ctx->onSuccess(out);
    }

    delete ctx;
}

void Uma_Engine::PlayFabPlayerManager::OnSetDisplayNameComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<SetDisplayNameContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (SUCCEEDED(hr))
    {
        if (ctx->onSuccess) ctx->onSuccess();
    }
    else
    {
        DispatchError(hr, "SetDisplayName", ctx->onFailure);
    }

    delete ctx;
}

void Uma_Engine::PlayFabPlayerManager::OnReadDataComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<ReadDataContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "ReadData (Receiving)", ctx->onFailure);
        delete ctx;
        return;
    }

    size_t bufferSize = 0;
    hr = PFPlayerDataManagementClientGetUserDataGetResultSize(async, &bufferSize);
    if (FAILED(hr))
    {
        DispatchError(hr, "ReadData (ResultSize)", ctx->onFailure);
        delete ctx;
        return;
    }

    std::vector<uint8_t> buffer(bufferSize);
    PFPlayerDataManagementClientGetUserDataResult* result = nullptr;
    size_t bufferUsed = 0;

    hr = PFPlayerDataManagementClientGetUserDataGetResult(
        async, bufferSize, buffer.data(), &result, &bufferUsed);
    if (FAILED(hr))
    {
        DispatchError(hr, "ReadData (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess)
    {
        std::string value;
        for (uint32_t i = 0; i < result->dataCount; ++i)
        {
            if (result->data[i].key && ctx->key == result->data[i].key)
            {
                if (result->data[i].value && result->data[i].value->value)
                    value = result->data[i].value->value;
                break;
            }
        }
        ctx->onSuccess(value);
    }

    delete ctx;
}

void Uma_Engine::PlayFabPlayerManager::OnReadMultipleDataComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<ReadMultipleDataContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "ReadMultipleData (Receiving)", ctx->onFailure);
        delete ctx;
        return;
    }

    size_t bufferSize = 0;
    hr = PFPlayerDataManagementClientGetUserDataGetResultSize(async, &bufferSize);
    if (FAILED(hr))
    {
        DispatchError(hr, "ReadMultipleData (ResultSize)", ctx->onFailure);
        delete ctx;
        return;
    }

    std::vector<uint8_t> buffer(bufferSize);
    PFPlayerDataManagementClientGetUserDataResult* result = nullptr;
    size_t bufferUsed = 0;

    hr = PFPlayerDataManagementClientGetUserDataGetResult(
        async, bufferSize, buffer.data(), &result, &bufferUsed);
    if (FAILED(hr))
    {
        DispatchError(hr, "ReadMultipleData (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess)
    {
        std::unordered_map<std::string, std::string> out;
        for (uint32_t i = 0; i < result->dataCount; ++i)
        {
            const char* k = result->data[i].key;
            const char* v = (result->data[i].value && result->data[i].value->value)
                ? result->data[i].value->value : "";
            out.emplace(k ? k : "", v);
        }
        ctx->onSuccess(out);
    }

    delete ctx;
}

void Uma_Engine::PlayFabPlayerManager::OnWriteDataComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<WriteDataContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (SUCCEEDED(hr))
    {
        if (ctx->onSuccess) ctx->onSuccess();
    }
    else
    {
        DispatchError(hr, "WriteData", ctx->onFailure);
    }

    delete ctx;
}

void Uma_Engine::PlayFabPlayerManager::OnGetTitleDataComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<GetTitleDataContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetTitleData (Receiving)", ctx->onFailure);
        delete ctx;
        return;
    }

    size_t bufferSize = 0;
    hr = PFTitleDataManagementClientGetTitleDataGetResultSize(async, &bufferSize);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetTitleData (ResultSize)", ctx->onFailure);
        delete ctx;
        return;
    }

    std::vector<uint8_t> buffer(bufferSize);
    PFTitleDataManagementGetTitleDataResult* result = nullptr;
    size_t bufferUsed = 0;

    hr = PFTitleDataManagementClientGetTitleDataGetResult(
        async, bufferSize, buffer.data(), &result, &bufferUsed);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetTitleData (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess)
    {
        std::string value;
        for (uint32_t i = 0; i < result->dataCount; ++i)
        {
            if (result->data[i].key && ctx->key == result->data[i].key)
            {
                if (result->data[i].value) value = result->data[i].value;
                break;
            }
        }
        ctx->onSuccess(value);
    }

    delete ctx;
}

void Uma_Engine::PlayFabPlayerManager::OnSubmitScoreComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<SubmitScoreContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (SUCCEEDED(hr))
    {
        if (ctx->onSuccess) ctx->onSuccess();
    }
    else
    {
        DispatchError(hr, "SubmitScore", ctx->onFailure);
    }

    delete ctx;
}

void Uma_Engine::PlayFabPlayerManager::OnGetMyStatsComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<GetMyStatsContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetMyStats (Receiving)", ctx->onFailure);
        delete ctx;
        return;
    }

    size_t bufferSize = 0;
    hr = PFStatisticsGetStatisticsGetResultSize(async, &bufferSize);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetMyStats (ResultSize)", ctx->onFailure);
        delete ctx;
        return;
    }

    std::vector<uint8_t> buffer(bufferSize);
    PFStatisticsGetStatisticsResponse* result = nullptr;
    size_t bufferUsed = 0;

    hr = PFStatisticsGetStatisticsGetResult(
        async, bufferSize, buffer.data(), &result, &bufferUsed);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetMyStats (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess)
    {
        for (uint32_t i = 0; i < result->statisticsCount; ++i)
        {
            const PFStatisticsEntityStatisticValue* val = result->statistics[i].value;
            if (!val || !val->name) continue;
            if (ctx->statName == val->name)
            {
                double score = 0.0;
                if (val->scoresCount > 0 && val->scores[0])
                    score = std::stod(val->scores[0]);
                ctx->onSuccess(ctx->statName, score);
                break;
            }
        }
    }

    delete ctx;
}

void Uma_Engine::PlayFabPlayerManager::OnGetLeaderboardComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<GetLeaderboardContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetLeaderboard (Receiving)", ctx->onFailure);
        delete ctx;
        return;
    }

    size_t bufferSize = 0;
    hr = PFLeaderboardsGetLeaderboardGetResultSize(async, &bufferSize);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetLeaderboard (ResultSize)", ctx->onFailure);
        delete ctx;
        return;
    }

    std::vector<uint8_t> buffer(bufferSize);
    PFLeaderboardsGetEntityLeaderboardResponse* result = nullptr;
    size_t bufferUsed = 0;

    hr = PFLeaderboardsGetLeaderboardGetResult(
        async, bufferSize, buffer.data(), &result, &bufferUsed);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetLeaderboard (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess)
        ctx->onSuccess(ParsePlayerLeaderboard(result));

    delete ctx;
}

void Uma_Engine::PlayFabPlayerManager::OnGetLeaderboardAroundMeComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<GetLeaderboardAroundMeContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetLeaderboardAroundMe (Receiving)", ctx->onFailure);
        delete ctx;
        return;
    }

    size_t bufferSize = 0;
    hr = PFLeaderboardsGetLeaderboardAroundEntityGetResultSize(async, &bufferSize);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetLeaderboardAroundMe (ResultSize)", ctx->onFailure);
        delete ctx;
        return;
    }

    std::vector<uint8_t> buffer(bufferSize);
    PFLeaderboardsGetEntityLeaderboardResponse* result = nullptr;
    size_t bufferUsed = 0;

    hr = PFLeaderboardsGetLeaderboardAroundEntityGetResult(
        async, bufferSize, buffer.data(), &result, &bufferUsed);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetLeaderboardAroundMe (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess)
        ctx->onSuccess(ParsePlayerLeaderboard(result));

    delete ctx;
}
