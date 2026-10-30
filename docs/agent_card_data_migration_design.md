# Agent Card Data Migration Design

## 1. Background

AgentManagerService (SA 185) has migrated from the `foundation` process to the
`subfoundation` process. The agent_card KV store was originally located at
`/data/service/el1/public/database/ability_manager_service/agent_card` (accessible
to the `foundation` process UID). After the migration, the new KV store is at
`/data/service/el1/public/database/agent_manager_service/agent_card` (accessible
to the `subfoundation` process UID).

The old path is no longer accessible from `subfoundation`, so existing data must
be migrated to the new store. Data loss is unacceptable.

## 2. Data Classification (Confirmed)

Agent card data has two sources, distinguished by `AgentCardUpdateSource` enum
(`agent_card_db_mgr.h:30-33`):

| Source | Enum Value | Origin | Trigger |
|--------|-----------|--------|---------|
| Static | `BUNDLE = 0` | `ohos.extension.agent` profile in the HAP | `HandleBundleInstall` via `AgentBundleEventCallback` (PACKAGE_ADDED/CHANGED) |
| Dynamic | `API = 1` | Runtime API call | `RegisterAgentCard` / `UpdateAgentCard` IPC |

Both are stored in the same KV store under key `{"bundleName":"...","userId":N}`,
in a single `cards` JSON array. Each entry carries `lastUpdateSource: "bundle"|"api"`.

## 3. Solution

### 3.1 Static Data (BUNDLE source) -- Re-fetch from bundleManager

Static data can be reconstructed from bundleManager at startup. The existing
`TryBackfillPreInstallCards` only handles **pre-installed** apps
(`bundleInfo.isPreInstallApp == true`). Non-pre-installed apps also need
backfill during migration.

- Extend backfill to process ALL apps (pre-installed + non-pre-installed)
  during the one-time migration.
- After migration, normal backfill continues to handle pre-installed apps only
  (existing behavior, for missed install events).

### 3.2 Dynamic Data (API source) -- Migrate via AMS IPC interface

Dynamic data cannot be reconstructed from bundleManager. It must be read from
the old KV store and written to the new one.

AMS (SA 180) still runs in the `foundation` process and can access the old
path. Add a new IPC interface in AMS that:
1. Opens the old KV store at the old path
2. Reads all entries
3. Filters only API-source cards
4. Returns as a JSON string (to avoid coupling AMS with `AgentCard` types)

AgentManagerService calls this interface at startup, parses the JSON string
back into `AgentCard` objects, and stores them in the new KV store with
`updateSource = API`.

### 3.3 JSON Format (AMS -> AgentManagerService)

```json
[
  {
    "bundleName": "com.example.app",
    "userId": 100,
    "cards": [
      { ...full AgentCard JSON... },
      { ...full AgentCard JSON... }
    ]
  },
  ...
]
```

Only entries with at least one API-source card are included. The `cards` array
contains only the API-source card JSON objects (no `lastUpdateSource` wrapper).

### 3.4 One-Time Migration Guard

The entire migration flow (dynamic data + full backfill) runs **once**. A
migration marker is stored in the new KV store as a special key
(`migration_status` -> `completed`). On subsequent starts, the normal
pre-install backfill runs instead.

## 4. Migration Flow

```
AgentManagerService::OnStart
  -> AddSystemAbilityListener(BMS, AccountMgr)

OnAddSystemAbility (both BMS + AccountMgr ready)
  -> TryMigrateAndBackfill()
       |
       +-- IsMigrationCompleted()?
       |     |
       |     +-- NO: Migration Flow (first time)
       |     |     1. MigrateDynamicAgentCards()  -- call AMS, get JSON, store API cards
       |     |     2. BackfillAllCards()          -- all apps from bundleManager
       |     |     3. MarkMigrationCompleted()
       |     |
       |     +-- YES: Normal Flow (subsequent starts)
       |           BackfillPreInstallCards()      -- pre-installed only (existing)
```

Ordering rationale: Dynamic data is migrated first, then static backfill runs.
The existing `HandleBundleInstall` merge logic (`ShouldKeepStoredBundleEntry`)
preserves API-source cards when versions are equal, so static backfill won't
clobber the migrated dynamic data.

## 5. IPC Interface Design

### 5.1 New IPC Code

`AbilityManagerInterfaceCode::GET_DYNAMIC_AGENT_CARDS_FOR_MIGRATION = 6187`
(appended after `UNREGISTER_WILL_INSIGHT_INTENT_EXECUTE_CALLBACK = 6186`).

### 5.2 Interface Method

```cpp
virtual int32_t GetDynamicAgentCardsForMigration(std::string &jsonStr)
{
    return ERR_OK;
}
```

Default body returns `ERR_OK` with empty string, so older stub/proxy
implementations remain compatible.

### 5.3 Permission Check

Only the `subfoundation` process may call this interface, enforced via
`CheckSpecificSystemAbilityAccessPermission("subfoundation")` (same pattern as
`VerifyAgentConnectRequest` which checks for `foundation`).

## 6. Code Changes Overview

### AMS (ability_runtime/services/abilitymgr/)

| File | Change |
|------|--------|
| `interfaces/inner_api/ability_manager/include/ability_manager_ipc_interface_code.h` | Add `GET_DYNAMIC_AGENT_CARDS_FOR_MIGRATION = 6187` |
| `interfaces/inner_api/ability_manager/include/ability_manager_interface.h` | Add virtual method with default body |
| `interfaces/inner_api/ability_manager/include/ability_manager_client.h` | Add client wrapper method |
| `interfaces/inner_api/ability_manager/src/ability_manager_client.cpp` | Implement client wrapper |
| `services/abilitymgr/include/ability_manager_proxy.h` | Add proxy override declaration |
| `services/abilitymgr/src/ability_manager_proxy.cpp` | Implement proxy: WriteInterfaceToken, SendRequest, ReadString |
| `services/abilitymgr/include/ability_manager_stub.h` | Add `GetDynamicAgentCardsForMigrationInner` declaration |
| `services/abilitymgr/src/ability_manager_stub.cpp` | Implement stub handler + dispatch in `OnRemoteRequestInnerTwentySecond` |
| `services/abilitymgr/include/ability_manager_service.h` | Add method override declaration |
| `services/abilitymgr/src/ability_manager_service.cpp` | Implement: permission check + delegate to helper |
| `services/abilitymgr/include/agent_card_migration_helper.h` | **New file**: helper class to read old KV store |
| `services/abilitymgr/src/agent_card_migration_helper.cpp` | **New file**: implementation |
| `services/abilitymgr/abilitymgr.gni` | Add `agent_card_migration_helper.cpp` to source list |

### AgentManagerService (agent_runtime_framework/services/agentmgr/)

| File | Change |
|------|--------|
| `agent_card_db_mgr.h` | Add `IsMigrationCompleted()` / `MarkMigrationCompleted()` |
| `agent_card_db_mgr.cpp` | Implement migration marker using special KV key |
| `agent_card_mgr.h` | Add `MigrateDynamicAgentCards()`, `BackfillAllCards()`, `HandleAllInstallBackfill()` |
| `agent_card_mgr.cpp` | Implement migration + full backfill |
| `agent_manager_service.h` | Rename `TryBackfillPreInstallCards` flow, add migration state |
| `agent_manager_service.cpp` | Implement `TryMigrateAndBackfill()` with one-time guard |

## 7. Edge Cases

| Scenario | Handling |
|----------|----------|
| Fresh install (no old KV store) | AMS opens old store with `createIfMissing=false`; store not found -> returns empty JSON -> migration marks completed |
| Old store corrupted | AMS returns error code; migration continues with backfill only; marks completed |
| Service restart after migration | Marker found -> skips migration, runs normal pre-install backfill |
| Non-pre-installed app installed after migration | Handled by existing `AgentBundleEventCallback` (PACKAGE_ADDED event) |
| Migration interrupted (service crash) | Marker not set -> migration re-runs on next start; idempotent (dynamic cards use InsertData which overwrites; static backfill merges) |

## 8. Compatibility

- The new IPC method has a default body `{ return ERR_OK; }`, so older proxy/stub
  implementations remain compatible during OTA mixed-version scenarios.
- The new IPC code is appended at the end of the enum (6187), preserving all
  existing transaction codes.
- The old KV store is opened read-only with `createIfMissing=false`; no
  modifications to the old store.
- The new KV store format is unchanged; migration marker uses a separate key
  namespace that does not collide with card data keys.
