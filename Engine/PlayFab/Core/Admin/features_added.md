# PlayFabAdminManager — Feature Reference

## Title Data
| Method | Description |
|---|---|
| `GetTitleData(keys, ...)` | Fetch one or more global KV entries. Empty keys = fetch all. |
| `SetTitleData(key, value, ...)` | Write a single KV pair (creates or overwrites). |
| `SetTitleDataBatch(kvPairs, ...)` | Write multiple KV pairs in one logical call. |
| `DeleteTitleData(key, ...)` | Clear a key by writing an empty string. |

## Statistics
| Method | Description |
|---|---|
| `CreateStatisticDefinition(name, aggregation, ...)` | Register a new stat with an aggregation method (Last / Min / Max / Sum). |
| `DeleteStatisticDefinition(name, ...)` | Remove a stat definition and all its data. |
| `UpdateStatistics(entityId, entityType, statName, score, ...)` | Write a stat value for a given entity. |
| `GetStatistics(entityId, entityType, ...)` | Read all current stat values for a given entity. |
| `IncrementStatisticVersion(name, ...)` | Bump the stat version to start a new season; archives old data. |
| `ListStatisticDefinitions(...)` | List all stat definition names registered on the title. |

## Leaderboards
| Method | Description |
|---|---|
| `CreateLeaderboardDefinition(name, columnName, sortDirection, sizeLimit, ...)` | Create a ranked leaderboard with a fixed sort direction. |
| `DeleteLeaderboardDefinition(name, ...)` | Remove a leaderboard definition and all its entries. |
| `GetLeaderboard(name, pageSize, startPosition, ...)` | Fetch a page of the global leaderboard from the top. |
| `GetLeaderboardAroundEntity(name, entityId, entityType, maxSurrounding, ...)` | Fetch entries centered around a specific entity's rank. |
| `UpdateLeaderboardEntries(name, entityId, score, metadata, ...)` | Server-side direct write to a leaderboard entry. |
| `IncrementLeaderboardVersion(name, ...)` | Reset rankings for a new season; archives current version. |
| `ListLeaderboardDefinitions(...)` | List all leaderboard definition names registered on the title. |

## CloudScript / Azure Functions
| Method | Description |
|---|---|
| `ExecuteFunction(functionName, paramJson, ...)` | Invoke a registered Azure Function by name, passing a JSON argument blob. |