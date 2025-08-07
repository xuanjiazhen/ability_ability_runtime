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

#include "ets_auto_fill_extension_util.h"

#include "ani_common_util.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *VIEW_DATA_BUNDLE_NAME = "bundleName";
constexpr const char *VIEW_DATA_MODULE_NAME = "moduleName";
constexpr const char *VIEW_DATA_ABILITY_NAME = "abilityName";
constexpr const char *VIEW_DATA_PAGEURL = "pageUrl";
constexpr const char *VIEW_DATA_USER_SELECTED = "isUserSelected";
constexpr const char *VIEW_DATA_OTHER_ACCOUNT = "isOtherAccount";
constexpr const char *VIEW_DATA_PAGE_NODE_INFOS = "pageNodeInfos";
constexpr const char *VIEW_DATA_VIEW_DATA = "viewData";
constexpr const char *VIEW_DATA_PAGE_RECT = "pageRect";
constexpr const char *PAGE_INFO_ID = "id";
constexpr const char *PAGE_INFO_DEPTH = "depth";
constexpr const char *PAGE_INFO_AUTOFILLTYPE = "autoFillType";
constexpr const char *PAGE_INFO_TAG = "tag";
constexpr const char *PAGE_INFO_VALUE = "value";
constexpr const char *PAGE_INFO_PLACEHOLDER = "placeholder";
constexpr const char *PAGE_INFO_META_DATA = "metadata";
constexpr const char *PAGE_INFO_PASSWORDRULES = "passwordRules";
constexpr const char *PAGE_INFO_ENABLEAUTOFILL = "enableAutoFill";
constexpr const char *PAGE_INFO_IS_FOCUS = "isFocus";
constexpr const char *PAGE_INFO_PAGE_NODE_RECT = "rect";
constexpr const char *RECT_POSITION_LEFT = "left";
constexpr const char *RECT_POSITION_TOP = "top";
constexpr const char *RECT_WIDTH = "width";
constexpr const char *RECT_HEIGHT = "height";
constexpr uint32_t PAGE_NODE_COUNT_MAX = 100;
} // namespace

void EtsAutoFillExtensionUtil::UnwrapViewData(ani_env *env, const ani_object object, AbilityBase::ViewData &viewData)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    ani_ref etsViewData = nullptr;
    if (!AppExecFwk::GetFieldRefByName(env, object, VIEW_DATA_VIEW_DATA, etsViewData)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "GetFieldRefByName failed");
        return;
    }
    if (etsViewData == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null etsViewData");
        return;
    }
    UnwrapViewDataString(env, static_cast<ani_object>(etsViewData), viewData);
    UnwrapViewDataBoolean(env, static_cast<ani_object>(etsViewData), viewData);
    ani_ref etsPageNodeInfos = nullptr;
    if (!AppExecFwk::GetFieldRefByName(env, static_cast<ani_object>(etsViewData), VIEW_DATA_PAGE_NODE_INFOS,
        etsPageNodeInfos)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "GetFieldRefByName failed");
        return;
    }
    if (etsPageNodeInfos != nullptr) {
        ani_status status = ANI_ERROR;
        ani_double etsProCount = 0.0;
        if ((status = env->Object_GetPropertyByName_Double(static_cast<ani_object>(etsPageNodeInfos), "length",
            &etsProCount)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Object_GetPropertyByName_Double failed, status: %{public}d", status);
            return;
        }
        for (uint32_t index = 0; index < static_cast<uint32_t>(etsProCount) && index < PAGE_NODE_COUNT_MAX; index++) {
            ani_ref etsNode = nullptr;
            if ((status = env->Object_CallMethodByName_Ref(static_cast<ani_object>(etsPageNodeInfos), "$_get",
                "I:Lstd/core/Object;", &etsNode, (ani_int)index)) != ANI_OK) {
                TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Object_CallMethodByName_Ref failed, status: %{public}d", status);
                return;
            }
            AbilityBase::PageNodeInfo node;
            UnwrapPageNodeInfo(env, static_cast<ani_object>(etsNode), node);
            viewData.nodes.emplace_back(node);
        }
    }
    ani_ref etsPageRect = nullptr;
    if (!AppExecFwk::GetFieldRefByName(env, static_cast<ani_object>(etsViewData), VIEW_DATA_PAGE_RECT, etsPageRect)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "GetFieldRefByName failed");
        return;
    }
    if (etsPageRect != nullptr) {
        UnwrapRectData(env, static_cast<ani_object>(etsPageRect), viewData.pageRect);
    }
}

