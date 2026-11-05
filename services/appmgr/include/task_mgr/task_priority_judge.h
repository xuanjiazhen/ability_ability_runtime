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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_TASK_PRIORITY_JUDGE_H
#define OHOS_APP_EXECFWK_TASK_MGR_TASK_PRIORITY_JUDGE_H

#include <cstdint>
#include <map>
#include <sys/types.h>
#include <memory>
#include <mutex>
#include <string>

#include "task_mgr/system_load_provider.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * @brief Judges the task priority level from the caller foreground state and the system
 *        load fusion level. The judge only caches its two inputs (the per-uid foreground
 *        ability counts and the fusion level); the wiring of the inputs (application state
 *        subscription, load listener registration) is done by TaskManagerService when the
 *        task manager assembles. All members are protected by judgeMutex_, a leaf lock that
 *        is only ever taken while (or without) the task record mutex and never calls out,
 *        so the lock order tasksMutex_ -> judgeMutex_ is respected everywhere (design
 *        ADR-4/ADR-5/ADR-6/ADR-7).
 */
class TaskPriorityJudge {
public:
    static TaskPriorityJudge &GetInstance();

    /**
     * @brief Compile-time default mapping matrix:
     *        foreground (the caller process hosts a foreground ability) -> HIGH;
     *        background & fusion level 5~7 (high load) -> LOW;
     *        background & fusion level 0~4 or the load unavailable -> MEDIUM.
     * @param foreground Indicates whether the caller process hosts a foreground ability.
     * @param loadLevel Indicates the fusion level (0~7); a negative value means unavailable.
     * @return Returns one of TASK_PRIORITY_HIGH / TASK_PRIORITY_MEDIUM / TASK_PRIORITY_LOW.
     */
    static int32_t DetermineLevel(bool foreground, int32_t loadLevel);

    /**
     * @brief Updates the foreground ability count of a caller process; called from the
     *        application state subscription hook whenever an ability of the process moves
     *        to or from the foreground. Terminated/ended abilities are reported as
     *        "left the foreground" so an abnormal death cannot leak the count (defense in
     *        depth; the count guard already tolerates the extra decrement).
     * @param pid Indicates the caller process id.
     * @param foreground Indicates true when an ability entered the foreground; false when
     *        it left the foreground (including termination).
     */
    void OnForegroundAbilityChanged(pid_t pid, bool foreground);

    /**
     * @brief Updates the cached fusion level; called from the load listener callback and
     *        once right after a successful listener registration.
     * @param loadLevel Indicates the new fusion level (0~7); values outside the range are
     *        treated as unavailable (fallback to the default level).
     */
    void OnSystemLoadLevelChanged(int32_t loadLevel);

    /**
     * @brief Judges the level for a caller process from the cached inputs.
     * @param pid Indicates the caller process id.
     * @return Returns the judged task level (HIGH/MEDIUM/LOW); falls back to the default
     *         level when the inputs are unavailable.
     */
    int32_t JudgeLevel(pid_t pid) const;

    /**
     * @brief Queries whether the caller process currently hosts any foreground ability.
     * @param pid Indicates the caller process id.
     * @return Returns true when at least one foreground ability exists.
     */
    bool IsForeground(pid_t pid) const;

    /**
     * @brief Queries the cached fusion level.
     * @return Returns the fusion level (0~7); -1 when unavailable.
     */
    int32_t CurrentLoadLevel() const;

    /**
     * @brief Injects the load provider; used by unit tests to stub the load source. When
     *        never called, the default ressched-backed provider is created lazily on the
     *        first RegisterSystemLoadListener.
     * @param provider Indicates the provider, or nullptr to restore the default.
     */
    void SetLoadProvider(const std::shared_ptr<ISystemLoadProvider> &provider);

    /**
     * @brief Registers the fusion level listener through the load provider; on failure the
     *        load input stays unavailable and every background task is judged with the
     *        fallback level (spec R-7).
     * @return Returns true when the listener is active.
     */
    bool RegisterSystemLoadListener();

    /**
     * @brief Sets the hook invoked (on the provider callback thread) after every fusion
     *        level cache update; TaskManagerService uses it to re-judge the running tasks.
     * @param hook Indicates the hook, or nullptr to clear.
     */
    void SetLevelChangedHook(std::function<void(int32_t)> hook);

    /**
     * @brief Unregisters the fusion level listener.
     */
    void UnregisterSystemLoadListener();

    /**
     * @brief Renders the judge state section for the --psched dump (cached fusion level,
     *        per-uid foreground ability counts and the listener registration state).
     * @return Returns the dump text.
     */
    std::string DumpJudgeState() const;

private:
    TaskPriorityJudge() = default;
    ~TaskPriorityJudge() = default;
    TaskPriorityJudge(const TaskPriorityJudge &) = delete;
    TaskPriorityJudge &operator=(const TaskPriorityJudge &) = delete;

    static bool IsHighLoad(int32_t loadLevel);

    // Invoked right after a fusion level cache update to drive the task re-judgment.
    void NotifyLevelChanged(int32_t level);

    std::shared_ptr<ISystemLoadProvider> loadProvider_;
    std::function<void(int32_t)> levelChangedHook_;
    mutable std::mutex judgeMutex_;
    std::map<pid_t, int32_t> fgAbilityCounts_;
    int32_t loadLevel_ = -1;
    bool loadListenerRegistered_ = false;
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_APP_EXECFWK_TASK_MGR_TASK_PRIORITY_JUDGE_H
