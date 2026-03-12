/*!
\file   PlayFabAdminManager.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Provides Admin-tier PlayFab API operations for Uma Engine.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "PlayFabAdminManager.h"

#include <utility>
#include <algorithm>
#include <iterator>
#include "../PlayFabManager.h"

// =============================================================================
//  Internal helpers
// =============================================================================

namespace
{
    /// Rehydrate a PFJsonObject into a std::string.
    /// PFJsonObject::stringValue is a null-terminated char* (may be nullptr).
    std::string JsonObjectToString(PFJsonObject const& json)
    {
        if (json.stringValue && json.stringValue[0] != '\0')
            return json.stringValue;
        return {};
    }
}

// =============================================================================
//  Lifecycle
// =============================================================================

void Uma_Engine::PlayFabAdminManager::SetTitleEntityHandle(PFEntityHandle titleEntityHandle)
{
    m_titleEntityHandle = titleEntityHandle;
}

bool Uma_Engine::PlayFabAdminManager::IsReady() const
{
    return m_titleEntityHandle != nullptr;
}

// =============================================================================
//  Title Data
// =============================================================================

void Uma_Engine::PlayFabAdminManager::GetTitleData(
    const std::vector<std::string>& keys,
    OnAdminDataSuccess               onSuccess,
    OnAdminFailure                   onFailure)
{
    std::vector<const char*> c_keys;
    c_keys.reserve(keys.size());
    std::transform(keys.begin(), keys.end(), std::back_inserter(c_keys),
        [](const std::string& k) { return k.c_str(); });

    PFTitleDataManagementGetTitleDataRequest request{};
    request.keys = c_keys.empty() ? nullptr : c_keys.data();
    request.keysCount = static_cast<uint32_t>(c_keys.size());

    auto* ctx = new GetTitleDataContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnGetTitleDataComplete;

    HRESULT hr = PFTitleDataManagementServerGetTitleDataAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "GetTitleData (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::SetTitleData(
    const std::string& key,
    const std::string& value,
    OnAdminSuccess     onSuccess,
    OnAdminFailure     onFailure)
{
    QueueTitleData(key, value, onSuccess, onFailure);
}

void Uma_Engine::PlayFabAdminManager::SetTitleDataBatch(
    const std::unordered_map<std::string, std::string>& kvPairs,
    OnAdminSuccess                                       onSuccess,
    OnAdminFailure                                       onFailure)
{
    if (kvPairs.empty()) return;

    for (auto const& pair : kvPairs)
    {
        QueueTitleData(pair.first, pair.second, onSuccess, onFailure);
    }
}

void Uma_Engine::PlayFabAdminManager::DeleteTitleData(
    const std::string& key,
    OnAdminSuccess     onSuccess,
    OnAdminFailure     onFailure)
{
    SetTitleData(key, "", std::move(onSuccess), std::move(onFailure));
}

// -- TitleData callbacks -------------------------------------------------------

void Uma_Engine::PlayFabAdminManager::OnGetTitleDataComplete(XAsyncBlock* async)
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
    hr = PFTitleDataManagementServerGetTitleDataGetResultSize(async, &bufferSize);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetTitleData (ResultSize)", ctx->onFailure);
        delete ctx;
        return;
    }

    std::vector<uint8_t> buffer(bufferSize);
    PFTitleDataManagementGetTitleDataResult* result = nullptr;
    size_t bufferUsed = 0;

    hr = PFTitleDataManagementServerGetTitleDataGetResult(
        async, bufferSize, buffer.data(), &result, &bufferUsed);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetTitleData (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess)
    {
        std::unordered_map<std::string, std::string> out;
        for (uint32_t i = 0; i < result->dataCount; ++i)
            out.emplace(result->data[i].key, result->data[i].value);
        ctx->onSuccess(out);
    }

    delete ctx;
}

void Uma_Engine::PlayFabAdminManager::OnSetTitleDataComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<SetTitleDataContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (SUCCEEDED(hr))
    {
        if (ctx->onSuccess) ctx->onSuccess();
    }
    else
    {
        DispatchError(hr, "SetTitleData", ctx->onFailure);
    }

    delete ctx;
}


// =============================================================================
//  Statistics
// =============================================================================

void Uma_Engine::PlayFabAdminManager::CreateStatisticDefinition(
    const std::string& name,
    const std::string& columnName,
    PFStatisticsStatisticAggregationMethod aggregation,
    OnAdminSuccess                         onSuccess,
    OnAdminFailure                         onFailure)
{
    PFVersionConfiguration versionCfg{};
    versionCfg.resetInterval = PFResetInterval::Manual;
    versionCfg.maxQueryableVersions = 1;

    PFStatisticsStatisticColumn column{};
    column.name = columnName.c_str(); // column name is distinct from stat name
    column.aggregationMethod = aggregation;

    const PFStatisticsStatisticColumn* columnPtrs[1] = { &column };

    PFStatisticsCreateStatisticDefinitionRequest request{};
    request.name = name.c_str();
    request.entityType = "title_player_account";
    request.columns = columnPtrs;
    request.columnsCount = 1;
    request.versionConfiguration = &versionCfg; // optional — omit for a stat with no auto-reset

    auto* ctx = new NoPayloadContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnNoPayloadComplete;

    HRESULT hr = PFStatisticsCreateStatisticDefinitionAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "CreateStatisticDefinition (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::DeleteStatisticDefinition(
    const std::string& name,
    OnAdminSuccess     onSuccess,
    OnAdminFailure     onFailure)
{
    PFStatisticsDeleteStatisticDefinitionRequest request{};
    request.name = name.c_str();

    auto* ctx = new NoPayloadContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnNoPayloadComplete;

    HRESULT hr = PFStatisticsDeleteStatisticDefinitionAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "DeleteStatisticDefinition (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::UpdateStatistics(
    const std::string& entityId,
    const std::string& entityType,
    const std::string& statName,
    const std::string& score,
    OnAdminSuccess     onSuccess,
    OnAdminFailure     onFailure)
{
    // Score must be passed as a const char* pointer-to-pointer
    const char* scorePtr = score.c_str();

    PFStatisticsStatisticUpdate statUpdate{};
    statUpdate.name = statName.c_str();
    statUpdate.scores = &scorePtr;
    statUpdate.scoresCount = 1;

    const PFStatisticsStatisticUpdate* statUpdatePtr = &statUpdate;

    PFEntityKey entityKey{};
    entityKey.id = entityId.c_str();
    entityKey.type = entityType.c_str();

    PFStatisticsUpdateStatisticsRequest request{};
    request.entity = &entityKey;
    request.statistics = &statUpdatePtr;
    request.statisticsCount = 1;

    auto* ctx = new NoPayloadContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnNoPayloadComplete;

    HRESULT hr = PFStatisticsUpdateStatisticsAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "UpdateStatistics (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::GetStatistics(
    const std::string& entityId,
    const std::string& entityType,
    OnStatisticsSuccess onSuccess,
    OnAdminFailure      onFailure)
{
    PFEntityKey entityKey{};
    entityKey.id = entityId.c_str();
    entityKey.type = entityType.c_str();

    PFStatisticsGetStatisticsRequest request{};
    request.entity = &entityKey;

    auto* ctx = new GetStatisticsContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnGetStatisticsComplete;

    HRESULT hr = PFStatisticsGetStatisticsAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "GetStatistics (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::IncrementStatisticVersion(
    const std::string& name,
    OnVersionSuccess   onSuccess,
    OnAdminFailure     onFailure)
{
    PFStatisticsIncrementStatisticVersionRequest request{};
    request.name = name.c_str();

    auto* ctx = new IncrementStatVersionContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnIncrementStatVersionComplete;

    HRESULT hr = PFStatisticsIncrementStatisticVersionAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "IncrementStatisticVersion (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::ListStatisticDefinitions(
    std::function<void(const std::vector<std::string>&)> onSuccess,
    OnAdminFailure                                        onFailure)
{
    PFStatisticsListStatisticDefinitionsRequest request{};

    auto* ctx = new ListStatDefinitionsContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnListStatDefinitionsComplete;

    HRESULT hr = PFStatisticsListStatisticDefinitionsAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "ListStatisticDefinitions (Launch)", ctx->onFailure);
        delete ctx;
    }
}

// -- Statistics callbacks ------------------------------------------------------

void Uma_Engine::PlayFabAdminManager::OnNoPayloadComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<NoPayloadContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (SUCCEEDED(hr))
    {
        if (ctx->onSuccess) ctx->onSuccess();
    }
    else
    {
        DispatchError(hr, "NoPayload operation", ctx->onFailure);
    }

    delete ctx;
}

void Uma_Engine::PlayFabAdminManager::OnGetStatisticsComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<GetStatisticsContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetStatistics (Receiving)", ctx->onFailure);
        delete ctx;
        return;
    }

    size_t bufferSize = 0;
    hr = PFStatisticsGetStatisticsGetResultSize(async, &bufferSize);
    if (FAILED(hr))
    {
        DispatchError(hr, "GetStatistics (ResultSize)", ctx->onFailure);
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
        DispatchError(hr, "GetStatistics (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess)
    {
        std::vector<StatisticValue> out;
        out.reserve(result->statisticsCount);

        for (uint32_t i = 0; i < result->statisticsCount; ++i)
        {
            const PFStatisticsEntityStatisticValue* val =
                result->statistics[i].value;

            StatisticValue sv;
            sv.name = val->name ? val->name : "";
            sv.version = val->version;
            sv.scores.reserve(val->scoresCount);

            for (uint32_t s = 0; s < val->scoresCount; ++s)
                sv.scores.emplace_back(val->scores[s] ? val->scores[s] : "");

            out.push_back(std::move(sv));
        }

        ctx->onSuccess(out);
    }

    delete ctx;
}

void Uma_Engine::PlayFabAdminManager::OnIncrementStatVersionComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<IncrementStatVersionContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "IncrementStatisticVersion", ctx->onFailure);
        delete ctx;
        return;
    }

    PFStatisticsIncrementStatisticVersionResponse result{};
    hr = PFStatisticsIncrementStatisticVersionGetResult(async, &result);
    if (FAILED(hr))
    {
        DispatchError(hr, "IncrementStatisticVersion (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess) ctx->onSuccess(result.version);

    delete ctx;
}

void Uma_Engine::PlayFabAdminManager::OnListStatDefinitionsComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<ListStatDefinitionsContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "ListStatisticDefinitions (Receiving)", ctx->onFailure);
        delete ctx;
        return;
    }

    size_t bufferSize = 0;
    hr = PFStatisticsListStatisticDefinitionsGetResultSize(async, &bufferSize);
    if (FAILED(hr))
    {
        DispatchError(hr, "ListStatisticDefinitions (ResultSize)", ctx->onFailure);
        delete ctx;
        return;
    }

    std::vector<uint8_t> buffer(bufferSize);
    PFStatisticsListStatisticDefinitionsResponse* result = nullptr;
    size_t bufferUsed = 0;

    hr = PFStatisticsListStatisticDefinitionsGetResult(
        async, bufferSize, buffer.data(), &result, &bufferUsed);
    if (FAILED(hr))
    {
        DispatchError(hr, "ListStatisticDefinitions (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess)
    {
        std::vector<std::string> names;
        names.reserve(result->statisticDefinitionsCount);
        for (uint32_t i = 0; i < result->statisticDefinitionsCount; ++i)
        {
            const char* n = result->statisticDefinitions[i]->name;
            names.emplace_back(n ? n : "");
        }
        ctx->onSuccess(names);
    }

    delete ctx;
}


// =============================================================================
//  Leaderboards
// =============================================================================

void Uma_Engine::PlayFabAdminManager::CreateLeaderboardDefinition(
    const std::string& name,
    const std::string& columnName,
    PFLeaderboardsLeaderboardSortDirection sortDirection,
    int32_t                                sizeLimit,
    OnAdminSuccess                         onSuccess,
    OnAdminFailure                         onFailure)
{
    PFLeaderboardsLeaderboardColumn column{};
    column.name = columnName.c_str();
    column.sortDirection = sortDirection;

    const PFLeaderboardsLeaderboardColumn* columnPtr = &column;

    // versionConfiguration is required (not _Maybenull_).
    // maxQueryableVersions must be >= 1 — PlayFab rejects 0 with E_PF_INVALID_PARAMS.
    PFVersionConfiguration versionCfg{};
    versionCfg.resetInterval = PFResetInterval::Manual;
    versionCfg.maxQueryableVersions = 1;

    PFLeaderboardsCreateLeaderboardDefinitionRequest request{};
    request.name = name.c_str();
    request.entityType = "title_player_account";
    request.columns = &columnPtr;
    request.columnsCount = 1;
    request.sizeLimit = sizeLimit;
    request.versionConfiguration = &versionCfg;

    auto* ctx = new NoPayloadContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnNoPayloadComplete;

    HRESULT hr = PFLeaderboardsCreateLeaderboardDefinitionAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "CreateLeaderboardDefinition (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::DeleteLeaderboardDefinition(
    const std::string& name,
    OnAdminSuccess     onSuccess,
    OnAdminFailure     onFailure)
{
    PFLeaderboardsDeleteLeaderboardDefinitionRequest request{};
    request.name = name.c_str();

    auto* ctx = new NoPayloadContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnNoPayloadComplete;

    HRESULT hr = PFLeaderboardsDeleteLeaderboardDefinitionAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "DeleteLeaderboardDefinition (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::GetLeaderboard(
    const std::string& leaderboardName,
    uint32_t             pageSize,
    const uint32_t* startPosition,
    OnLeaderboardSuccess onSuccess,
    OnAdminFailure       onFailure)
{
    PFLeaderboardsGetEntityLeaderboardRequest request{};
    request.leaderboardName = leaderboardName.c_str();
    request.pageSize = pageSize;
    request.startingPosition = startPosition;

    auto* ctx = new GetLeaderboardContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnGetLeaderboardComplete;

    HRESULT hr = PFLeaderboardsGetLeaderboardAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "GetLeaderboard (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::GetLeaderboardAroundEntity(
    const std::string& leaderboardName,
    const std::string& entityId,
    const std::string& entityType,
    uint32_t             maxSurroundingEntries,
    OnLeaderboardSuccess onSuccess,
    OnAdminFailure       onFailure)
{
    PFEntityKey entityKey{};
    entityKey.id = entityId.c_str();
    entityKey.type = entityType.c_str();

    PFLeaderboardsGetLeaderboardAroundEntityRequest request{};
    request.leaderboardName = leaderboardName.c_str();
    request.entity = &entityKey;
    request.maxSurroundingEntries = maxSurroundingEntries;

    auto* ctx = new GetLeaderboardContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnGetLeaderboardComplete;

    HRESULT hr = PFLeaderboardsGetLeaderboardAroundEntityAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "GetLeaderboardAroundEntity (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::UpdateLeaderboardEntries(
    const std::string& leaderboardName,
    const std::string& entityId,
    const std::string& score,
    const std::string& metadata,
    OnAdminSuccess     onSuccess,
    OnAdminFailure     onFailure)
{
    const char* scorePtr = score.c_str();

    PFLeaderboardsLeaderboardEntryUpdate entry{};
    entry.entityId = entityId.c_str();
    entry.scores = &scorePtr;
    entry.scoresCount = 1;
    entry.metadata = metadata.empty() ? nullptr : metadata.c_str();

    const PFLeaderboardsLeaderboardEntryUpdate* entryPtr = &entry;

    PFLeaderboardsUpdateLeaderboardEntriesRequest request{};
    request.leaderboardName = leaderboardName.c_str();
    request.entries = &entryPtr;
    request.entriesCount = 1;

    auto* ctx = new NoPayloadContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnNoPayloadComplete;

    HRESULT hr = PFLeaderboardsUpdateLeaderboardEntriesAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "UpdateLeaderboardEntries (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::IncrementLeaderboardVersion(
    const std::string& name,
    OnVersionSuccess   onSuccess,
    OnAdminFailure     onFailure)
{
    PFLeaderboardsIncrementLeaderboardVersionRequest request{};
    request.name = name.c_str();

    auto* ctx = new IncrementLBVersionContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnIncrementLBVersionComplete;

    HRESULT hr = PFLeaderboardsIncrementLeaderboardVersionAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "IncrementLeaderboardVersion (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::ListLeaderboardDefinitions(
    std::function<void(const std::vector<std::string>&)> onSuccess,
    OnAdminFailure                                        onFailure)
{
    PFLeaderboardsListLeaderboardDefinitionsRequest request{};

    auto* ctx = new ListLBDefinitionsContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnListLBDefinitionsComplete;

    HRESULT hr = PFLeaderboardsListLeaderboardDefinitionsAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "ListLeaderboardDefinitions (Launch)", ctx->onFailure);
        delete ctx;
    }
}

// -- Leaderboard callbacks -----------------------------------------------------

void Uma_Engine::PlayFabAdminManager::OnGetLeaderboardComplete(XAsyncBlock* async)
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
        ctx->onSuccess(ParseLeaderboardResponse(result), result->version);

    delete ctx;
}

void Uma_Engine::PlayFabAdminManager::OnIncrementLBVersionComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<IncrementLBVersionContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "IncrementLeaderboardVersion", ctx->onFailure);
        delete ctx;
        return;
    }

    PFLeaderboardsIncrementLeaderboardVersionResponse result{};
    hr = PFLeaderboardsIncrementLeaderboardVersionGetResult(async, &result);
    if (FAILED(hr))
    {
        DispatchError(hr, "IncrementLeaderboardVersion (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess) ctx->onSuccess(result.version);

    delete ctx;
}

void Uma_Engine::PlayFabAdminManager::OnListLBDefinitionsComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<ListLBDefinitionsContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "ListLeaderboardDefinitions (Receiving)", ctx->onFailure);
        delete ctx;
        return;
    }

    size_t bufferSize = 0;
    hr = PFLeaderboardsListLeaderboardDefinitionsGetResultSize(async, &bufferSize);
    if (FAILED(hr))
    {
        DispatchError(hr, "ListLeaderboardDefinitions (ResultSize)", ctx->onFailure);
        delete ctx;
        return;
    }

    std::vector<uint8_t> buffer(bufferSize);
    PFLeaderboardsListLeaderboardDefinitionsResponse* result = nullptr;
    size_t bufferUsed = 0;

    hr = PFLeaderboardsListLeaderboardDefinitionsGetResult(
        async, bufferSize, buffer.data(), &result, &bufferUsed);
    if (FAILED(hr))
    {
        DispatchError(hr, "ListLeaderboardDefinitions (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess)
    {
        std::vector<std::string> names;
        names.reserve(result->leaderboardDefinitionsCount);
        for (uint32_t i = 0; i < result->leaderboardDefinitionsCount; ++i)
        {
            const char* n = result->leaderboardDefinitions[i]->name;
            names.emplace_back(n ? n : "");
        }
        ctx->onSuccess(names);
    }

    delete ctx;
}


// =============================================================================
//  CloudScript / Azure Functions
// =============================================================================

void Uma_Engine::PlayFabAdminManager::ExecuteFunction(
    const std::string& functionName,
    const std::string& functionParamJson,
    OnFunctionSuccess  onSuccess,
    OnAdminFailure     onFailure)
{
    PFJsonObject param{};
    param.stringValue = functionParamJson.empty() ? nullptr : functionParamJson.c_str();

    PFCloudScriptExecuteFunctionRequest request{};
    request.functionName = functionName.c_str();
    request.functionParameter = param;

    auto* ctx = new ExecuteFunctionContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnExecuteFunctionComplete;

    HRESULT hr = PFCloudScriptExecuteFunctionAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "ExecuteFunction (Launch)", ctx->onFailure);
        delete ctx;
    }
}

void Uma_Engine::PlayFabAdminManager::QueueTitleData(const std::string& key, const std::string& value, OnAdminSuccess onSuccess, OnAdminFailure onFailure)
{
    TitleDataWriteCommand cmd{
        key,
        value,
        onSuccess,
        onFailure
    };

    m_titleDataQueue.push(cmd);
}

void Uma_Engine::PlayFabAdminManager::Update()
{
    if (m_titleDataWriteInFlight || m_titleDataQueue.empty()) return;

    m_titleDataWriteInFlight = true;
    auto cmd = m_titleDataQueue.front();
    m_titleDataQueue.pop();

    ProcessSetTitleDataQueue(cmd.key, cmd.value,
        [this, cmd]() {
            m_titleDataWriteInFlight = false;
            if (cmd.onSuccess) cmd.onSuccess();
            // next command picked up next Update() tick
        },
        [this, cmd](HRESULT hr, const std::string& msg) {
            m_titleDataWriteInFlight = false;
            if (cmd.onFailure) cmd.onFailure(hr, msg);
            // queue is NOT cleared — remaining commands still attempt
        }
    );
}

void Uma_Engine::PlayFabAdminManager::OnExecuteFunctionComplete(XAsyncBlock* async)
{
    auto* ctx = reinterpret_cast<ExecuteFunctionContext*>(async->context);

    HRESULT hr = XAsyncGetStatus(async, false);
    if (FAILED(hr))
    {
        DispatchError(hr, "ExecuteFunction (Receiving)", ctx->onFailure);
        delete ctx;
        return;
    }

    size_t bufferSize = 0;
    hr = PFCloudScriptExecuteFunctionGetResultSize(async, &bufferSize);
    if (FAILED(hr))
    {
        DispatchError(hr, "ExecuteFunction (ResultSize)", ctx->onFailure);
        delete ctx;
        return;
    }

    std::vector<uint8_t> buffer(bufferSize);
    PFCloudScriptExecuteFunctionResult* result = nullptr;
    size_t bufferUsed = 0;

    hr = PFCloudScriptExecuteFunctionGetResult(
        async, bufferSize, buffer.data(), &result, &bufferUsed);
    if (FAILED(hr))
    {
        DispatchError(hr, "ExecuteFunction (GetResult)", ctx->onFailure);
        delete ctx;
        return;
    }

    // If the Azure Function itself reported an error, surface it as a failure
    if (result->error)
    {
        std::string errMsg = "[ExecuteFunction] Azure Function error: ";
        errMsg += result->error->error ? result->error->error : "unknown";
        errMsg += " — ";
        errMsg += result->error->message ? result->error->message : "";
        DispatchError(E_FAIL, errMsg, ctx->onFailure);
        delete ctx;
        return;
    }

    if (ctx->onSuccess)
    {
        std::string fnName = result->functionName ? result->functionName : "";
        std::string resultStr = JsonObjectToString(result->functionResult);
        ctx->onSuccess(fnName, resultStr);
    }

    delete ctx;
}


// =============================================================================
//  Private helpers
// =============================================================================

std::vector<Uma_Engine::LeaderboardEntry>
Uma_Engine::PlayFabAdminManager::ParseLeaderboardResponse(
    const PFLeaderboardsGetEntityLeaderboardResponse* response)
{
    std::vector<LeaderboardEntry> entries;
    if (!response) return entries;

    entries.reserve(response->rankingsCount);

    for (uint32_t i = 0; i < response->rankingsCount; ++i)
    {
        const PFLeaderboardsEntityLeaderboardEntry* raw = response->rankings[i];

        LeaderboardEntry entry;
        entry.rank = raw->rank;
        entry.displayName = raw->displayName ? raw->displayName : "";
        entry.metadata = raw->metadata ? raw->metadata : "";

        if (raw->entity)
            entry.entityId = raw->entity->id ? raw->entity->id : "";

        entry.scores.reserve(raw->scoresCount);
        for (uint32_t s = 0; s < raw->scoresCount; ++s)
            entry.scores.emplace_back(raw->scores[s] ? raw->scores[s] : "");

        entries.push_back(std::move(entry));
    }

    return entries;
}

void Uma_Engine::PlayFabAdminManager::DispatchError(
    HRESULT               hr,
    const std::string& context,
    const OnAdminFailure& onFailure)
{
    if (!onFailure) return;

    std::string msg = "[PlayFabAdminManager] " + context
        + " failed: 0x" + [](HRESULT h) {
        char buf[12];
        snprintf(buf, sizeof(buf), "%08X", static_cast<unsigned>(h));
        return std::string(buf);
        }(hr);

    onFailure(hr, msg);
}

void Uma_Engine::PlayFabAdminManager::ProcessSetTitleDataQueue(const std::string& key, const std::string& value, OnAdminSuccess onSuccess, OnAdminFailure onFailure)
{
    PFTitleDataManagementSetTitleDataRequest request{};
    request.key = key.c_str();
    request.value = value.c_str();

    auto* ctx = new SetTitleDataContext{ std::move(onSuccess), std::move(onFailure) };
    ctx->async.queue = nullptr;
    ctx->async.context = ctx;
    ctx->async.callback = OnSetTitleDataComplete;

    HRESULT hr = PFTitleDataManagementServerSetTitleDataAsync(
        m_titleEntityHandle, &request, &ctx->async);

    if (FAILED(hr))
    {
        DispatchError(hr, "SetTitleData (Launch)", ctx->onFailure);
        delete ctx;
    }
}
