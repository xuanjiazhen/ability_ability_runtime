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
#include "ani_common_want.h"
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
    ani_object etsViewData = AppExecFwk::GetPropertyValueByName(env, object, VIEW_DATA_VIEW_DATA);
    if (etsViewData == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null etsViewData");
        return;
    }
    UnwrapViewDataString(env, etsViewData, viewData);
    ani_boolean isUserSelected = false;
    if (AppExecFwk::IsExistsProperty(env, etsViewData, VIEW_DATA_USER_SELECTED)) {
        isUserSelected = AppExecFwk::GetBoolOrUndefined(env, etsViewData, VIEW_DATA_USER_SELECTED);
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "isUserSelected:%{public}hhu", isUserSelected);
        viewData.isUserSelected = isUserSelected;
    }
    ani_boolean isOtherAccount = false;
    if (AppExecFwk::IsExistsProperty(env, etsViewData, VIEW_DATA_OTHER_ACCOUNT)) {
        isOtherAccount = AppExecFwk::GetBoolOrUndefined(env, etsViewData, VIEW_DATA_OTHER_ACCOUNT);
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "isOtherAccount:%{public}hhu", isOtherAccount);
        viewData.isOtherAccount = isOtherAccount;
    }
    ani_object etsPageNodeInfos = AppExecFwk::GetPropertyValueByName(env, etsViewData, VIEW_DATA_PAGE_NODE_INFOS);
    if (etsPageNodeInfos != nullptr) {
        ani_double etsProCount = 0.0;
        if (!AppExecFwk::GetDoublePropertyValue(env, etsViewData, VIEW_DATA_PAGE_NODE_INFOS, etsProCount)) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "get length fail");
            return;
        }
        for (uint32_t index = 0; index < static_cast<uint32_t>(etsProCount) && index < PAGE_NODE_COUNT_MAX; index++) {
            ani_ref etsNode = nullptr;
            if (ANI_OK != (env->Object_CallMethodByName_Ref(etsViewData, "$_get", "I:Lstd/core/Object;", &etsNode,
                (ani_int)index))) {
                TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Object_CallMethodByName_Ref _get Failed");
                return;
            }
            AbilityBase::PageNodeInfo node;
            UnwrapPageNodeInfo(env, static_cast<ani_object>(etsNode), node);
            viewData.nodes.emplace_back(node);
        }
    }
    ani_object etsPageRect = AppExecFwk::GetPropertyValueByName(env, etsViewData, VIEW_DATA_PAGE_RECT);
    if (etsPageRect != nullptr) {
        UnwrapRectData(env, etsPageRect, viewData.pageRect);
    }
}

void EtsAutoFillExtensionUtil::UnwrapPageNodeInfo(ani_env *env, const ani_object object,
    AbilityBase::PageNodeInfo &node)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    ani_double id = 0.0;
    if (AppExecFwk::GetDoubleOrUndefined(env, object, PAGE_INFO_ID, id)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "id:%{public}f", id);
        node.id = id;
    }
    ani_double depth = 0.0;
    if (AppExecFwk::GetDoubleOrUndefined(env, object, PAGE_INFO_DEPTH, depth)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "depth:%{public}f", depth);
        node.depth = depth;
    }
    ani_double autoFillType = 0.0;
    if (AppExecFwk::GetDoubleOrUndefined(env, object, PAGE_INFO_AUTOFILLTYPE, autoFillType)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "autoFillType:%{public}f", autoFillType);
        node.autoFillType = static_cast<AbilityBase::AutoFillType>(autoFillType);
    }
    UnwrapPageNodeInfoString(env, object, node);
    ani_boolean enableAutoFill = false;
    if (AppExecFwk::IsExistsProperty(env, object, PAGE_INFO_ENABLEAUTOFILL)) {
        enableAutoFill = AppExecFwk::GetBoolOrUndefined(env, object, PAGE_INFO_ENABLEAUTOFILL);
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "enableAutoFill:%{public}hhu", enableAutoFill);
        node.enableAutoFill = enableAutoFill;
    }
    ani_object etsRect = AppExecFwk::GetPropertyValueByName(env, object, PAGE_INFO_PAGE_NODE_RECT);
    if (etsRect != nullptr) {
        UnwrapRectData(env, etsRect, node.rect);
    }
    ani_boolean isFocus = false;
    if (AppExecFwk::IsExistsProperty(env, object, PAGE_INFO_IS_FOCUS)) {
        isFocus = AppExecFwk::GetBoolOrUndefined(env, object, PAGE_INFO_IS_FOCUS);
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "isFocus:%{public}hhu", isFocus);
        node.isFocus = isFocus;
    }
}

