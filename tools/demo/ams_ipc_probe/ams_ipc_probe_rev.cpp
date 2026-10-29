/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Reverse-IPC probe bin: verifies AMS/AppMgr can call BACK into an
// appSpawn fork+exec'd bin via binder.
//
// Mechanism (inner_api IApplicationStateObserver):
//   1. bin subclasses ApplicationStateObserverStub, overrides
//      OnForegroundApplicationChanged / OnProcessDied / OnProcessCreated.
//   2. bin calls AppMgrClient::RegisterApplicationStateObserver(obs)  -- a
//      FORWARD binder transaction to AppMgr (SA 501) registering the stub.
//   3. When ANY application foregrounds/backgrounds/dies, AppMgr issues a
//      binder transaction INTO this bin's process invoking the overridden
//      callback. The callback FIRING => reverse IPC (AMS/AppMgr -> bin) works.
//
// To trigger while the bin runs: launch / foreground / background / kill any
// other app on the device. One state change is enough.
//
// PERMISSION REQUIREMENT:
//   AppStateObserverManager::RegisterApplicationStateObserver /
//   UnregisterApplicationStateObserver both call
//   PermissionVerification::VerifyAppStateObserverPermission()
//   (app_state_observer_manager.cpp + permission_verification.cpp), which
//   requires the caller's token to hold
//       ohos.permission.RUNNING_STATE_OBSERVER   (system_grant, system API)
//   The spawned bin inherits the donor app's accessTokenIdEx, so the DONOR app
//   would normally have to carry this permission.
//
//   DEMO BYPASS: the VerifyAppStateObserverPermission() check in
//   AppStateObserverManager RegisterApplicationStateObserver and
//   UnregisterApplicationStateObserver (services/appmgr/src/
//   app_state_observer_manager.cpp) has been COMMENTED OUT so the bin can
//   register without the permission. This is a SERVICE-SIDE change -- the
//   rebuilt AppMgrService libabilityms must be deployed to the device for the
//   bypass to take effect. Restore those checks before any merge.
//
// Why not Connect-to-nonexistent (used by the forward NDK probe): a non-existent
// extension target makes ConnectAbilityCommon return the error SYNCHRONOUSLY
// (ability_manager_service.cpp:6373-6383 returns ConnectLocalAbility's errCode
// directly), so OnFailed never fires. The observer pattern has no such issue --
// AMS actively pushes state changes to all registered observers, no target needed.

#include <atomic>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <thread>
#include <unistd.h>

#include "hilog/log.h"

#include "app_mgr_client.h"
#include "application_state_observer_stub.h"
#include "iapplication_state_observer.h"
#include "app_state_data.h"
#include "process_data.h"

namespace {
constexpr const char *PROBE_TAG = "ams_ipc_probe_rev";
constexpr unsigned int PROBE_DOMAIN = 0xD003A01;

#define PROBE_LOGI(...) ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, PROBE_DOMAIN, PROBE_TAG, ##__VA_ARGS__))
#define PROBE_LOGE(...) ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, PROBE_DOMAIN, PROBE_TAG, ##__VA_ARGS__))

void Emit(bool isError, const char *fmt, ...)
{
    char buf[1024] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (isError) {
        PROBE_LOGE("%{public}s", buf);
    } else {
        PROBE_LOGI("%{public}s", buf);
    }
    printf("%s\n", buf);
    fflush(stdout);
    fprintf(stderr, "%s\n", buf);
    FILE *fp = fopen("/data/local/tmp/ams_ipc_probe.log", "a");
    if (fp != nullptr) {
        fprintf(fp, "%s\n", buf);
        fclose(fp);
    }
}

// 0 = not fired; 1 = OnForegroundApplicationChanged; 2 = OnProcessDied; 3 = other
std::atomic<int> g_revFired{0};
} // namespace

class ProbeObserver : public OHOS::AppExecFwk::ApplicationStateObserverStub {
public:
    void OnForegroundApplicationChanged(const OHOS::AppExecFwk::AppStateData &d) override
    {
        g_revFired.store(1);
        Emit(false, "OnForegroundApplicationChanged CALLBACK FIRED (AMS->bin) "
                    "bundle=%s pid=%d uid=%d state=%d",
             d.bundleName.c_str(), d.pid, d.uid, d.state);
    }

    void OnProcessDied(const OHOS::AppExecFwk::ProcessData &d) override
    {
        g_revFired.store(2);
        Emit(false, "OnProcessDied CALLBACK FIRED (AMS->bin) bundle=%s pid=%d",
             d.bundleName.c_str(), d.pid);
    }

    void OnProcessCreated(const OHOS::AppExecFwk::ProcessData &d) override
    {
        g_revFired.store(3);
        Emit(false, "OnProcessCreated CALLBACK FIRED (AMS->bin) bundle=%s pid=%d",
             d.bundleName.c_str(), d.pid);
    }
    // other pure-virtuals inherit ApplicationStateObserverStub's default no-op bodies
};

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    Emit(false, "ams_ipc_probe_rev: start pid=%d uid=%d", static_cast<int>(getpid()),
         static_cast<int>(getuid()));

    OHOS::AppExecFwk::AppMgrClient client;
    OHOS::sptr<ProbeObserver> observer = new ProbeObserver();

    // Forward IPC: register the observer stub with AppMgr (SA 501).
    int32_t regRet = client.RegisterApplicationStateObserver(observer);
    Emit(false, "[FWD] RegisterApplicationStateObserver ret=%d", regRet);
    if (regRet != 0) {
        Emit(true, "ams_ipc_probe_rev: register failed (ret=%d). If ret=201 (ERR_PERMISSION_DENIED) "
                   "the donor app lacks ohos.permission.RUNNING_STATE_OBSERVER -- required by "
                   "AppStateObserverManager::RegisterApplicationStateObserver. Use a system donor "
                   "app that holds it, or pre-grant via `bm grant`.", regRet);
        return 1;
    }
    bool fwdOk = (regRet == 0);

    // Reverse IPC: wait for AppMgr to call back. Trigger by changing any app's
    // state on the device (launch/fg/bg/kill any app).
    Emit(false, "ams_ipc_probe_rev: NOW trigger an app state change on the device "
                "(launch/foreground/background/kill any app). Waiting up to 25s...");

    g_revFired.store(0);
    for (int i = 0; i < 250 && g_revFired.load() == 0; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    int rev = g_revFired.load();
    bool revOk = (rev != 0);
    const char *which = rev == 1 ? "OnForegroundApplicationChanged"
                        : rev == 2 ? "OnProcessDied"
                        : rev == 3 ? "OnProcessCreated"
                                   : "(none)";
    Emit(false, "[REV] callback fired=%s (%s)", revOk ? "YES" : "NO", which);

    int32_t unregRet = client.UnregisterApplicationStateObserver(observer);
    Emit(false, "UnregisterApplicationStateObserver ret=%d", unregRet);

    Emit(false, "ams_ipc_probe_rev: verdict FWD(bin->AppMgr)=%s REV(AppMgr->bin)=%s",
         fwdOk ? "OK" : "FAILED", revOk ? "OK" : "FAILED");
    if (!revOk) {
        Emit(true, "ams_ipc_probe_rev: reverse IPC NOT observed in 25s. Did you "
                   "trigger an app state change? (e.g. `aa start -b <anyBundle> "
                   "-a <anyAbility>` or tap an app icon)");
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    Emit(false, "ams_ipc_probe_rev: done");
    return (fwdOk && revOk) ? 0 : 1;
}
