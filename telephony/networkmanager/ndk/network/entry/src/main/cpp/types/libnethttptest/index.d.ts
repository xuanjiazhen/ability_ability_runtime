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
export const Http_CreateHeaders: () => number;
export const Http_SetHeaderValue: (a: number, b: string) => number;
export const Http_GetHeaderValue: (a: number) => number;
export const Http_GetHeaderEntries: (a: number) => number;
export const Http_DestroyHeaderEntries: (a: number) => number;
export const Http_CreateRequest: (a: string) => number;
export const Http_RequestRequest: (a: number, b: string) => number;
export const Http_Destroy: (a : string) => number;
