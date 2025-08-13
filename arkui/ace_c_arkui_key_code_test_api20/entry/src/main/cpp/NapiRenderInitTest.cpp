/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "common/Common.h"
#include "keyevent/KeyEventTest.h"
#include "focuscontrol/FocusControlTest.h"
#include "nodeevent/NodeEventTest.h"
#include "nodeadapter/NodeAdapterTest.h"
#include "nodecontent/NodeContentTest.h"
#include "snapshot/SnapShotTest.h"
#include "scope/RunTaskInScopeTest.h"
#include "nodenapi/NodeNapiTest.h"
#include "nodeutilstest/NodeUtilsTest.h"

namespace ArkUINodecapiTest {
EXTERN_C_START
static void RenderInitPush0(napi_property_descriptor* desc)
{
    napi_property_descriptor descTemp[] = {
        {"FocusControlTest_001", nullptr, FocusControlTest::FocusControlTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"FocusControlTest_002", nullptr, FocusControlTest::FocusControlTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"FocusControlTest_003", nullptr, FocusControlTest::FocusControlTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"FocusControlTest_004", nullptr, FocusControlTest::FocusControlTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"IsNumLockOnTest_002", nullptr, KeyEventTest::IsNumLockOnTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"IsCapsLockOnTest_002", nullptr, KeyEventTest::IsCapsLockOnTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"IsScrollLockOnTest_002", nullptr, KeyEventTest::IsScrollLockOnTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CreateTextInputNode", nullptr, KeyEventTest::CreateTextInputNode,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"IsNumLockOnTest_001", nullptr, KeyEventTest::IsNumLockOnTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"IsCapsLockOnTest_001", nullptr, KeyEventTest::IsCapsLockOnTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"IsScrollLockOnTest_001", nullptr, KeyEventTest::IsScrollLockOnTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNumberValueTest_001", nullptr, NodeEventTest::GetNumberValueTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNumberValueTest_002", nullptr, NodeEventTest::GetNumberValueTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNumberValueTest_003", nullptr, NodeEventTest::GetNumberValueTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetStringValueTest_001", nullptr, NodeEventTest::GetStringValueTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetStringValueTest_002", nullptr, NodeEventTest::GetStringValueTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetStringValueTest_003", nullptr, NodeEventTest::GetStringValueTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetReturnNumberValueTest_001", nullptr, NodeEventTest::SetReturnNumberValueTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetReturnNumberValueTest_002", nullptr, NodeEventTest::SetReturnNumberValueTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetReturnNumberValueTest_003", nullptr, NodeEventTest::SetReturnNumberValueTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CreateNode", nullptr, NodeEventTest::CreateNode,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index] = descTemp[index];
    }
}

static void RenderInitPush1(napi_property_descriptor* desc)
{
    const uint32_t offset = 21;
    napi_property_descriptor descTemp[] = {
        {"SetTotalNodeCountTest_001", nullptr, NodeAdapterTest::SetTotalNodeCountTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetTotalNodeCountTest_002", nullptr, NodeAdapterTest::SetTotalNodeCountTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RegisterEventReceiverTest_001", nullptr, NodeAdapterTest::RegisterEventReceiverTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"ReloadAllItemsTest_001", nullptr, NodeAdapterTest::ReloadAllItemsTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"ReloadAllItemsTest_002", nullptr, NodeAdapterTest::ReloadAllItemsTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"ReloadItemTest_001", nullptr, NodeAdapterTest::ReloadItemTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"ReloadItemTest_002", nullptr, NodeAdapterTest::ReloadItemTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RemoveItemTest_001", nullptr, NodeAdapterTest::RemoveItemTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RemoveItemTest_002", nullptr, NodeAdapterTest::RemoveItemTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"InsertItemTest_001", nullptr, NodeAdapterTest::InsertItemTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"InsertItemTest_002", nullptr, NodeAdapterTest::InsertItemTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"MoveItemTest_001", nullptr, NodeAdapterTest::MoveItemTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"MoveItemTest_002", nullptr, NodeAdapterTest::MoveItemTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetAllItemsTest_001", nullptr, NodeAdapterTest::GetAllItemsTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetAllItemsTest_002", nullptr, NodeAdapterTest::GetAllItemsTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetItemTest_001", nullptr, NodeAdapterTest::SetItemTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetItemTest_002", nullptr, NodeAdapterTest::SetItemTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetNodeIdTest_001", nullptr, NodeAdapterTest::SetNodeIdTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetNodeIdTest_002", nullptr, NodeAdapterTest::SetNodeIdTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CreateAdapterNode", nullptr, NodeAdapterTest::CreateAdapterNode,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static void RenderInitPush2(napi_property_descriptor* desc)
{
    const uint32_t offset = 41;
    napi_property_descriptor descTemp[] = {
        {"RegisterCallbackTest_001", nullptr, NodeContentTest::RegisterCallbackTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RegisterCallbackTest_002", nullptr, NodeContentTest::RegisterCallbackTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetUserDataTest_001", nullptr, NodeContentTest::SetUserDataTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetUserDataTest_002", nullptr, NodeContentTest::SetUserDataTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"AddNodeTest_001", nullptr, NodeContentTest::AddNodeTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"AddNodeTest_002", nullptr, NodeContentTest::AddNodeTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RemoveNodeTest_001", nullptr, NodeContentTest::RemoveNodeTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RemoveNodeTest_002", nullptr, NodeContentTest::RemoveNodeTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"InsertNodeTest_001", nullptr, NodeContentTest::InsertNodeTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"InsertNodeTest_002", nullptr, NodeContentTest::InsertNodeTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"createNode1", nullptr, NodeContentTest::createNode1, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeSnapshotTest_001", nullptr, SnapShotTest::GetNodeSnapshotTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeSnapshotTest_002", nullptr, SnapShotTest::GetNodeSnapshotTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeSnapshotTest_003", nullptr, SnapShotTest::GetNodeSnapshotTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeSnapshotTest_004", nullptr, SnapShotTest::GetNodeSnapshotTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"testRunTaskInScope", nullptr, RunTaskInScopeTest::testRunTaskInScope,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RunTaskInScopeTest_001", nullptr, RunTaskInScopeTest::RunTaskInScopeTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RunTaskInScopeTest_002", nullptr, RunTaskInScopeTest::RunTaskInScopeTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RunTaskInScopeTest_003", nullptr, RunTaskInScopeTest::RunTaskInScopeTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RunTaskInScopeTest_004", nullptr, RunTaskInScopeTest::RunTaskInScopeTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static void RenderInitPush3(napi_property_descriptor* desc)
{
    const uint32_t offset = 61;
    napi_property_descriptor descTemp[] = {
        {"GetNodeHandleFromNapiValueTest_001", nullptr, NodeNapiTest::GetNodeHandleFromNapiValueTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeHandleFromNapiValueTest_002", nullptr, NodeNapiTest::GetNodeHandleFromNapiValueTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetContextFromNapiValueTest_001", nullptr, NodeNapiTest::GetContextFromNapiValueTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetContextFromNapiValueTest_002", nullptr, NodeNapiTest::GetContextFromNapiValueTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeContentFromNapiValueTest_001", nullptr, NodeNapiTest::GetNodeContentFromNapiValueTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static void RenderInitPush4(napi_property_descriptor* desc)
{
    const uint32_t offset = 66;
    napi_property_descriptor descTemp[] = {
        {"GetNodeContentFromNapiValueTest_002", nullptr, NodeNapiTest::GetNodeContentFromNapiValueTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetDrawableDescriptorFromNapiValueTest_001", nullptr,
            NodeNapiTest::GetDrawableDescriptorFromNapiValueTest_001, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetDrawableDescriptorFromNapiValueTest_002", nullptr,
            NodeNapiTest::GetDrawableDescriptorFromNapiValueTest_002, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetDrawableDescriptorFromResourceNapiValueTest_001", nullptr,
            NodeNapiTest::GetDrawableDescriptorFromResourceTest_001, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetDrawableDescriptorFromResourceNapiValueTest_002", nullptr,
            NodeNapiTest::GetDrawableDescriptorFromResourceTest_002, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CreateFrameNode", nullptr, NodeNapiTest::CreateFrameNode,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CreateUIContext", nullptr, NodeNapiTest::CreateUIContext,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CreateNodeContent", nullptr, NodeNapiTest::CreateNodeContent,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CreateDrawableDescriptor", nullptr, NodeNapiTest::CreateDrawableDescriptor,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CreateResource", nullptr, NodeNapiTest::CreateResource,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CreateNavigation", nullptr, NodeNapiTest::CreateNavigation,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavigationIdTest_001", nullptr, NodeNapiTest::GetNavigationIdTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavigationIdTest_002", nullptr, NodeNapiTest::GetNavigationIdTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavigationIdTest_003", nullptr, NodeNapiTest::GetNavigationIdTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavigationIdTest_004", nullptr, NodeNapiTest::GetNavigationIdTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavigationIdTest_005", nullptr, NodeNapiTest::GetNavigationIdTest_005,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavigationIdTest_006", nullptr, NodeNapiTest::GetNavigationIdTest_006,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CreateNavDestination", nullptr, NodeNapiTest::CreateNavDestination,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationNameTest_001", nullptr, NodeNapiTest::GetNavDestinationNameTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationNameTest_002", nullptr, NodeNapiTest::GetNavDestinationNameTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationNameTest_003", nullptr, NodeNapiTest::GetNavDestinationNameTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static void RenderInitPush5(napi_property_descriptor* desc)
{
    const uint32_t offset = 87;
    napi_property_descriptor descTemp[] = {
        {"GetNavDestinationNameTest_004", nullptr, NodeNapiTest::GetNavDestinationNameTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationNameTest_005", nullptr, NodeNapiTest::GetNavDestinationNameTest_005,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationNameTest_006", nullptr, NodeNapiTest::GetNavDestinationNameTest_006,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavStackLengthTest_001", nullptr, NodeNapiTest::GetNavStackLengthTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavStackLengthTest_002", nullptr, NodeNapiTest::GetNavStackLengthTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavStackLengthTest_003", nullptr, NodeNapiTest::GetNavStackLengthTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationNameByIndexTest_001", nullptr, NodeNapiTest::GetNavDestinationNameByIndexTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationNameByIndexTest_002", nullptr, NodeNapiTest::GetNavDestinationNameByIndexTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationNameByIndexTest_003", nullptr, NodeNapiTest::GetNavDestinationNameByIndexTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationNameByIndexTest_004", nullptr, NodeNapiTest::GetNavDestinationNameByIndexTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationNameByIndexTest_005", nullptr, NodeNapiTest::GetNavDestinationNameByIndexTest_005,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationNameByIndexTest_006", nullptr, NodeNapiTest::GetNavDestinationNameByIndexTest_006,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationNameByIndexTest_007", nullptr, NodeNapiTest::GetNavDestinationNameByIndexTest_007,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationIdTest_001", nullptr, NodeNapiTest::GetNavDestinationIdTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationIdTest_002", nullptr, NodeNapiTest::GetNavDestinationIdTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationIdTest_003", nullptr, NodeNapiTest::GetNavDestinationIdTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationIdTest_004", nullptr, NodeNapiTest::GetNavDestinationIdTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationIdTest_005", nullptr, NodeNapiTest::GetNavDestinationIdTest_005,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationIdTest_006", nullptr, NodeNapiTest::GetNavDestinationIdTest_006,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationStateTest_001", nullptr, NodeNapiTest::GetNavDestinationStateTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationStateTest_002", nullptr, NodeNapiTest::GetNavDestinationStateTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static void RenderInitPush6(napi_property_descriptor* desc)
{
    const uint32_t offset = 108;
    napi_property_descriptor descTemp[] = {
        {"GetNavDestinationStateTest_003", nullptr, NodeNapiTest::GetNavDestinationStateTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationStateTest_004", nullptr, NodeNapiTest::GetNavDestinationStateTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationIndexTest_001", nullptr, NodeNapiTest::GetNavDestinationIndexTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationIndexTest_002", nullptr, NodeNapiTest::GetNavDestinationIndexTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationIndexTest_003", nullptr, NodeNapiTest::GetNavDestinationIndexTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNavDestinationIndexTest_004", nullptr, NodeNapiTest::GetNavDestinationIndexTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageIndexTest_001", nullptr, NodeNapiTest::GetRouterPageIndexTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageIndexTest_002", nullptr, NodeNapiTest::GetRouterPageIndexTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageIndexTest_003", nullptr, NodeNapiTest::GetRouterPageIndexTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageIndexTest_004", nullptr, NodeNapiTest::GetRouterPageIndexTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageNameTest_001", nullptr, NodeNapiTest::GetRouterPageNameTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageNameTest_002", nullptr, NodeNapiTest::GetRouterPageNameTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageNameTest_003", nullptr, NodeNapiTest::GetRouterPageNameTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageNameTest_004", nullptr, NodeNapiTest::GetRouterPageNameTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageNameTest_005", nullptr, NodeNapiTest::GetRouterPageNameTest_005,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageNameTest_006", nullptr, NodeNapiTest::GetRouterPageNameTest_006,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPagePathTest_001", nullptr, NodeNapiTest::GetRouterPagePathTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPagePathTest_002", nullptr, NodeNapiTest::GetRouterPagePathTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPagePathTest_003", nullptr, NodeNapiTest::GetRouterPagePathTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPagePathTest_004", nullptr, NodeNapiTest::GetRouterPagePathTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPagePathTest_005", nullptr, NodeNapiTest::GetRouterPagePathTest_005,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static void RenderInitPush7(napi_property_descriptor* desc)
{
    const uint32_t offset = 129;
    napi_property_descriptor descTemp[] = {
        {"GetRouterPagePathTest_006", nullptr, NodeNapiTest::GetRouterPagePathTest_006,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageStateTest_001", nullptr, NodeNapiTest::GetRouterPageStateTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageStateTest_002", nullptr, NodeNapiTest::GetRouterPageStateTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageStateTest_003", nullptr, NodeNapiTest::GetRouterPageStateTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageStateTest_004", nullptr, NodeNapiTest::GetRouterPageStateTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageIdTest_001", nullptr, NodeNapiTest::GetRouterPageIdTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageIdTest_002", nullptr, NodeNapiTest::GetRouterPageIdTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageIdTest_003", nullptr, NodeNapiTest::GetRouterPageIdTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageIdTest_004", nullptr, NodeNapiTest::GetRouterPageIdTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageIdTest_005", nullptr, NodeNapiTest::GetRouterPageIdTest_005,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetRouterPageIdTest_006", nullptr, NodeNapiTest::GetRouterPageIdTest_006,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}

static void RenderInitPush8(napi_property_descriptor* desc)
{
    const uint32_t offset = 140;
    napi_property_descriptor descTemp[] = {
        {"GetNodeTypeTest_001", nullptr, NodeUtilsTest::GetNodeTypeTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_002", nullptr, NodeUtilsTest::GetNodeTypeTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_003", nullptr, NodeUtilsTest::GetNodeTypeTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_004", nullptr, NodeUtilsTest::GetNodeTypeTest_004,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_005", nullptr, NodeUtilsTest::GetNodeTypeTest_005,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_006", nullptr, NodeUtilsTest::GetNodeTypeTest_006,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_007", nullptr, NodeUtilsTest::GetNodeTypeTest_007,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_008", nullptr, NodeUtilsTest::GetNodeTypeTest_008,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_009", nullptr, NodeUtilsTest::GetNodeTypeTest_009,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_010", nullptr, NodeUtilsTest::GetNodeTypeTest_010,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_011", nullptr, NodeUtilsTest::GetNodeTypeTest_011,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_012", nullptr, NodeUtilsTest::GetNodeTypeTest_012,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_013", nullptr, NodeUtilsTest::GetNodeTypeTest_013,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_014", nullptr, NodeUtilsTest::GetNodeTypeTest_014,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_015", nullptr, NodeUtilsTest::GetNodeTypeTest_015,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_016", nullptr, NodeUtilsTest::GetNodeTypeTest_016,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_017", nullptr, NodeUtilsTest::GetNodeTypeTest_017,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_018", nullptr, NodeUtilsTest::GetNodeTypeTest_018,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_019", nullptr, NodeUtilsTest::GetNodeTypeTest_019,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_020", nullptr, NodeUtilsTest::GetNodeTypeTest_020,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static void RenderInitPush9(napi_property_descriptor* desc)
{
    const uint32_t offset = 160;
    napi_property_descriptor descTemp[] = {
        {"GetNodeTypeTest_021", nullptr, NodeUtilsTest::GetNodeTypeTest_021,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_022", nullptr, NodeUtilsTest::GetNodeTypeTest_022,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_023", nullptr, NodeUtilsTest::GetNodeTypeTest_023,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_024", nullptr, NodeUtilsTest::GetNodeTypeTest_024,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_025", nullptr, NodeUtilsTest::GetNodeTypeTest_025,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_026", nullptr, NodeUtilsTest::GetNodeTypeTest_026,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_027", nullptr, NodeUtilsTest::GetNodeTypeTest_027,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_028", nullptr, NodeUtilsTest::GetNodeTypeTest_028,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_029", nullptr, NodeUtilsTest::GetNodeTypeTest_029,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_030", nullptr, NodeUtilsTest::GetNodeTypeTest_030,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_031", nullptr, NodeUtilsTest::GetNodeTypeTest_031,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_032", nullptr, NodeUtilsTest::GetNodeTypeTest_032,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_033", nullptr, NodeUtilsTest::GetNodeTypeTest_033,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_034", nullptr, NodeUtilsTest::GetNodeTypeTest_034,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_035", nullptr, NodeUtilsTest::GetNodeTypeTest_035,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_036", nullptr, NodeUtilsTest::GetNodeTypeTest_036,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_037", nullptr, NodeUtilsTest::GetNodeTypeTest_037,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_038", nullptr, NodeUtilsTest::GetNodeTypeTest_038,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_039", nullptr, NodeUtilsTest::GetNodeTypeTest_039,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeTypeTest_040", nullptr, NodeUtilsTest::GetNodeTypeTest_040,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static void RenderInitPush10(napi_property_descriptor* desc)
{
    const uint32_t offset = 180;
    napi_property_descriptor descTemp[] = {
        {"GetLayoutSizeTest_001", nullptr, NodeUtilsTest::GetLayoutSizeTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetLayoutSizeTest_002", nullptr, NodeUtilsTest::GetLayoutSizeTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetLayoutPositionTest_001", nullptr, NodeUtilsTest::GetLayoutPositionTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetLayoutPositionTest_002", nullptr, NodeUtilsTest::GetLayoutPositionTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetLayoutPositionInWindowTest_001", nullptr, NodeUtilsTest::GetLayoutPositionInWindowTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetLayoutPositionInWindowTest_002", nullptr, NodeUtilsTest::GetLayoutPositionInWindowTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetLayoutPositionInScreenTest_001", nullptr, NodeUtilsTest::GetLayoutPositionInScreenTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetLayoutPositionInScreenTest_002", nullptr, NodeUtilsTest::GetLayoutPositionInScreenTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetLayoutPositionInGlobalDisplayTest_001", nullptr, NodeUtilsTest::GetLayoutPositionInGlobalDisplayTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetLayoutPositionInGlobalDisplayTest_002", nullptr, NodeUtilsTest::GetLayoutPositionInGlobalDisplayTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetPositionWithTranslateInWindowTest_001", nullptr, NodeUtilsTest::GetPositionWithTranslateInWindowTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetPositionWithTranslateInWindowTest_002", nullptr, NodeUtilsTest::GetPositionWithTranslateInWindowTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetPositionWithTranslateInScreenTest_001", nullptr, NodeUtilsTest::GetPositionWithTranslateInScreenTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetPositionWithTranslateInScreenTest_002", nullptr, NodeUtilsTest::GetPositionWithTranslateInScreenTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetCustomPropertyTest_001", nullptr, NodeUtilsTest::GetCustomPropertyTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetActiveChildrenInfoTest_001", nullptr, NodeUtilsTest::GetActiveChildrenInfoTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetActiveChildrenInfoTest_002", nullptr, NodeUtilsTest::GetActiveChildrenInfoTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"MoveToTest_001", nullptr, NodeUtilsTest::MoveToTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetCrossLanguageOptionTest_001", nullptr, NodeUtilsTest::SetCrossLanguageOptionTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetCrossLanguageOptionTest_002", nullptr, NodeUtilsTest::SetCrossLanguageOptionTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}

static void RenderInitPush11(napi_property_descriptor* desc)
{
    const uint32_t offset = 200;
    napi_property_descriptor descTemp[] = {
        {"GetCrossLanguageOptionTest_001", nullptr, NodeUtilsTest::GetCrossLanguageOptionTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetCrossLanguageOptionTest_002", nullptr, NodeUtilsTest::GetCrossLanguageOptionTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetForceDarkConfigTest_001", nullptr, NodeUtilsTest::SetForceDarkConfigTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetForceDarkConfigTest_002", nullptr, NodeUtilsTest::SetForceDarkConfigTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetForceDarkConfigTest_003", nullptr, NodeUtilsTest::SetForceDarkConfigTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetCustomSpanMeasureInfoTest_001", nullptr, NodeUtilsTest::GetCustomSpanMeasureInfoTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetCustomSpanMeasureInfoTest_002", nullptr, NodeUtilsTest::GetCustomSpanMeasureInfoTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetCustomSpanMetricsTest_001", nullptr, NodeUtilsTest::SetCustomSpanMetricsTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"SetCustomSpanMetricsTest_002", nullptr, NodeUtilsTest::SetCustomSpanMetricsTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetCustomSpanDrawInfoTest_001", nullptr, NodeUtilsTest::GetCustomSpanDrawInfoTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetCustomSpanDrawInfoTest_002", nullptr, NodeUtilsTest::GetCustomSpanDrawInfoTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CreateCustomSpanNode", nullptr, NodeUtilsTest::CreateCustomSpanNode,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CloseAllSwipeActionsTest_001", nullptr, NodeUtilsTest::CloseAllSwipeActionsTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CloseAllSwipeActionsTest_002", nullptr, NodeUtilsTest::CloseAllSwipeActionsTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"CloseAllSwipeActionsTest_003", nullptr, NodeUtilsTest::CloseAllSwipeActionsTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RegisterSystemColorModeChangeTest_001", nullptr, NodeUtilsTest::RegisterSystemColorModeChangeTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RegisterSystemColorModeChangeTest_002", nullptr, NodeUtilsTest::RegisterSystemColorModeChangeTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RegisterSystemFontStyleChangeTest_001", nullptr, NodeUtilsTest::RegisterSystemFontStyleChangeTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RegisterSystemFontStyleChangeTest_002", nullptr, NodeUtilsTest::RegisterSystemFontStyleChangeTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RegisterLayoutCallbackOnNodeTest_001", nullptr, NodeUtilsTest::RegisterLayoutCallbackOnNodeTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}

static void RenderInitPush12(napi_property_descriptor* desc)
{
    const uint32_t offset = 220;
    napi_property_descriptor descTemp[] = {
        {"RegisterLayoutCallbackOnNodeTest_002", nullptr, NodeUtilsTest::RegisterLayoutCallbackOnNodeTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RegisterDrawCallbackOnNodeTest_001", nullptr, NodeUtilsTest::RegisterDrawCallbackOnNodeTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RegisterDrawCallbackOnNodeTest_002", nullptr, NodeUtilsTest::RegisterDrawCallbackOnNodeTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"UnregisterLayoutCallbackOnNodeTest_001", nullptr, NodeUtilsTest::UnregisterLayoutCallbackOnNodeTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"UnregisterLayoutCallbackOnNodeTest_002", nullptr, NodeUtilsTest::UnregisterLayoutCallbackOnNodeTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"UnregisterDrawCallbackOnNodeTest_001", nullptr, NodeUtilsTest::UnregisterDrawCallbackOnNodeTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"UnregisterDrawCallbackOnNodeTest_002", nullptr, NodeUtilsTest::UnregisterDrawCallbackOnNodeTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetPositionToParentTest_001", nullptr, NodeUtilsTest::GetPositionToParentTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetPositionToParentTest_002", nullptr, NodeUtilsTest::GetPositionToParentTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeHandleByUniqueIdTest_001", nullptr, NodeUtilsTest::GetNodeHandleByUniqueIdTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeHandleByUniqueIdTest_002", nullptr, NodeUtilsTest::GetNodeHandleByUniqueIdTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeHandleByUniqueIdTest_003", nullptr, NodeUtilsTest::GetNodeHandleByUniqueIdTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeUniqueIdTest_001", nullptr, NodeUtilsTest::GetNodeUniqueIdTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeUniqueIdTest_002", nullptr, NodeUtilsTest::GetNodeUniqueIdTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetNodeUniqueIdTest_003", nullptr, NodeUtilsTest::GetNodeUniqueIdTest_003,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"AddSupportedUIStatesTest_001", nullptr, NodeUtilsTest::AddSupportedUIStatesTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"AddSupportedUIStatesTest_002", nullptr, NodeUtilsTest::AddSupportedUIStatesTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RemoveSupportedUIStatesTest_001", nullptr, NodeUtilsTest::RemoveSupportedUIStatesTest_001,
            nullptr, nullptr, nullptr, napi_default, nullptr},
        {"RemoveSupportedUIStatesTest_002", nullptr, NodeUtilsTest::RemoveSupportedUIStatesTest_002,
            nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}

static napi_value RenderInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[300] = {};
    RenderInitPush0(desc);
    RenderInitPush1(desc);
    RenderInitPush2(desc);
    RenderInitPush3(desc);
    RenderInitPush4(desc);
    RenderInitPush5(desc);
    RenderInitPush6(desc);
    RenderInitPush7(desc);
    RenderInitPush8(desc);
    RenderInitPush9(desc);
    RenderInitPush10(desc);
    RenderInitPush11(desc);
    RenderInitPush12(desc);
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module noderenderndkModule = {.nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = RenderInit,
    .nm_modname = "noderenderndk",
    .nm_priv = ((void *)0),
    .reserved = {0}};

extern "C" __attribute__((constructor)) void RegisterModule(void) { napi_module_register(&noderenderndkModule); }
} // namespace ArkUICapiTest