void EtsAutoFillExtensionUtil::UnwrapPageNodeInfo(ani_env *env, const ani_object object,
    AbilityBase::PageNodeInfo &node)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    ani_int id = 0;
    if (AppExecFwk::GetFieldIntByName(env, object, PAGE_INFO_ID, id)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "id:%{public}d", id);
        node.id = id;
    }
    ani_int depth = 0;
    if (AppExecFwk::GetFieldIntByName(env, object, PAGE_INFO_DEPTH, depth)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "id:%{public}d", depth);
        node.depth = depth;
    }
    ani_double autoFillType = 0.0;
    if (AppExecFwk::GetFieldDoubleByName(env, object, PAGE_INFO_AUTOFILLTYPE, autoFillType)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "id:%{public}f", autoFillType);
        node.autoFillType = static_cast<AbilityBase::AutoFillType>(autoFillType);
    }
    UnwrapPageNodeInfoString(env, object, node);
    bool enableAutoFill = false;
    if (AppExecFwk::GetFieldBoolByName(env, object, PAGE_INFO_ENABLEAUTOFILL, enableAutoFill)) {
        node.enableAutoFill = enableAutoFill;
    }
    ani_ref etsRect = nullptr;
    if (!AppExecFwk::GetFieldRefByName(env, object, PAGE_INFO_PAGE_NODE_RECT, etsRect)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "GetFieldRefByName failed");
        return;
    }
    if (etsRect != nullptr) {
        UnwrapRectData(env, static_cast<ani_object>(etsRect), node.rect);
    }
    bool isFocus = false;
    if (AppExecFwk::GetFieldBoolByName(env, object, PAGE_INFO_IS_FOCUS, isFocus)) {
        node.isFocus = isFocus;
    }
}

void EtsAutoFillExtensionUtil::UnwrapRectData(ani_env *env, const ani_object object, AbilityBase::Rect &rect)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    ani_double position = 0.0;
    if (AppExecFwk::GetFieldDoubleByName(env, object, RECT_POSITION_LEFT, position)) {
        rect.left = position;
    }
    if (AppExecFwk::GetFieldDoubleByName(env, object, RECT_POSITION_TOP, position)) {
        rect.top = position;
    }
    if (AppExecFwk::GetFieldDoubleByName(env, object, RECT_WIDTH, position)) {
        rect.width = position;
    }
    if (AppExecFwk::GetFieldDoubleByName(env, object, RECT_HEIGHT, position)) {
        rect.height = position;
    }
}

void EtsAutoFillExtensionUtil::UnwrapFillResponse(ani_env *env, const ani_object object, FillResponse &response)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    UnwrapViewData(env, object, response.viewData);
}

void EtsAutoFillExtensionUtil::UnwrapPopupSize(ani_env *env, const ani_object object, PopupSize &popupSize)
{
    ani_double width = 0.0;
    if (AppExecFwk::GetFieldDoubleByName(env, object, RECT_WIDTH, width)) {
        popupSize.width = width;
    }
    ani_double height = 0.0;
    if (AppExecFwk::GetFieldDoubleByName(env, object, RECT_HEIGHT, height)) {
        popupSize.height = height;
    }
}

void EtsAutoFillExtensionUtil::UnwrapViewDataString(ani_env *env, const ani_object object,
    AbilityBase::ViewData &viewData)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    std::string bundleName = "";
    if (AppExecFwk::GetFieldStringByName(env, object, VIEW_DATA_BUNDLE_NAME, bundleName)) {
        viewData.bundleName = bundleName;
    }
    std::string moduleName = "";
    if (AppExecFwk::GetFieldStringByName(env, object, VIEW_DATA_MODULE_NAME, moduleName)) {
        viewData.moduleName = moduleName;
    }
    std::string abilityName = "";
    if (AppExecFwk::GetFieldStringByName(env, object, VIEW_DATA_ABILITY_NAME, abilityName)) {
        viewData.abilityName = abilityName;
    }
    std::string pageUrl = "";
    if (AppExecFwk::GetFieldStringByName(env, object, VIEW_DATA_PAGEURL, pageUrl)) {
        viewData.pageUrl = pageUrl;
    }
}

void EtsAutoFillExtensionUtil::UnwrapViewDataBoolean(ani_env *env, const ani_object object,
    AbilityBase::ViewData &viewData)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    bool isUserSelected = false;
    if (AppExecFwk::GetFieldBoolByName(env, object, VIEW_DATA_USER_SELECTED, isUserSelected)) {
        viewData.isUserSelected = isUserSelected;
    }
    bool isOtherAccount = false;
    if (AppExecFwk::GetFieldBoolByName(env, object, VIEW_DATA_OTHER_ACCOUNT, isOtherAccount)) {
        viewData.isOtherAccount = isOtherAccount;
    }
}

void EtsAutoFillExtensionUtil::UnwrapPageNodeInfoString(ani_env *env, const ani_object object,
    AbilityBase::PageNodeInfo &node)
{
    std::string tag = "";
    if (AppExecFwk::GetFieldStringByName(env, object, PAGE_INFO_TAG, tag)) {
        node.tag = tag;
    }
    std::string value = "";
    if (AppExecFwk::GetFieldStringByName(env, object, PAGE_INFO_VALUE, value)) {
        node.value = value;
    }
    std::string passwordRules = "";
    if (AppExecFwk::GetFieldStringByName(env, object, PAGE_INFO_PASSWORDRULES, passwordRules)) {
        node.passwordRules = passwordRules;
    }
    std::string placeholder = "";
    if (AppExecFwk::GetFieldStringByName(env, object, PAGE_INFO_PLACEHOLDER, placeholder)) {
        node.placeholder = placeholder;
    }
    std::string metadata = "";
    if (AppExecFwk::GetFieldStringByName(env, object, PAGE_INFO_META_DATA, metadata)) {
        node.metadata = metadata;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS