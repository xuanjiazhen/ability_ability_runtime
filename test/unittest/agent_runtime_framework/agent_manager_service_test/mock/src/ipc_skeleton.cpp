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

#include "ipc_skeleton.h"

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif

static pid_t g_mockCallingUid = 200000;
static pid_t g_mockCallingPid = 1000;
static pid_t g_mockCallingTokenId = 0;

bool IPCSkeleton::SetMaxWorkThreadNum(int maxThreadNum)
{
    return true;
}

void IPCSkeleton::JoinWorkThread()
{}

void IPCSkeleton::StopWorkThread()
{}

pid_t IPCSkeleton::GetCallingUid()
{
    return g_mockCallingUid;
}

pid_t IPCSkeleton::GetCallingPid()
{
    return g_mockCallingPid;
}

uint32_t IPCSkeleton::GetCallingTokenID()
{
    return g_mockCallingTokenId;
}

std::string IPCSkeleton::GetLocalDeviceID()
{
    return "";
}

std::string IPCSkeleton::GetCallingDeviceID()
{
    return "";
}

bool IPCSkeleton::IsLocalCalling()
{
    return true;
}

IPCSkeleton &IPCSkeleton::GetInstance()
{
    static IPCSkeleton instance;
    return instance;
}

sptr<IRemoteObject> IPCSkeleton::GetContextObject()
{
    return nullptr;
}

bool IPCSkeleton::SetContextObject(sptr<IRemoteObject> &object)
{
    return true;
}

int IPCSkeleton::FlushCommands(IRemoteObject *object)
{
    return 0;
}

std::string IPCSkeleton::ResetCallingIdentity()
{
    return "";
}

bool IPCSkeleton::SetCallingIdentity(std::string &identity)
{
    return true;
}

void IPCSkeleton::SetCallingUid(pid_t uid)
{
    g_mockCallingUid = uid;
}

uint32_t IPCSkeleton::SetCallingTokenID(pid_t tokenId)
{
    g_mockCallingTokenId = tokenId;
    return g_mockCallingTokenId;
}
}  // namespace OHOS