void EtsAutoFillExtensionUtil::UnwrapRectData(ani_env *env, const ani_object object, AbilityBase::Rect &rect)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    ani_double position = 0.0;
    if (AppExecFwk::GetDoubleOrUndefined(env, object, RECT_POSITION_LEFT, position)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "left:%{public}f", position);
        rect.left = position;
    }
    if (AppExecFwk::GetDoubleOrUndefined(env, object, RECT_POSITION_TOP, position)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "top:%{public}f", position);
        rect.top = position;
    }
    if (AppExecFwk::GetDoubleOrUndefined(env, object, RECT_WIDTH, position)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "width:%{public}f", position);
        rect.width = position;
    }
    if (AppExecFwk::GetDoubleOrUndefined(env, object, RECT_HEIGHT, position)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "height:%{public}f", position);
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
    if (AppExecFwk::GetDoubleOrUndefined(env, object, RECT_WIDTH, width)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "width:%{public}f", width);
        popupSize.width = width;
    }
    ani_double height = 0.0;
    if (AppExecFwk::GetDoubleOrUndefined(env, object, RECT_HEIGHT, height)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "height:%{public}f", height);
        popupSize.height = height;
    }
}

void EtsAutoFillExtensionUtil::UnwrapViewDataString(ani_env *env, const ani_object object,
    AbilityBase::ViewData &viewData)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    std::string bundleName = "";
    if (AppExecFwk::GetStringOrUndefined(env, object, VIEW_DATA_BUNDLE_NAME, bundleName)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "The parsed bundleName part %{public}s", bundleName.c_str());
        viewData.bundleName = bundleName;
    }
    std::string moduleName = "";
    if (AppExecFwk::GetStringOrUndefined(env, object, VIEW_DATA_MODULE_NAME, moduleName)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "The parsed moduleName part %{public}s", moduleName.c_str());
        viewData.moduleName = moduleName;
    }
    std::string abilityName = "";
    if (AppExecFwk::GetStringOrUndefined(env, object, VIEW_DATA_ABILITY_NAME, abilityName)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "The parsed abilityName part %{public}s", abilityName.c_str());
        viewData.abilityName = abilityName;
    }
    std::string pageUrl = "";
    if (AppExecFwk::GetStringOrUndefined(env, object, VIEW_DATA_PAGEURL, pageUrl)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "The parsed pageUrl part %{public}s", pageUrl.c_str());
        viewData.pageUrl = pageUrl;
    }
}

void EtsAutoFillExtensionUtil::UnwrapPageNodeInfoString(ani_env *env, const ani_object object,
    AbilityBase::PageNodeInfo &node)
{
    std::string tag = "";
    if (AppExecFwk::GetStringOrUndefined(env, object, PAGE_INFO_TAG, tag)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "The parsed tag part %{public}s", tag.c_str());
        node.tag = tag;
    }
    std::string value = "";
    if (AppExecFwk::GetStringOrUndefined(env, object, PAGE_INFO_VALUE, value)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "The parsed value part %{public}s", value.c_str());
        node.value = value;
    }
    std::string passwordRules = "";
    if (AppExecFwk::GetStringOrUndefined(env, object, PAGE_INFO_PASSWORDRULES, passwordRules)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "The parsed passwordRules part %{public}s", passwordRules.c_str());
        node.passwordRules = passwordRules;
    }
    std::string placeholder = "";
    if (AppExecFwk::GetStringOrUndefined(env, object, PAGE_INFO_PLACEHOLDER, placeholder)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "The parsed placeholder part %{public}s", placeholder.c_str());
        node.placeholder = placeholder;
    }
    std::string metadata = "";
    if (AppExecFwk::GetStringOrUndefined(env, object, PAGE_INFO_META_DATA, metadata)) {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "The parsed metadata part %{public}s", metadata.c_str());
        node.metadata = metadata;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS