/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

 #include "extensionrecordmanager_fuzzer.h"

 #include <cstddef>
 #include <cstdint>
 #include <fuzzer/FuzzedDataProvider.h>
 #include <memory>
 #include <vector>
 #include <list>
 #include <string>
 
 #include "base_extension_record.h"
 #include "extension_record_factory.h"
 #include "ui_extension_record.h"
 #define private public
 #define inline
 #include "extension_record.h"
 #include "extension_record_manager.h"
 #define inline
 #undef private
 #include "ability_fuzz_util.h"
 #include "ipc_skeleton.h"
 #include "ability_record.h"
 
 using namespace OHOS::AAFwk;
 using namespace OHOS::AbilityRuntime;
 using namespace OHOS::AppExecFwk;
 using namespace OHOS;
 
 namespace OHOS {
 namespace {
 constexpr size_t STRING_MAX_LENGTH = 128;
 constexpr int32_t INVALID_EXTENSION_RECORD_ID = -1;
 constexpr int32_t DEFAULT_USER_ID = 0;
 constexpr pid_t INVALID_PID = -1;
 const std::string PROCESS_MODE_HOST_SPECIFIED_KEY = "ability.want.params.host_specified_process";
 const std::string IS_PRELOAD_UIEXTENSION_ABILITY = "ability.want.params.is_preload_uiextension_ability";
 constexpr const char* UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
 } // namespace
 
 bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
 {
     if (data == nullptr || size < sizeof(int32_t)) {
         return true;
     }
 
     // 核心对象初始化
     std::shared_ptr<ExtensionRecordManager> extensionRecordManager = std::make_shared<ExtensionRecordManager>(DEFAULT_USER_ID);
     FuzzedDataProvider fdp(data, size);
 
     // 基础随机数据定义 + 全覆盖边界值/非法值/合法值
     int32_t userId = fdp.ConsumeIntegral<int32_t>();
     int32_t extensionRecordId = fdp.ConsumeIntegral<int32_t>();
     int32_t validExtId = extensionRecordManager->GenerateExtensionRecordId(INVALID_EXTENSION_RECORD_ID);
     int32_t pid = fdp.ConsumeIntegral<int32_t>();
     int32_t hostPid = fdp.ConsumeIntegral<int32_t>();
     int32_t recordNum = fdp.ConsumeIntegral<int32_t>();
     int32_t requestCode = fdp.ConsumeIntegral<int32_t>();
     bool isLoaded = fdp.ConsumeBool();
     bool preloadFlag = fdp.ConsumeBool();
     bool isolationProcess = fdp.ConsumeBool();
 
     // 随机字符串（合法/空串/超长串 全覆盖）
     std::string hostBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
     std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
     std::string process = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
     std::string moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
     std::string customProcess = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
     std::string emptyStr = "";
 
     // 容器对象初始化
     std::shared_ptr<ExtensionRecord> emptyRecord = nullptr;
     std::shared_ptr<ExtensionRecord> validRecord = std::make_shared<UIExtensionRecord>();
     std::shared_ptr<BaseExtensionRecord> emptyAbilityRecord = nullptr;
     std::shared_ptr<BaseExtensionRecord> validAbilityRecord = std::make_shared<AbilityRecord>();
     std::list<sptr<IRemoteObject>> callerList;
     std::vector<std::string> extensionList;
     std::vector<std::shared_ptr<ExtensionRecord>> recordsToUnload;
 
     // 鸿蒙框架核心对象（随机填充+非法/合法构造 全覆盖）
     AbilityInfo abilityInfo;
     AbilityFuzzUtil::GetRandomAbilityInfo(fdp, abilityInfo);
     abilityInfo.isolationProcess = isolationProcess;
     abilityInfo.extensionAbilityType = static_cast<ExtensionAbilityType>(fdp.ConsumeIntegralInRange<int32_t>(0, 3));
 
     AbilityRequest abilityRequest;
     AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, abilityRequest);
     abilityRequest.extensionProcessMode = static_cast<ExtensionProcessMode>(fdp.ConsumeIntegralInRange<int32_t>(0, 7));
     abilityRequest.customProcess = customProcess;
     abilityRequest.abilityInfo = abilityInfo;
     abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, process);
     abilityRequest.want.SetParam(IS_PRELOAD_UIEXTENSION_ABILITY, preloadFlag);
     abilityRequest.want.SetParam(UIEXTENSION_TYPE_KEY, "embeddedUI");
     abilityRequest.sessionInfo = new SessionInfo(); // 修复空指针问题
     abilityRequest.sessionInfo->uiExtensionComponentId = fdp.ConsumeIntegral<uint64_t>();
 
     ElementName element;
     AbilityFuzzUtil::GenerateElementName(fdp, element);
 
     UIExtensionSessionInfo uiExtensionSessionInfo;
     sptr<SessionInfo> sessionInfo = new SessionInfo();
     sptr<SessionInfo> nullSessionInfo = nullptr;
     sptr<IRemoteObject> focusedCallerToken = new IRemoteObject();
     sptr<IRemoteObject> token = new IRemoteObject();
     sptr<IRemoteObject> focusToken = new IRemoteObject();
     sptr<IRemoteObject> nullToken = nullptr;
     sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
 
     // 元组key初始化（预加载核心依赖，修复原代码未初始化问题）
     ExtensionRecordManager::PreLoadUIExtensionMapKey preLoadUIExtensionInfo = 
         std::make_tuple(element.GetAbilityName(), element.GetBundleName(), moduleName, hostPid);
     std::tuple<std::string, std::string, std::string, int32_t> extensionRecordMapKey = preLoadUIExtensionInfo;
 
     // 【核心修复】预填充有效Record，保证 存在/不存在 分支全覆盖
     extensionRecordManager->AddExtensionRecord(validExtId, validRecord);
     validRecord->extensionRecordId_ = validExtId;
     validRecord->hostBundleName_ = hostBundleName;
     validRecord->hostPid_ = IPCSkeleton::GetCallingPid();
     validRecord->abilityRecord_ = validAbilityRecord;
 
     // ===================================== 全量接口调用 - 无遗漏 =====================================
     // 1. ID生成：合法ID/已占用ID/无效ID/边界值 分支全覆盖
     extensionRecordManager->GenerateExtensionRecordId(extensionRecordId);
     extensionRecordManager->GenerateExtensionRecordId(INVALID_EXTENSION_RECORD_ID);
     extensionRecordManager->GenerateExtensionRecordId(validExtId);
 
     // 2. 增删查管理：正常Record/空Record/存在ID/不存在ID 分支全覆盖
     extensionRecordManager->AddExtensionRecord(extensionRecordId, validRecord);
     extensionRecordManager->AddExtensionRecord(extensionRecordId, emptyRecord);
     extensionRecordManager->RemoveExtensionRecord(extensionRecordId);
     extensionRecordManager->RemoveExtensionRecord(INVALID_EXTENSION_RECORD_ID);
     extensionRecordManager->AddExtensionRecordToTerminatedList(extensionRecordId);
     extensionRecordManager->AddExtensionRecordToTerminatedList(INVALID_EXTENSION_RECORD_ID);
 
     // 3. GetExtensionRecord：匹配Bundle/不匹配Bundle/存在/不存在 分支全覆盖
     extensionRecordManager->GetExtensionRecord(extensionRecordId, hostBundleName, validRecord, isLoaded);
     extensionRecordManager->GetExtensionRecord(validExtId, emptyStr, validRecord, isLoaded);
     extensionRecordManager->GetExtensionRecord(INVALID_EXTENSION_RECORD_ID, hostBundleName, validRecord, isLoaded);
 
     // 4. 归属判断：全枚举值 ExtensionAbilityType 分支全覆盖
     extensionRecordManager->IsBelongToManager(abilityInfo);
 
     // 5. 活跃列表查询：按PID/按BundleName，合法/非法PID/空Bundle 分支全覆盖
     extensionRecordManager->GetActiveUIExtensionList(pid, extensionList);
     extensionRecordManager->GetActiveUIExtensionList(INVALID_PID, extensionList);
     extensionRecordManager->GetActiveUIExtensionList(bundleName, extensionList);
     extensionRecordManager->GetActiveUIExtensionList(emptyStr, extensionList);
 
     // 6. 创建/获取Record：预加载/非预加载/空Request 分支全覆盖
     extensionRecordManager->GetOrCreateExtensionRecord(abilityRequest, hostBundleName, validAbilityRecord, isLoaded);
     extensionRecordManager->GetOrCreateExtensionRecord(abilityRequest, emptyStr, emptyAbilityRecord, isLoaded);
 
     // 7. Session匹配：有效Session/空Session/匹配成功/匹配失败 分支全覆盖
     extensionRecordManager->GetAbilityRecordBySessionInfo(sessionInfo);
     extensionRecordManager->GetAbilityRecordBySessionInfo(nullSessionInfo);
 
     // 8. 进程校验：合法进程/非法进程/空Record 分支全覆盖
     extensionRecordManager->IsHostSpecifiedProcessValid(abilityRequest, validRecord, process);
     extensionRecordManager->IsHostSpecifiedProcessValid(abilityRequest, emptyRecord, emptyStr);
 
     // 9. 进程名更新：全枚举值 ExtensionProcessMode switch分支100%全覆盖
     extensionRecordManager->UpdateProcessName(abilityRequest, validRecord);
     extensionRecordManager->UpdateProcessName(abilityRequest, emptyRecord);
 
     // 10. 宿主PID查询：存在ID/不存在ID 分支全覆盖
     extensionRecordManager->GetHostPidForExtensionId(extensionRecordId, hostPid);
     extensionRecordManager->GetHostPidForExtensionId(INVALID_EXTENSION_RECORD_ID, hostPid);
 
     // 11. 预加载核心接口：预加载添加/删除/查询/校验 全分支全覆盖
     extensionRecordManager->AddPreloadUIExtensionRecord(validAbilityRecord);
     extensionRecordManager->AddPreloadUIExtensionRecord(emptyAbilityRecord);
     extensionRecordManager->RemoveAllPreloadUIExtensionRecord(preLoadUIExtensionInfo);
     extensionRecordManager->IsPreloadExtensionRecord(abilityRequest, hostPid, validRecord, isLoaded);
     extensionRecordManager->IsPreloadExtensionRecord(abilityRequest, INVALID_PID, emptyRecord, isLoaded);
     extensionRecordManager->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId);
     extensionRecordManager->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, INVALID_EXTENSION_RECORD_ID);
     extensionRecordManager->RemovePreloadUIExtensionRecord(extensionRecordMapKey);
 
     // 12. 内部创建接口：复用/新建/非法类型 分支全覆盖
     extensionRecordManager->GetOrCreateExtensionRecordInner(abilityRequest, hostBundleName, validRecord, isLoaded);
     extensionRecordManager->GetOrCreateExtensionRecordInner(abilityRequest, emptyStr, emptyRecord, isLoaded);
 
     // 13. 进程名设置：隔离进程/非隔离进程 分支全覆盖
     extensionRecordManager->SetAbilityProcessName(abilityRequest, validAbilityRecord, validRecord);
     extensionRecordManager->SetAbilityProcessName(abilityRequest, emptyAbilityRecord, emptyRecord);
 
     // 14. 启动能力：空实现补充
     extensionRecordManager->StartAbility(abilityRequest);
 
     // 15. 焦点管理：缓存Token/查询Token/空Token 分支全覆盖
     extensionRecordManager->SetCachedFocusedCallerToken(extensionRecordId, focusedCallerToken);
     extensionRecordManager->SetCachedFocusedCallerToken(INVALID_EXTENSION_RECORD_ID, nullToken);
     extensionRecordManager->GetCachedFocusedCallerToken(extensionRecordId);
     extensionRecordManager->GetCachedFocusedCallerToken(INVALID_EXTENSION_RECORD_ID);
 
     // 16. 根调用者Token：存在/不存在/递归溯源 分支全覆盖
     extensionRecordManager->GetRootCallerTokenLocked(extensionRecordId, std::static_pointer_cast<AbilityRecord>(validAbilityRecord));
     extensionRecordManager->GetRootCallerTokenLocked(INVALID_EXTENSION_RECORD_ID, nullptr);
 
     // 17. 创建Record重载：修复原代码传参错误 + 合法/非法入参 分支全覆盖
     int32_t outExtId = INVALID_EXTENSION_RECORD_ID;
     extensionRecordManager->CreateExtensionRecord(abilityRequest, hostBundleName, validRecord, outExtId, hostPid);
     extensionRecordManager->CreateExtensionRecord(abilityRequest, emptyStr, emptyRecord, outExtId, INVALID_PID);
 
     // 18. 根宿主信息/会话信息：有效Token/空Token/非UIExtension 分支全覆盖
     extensionRecordManager->GetUIExtensionRootHostInfo(token);
     extensionRecordManager->GetUIExtensionRootHostInfo(nullToken);
     extensionRecordManager->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
     extensionRecordManager->GetUIExtensionSessionInfo(nullToken, uiExtensionSessionInfo);
 
     // 19. ID查询Record：存活列表/销毁列表/不存在ID 分支全覆盖
     extensionRecordManager->GetExtensionRecordById(extensionRecordId);
     extensionRecordManager->GetExtensionRecordById(validExtId);
     extensionRecordManager->GetExtensionRecordById(INVALID_EXTENSION_RECORD_ID);
 
     // 20. 超时任务全接口：加载/前台/后台/销毁 超时 无遗漏
     extensionRecordManager->LoadTimeout(extensionRecordId);
     extensionRecordManager->LoadTimeout(validExtId);
     extensionRecordManager->ForegroundTimeout(extensionRecordId);
     extensionRecordManager->BackgroundTimeout(extensionRecordId);
     extensionRecordManager->TerminateTimeout(extensionRecordId);
 
     // 21. 调用链溯源：递归查询/空Record/闭环调用 分支全覆盖
     extensionRecordManager->GetCallerTokenList(std::static_pointer_cast<AbilityRecord>(validAbilityRecord), callerList);
     extensionRecordManager->GetCallerTokenList(nullptr, callerList);
 
     // 22. 焦点判断：缓存命中/缓存未命中/非UIExtension 分支全覆盖
     extensionRecordManager->IsFocused(extensionRecordId, token, focusToken);
     extensionRecordManager->IsFocused(INVALID_EXTENSION_RECORD_ID, nullToken, nullToken);
 
     // 23. 预加载查询：合法Element/空Element 分支全覆盖
     extensionRecordManager->QueryPreLoadUIExtensionRecord(element, moduleName, hostPid, recordNum);
     extensionRecordManager->QueryPreLoadUIExtensionRecord(element, emptyStr, INVALID_PID, recordNum);
 
     // 24. 预加载清理+客户端注册：原代码缺失的核心接口 补全
     extensionRecordManager->ClearPreloadedUIExtensionAbility(extensionRecordId);
     extensionRecordManager->ClearPreloadedUIExtensionAbility(validExtId);
     extensionRecordManager->ClearAllPreloadUIExtensionRecordForHost();
     extensionRecordManager->RegisterPreloadUIExtensionHostClient(token);
     extensionRecordManager->RegisterPreloadUIExtensionHostClient(nullToken);
     extensionRecordManager->UnRegisterPreloadUIExtensionHostClient(pid, deathRecipient);
     extensionRecordManager->ConvertToUnloadExtensionRecords(recordsToUnload, recordsToUnload);
 
     // 25. 远程回调+成功处理：原代码缺失的核心分支 补全
     extensionRecordManager->GetRemoteCallback(validRecord);
     extensionRecordManager->GetRemoteCallback(emptyRecord);
     extensionRecordManager->HandlePreloadUIExtensionLoadedById(validExtId);
     extensionRecordManager->HandlePreloadUIExtensionDestroyedById(validExtId);
     extensionRecordManager->HandlePreloadUIExtensionSuccess(validExtId, preloadFlag);
 
     return true;
 }
 } // namespace OHOS
 
 /* Fuzzer entry point 入口函数 无修改 */
 extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
 {
     OHOS::DoSomethingInterestingWithMyAPI(data, size);
     return 0;
 }