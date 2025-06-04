/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

import { AutoFillType } from 'application.AutoFillType';
import type CustomData from 'application.CustomData';
import type AutoFillPopupConfig from 'application.AutoFillPopupConfig';
import type ViewData from 'application.ViewData';

export interface FillRequest {
  type: AutoFillType;
  viewData: ViewData;
  customData: CustomData;
  isPopup: boolean;
}
export interface SaveRequest {
  viewData: ViewData;
}
export interface UpdateRequest {
  viewData: ViewData;
}
export interface FillResponse {
  viewData: ViewData;
}
export interface FillRequestCallback {
  onSuccess(response: FillResponse): void;
  onFailure(): void;
  onCancel(fillContent?: string): void;
  setAutoFillPopupConfig(autoFillPopupConfig: AutoFillPopupConfig): void;
}
export interface SaveRequestCallback {
  onSuccess(): void;
  onFailure(): void;
}

class FillRequestInner implements FillRequest {
  public type: AutoFillType = AutoFillType.UNSPECIFIED;
  public viewData: ViewData = {};
  public customData: CustomData = {};
  public isPopup: boolean = false;
}
class SaveRequestInner implements SaveRequest {
  public viewData: ViewData = {};
}
class UpdateRequestInner implements UpdateRequest {
  public viewData: ViewData = {};
}
class FillResponseInner implements FillResponse {
  public viewData: ViewData = {};
}
class FillRequestCallbackInner implements FillRequestCallback {
  public native onSuccess(response: FillResponse): void;
  public native onFailure(): void;
  public native onCancel(fillContent?: string): void;
  public native setAutoFillPopupConfig(autoFillPopupConfig: AutoFillPopupConfig): void;
}
class SaveRequestCallbackInner implements SaveRequestCallback {
  public native onSuccess(): void;
  public native onFailure(): void;
}