# PlayFabPlayerManager — Feature Reference

## Authentication
| Method | Description |
|---|---|
| `LoginWithCustomID(id, createAcct, ...)` | Log in as a player using a custom ID. Pass createAcct = true to auto-create on first login. Use a locally stored GUID for guest/random accounts. |
| `LoginWithEmail(email, password, ...)` | Log in as a named player with email and password. |
| `RegisterAccount(email, password, username, ...)` | Create a new named PlayFab account with email, password, and display username. |
| `Logout()` | Close the current player entity handle and clear the logged-in state. |

## Account
| Method | Description |
|---|---|
| `GetAccountInfo(...)` | Fetch the current player's account details (PlayFab ID, username, created date). |
| `SetDisplayName(name, ...)` | Set or update the display name shown on leaderboards and profiles. |
| `UpgradeGuestAccount(email, password, username, ...)` | Attach email/password credentials to an existing guest account, preserving all progress. |

## Player Data
| Method | Description |
|---|---|
| `ReadData(key, ...)` | Fetch a single KV entry from the player's private user data store. |
| `ReadMultipleData(keys, ...)` | Fetch multiple KV entries in one call. Empty keys = fetch all. |
| `WriteData(key, value, ...)` | Write a single KV pair to the player's private user data store (creates or overwrites). |

## Title Data
| Method | Description |
|---|---|
| `GetTitleData(key, ...)` | Read a global KV entry set by the developer in the PlayFab dashboard. Read-only from the client. |

## Statistics
| Method | Description |
|---|---|
| `SubmitScore(statName, value, ...)` | Update a stat on the player's entity profile. Automatically propagates to any linked leaderboard. |
| `GetMyStats(statName, ...)` | Read the player's current value for a named statistic. |

## Leaderboards
| Method | Description |
|---|---|
| `GetLeaderboard(name, pageSize, startPos, ...)` | Fetch a page of the global leaderboard from a given position. pageSize max 1000, startPos 1 = top. |
| `GetLeaderboardAroundMe(name, pageSize, ...)` | Fetch leaderboard entries centred on the current player's rank. |
| `GetFriendLeaderboard(name, ...)` | Fetch a leaderboard filtered to the player's friends only. Max 25 entries. |