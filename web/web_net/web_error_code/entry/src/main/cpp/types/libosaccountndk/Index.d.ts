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
// database/data/data_asset.h
export const webErrCodeSuccess: () => number;
export const getURISuccess: () => number;
export const getPathSuccess: () => number;
export const getCreateTimeSuccess: () => number;
export const getModifyTimeSuccess: () => number;
export const getSizeSuccess: () => number;
export const getStatusSuccess: () => number;
// database/data/oh_data_value.h
export const destroy: () => number;
export const putNull: () => number;
export const putReal: () => number;
export const putText: () => number;
export const putBlob: () => number;
export const putAsset: () => number;
export const putAssets: () => number;
export const putUnlimitedInt: () => number;
export const getType: () => number;
export const isNull: () => number;
export const testGetIntInvalidParam: () => number;
export const testGetIntTypeMismatch: () => number;
export const testGetIntDataTypeNull: () => number;
export const testGetRealInvalidParam: () => number;
export const testGetTextInvalidParam: () => number;
export const testGetTextTypeMismatch: () => number;
export const testGetTextDataTypeNull: () => number;
export const testGetBlobInvalidParam: () => number;
export const testGetBlobTypeMismatch: () => number;
export const testGetBlobDataTypeNull: () => number;
export const testGetAssetInvalidParam: () => number;
export const testGetAssetTypeMismatch: () => number;
export const testGetAssetDataTypeNull: () => number;
export const testGetAssetsCountInvalidParam: () => number;
export const testGetAssetsCountTypeMismatch: () => number;
export const testGetAssetsCountDataTypeNull: () => number;
export const testGetAssetsInvalidParam: () => number;
export const testGetAssetsTypeMismatch: () => number;
export const testGetAssetsDataTypeNull: () => number;
export const testGetFloatVectorCountInvalidParam: () => number;
export const testGetFloatVectorCountTypeMismatch: () => number;
export const testGetFloatVectorCountTypeNull: () => number;

export const testGetFloatVectorInvalidParam: () => number;
export const testGetFloatVectorTypeMismatch: () => number;
export const testGetFloatVectorTypeNull: () => number;

export const testGetUnlimitedIntBandInvalidParam: () => number;
export const testGetUnlimitedIntBandTypeMismatch: () => number;
export const testGetUnlimitedIntBandTypeNull: () => number;




















