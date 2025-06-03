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

import { AutoFillType } from 'application.AutoFillType';
import type AutoFillRect from 'application.AutoFillRect';

export default interface PageNodeInfo {
  id: number;
  depth: number;
  autoFillType: AutoFillType;
  tag: string;
  value: string;
  placeholder?: string;
  passwordRules?: string;
  enableAutoFill: boolean;
  rect: AutoFillRect;
  isFocus: boolean;
  metadata?: string;
}

class PageNodeInfoImpl implements PageNodeInfo {
  public id: number = 0;
  public depth: number = 0;
  public autoFillType: AutoFillType = AutoFillType.UNSPECIFIED;
  public tag: string = '';
  public value: string = '';
  public placeholder?: string;
  public passwordRules?: string;
  public enableAutoFill: boolean = false;
  public rect: AutoFillRect = {};
  public isFocus: boolean = false;
  public metadata?: string;
}