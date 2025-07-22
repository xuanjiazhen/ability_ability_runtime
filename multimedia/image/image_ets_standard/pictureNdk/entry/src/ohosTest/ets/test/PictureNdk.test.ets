/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import { describe, beforeAll, afterEach, it, expect, Level } from '@ohos/hypium';
import ndkTest from 'libPictureNdk.so';
import image from "@ohos.multimedia.image";
import fs from '@ohos.file.fs';
import hilog from '@ohos.hilog';
import { BusinessError } from '@kit.BasicServicesKit';

import display from '@ohos.display';
import hdrCapability from '@ohos.graphics.hdrCapability';

interface picTypeValue {
  name:string
  fileName:string
  value?:string
  initValue?:string
  modifyData?:string
  compareData?:string
  errorData?: string
}

const domain = 0x0000;
const testTag = 'pictureNdkTest';
const testFileJpg = 'JPEGISOSingle.jpg';
const testFileHeic = 'HEIFISOMultiChannelBaseColor0512V12.heic';
const testNoAuxiliaryPictureJpg = 'test.jpg';
const testFileWithFiveAuxiliaryPictureJpg = 'AllAuxiliaryPictures_exclude_fragment.jpg';
const testFileWithFiveAuxiliaryPictureHeic = 'all_auxtype.heic';
const testNoAuxiliaryPictureHeic = '0801_sdr2.heic';
const testFileUnrefocusHeic = "IMG_825866834_004.heic";
const testFileFragmentHeic = 'HeicFragmentMap.heic';
const testFileFragmentJpg = 'JpegFragmentMap.jpg';
const testFilePng = "test.png";
const testFileAllAuxiliaryPictures = "AllAuxiliaryPictures.jpg";

const testFileGainMap = 'GainMap.dat';
const testFileUnrefocus = 'UnrefocusMap.dat';
const testFileDepth = 'DepthMap.dat';
const testFileLinear = 'LinearMap.dat';
const testFileFragment = 'FragmentMap.dat';
const testFileGainMapSize: image.Size = { height: 960, width: 720 };
const testFileUnrefocusSize: image.Size = { height: 4096, width: 3072 };
const testFileDepthSize: image.Size = { height: 4096, width: 3072 };
const testFileLinearSize: image.Size = { height: 4096, width: 3072 };
const testFileFragmentSize: image.Size = { height: 4096, width: 3072 };

const imageFormatPng = 'image/png';
const imageFormatJpeg = 'image/jpeg';
const imageFormatHeif = 'image/heif';

const imageWidthKey = 'ImageWidth';
const imageLengthKey = 'ImageLength';
const imageWidthValue = '20';
const imageHeightValue = "512";
const imageLengthValue = '15';
const fragmentImageWidthKey = 'FragmentImageWidth';
const fragmentImageHeightKey = 'FragmentImageHeight';
const fragmentImageWidthValue = '512';
const fragmentImageHeightValue = '512';
const xInOriginalKey = "XInOriginal";
const yInOriginalKey = "YInOriginal";
const xInOriginalValue = '0';
const yInOriginalValue = '0';
const orientationKey = 'Orientation';
const orientationValue = '1';

const imageWidthKeyErr ="ImageWidthErr";
const imageHeightKeyErr ="ImageHeightErr";
const imageLengthKeyErr = 'ImageLengthErr';
const imageWidthValueErr = "-1";
const imageHeightValueErr = "-1";
const fragmentImageWidthKeyErr = "FragmentImageWidthErr";
const fragmentImageHeightKeyErr = "FragmentImageHeightErr";
const fragmentImageWidthValueErr = "-1";
const fragmentImageHeightValueErr = "-1";
const xInOriginalKeyErr = "XInOriginalErr";
const yInOriginalKeyErr = "YInOriginalErr";
const xInOriginalValueErr = "-1";
const yInOriginalValueErr = "-1";
const orientationKeyErr = "OrientationErr";
const orientationValueErr = "-1";

const testAllAuxiliaryPicturesExcludeFragmentJpg = "AllAuxiliaryPictures_exclude_fragment.jpg";

let isSupportHEIFDecode: boolean;
let fileDir: string | undefined;

enum ErrorCode {
  /** operation success */
  IMAGE_SUCCESS = 0,
  /** invalid parameter */
  IMAGE_BAD_PARAMETER = 401,
  /** unsupported metadata */
  IMAGE_UNSUPPORTED_METADATA = 7600202,
  /** encode failed */
  IMAGE_ENCODE_FAILED = 7800301,
  /** decode failed */
  IMAGE_DECODE_FAILED = 7700301,
};

interface ExifInfo {
  key: string,
  value: string
}

const metadataDefaultAllKey: Array<ExifInfo> = [
  {key: "ApertureValue", value: "2.00 EV (f/2.0)"},
  {key: "BitsPerSample", value: "8, 8, 8"},
  {key: "BrightnessValue", value: "-2.00 EV (0.86 cd/m^2)"},
  {key: "ColorSpace", value: "Uncalibrated"},
  {key: "ComponentsConfiguration", value: "Y Cb Cr -"},
  {key: "Contrast", value: "Normal"},
  {key: "CustomRendered", value: "Custom process"},
  {key: "DateTimeDigitized", value: "2024:01:11 09:39:58"},
  {key: "DateTimeOriginal", value: "2024:01:11 09:39:58"},
  {key: "ExposureBiasValue", value: "0.00 EV"},
  {key: "ExposureMode", value: "Auto exposure"},
  {key: "ExposureProgram", value: "Normal program"},
  {key: "ExposureTime", value: "1/590 sec."},
  {key: "FNumber", value: "f/2.0"},
  {key: "FileSource", value: "DSC"},
  {key: "Flash", value: "Flash did not fire"},
  {key: "FlashpixVersion", value: "FlashPix Version 1.0"},
  {key: "FocalLength", value: "6.3 mm"},
  {key: "FocalLengthIn35mmFilm", value: "27"},
  {key: "GPSAltitude", value: "0.00"},
  {key: "GPSAltitudeRef", value: "Sea level reference"},
  {key: "GPSDateStamp", value: "2024:01:11"},
  {key: "GPSTimeStamp", value: "01:39:58.00"},
  {key: "GPSVersionID", value: "2.2.0.0"},
  {key: "GainControl", value: "Normal"},
  {key: "HwMnoteBurstNumber", value: "default_exif_value"},
  {key: "HwMnoteCaptureMode", value: "0"},
  {key: "HwMnoteCloudEnhancementMode", value: "default_exif_value"},
  {key: "HwMnoteFaceConf", value: "default_exif_value"},
  {key: "HwMnoteFaceCount", value: "default_exif_value"},
  {key: "HwMnoteFaceLeyeCenter", value: "default_exif_value"},
  {key: "HwMnoteFaceMouthCenter", value: "default_exif_value"},
  {key: "HwMnoteFacePointer", value: "188"},
  {key: "HwMnoteFaceRect", value: "default_exif_value"},
  {key: "HwMnoteFaceReyeCenter", value: "default_exif_value"},
  {key: "HwMnoteFaceSmileScore", value: "default_exif_value"},
  {key: "HwMnoteFaceVersion", value: "1207959808"},
  {key: "HwMnoteFocusMode", value: "default_exif_value"},
  {key: "HwMnoteFrontCamera", value: "0"},
  {key: "HwMnoteIsXmageSupported", value: "1"},
  {key: "HwMnotePhysicalAperture", value: "1"},
  {key: "HwMnotePitchAngle", value: "-83"},
  {key: "HwMnoteRollAngle", value: "26"},
  {key: "HwMnoteSceneBeachConf", value: "default_exif_value"},
  {key: "HwMnoteSceneBlueSkyConf", value: "default_exif_value"},
  {key: "HwMnoteSceneFlowersConf", value: "default_exif_value"},
  {key: "HwMnoteSceneFoodConf", value: "default_exif_value"},
  {key: "HwMnoteSceneGreenPlantConf", value: "default_exif_value"},
  {key: "HwMnoteSceneNightConf", value: "default_exif_value"},
  {key: "HwMnoteScenePointer", value: "170"},
  {key: "HwMnoteSceneSnowConf", value: "default_exif_value"},
  {key: "HwMnoteSceneStageConf", value: "default_exif_value"},
  {key: "HwMnoteSceneSunsetConf", value: "default_exif_value"},
  {key: "HwMnoteSceneTextConf", value: "default_exif_value"},
  {key: "HwMnoteSceneVersion", value: "1207959808"},
  {key: "HwMnoteWindSnapshotMode", value: "default_exif_value"},
  {key: "HwMnoteXmageBottom", value: "0"},
  {key: "HwMnoteXmageLeft", value: "0"},
  {key: "HwMnoteXmageMode", value: "0"},
  {key: "HwMnoteXmageRight", value: "0"},
  {key: "HwMnoteXmageTop", value: "0"},
  {key: "ISOSpeedRatings", value: "160"},
  {key: "ImageDescription", value: "_cuva"},
  {key: "ImageLength", value: "4000"},
  {key: "ImageWidth", value: "3000"},
  {key: "LightSource", value: "Daylight"},
  {key: "MakerNote", value: "HwMnoteScenePointer:170,HwMnoteSceneVersion:1207959808,HwMnoteFacePointer:188,HwMnoteFaceVersion:1207959808,HwMnoteCaptureMode:0,HwMnoteFrontCamera:0,HwMnoteRollAngle:26,HwMnotePitchAngle:-83,HwMnotePhysicalAperture:1,HwMnoteIsXmageSupported:1,HwMnoteXmageMode:0,HwMnoteXmageLeft:0,HwMnoteXmageTop:0,HwMnoteXmageRight:0,HwMnoteXmageBottom:0"},
  {key: "MeteringMode", value: "Pattern"},
  {key: "MicroVideoPresentationTimestampUS", value: "default_exif_value"},
  {key: "MovingPhotoId", value: "default_exif_value"},
  {key: "MovingPhotoVersion", value: "default_exif_value"},
  {key: "Orientation", value: "Unknown value 0"},
  {key: "PhotographicSensitivity", value: "160"},
  {key: "PixelXDimension", value: "4000"},
  {key: "PixelYDimension", value: "3000"},
  {key: "ResolutionUnit", value: "Inch"},
  {key: "Saturation", value: "Normal"},
  {key: "SceneCaptureType", value: "Standard"},
  {key: "SceneType", value: "Directly photographed"},
  {key: "SensingMethod", value: "One-chip color area sensor"},
  {key: "Sharpness", value: "Normal"},
  {key: "ShutterSpeedValue", value: "29.00 EV (1/536870912 sec.)"},
  {key: "Software", value: "MNA-AL00 4.0.0.120(C00E116R3P7)"},
  {key: "SubjectDistanceRange", value: "Unknown"},
  {key: "SubsecTime", value: "427000"},
  {key: "SubsecTimeDigitized", value: "427000"},
  {key: "SubsecTimeOriginal", value: "427000"},
  {key: "WhiteBalance", value: "Auto white balance"},
  {key: "XResolution", value: "72"},
  {key: "YCbCrPositioning", value: "Centered"},
  {key: "YResolution", value: "72"},
]

const getBuffer = (fileName: string) => {
  let filePath = getFilePath(fileName);
  let file = fs.openSync(filePath);
  const stats = fs.statSync(filePath);
  const fileSize = stats.size;
  const bufferRead = new ArrayBuffer(fileSize)
  fs.readSync(file.fd, bufferRead)
  return bufferRead
}

const testCreateImageSource = (caseName: string, fileName: string) => {
  let filePath = getFilePath(fileName);
  hilog.info(domain, testTag, caseName + `: image filePath ` + filePath);
  ndkTest.createTestImageSource(filePath);
}

const testCreatePictureByPixelMap = (caseName: string, fileName: string) => {
  testCreateImageSource(caseName, fileName)
  ndkTest.createTestPixelmapByImageSource(image.PixelMapFormat.RGBA_8888);
  let result: number = ndkTest.CreatePictureByPixelmap();
  expect(result).assertEqual(ErrorCode.IMAGE_SUCCESS);
}

const newCreatePictureByPixelMap = (caseName: string, fileName: string, done: Function) => {
  if((!fileName.toLowerCase().endsWith('.heic')) || (fileName.toLowerCase().endsWith('.heic') && isSupportHEIFDecode)){
    testCreateImageSource(caseName, fileName)
    ndkTest.createTestPixelmapByImageSource(image.PixelMapFormat.RGBA_8888);
    let result: number = ndkTest.CreatePictureByPixelmap();
    expect(result).assertEqual(ErrorCode.IMAGE_SUCCESS);
  }else{
    expect(true).assertTrue()
    done()
  }
}

const testCreatePictureByImageSource = (caseName: string, fileName: string, 
  inAuxTpyes: Array<image.AuxiliaryPictureType>) => {
  testCreateImageSource(caseName, fileName)
  ndkTest.CreatePictureDecodingOptions();
  ndkTest.SetDesiredAuxiliaryPictures(inAuxTpyes.length, inAuxTpyes);
  ndkTest.CreatePictureByImageSource();
}

const testCreateAuxiliaryPicture = (auxType: image.AuxiliaryPictureType): image.AuxiliaryPicture => {
  let fileName = '';
  let auxSize: image.Size = {height: 180, width: 240};
  let auxBuffer = createFillBuffer(172800, 4);
  switch (auxType) {
    case image.AuxiliaryPictureType.GAINMAP: 
      fileName = testFileGainMap;
      auxSize = testFileGainMapSize;
      auxBuffer = getBuffer(fileName);
      break;
    default: 
      break;
  }
  let auxPicture: image.AuxiliaryPicture =
    ndkTest.CreateAuxiliaryPicture(auxBuffer, auxSize, auxType);
  return auxPicture;
}

const testSetAuxiliaryPicture = (done: Function, auxType: image.AuxiliaryPictureType, caseName: string) => {
  hilog.info(domain, testTag, caseName + ` begin`);
  try {
    testCreatePictureByPixelMap(caseName, testFileJpg);
    let auxPicture: image.AuxiliaryPicture = testCreateAuxiliaryPicture(auxType);
    
    expect(ndkTest.SetAuxiliaryPicture(auxType, auxPicture)).assertEqual(ErrorCode.IMAGE_SUCCESS);
    expect(ndkTest.GetAuxiliaryPicture(auxType) != undefined).assertTrue();
    done();
  } catch (err) {
    hilog.error(domain, testTag, caseName + `: Test exception ` + err);
    expect().assertFalse();
    done();
  }
}

const testCasesOfCreateAuxiliaryPicture = (done: Function, auxType: image.AuxiliaryPictureType,caseName: string) => {
  hilog.info(domain, testTag, caseName + ` begin`);
  try {
    testCreateAuxiliaryPicture(auxType);

    expect(ndkTest.AuxiliaryPictureGetType()).assertEqual(auxType);
    done();
  } catch (err) {
    hilog.error(domain, testTag, caseName + `: Test exception ` + err);
    expect().assertFalse();
    done();
  }
}

const testAuxiliaryPictureReadWritePixels = (done: Function, fileName: string,
  auxType: image.AuxiliaryPictureType, caseName: string) => {
  hilog.info(domain, testTag, caseName + ` begin`);
  try {
    let inAuxTpyes: Array<image.AuxiliaryPictureType> = [auxType];
    testCreatePictureByImageSource(caseName, fileName, inAuxTpyes);

    let auxPicture: image.AuxiliaryPicture = ndkTest.GetAuxiliaryPicture(auxType);
    expect(auxPicture != undefined).assertTrue();

    let info: ndkTest.IAuxInfo = ndkTest.AuxiliaryPictureGetInfo();
    let writeBuffer = createFillBuffer(info.width * info.height * 4, 4);
    let readBuffer: ArrayBuffer = new ArrayBuffer(writeBuffer.byteLength);
    expect(ndkTest.AuxiliaryPictureWritePixels(writeBuffer)).assertEqual(ErrorCode.IMAGE_SUCCESS);
    expect(ndkTest.AuxiliaryPictureReadPixels(readBuffer)).assertEqual(ErrorCode.IMAGE_SUCCESS);
    expect(compareArrayBuffer(readBuffer, writeBuffer)).assertTrue();
    done();
  } catch (err) {
    hilog.error(domain, testTag, caseName + `: Test exception ` + err);
    expect().assertFalse();
    done();
  }
}

const testAuxiliaryPictureInfoSetTypeAndGetType = (done: Function, auxType: image.AuxiliaryPictureType,
  caseName: string) => {
  hilog.info(domain, testTag, caseName + ` begin`);
  try {
    ndkTest.CreateAuxiliaryPictureInfo();
    expect(ndkTest.AuxiliaryPictureInfoSetType(auxType)).assertEqual(ErrorCode.IMAGE_SUCCESS);

    expect(ndkTest.AuxiliaryPictureInfoGetType()).assertEqual(auxType);
    done();
  } catch (err) {
    hilog.error(domain, testTag, caseName + `: Test exception ` + err);
    expect().assertFalse();
    done();
  }
}

const testSetAndGetPixelFormatByCreateAuxInfo = (done: Function, pixelFormat: image.PixelMapFormat,
  caseName: string) => {
  hilog.info(domain, testTag, caseName + ` begin`);
  try {
    ndkTest.CreateAuxiliaryPictureInfo();
    expect(ndkTest.AuxiliaryPictureInfoSetPixelFormat(pixelFormat)).assertEqual(ErrorCode.IMAGE_SUCCESS);

    expect(ndkTest.AuxiliaryPictureInfoGetPixelFormat()).assertEqual(pixelFormat);
    done();
  } catch (err) {
    hilog.error(domain, testTag, caseName + `: Test exception ` + err);
    expect().assertFalse();
    done();
  }
}

const testSetAndGetPixelFormat = (done: Function, auxType: image.AuxiliaryPictureType, 
  pixelMapFormat: image.PixelMapFormat, caseName: string) => {
  hilog.info(domain, testTag, caseName +` begin`);
  try {
    testCreateAuxiliaryPicture(auxType);

    ndkTest.AuxiliaryPictureGetInfo();
    expect(ndkTest.AuxiliaryPictureInfoSetPixelFormat(pixelMapFormat)).assertEqual(ErrorCode.IMAGE_SUCCESS);
    if (pixelMapFormat == image.PixelMapFormat.RGBA_F16) {
      expect(ndkTest.AuxiliaryPictureSetInfoToCtx()).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
    } else {
      expect(ndkTest.AuxiliaryPictureSetInfoToCtx()).assertEqual(ErrorCode.IMAGE_SUCCESS);
      ndkTest.AuxiliaryPictureGetInfo();
      expect(ndkTest.AuxiliaryPictureInfoGetPixelFormat()).assertEqual(pixelMapFormat);
    }
    done();
  } catch (err) {
    hilog.error(domain, testTag, caseName + `: Test exception ` + err);
    expect().assertFalse();
    done();
  }
}

const testPixelMapFormatInPicture = (done: Function, fileName: string, 
  pixelMapFormat: image.PixelMapFormat, caseName: string) => {
  hilog.info(domain, testTag, caseName + ` begin`);
  try {
    testCreateImageSource(caseName, fileName)
    if(pixelMapFormat == image.PixelMapFormat.ALPHA_8) {
      expect(ndkTest.createTestPixelmapByImageSource(pixelMapFormat)).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
    } else {
      expect(ndkTest.createTestPixelmapByImageSource(pixelMapFormat)).assertEqual(ErrorCode.IMAGE_SUCCESS);
    }

    expect(ndkTest.CreatePictureByPixelmap()).assertEqual(ErrorCode.IMAGE_SUCCESS);
    expect(ndkTest.GetMainPixelmapInfo().pixelFormat).assertEqual(pixelMapFormat);
    done();
  } catch (err) {
    hilog.error(domain, testTag, caseName + `: Test exception ` + err);
    expect().assertFalse();
    done();
  }
}

function getFilePath(fileName: string): string {
  let filePath = AppStorage.get('pathDir') + "/" + fileName;
  return filePath;
}

function getCachePath(fileName: string): string {
  let filePath = AppStorage.get('cacheDir') + "/" + fileName;
  return filePath;
}

function compareArray(array1: Array<number>, array2: Array<number>): boolean {
  if (array1.length != array2.length) {
    return false;
  }

  for (let i = 0; i < array1.length; i++) {
    if (array1[i] != array2[i]) {
      return false;
    }
  }
  return true;
}

function compareArrayBuffer(buffer1: ArrayBuffer, buffer2: ArrayBuffer): boolean {
  if (buffer1.byteLength != buffer2.byteLength) {
    return false;
  }
  return ndkTest.compareArrayBuffer(buffer1, buffer2);
}

function compareArrayForLessTimes(buffer1: ArrayBuffer, buffer2: ArrayBuffer): boolean {
  if (buffer1.byteLength != buffer2.byteLength) {
    return false;
  }

  let view1 = new Uint8Array(buffer1);
  let view2 = new Uint8Array(buffer2);
  for (let i = 0; i < 100; i++) {
    if (view1[i] != view2[i]) {
      return false;
    }
  }
  return true;
}

function createDefaultExifMetadata(): image.Metadata {
  let metadata: image.Metadata = ndkTest.MetadataCreate(image.MetadataType.EXIF_METADATA);
  ndkTest.MetadataSetProperty(imageLengthKey, imageLengthValue);
  ndkTest.MetadataSetProperty(imageWidthKey, imageWidthValue);
  return metadata;
}

function createDefaultFragmentMetadata(): image.Metadata {
  let metadata: image.Metadata = ndkTest.MetadataCreate(image.MetadataType.FRAGMENT_METADATA);
  ndkTest.MetadataSetProperty(fragmentImageWidthKey, fragmentImageWidthValue);
  ndkTest.MetadataSetProperty(fragmentImageHeightKey, fragmentImageHeightValue);
  ndkTest.MetadataSetProperty(xInOriginalKey, xInOriginalValue);
  ndkTest.MetadataSetProperty(yInOriginalKey, yInOriginalValue);
  return metadata;
}

function createFillBuffer(size: number, value: number): ArrayBuffer {
  let buffer = new ArrayBuffer(size);
  let view = new Uint8Array(buffer);
  view.fill(value);
  return buffer;
}

async function sleep(delay: number): Promise<void> {
  return new Promise((resolve, _) => {
    setTimeout(async () => {
      resolve();
    }, delay);
  });
};

export default function pictureNdkTest() {
  describe('pictureNdkTest', () => {
    let isSupportHdr: boolean = false;
    fileDir = AppStorage.get('pathDir')
    beforeAll(async () => {
      isSupportHEIFDecode = 
        image.createImageSource(fileDir + '/' + 'test.jpg').supportedFormats.includes('image/heic')
      isSupportHdr = await fs.access('/system/lib64/ndk/libvideo_processing_capi_impl.so').then((res: boolean) => {
        if (res) {
          console.info("file exists");
          return true;
        } else {
          console.info("file not exists");
          return false;
        }
      }).catch((err: BusinessError) => {
        console.error("access failed with error message: " + err.message + ", error code: " + err.code);
        return false;
      });
    })

    afterEach(() => {
      try {
        ndkTest.releaseTestContext();
      } catch (err) {
        hilog.error(domain, testTag, `afterEach: Test exception ` + err);
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_CREATEPICTUREDECODINGOPTIONS_0100
     * @tc.name      : testImageSourceCreatePictureDecodingOptions
     * @tc.desc      : Test create default picture decoding options.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_CREATEPICTUREDECODINGOPTIONS_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_CREATEPICTUREDECODINGOPTIONS_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        let auxType: image.AuxiliaryPictureType = image.AuxiliaryPictureType.GAINMAP;
        testCreateAuxiliaryPicture(auxType);

        expect(ndkTest.AuxiliaryPictureGetType()).assertEqual(auxType);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_CREATEPICTURE_0100
     * @tc.name      : testImageSourceCreatePicture
     * @tc.desc      : Test create default picture by ImageSource.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_CREATEPICTURE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_CREATEPICTURE_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreateImageSource(caseName, testFileJpg);
        ndkTest.CreatePictureDecodingOptions();

        expect(ndkTest.CreatePictureByImageSource()).assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_SETDESIREDAUXILIARYPICTURES_0100
     * @tc.name      : testImageSourceSetDesiredAuxiliaryPictures
     * @tc.desc      : Test set desired auxiliary pictures to decoding options.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_SETDESIREDAUXILIARYPICTURES_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_SETDESIREDAUXILIARYPICTURES_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        let auxType: image.AuxiliaryPictureType = image.AuxiliaryPictureType.LINEAR_MAP;
        testCreateAuxiliaryPicture(auxType);
        
        expect(ndkTest.AuxiliaryPictureGetType()).assertEqual(auxType);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_GETDESIREDAUXILIARYPICTURES_0100
     * @tc.name      : testImageSourceGetDesiredAuxiliaryPictures
     * @tc.desc      : Test get desired AuxiliaryPicture from DecodingOptions.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_GETDESIREDAUXILIARYPICTURES_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_GETDESIREDAUXILIARYPICTURES_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.CreatePictureDecodingOptions();
        let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP];
        ndkTest.SetDesiredAuxiliaryPictures(inAuxTpyes.length, inAuxTpyes);
        let outAuxTpyes: Array<image.AuxiliaryPictureType> = ndkTest.GetDesiredAuxiliaryPictures();

        expect(compareArray(inAuxTpyes, outAuxTpyes)).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_RELEASE_0100
     * @tc.name      : testImageSourceReleasePictureDecodingOptions
     * @tc.desc      : Test release the created DecodingOptionsForPicture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_RELEASE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_IMAGESOURCE_RELEASE_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.CreatePictureDecodingOptions();
        let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP,
          image.AuxiliaryPictureType.DEPTH_MAP, image.AuxiliaryPictureType.UNREFOCUS_MAP,
          image.AuxiliaryPictureType.LINEAR_MAP, image.AuxiliaryPictureType.FRAGMENT_MAP];
        ndkTest.SetDesiredAuxiliaryPictures(inAuxTpyes.length, inAuxTpyes);

        expect(ndkTest.ReleasePictureDecodingOptions()).assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0100
     * @tc.name      : testSetAuxiliaryPicture
     * @tc.desc      : Test inserting AuxiliaryPicture with GAIN_MAP type into Picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAuxiliaryPicture(done, image.AuxiliaryPictureType.GAINMAP, 
        `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0100`)
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0200
     * @tc.name      : testSetAuxiliaryPicture
     * @tc.desc      : Test inserting AuxiliaryPicture with DEPTH_MAP type into Picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAuxiliaryPicture(done, image.AuxiliaryPictureType.DEPTH_MAP, 
        `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0200`)
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0300
     * @tc.name      : testSetAuxiliaryPicture
     * @tc.desc      : Test inserting AuxiliaryPicture with UNREFOCUS_MAP type into Picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAuxiliaryPicture(done, image.AuxiliaryPictureType.UNREFOCUS_MAP, 
        `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0300`)
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0400
     * @tc.name      : testSetAuxiliaryPicture
     * @tc.desc      : Test inserting AuxiliaryPicture with LINEAR_MAP type into Picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAuxiliaryPicture(done, image.AuxiliaryPictureType.LINEAR_MAP, 
        `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0400`)
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0500
     * @tc.name      : testSetAuxiliaryPicture
     * @tc.desc      : Test inserting AuxiliaryPicture with FRAGMENT_MAP type into Picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAuxiliaryPicture(done, image.AuxiliaryPictureType.FRAGMENT_MAP, 
        `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_0500`)
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_ERROR_0600
     * @tc.name      : testPictureSetAuxiliaryPicture
     * @tc.desc      : Test set AuxiliaryPicture to picture. Type error.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_ERROR_0600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_ERROR_0600`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreatePictureByPixelMap(caseName, testFileJpg);

        let gainmapType: image.AuxiliaryPictureType = image.AuxiliaryPictureType.GAINMAP;
        let depthType: image.AuxiliaryPictureType = image.AuxiliaryPictureType.DEPTH_MAP;
        let auxPicture: image.AuxiliaryPicture = testCreateAuxiliaryPicture(gainmapType);

        expect(ndkTest.SetAuxiliaryPictureErrorCode(depthType, auxPicture)).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_ERROR_0700
     * @tc.name      : testPictureSetAuxiliaryPicture
     * @tc.desc      : Test set AuxiliaryPicture to picture. The AuxiliaryPicture object is empty.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_ERROR_0700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETAUXILIARYPICTURE_ERROR_0700`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreatePictureByPixelMap(caseName, testFileJpg);

        let auxPicture: null = null;
        let depthType: image.AuxiliaryPictureType = image.AuxiliaryPictureType.DEPTH_MAP;
        expect(ndkTest.SetAuxiliaryPictureErrorCode(depthType, auxPicture)).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETAUXILIARYPICTURE_ERROR_0200
     * @tc.name      : testPictureGetAuxiliaryPicture
     * @tc.desc      : Test get AuxiliaryPicture from picture failed Unable to get the correct type.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETAUXILIARYPICTURE_ERROR_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETAUXILIARYPICTURE_ERROR_0200`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP,
          image.AuxiliaryPictureType.UNREFOCUS_MAP];
        testCreatePictureByImageSource(caseName, testFileWithFiveAuxiliaryPictureJpg, inAuxTpyes);

        let noexistTypes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.DEPTH_MAP,
          image.AuxiliaryPictureType.LINEAR_MAP,image.AuxiliaryPictureType.FRAGMENT_MAP];

        expect(ndkTest.GetAuxiliaryPicture(noexistTypes[0]) != undefined).assertFalse();
        expect(ndkTest.GetAuxiliaryPicture(noexistTypes[1]) != undefined).assertFalse();
        expect(ndkTest.GetAuxiliaryPicture(noexistTypes[2]) != undefined).assertFalse();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETAUXILIARYPICTURE_ERROR_0400
     * @tc.name      : testPictureGetAuxiliaryPicture
     * @tc.desc      : Test get  AuxiliaryPicture from picture failed Unable to get the correct type.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETAUXILIARYPICTURE_ERROR_0400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETAUXILIARYPICTURE_ERROR_0400`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP,
          image.AuxiliaryPictureType.DEPTH_MAP, image.AuxiliaryPictureType.FRAGMENT_MAP];
        testCreatePictureByImageSource(caseName, testFileWithFiveAuxiliaryPictureJpg, inAuxTpyes);

        let noexistTypes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.UNREFOCUS_MAP,
          image.AuxiliaryPictureType.LINEAR_MAP,image.AuxiliaryPictureType.FRAGMENT_MAP];

        expect(ndkTest.GetAuxiliaryPictureErrorCode(noexistTypes[0])).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        expect(ndkTest.GetAuxiliaryPictureErrorCode(noexistTypes[1])).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        expect(ndkTest.GetAuxiliaryPictureErrorCode(noexistTypes[2])).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETAUXILIARYPICTURE_0500
     * @tc.name      : testGetAuxiliaryPicture
     * @tc.desc      : Test get AuxiliaryPicture from Picture
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETAUXILIARYPICTURE_0500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETAUXILIARYPICTURE_0500`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreatePictureByPixelMap(caseName, testFileJpg);

        let gainmapType: image.AuxiliaryPictureType = image.AuxiliaryPictureType.GAINMAP;
        let auxPicture: image.AuxiliaryPicture = testCreateAuxiliaryPicture(gainmapType);

        ndkTest.SetAuxiliaryPicture(gainmapType, auxPicture);
        expect(ndkTest.GetAuxiliaryPicture(gainmapType) != undefined).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETAUXILIARYPICTURE_ERROR_0600
     * @tc.name      : testPictureGetAuxiliaryPicture
     * @tc.desc      : Prerequisite: Create an picture from PixelMap.
     *                 Test get AuxiliaryPicture from picture. there is no AuxiliaryPicture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETAUXILIARYPICTURE_ERROR_0600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETAUXILIARYPICTURE_ERROR_0600`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreatePictureByPixelMap(caseName, testNoAuxiliaryPictureJpg);

        let gainmapType: image.AuxiliaryPictureType = image.AuxiliaryPictureType.GAINMAP;
        expect(ndkTest.GetAuxiliaryPictureErrorCode(gainmapType)).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_CREATE_0100
     * @tc.name      : testMetadataCreate
     * @tc.desc      : Test create exif metadata.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_CREATE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_CREATE_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        expect(ndkTest.MetadataCreate(image.MetadataType.EXIF_METADATA) != undefined).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_CREATE_0200
     * @tc.name      : testMetadataCreate
     * @tc.desc      : Test create fragment metadata.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_CREATE_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_CREATE_0200`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        expect(ndkTest.MetadataCreate(image.MetadataType.FRAGMENT_METADATA) != undefined).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_0100
     * @tc.name      : testMetadataSetProperty
     * @tc.desc      : Test set ptoperty to exif metadata.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.EXIF_METADATA);
        
        expect(ndkTest.MetadataSetProperty(imageLengthKey, imageLengthValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(imageWidthKey, imageWidthValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(orientationKey, orientationValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_0200
     * @tc.name      : testMetadataSetProperty
     * @tc.desc      : Test set ptoperty to fragment metadata.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_0200`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.FRAGMENT_METADATA);

        expect(ndkTest.MetadataSetProperty(fragmentImageWidthKey, fragmentImageWidthValue))
          .assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(fragmentImageHeightKey, fragmentImageHeightValue))
          .assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(xInOriginalKey, xInOriginalValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(yInOriginalKey, yInOriginalValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0300
     * @tc.name      : testMetadataSetProperty
     * @tc.desc      : Test set property to exif metadata with illegal.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0300`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.EXIF_METADATA);

        expect(ndkTest.MetadataSetPropertyErrorCode(imageWidthKeyErr, imageWidthValue))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(imageHeightKeyErr, imageHeightValue))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetProperty(orientationKey, orientationValue))
          .assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0400
     * @tc.name      : testMetadataSetProperty
     * @tc.desc      : Test set property to exif metadata with illegal.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0400`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.EXIF_METADATA);

        expect(ndkTest.MetadataSetPropertyErrorCode(imageWidthKeyErr, imageWidthValue))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(imageHeightKeyErr, imageHeightValue))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(orientationKeyErr, orientationValue))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0500
     * @tc.name      : testMetadataSetProperty
     * @tc.desc      : Test set property to exif metadata with illegal.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0500`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.EXIF_METADATA);

        expect(ndkTest.MetadataSetPropertyErrorCode(imageWidthKey, imageWidthValueErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(imageHeightKeyErr, imageHeightValueErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetProperty(orientationKey, orientationValue))
          .assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0600
     * @tc.name      : testMetadataSetProperty
     * @tc.desc      : Test set property to exif metadata with illegal.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0600`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.EXIF_METADATA);

        expect(ndkTest.MetadataSetPropertyErrorCode(imageWidthKey, imageWidthValueErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(imageHeightKeyErr, imageHeightValueErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(orientationKeyErr, orientationValueErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0700
     * @tc.name      : testMetadataSetProperty
     * @tc.desc      : Test set property to fragment metadata with illegal.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0700`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.FRAGMENT_METADATA);

        expect(ndkTest.MetadataSetPropertyErrorCode(xInOriginalKeyErr, xInOriginalValue))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(yInOriginalKeyErr, yInOriginalValue))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetProperty(fragmentImageWidthKey, fragmentImageWidthValue))
          .assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(fragmentImageHeightKey, fragmentImageHeightValue))
          .assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0800
     * @tc.name      : testMetadataSetProperty
     * @tc.desc      : Test set property to fragment metadata with illegal.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_SETPROPERTY_ERROR_0800`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.FRAGMENT_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(fragmentImageWidthKeyErr,fragmentImageWidthValue))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(fragmentImageHeightKeyErr,fragmentImageHeightValue))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(xInOriginalKeyErr, xInOriginalValue))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(yInOriginalKeyErr, yInOriginalValue))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_GETPROPERTY_0100
     * @tc.name      : testMetadataGetProperty
     * @tc.desc      : Test get ptoperty from metadata.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_GETPROPERTY_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_GETPROPERTY_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        createDefaultExifMetadata();

        expect(ndkTest.MetadataGetProperty(imageLengthKey)).assertEqual(imageLengthValue);
        expect(ndkTest.MetadataGetProperty(imageWidthKey)).assertEqual(imageWidthValue);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETMETADATA_0100
     * @tc.name      : testPictureSetMetadata
     * @tc.desc      : Test set exif metadata to picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETMETADATA_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETMETADATA_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreatePictureByPixelMap(caseName, testFileJpg);

        let inMetadata: image.Metadata = createDefaultExifMetadata();
        expect(inMetadata != undefined).assertTrue();
        expect(ndkTest.PictureSetMetadata(image.MetadataType.EXIF_METADATA, inMetadata))
          .assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETMETADATA_0200
     * @tc.name      : testPictureSetMetadata
     * @tc.desc      : Test set fragment metadata to picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETMETADATA_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_SETMETADATA_0200`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreatePictureByPixelMap(caseName, testFileJpg);

        let inMetadata: image.Metadata = createDefaultFragmentMetadata();
        expect(inMetadata != undefined).assertTrue();
        expect(ndkTest.PictureSetMetadataErrorCode(image.MetadataType.FRAGMENT_METADATA, inMetadata))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_0100
     * @tc.name      : testPictureGetMetadata
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreatePictureByPixelMap(caseName, testFileJpg);

        let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
        let inMetadata: image.Metadata = createDefaultExifMetadata();
        expect(inMetadata != undefined).assertTrue();
        ndkTest.PictureSetMetadata(exifType, inMetadata);

        expect(ndkTest.PictureGetMetadata(exifType) != undefined).assertTrue();
        expect(ndkTest.pictureMetadataGetProperty(exifType, imageLengthKey)).assertEqual(imageLengthValue);
        expect(ndkTest.pictureMetadataGetProperty(exifType, imageWidthKey)).assertEqual(imageWidthValue);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_ERROR_0200
     * @tc.name      : testPictureGetMetadata
     * @tc.desc      : Test get metadata from picture with illegal type.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_ERROR_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_ERROR_0200`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreatePictureByPixelMap(caseName, testFileJpg);

        let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
        let fragmentType: image.MetadataType = image.MetadataType.FRAGMENT_METADATA;
        let inMetadata: image.Metadata = createDefaultExifMetadata();
        expect(inMetadata != undefined).assertTrue();
        ndkTest.PictureSetMetadata(exifType, inMetadata);

        expect(ndkTest.PictureGetMetadataErrorCode(fragmentType)).assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMAINPIXELMAP_0100
     * @tc.name      : testGetMainPixelmap
     * @tc.desc      : Test get MainPixelmap from Picture
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMAINPIXELMAP_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMAINPIXELMAP_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreatePictureByPixelMap(caseName, testNoAuxiliaryPictureJpg);
        const auxInfo: ndkTest.IAuxInfo = ndkTest.GetMainPixelmapInfo();

        let auxPicture: image.AuxiliaryPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP, auxPicture);

        const mapInfo: ndkTest.IAuxInfo = ndkTest.GetMainPixelmapInfo();

        expect(mapInfo.width).assertEqual(auxInfo.width);
        expect(mapInfo.height).assertEqual(auxInfo.height);
        expect(mapInfo.pixelFormat).assertEqual(auxInfo.pixelFormat);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETGAINMAPPIXELMAP_0100
     * @tc.name      : testGetGainmapPixelmap
     * @tc.desc      : Test get GainmapPixelmap from Picture
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETGAINMAPPIXELMAP_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETGAINMAPPIXELMAP_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreatePictureByPixelMap(caseName, testFileJpg);
        let auxPicture: image.AuxiliaryPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP, auxPicture);
        const auxInfo: ndkTest.IAuxInfo = ndkTest.GetGainmapPixelmapInfo();

        auxPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP, auxPicture);

        const mapInfo: ndkTest.IAuxInfo = ndkTest.GetGainmapPixelmapInfo();

        expect(mapInfo.width).assertEqual(auxInfo.width);
        expect(mapInfo.height).assertEqual(auxInfo.height);
        expect(mapInfo.pixelFormat).assertEqual(auxInfo.pixelFormat);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_RELEASE_0100
     * @tc.name      : testPictureRelease
     * @tc.desc      : Test release the created picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_RELEASE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_RELEASE_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreatePictureByPixelMap(caseName, testFileJpg);

        expect(ndkTest.ReleasePicture()).assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_RELEASE_0100
     * @tc.name      : testMetadataRelease
     * @tc.desc      : Test release the EXIF metadata.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_RELEASE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_RELEASE_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        createDefaultExifMetadata();

        expect(ndkTest.MetadataRelease()).assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_METADATA_RELEASE_0200
     * @tc.name      : testMetadataRelease
     * @tc.desc      : Test release the FRAGMENT metadata.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_METADATA_RELEASE_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_METADATA_RELEASE_0200`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        createDefaultFragmentMetadata();

        expect(ndkTest.MetadataRelease()).assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0100
     * @tc.name      : testCreateAuxiliaryPicture
     * @tc.desc      : Test the creation of AuxiliaryPicture with GAIN_MAP type.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testCasesOfCreateAuxiliaryPicture(done, image.AuxiliaryPictureType.GAINMAP,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0100')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0200
     * @tc.name      : testCreateAuxiliaryPicture
     * @tc.desc      : Test the creation of AuxiliaryPicture with DEPTH_MAP type.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testCasesOfCreateAuxiliaryPicture(done, image.AuxiliaryPictureType.DEPTH_MAP,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0200')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0300
     * @tc.name      : testCreateAuxiliaryPicture
     * @tc.desc      : Test the creation of AuxiliaryPicture with UNREFOCUS_MAP type.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testCasesOfCreateAuxiliaryPicture(done, image.AuxiliaryPictureType.UNREFOCUS_MAP,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0300')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0400
     * @tc.name      : testCreateAuxiliaryPicture
     * @tc.desc      : Test the creation of AuxiliaryPicture with LINEAR_MAP type.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testCasesOfCreateAuxiliaryPicture(done, image.AuxiliaryPictureType.LINEAR_MAP,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0400')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0500
     * @tc.name      : testCreateAuxiliaryPicture
     * @tc.desc      : Test the creation of AuxiliaryPicture with FRAGMENT_MAP type.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testCasesOfCreateAuxiliaryPicture(done, image.AuxiliaryPictureType.FRAGMENT_MAP,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_0500')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_ERROR_0600
     * @tc.name      : testCreateAuxiliaryPicture
     * @tc.desc      : Test the creation of AuxiliaryPicture with illegal type.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_ERROR_0600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_CREATE_ERROR_0600`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        let auxType: image.AuxiliaryPictureType = -1;
        let auxSize: image.Size = {height: 180, width: 240};
        let auxBuffer = createFillBuffer(172800, 4);

        expect(ndkTest.CreateAuxiliaryPictureErrorCode(auxBuffer, auxSize, auxType))
          .assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETMETADATA_0100
     * @tc.name      : testAuxiliaryPictureSetMetadata
     * @tc.desc      : Test set fragment metadata to auxiliary picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETMETADATA_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETMETADATA_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreateAuxiliaryPicture(image.AuxiliaryPictureType.FRAGMENT_MAP);

        let inMetadata: image.Metadata = createDefaultFragmentMetadata();
        expect(inMetadata != undefined).assertTrue();

        expect(ndkTest.AuxiliaryPictureSetMetadata(image.MetadataType.FRAGMENT_METADATA, inMetadata))
          .assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_GETMETADATA_0100
     * @tc.name      : testAuxiliaryPictureGetMetadata
     * @tc.desc      : Test get metadata from fragment auxiliary picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_GETMETADATA_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_GETMETADATA_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreateAuxiliaryPicture(image.AuxiliaryPictureType.FRAGMENT_MAP);

        let fragmentType: image.MetadataType = image.MetadataType.FRAGMENT_METADATA;
        let inMetadata: image.Metadata = createDefaultFragmentMetadata();
        expect(inMetadata != undefined).assertTrue();
        ndkTest.AuxiliaryPictureSetMetadata(fragmentType, inMetadata);

        expect(ndkTest.AuxiliaryPictureGetMetadataErrorCode(fragmentType)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_GETMETADATA_ERROR_0200
     * @tc.name      : testAuxiliaryPictureGetMetadata
     * @tc.desc      : Test get metadata from fragment auxiliary picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_GETMETADATA_ERROR_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_GETMETADATA_ERROR_0200`;
      hilog.info(domain, testTag, caseName + ` begin`);
        try {
        testCreateAuxiliaryPicture(image.AuxiliaryPictureType.FRAGMENT_MAP);

        let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
        let fragmentType: image.MetadataType = image.MetadataType.FRAGMENT_METADATA;
        let inMetadata: image.Metadata = createDefaultFragmentMetadata();
        expect(inMetadata != undefined).assertTrue();
        ndkTest.AuxiliaryPictureSetMetadata(fragmentType, inMetadata);

        expect(ndkTest.AuxiliaryPictureGetMetadataErrorCode(exifType)).assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        done();
       } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
       }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_WRITEPIXELS_0100
     * @tc.name      : testAuxiliaryPictureWritePixels
     * @tc.desc      : Test write pixels to AuxiliaryPicture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_WRITEPIXELS_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_WRITEPIXELS_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        let auxSize: image.Size = testFileGainMapSize;
        let buffer = getBuffer(testFileGainMap);
        ndkTest.CreateAuxiliaryPicture(buffer, auxSize, image.AuxiliaryPictureType.GAINMAP);

        expect(ndkTest.AuxiliaryPictureWritePixels(buffer)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_WRITEPIXELS_ERROR_0200
     * @tc.name      : testAuxiliaryPictureWritePixels
     * @tc.desc      : Test write pixels to AuxiliaryPicture.The pixel data buffer size is abnormal.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_WRITEPIXELS_ERROR_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_WRITEPIXELS_ERROR_0200`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        let auxSize: image.Size = testFileGainMapSize;
        let buffer = getBuffer(testFileGainMap);
        let writeBuffer = createFillBuffer(0, 0);
        ndkTest.CreateAuxiliaryPicture(buffer, auxSize, image.AuxiliaryPictureType.GAINMAP);
        expect(ndkTest.AuxiliaryPictureWritePixelsErrorCode(writeBuffer)).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_READPIXELS_0100
     * @tc.name      : testAuxiliaryPictureReadPixels
     * @tc.desc      : Test read pixels from AuxiliaryPicture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_READPIXELS_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_READPIXELS_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        let auxSize: image.Size = testFileGainMapSize;
        let writeBuffer = getBuffer(testFileGainMap);
        let readBuffer: ArrayBuffer = new ArrayBuffer(writeBuffer.byteLength);

        ndkTest.CreateAuxiliaryPicture(writeBuffer, auxSize, image.AuxiliaryPictureType.GAINMAP);

        ndkTest.AuxiliaryPictureWritePixels(writeBuffer);
        expect(ndkTest.AuxiliaryPictureReadPixels(readBuffer)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(compareArrayBuffer(readBuffer, writeBuffer)).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_READPIXELS_0200
     * @tc.name      : testAuxiliaryPictureReadPixels
     * @tc.desc      : Test read pixels from AuxiliaryPicture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_READPIXELS_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_READPIXELS_0200`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        let filePath = getFilePath(testFileJpg);
        ndkTest.createTestImageSource(filePath);
        ndkTest.createTestPixelmapByImageSource(image.PixelMapFormat.RGBA_8888);

        let buffer: ArrayBuffer = new ArrayBuffer(10000 * 10000);
        buffer = ndkTest.GetPixelmapBuffer(buffer);
        testCreateAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);
        ndkTest.AuxiliaryPictureWritePixels(buffer);
        let readBuffer: ArrayBuffer = new ArrayBuffer(10000 * 10000);
        expect(ndkTest.AuxiliaryPictureReadPixels(readBuffer)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(compareArrayForLessTimes(readBuffer, buffer)).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_READPIXELS_0300
     * @tc.name      : testAuxiliaryPictureReadPixels
     * @tc.desc      : Test read pixels from AuxiliaryPicture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_READPIXELS_0300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_READPIXELS_0300`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        let filePath = getFilePath(testFileJpg);
        ndkTest.createTestImageSource(filePath);
        ndkTest.createTestPixelmapByImageSource(image.PixelMapFormat.NV12);

        let buffer: ArrayBuffer = new ArrayBuffer(10000 * 10000);
        buffer = ndkTest.GetPixelmapBuffer(buffer);
        testCreateAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);
        ndkTest.AuxiliaryPictureWritePixels(buffer);
        let readBuffer: ArrayBuffer = new ArrayBuffer(10000 * 10000);
        expect(ndkTest.AuxiliaryPictureReadPixels(readBuffer)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(compareArrayForLessTimes(readBuffer, buffer)).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_READPIXELS_0400
     * @tc.name      : testAuxiliaryPictureReadPixels
     * @tc.desc      : Test read pixels from AuxiliaryPicture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_READPIXELS_0400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_READPIXELS_0400`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        let filePath = getFilePath(testFileJpg);
        ndkTest.createTestImageSource(filePath);
        ndkTest.createTestPixelmapByImageSource(image.PixelMapFormat.NV21);

        let buffer: ArrayBuffer = new ArrayBuffer(10000 * 10000);
        buffer = ndkTest.GetPixelmapBuffer(buffer);
        testCreateAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);
        ndkTest.AuxiliaryPictureWritePixels(buffer);
        let readBuffer: ArrayBuffer = new ArrayBuffer(10000 * 10000);
        expect(ndkTest.AuxiliaryPictureReadPixels(readBuffer)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(compareArrayForLessTimes(readBuffer, buffer)).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0100
     * @tc.name      : testAuxiliaryPictureSetInfo
     * @tc.desc      : Test set type to AuxiliaryPictureInfo,then set AuxiliaryPictureInfo to 
                       AuxiliaryPicture which get from Picture created by ImageSource.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0100`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP];
          testCreatePictureByImageSource(caseName, testFileJpg, inAuxTpyes);
          ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);
  
          let auxInfo: ndkTest.IAuxInfo = {
            auxiliaryPictureType: image.AuxiliaryPictureType.UNREFOCUS_MAP,
            width: ndkTest.AuxiliaryPictureGetInfo().width,
            height: ndkTest.AuxiliaryPictureGetInfo().height,
            rowStride: ndkTest.AuxiliaryPictureGetInfo().rowStride,
            pixelFormat: ndkTest.AuxiliaryPictureGetInfo().pixelFormat
          };
          ndkTest.AuxiliaryPictureSetInfo(auxInfo);
  
          let newType: image.AuxiliaryPictureType = image.AuxiliaryPictureType.UNREFOCUS_MAP;
          expect(ndkTest.AuxiliaryPictureGetInfo().auxiliaryPictureType).assertEqual(newType);
          ndkTest.ReleaseAuxiliaryPicture();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0200
     * @tc.name      : testAuxiliaryPictureSetInfo
     * @tc.desc      : Test set size to AuxiliaryPictureInfo,then set AuxiliaryPictureInfo to 
                       AuxiliaryPicture which get from Picture created by ImageSource.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0200`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP];
          testCreatePictureByImageSource(caseName, testFileJpg, inAuxTpyes);
          ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);
  
          let auxInfo: ndkTest.IAuxInfo = {
            auxiliaryPictureType: ndkTest.AuxiliaryPictureGetInfo().auxiliaryPictureType,
            width: 30,
            height: 40,
            rowStride: ndkTest.AuxiliaryPictureGetInfo().rowStride,
            pixelFormat: ndkTest.AuxiliaryPictureGetInfo().pixelFormat
          };
          ndkTest.AuxiliaryPictureSetInfo(auxInfo);
  
          expect(ndkTest.AuxiliaryPictureGetInfo().width).assertEqual(30);
          expect(ndkTest.AuxiliaryPictureGetInfo().height).assertEqual(40);
          ndkTest.ReleaseAuxiliaryPicture();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0300
     * @tc.name      : testAuxiliaryPictureSetInfo
     * @tc.desc      : Test set rowStride to AuxiliaryPictureInfo,then set AuxiliaryPictureInfo to 
                       AuxiliaryPicture which get from Picture created by ImageSource.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0300`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP];
          testCreatePictureByImageSource(caseName, testFileJpg, inAuxTpyes);
          ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);
  
          let auxInfo: ndkTest.IAuxInfo = {
            auxiliaryPictureType: ndkTest.AuxiliaryPictureGetInfo().auxiliaryPictureType,
            width: ndkTest.AuxiliaryPictureGetInfo().width,
            height: ndkTest.AuxiliaryPictureGetInfo().height,
            rowStride: 20,
            pixelFormat: ndkTest.AuxiliaryPictureGetInfo().pixelFormat
          };
          ndkTest.AuxiliaryPictureSetInfo(auxInfo);
  
          expect(ndkTest.AuxiliaryPictureGetInfo().rowStride).assertEqual(20);
          ndkTest.ReleaseAuxiliaryPicture();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0400
     * @tc.name      : testAuxiliaryPictureSetInfo
     * @tc.desc      : Test set pixelFormat to AuxiliaryPictureInfo,then set AuxiliaryPictureInfo to 
                       AuxiliaryPicture which get from Picture created by ImageSource.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0400`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP];
          testCreatePictureByImageSource(caseName, testFileJpg, inAuxTpyes);
          ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);
  
          let auxInfo: ndkTest.IAuxInfo = {
            auxiliaryPictureType: ndkTest.AuxiliaryPictureGetInfo().auxiliaryPictureType,
            width: ndkTest.AuxiliaryPictureGetInfo().width,
            height: ndkTest.AuxiliaryPictureGetInfo().height,
            rowStride: ndkTest.AuxiliaryPictureGetInfo().rowStride,
            pixelFormat:  image.PixelMapFormat.RGBA_8888
          };
          ndkTest.AuxiliaryPictureSetInfo(auxInfo);
  
          expect(ndkTest.AuxiliaryPictureGetInfo().pixelFormat).assertEqual(auxInfo.pixelFormat);
          ndkTest.ReleaseAuxiliaryPicture();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0500
     * @tc.name      : testAuxiliaryPictureSetInfo
     * @tc.desc      : Test set auxiliaryPictureInfo to auxiliaryPicture
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_SETINFO_0500`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreateAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);

        let auxInfo: ndkTest.IAuxInfo = {
          auxiliaryPictureType: image.AuxiliaryPictureType.GAINMAP,
          width: 10,
          height: 20,
          rowStride: 30,
          pixelFormat: image.PixelMapFormat.RGBA_8888
        };
        expect(ndkTest.AuxiliaryPictureSetInfo(auxInfo)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_GETINFO_0100
     * @tc.name      : testAuxiliaryPictureGetInfo
     * @tc.desc      : Test get auxiliaryPictureInfo from auxiliaryPicture
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_GETINFO_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_GETINFO_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreateAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);

        let auxInfo: ndkTest.IAuxInfo = {
          auxiliaryPictureType: image.AuxiliaryPictureType.GAINMAP,
          width: 10,
          height: 20,
          rowStride: 30,
          pixelFormat: image.PixelMapFormat.RGBA_8888
        };
        ndkTest.AuxiliaryPictureSetInfo(auxInfo);

        expect(ndkTest.AuxiliaryPictureGetInfo().auxiliaryPictureType).assertEqual(auxInfo.auxiliaryPictureType);
        expect(ndkTest.AuxiliaryPictureGetInfo().width).assertEqual(auxInfo.width);
        expect(ndkTest.AuxiliaryPictureGetInfo().height).assertEqual(auxInfo.height);
        expect(ndkTest.AuxiliaryPictureGetInfo().rowStride).assertEqual(auxInfo.rowStride);
        expect(ndkTest.AuxiliaryPictureGetInfo().pixelFormat).assertEqual(auxInfo.pixelFormat);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_RELEASE_0100
     * @tc.name      : testAuxiliaryPictureRelease
     * @tc.desc      : Test release the created (GAINMAP/DEPTH_MAP/UNREFOCUS_MAP/LINEAR_MAP/FRAGMENT_MAP) object.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_RELEASE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_RELEASE_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        let auxPicture: image.AuxiliaryPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP, auxPicture);
        expect(ndkTest.ReleaseAuxiliaryPicture()).assertEqual(ErrorCode.IMAGE_SUCCESS);

        auxPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP, auxPicture);
        expect(ndkTest.ReleaseAuxiliaryPicture()).assertEqual(ErrorCode.IMAGE_SUCCESS);

        auxPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.UNREFOCUS_MAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.UNREFOCUS_MAP, auxPicture);
        expect(ndkTest.ReleaseAuxiliaryPicture()).assertEqual(ErrorCode.IMAGE_SUCCESS);

        auxPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.LINEAR_MAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.LINEAR_MAP, auxPicture);
        expect(ndkTest.ReleaseAuxiliaryPicture()).assertEqual(ErrorCode.IMAGE_SUCCESS);
 
        auxPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.FRAGMENT_MAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.FRAGMENT_MAP, auxPicture);
        expect(ndkTest.ReleaseAuxiliaryPicture()).assertEqual(ErrorCode.IMAGE_SUCCESS);

        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_RELEASE_0200
     * @tc.name      : testAuxiliaryPictureRelease
     * @tc.desc      : Test release the AuxiliaryPicture added to picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_RELEASE_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTURE_RELEASE_0200`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreatePictureByPixelMap(caseName, testFileWithFiveAuxiliaryPictureJpg);

        let auxPicture: image.AuxiliaryPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP, auxPicture);
        expect(ndkTest.ReleaseAuxiliaryPicture()).assertEqual(ErrorCode.IMAGE_SUCCESS);

        auxPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP, auxPicture);
        expect(ndkTest.ReleaseAuxiliaryPicture()).assertEqual(ErrorCode.IMAGE_SUCCESS);

        auxPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.UNREFOCUS_MAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.UNREFOCUS_MAP, auxPicture);
        expect(ndkTest.ReleaseAuxiliaryPicture()).assertEqual(ErrorCode.IMAGE_SUCCESS);

        auxPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.LINEAR_MAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.LINEAR_MAP, auxPicture);
        expect(ndkTest.ReleaseAuxiliaryPicture()).assertEqual(ErrorCode.IMAGE_SUCCESS);

        auxPicture = testCreateAuxiliaryPicture(image.AuxiliaryPictureType.FRAGMENT_MAP);
        ndkTest.SetAuxiliaryPicture(image.AuxiliaryPictureType.FRAGMENT_MAP, auxPicture);
        expect(ndkTest.ReleaseAuxiliaryPicture()).assertEqual(ErrorCode.IMAGE_SUCCESS);

        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_CREATE_0100
     * @tc.name      : testAuxiliaryPictureInfoCrerate
     * @tc.desc      : Test create auxiliaryPictureInfo
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_CREATE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_CREATE_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        expect(ndkTest.CreateAuxiliaryPictureInfo() != undefined).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0100
     * @tc.name      : testAuxiliaryPictureInfoSetTypeGetType
     * @tc.desc      : Test set and get type from auxiliaryPictureInfo
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testAuxiliaryPictureInfoSetTypeAndGetType(done, image.AuxiliaryPictureType.GAINMAP,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0100');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0200
     * @tc.name      : testAuxiliaryPictureInfoSetTypeGetType
     * @tc.desc      : Test set and get type from auxiliaryPictureInfo
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testAuxiliaryPictureInfoSetTypeAndGetType(done, image.AuxiliaryPictureType.DEPTH_MAP,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0200');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0300
     * @tc.name      : testAuxiliaryPictureInfoSetTypeGetType
     * @tc.desc      : Test set and get type from auxiliaryPictureInfo
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testAuxiliaryPictureInfoSetTypeAndGetType(done, image.AuxiliaryPictureType.UNREFOCUS_MAP,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0300');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0400
     * @tc.name      : testAuxiliaryPictureInfoSetTypeGetType
     * @tc.desc      : Test set and get type from auxiliaryPictureInfo
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testAuxiliaryPictureInfoSetTypeAndGetType(done, image.AuxiliaryPictureType.LINEAR_MAP,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0400');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0500
     * @tc.name      : testAuxiliaryPictureInfoSetTypeGetType
     * @tc.desc      : Test set and get type from auxiliaryPictureInfo
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testAuxiliaryPictureInfoSetTypeAndGetType(done, image.AuxiliaryPictureType.FRAGMENT_MAP,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_GETTYPE_0500');
    })


    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_ERROR_0600
     * @tc.name      : testAuxiliaryPictureInfoSetType
     * @tc.desc      : Test set illegal type to AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_ERROR_0600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETTYPE_ERROR_0600`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.CreateAuxiliaryPictureInfo();

        let auxType: number = -1;
        expect(ndkTest.AuxiliaryPictureInfoSetTypeErrorCode(auxType)).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETSIZE_GETSIZE_0100
     * @tc.name      : testAuxiliaryPictureInfoSetSizeGetSize
     * @tc.desc      : Test set and get normal size about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETSIZE_GETSIZE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETSIZE_GETSIZE_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.CreateAuxiliaryPictureInfo();
        let inSize: image.Size = {height: 15, width: 20};
        expect(ndkTest.AuxiliaryPictureInfoSetSize(inSize)).assertEqual(ErrorCode.IMAGE_SUCCESS);

        let outSize: image.Size = ndkTest.AuxiliaryPictureInfoGetSize();
        expect(outSize.width).assertEqual(inSize.width);
        expect(outSize.height).assertEqual(inSize.height);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETROWSTRIDE_GETROWSTRIDE_0100
     * @tc.name      : testAuxiliaryPictureInfoSetRowStrideGetRowStride
     * @tc.desc      : Test set and get normal rowStride about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETROWSTRIDE_GETROWSTRIDE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETROWSTRIDE_GETROWSTRIDE_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.CreateAuxiliaryPictureInfo();
        let rowStride = 40;
        expect(ndkTest.AuxiliaryPictureInfoSetRowStride(rowStride)).assertEqual(ErrorCode.IMAGE_SUCCESS);

        expect(ndkTest.AuxiliaryPictureInfoGetRowStride()).assertEqual(rowStride);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_RELEASE_0100
     * @tc.name      : testAuxiliaryPictureInfoRelease
     * @tc.desc      : Test release AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_RELEASE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_RELEASE_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.CreateAuxiliaryPictureInfo();

        expect(ndkTest.ReleaseAuxiliaryPictureInfo()).assertEqual(ErrorCode.IMAGE_SUCCESS);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_GETPROPERTY_0100
     * @tc.name      : testMetadataGetProperty
     * @tc.desc      : Test get exif metadata property from PictureMetadata that contains some wrong key values
     * @tc.size      : MediumTest
     * @tc.type      : Reliability
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_GETPROPERTY_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_GETPROPERTY_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.EXIF_METADATA);

        expect(ndkTest.MetadataSetProperty(imageLengthKey, imageLengthValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(imageWidthKey, imageWidthValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(orientationKey, orientationValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);

        expect(ndkTest.MetadataGetProperty(imageLengthKey)).assertEqual(imageLengthValue);
        expect(ndkTest.MetadataGetProperty(imageWidthKey)).assertEqual(imageWidthValue);
        expect(ndkTest.MetadataGetPropertyErrorCode(orientationKeyErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_GETPROPERTY_ERROR_0200
     * @tc.name      : testMetadataGetProperty
     * @tc.desc      : Test get exif metadata property from PictureMetadata that contains all wrong key values
     * @tc.size      : MediumTest
     * @tc.type      : Reliability
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_GETPROPERTY_ERROR_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_GETPROPERTY_ERROR_0200`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.EXIF_METADATA);

        expect(ndkTest.MetadataSetProperty(imageLengthKey, imageLengthValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(imageWidthKey, imageWidthValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(orientationKey, orientationValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);

        expect(ndkTest.MetadataGetPropertyErrorCode(imageLengthKeyErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataGetPropertyErrorCode(imageWidthKeyErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataGetPropertyErrorCode(orientationKeyErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_GETPROPERTY_0300
     * @tc.name      : testMetadataGetProperty
     * @tc.desc      : Test get fragment metadata property from PictureMetadata that contains some wrong key values
     * @tc.size      : MediumTest
     * @tc.type      : Reliability
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_GETPROPERTY_0300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_GETPROPERTY_0300`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.FRAGMENT_METADATA);

        expect(ndkTest.MetadataSetProperty(fragmentImageWidthKey, fragmentImageWidthValue))
          .assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(fragmentImageHeightKey, fragmentImageHeightValue))
          .assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(xInOriginalKey, xInOriginalValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(yInOriginalKey, yInOriginalValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);

        expect(ndkTest.MetadataGetPropertyErrorCode(fragmentImageWidthKeyErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataGetPropertyErrorCode(fragmentImageHeightKeyErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataGetProperty(xInOriginalKey)).assertEqual(xInOriginalValue);
        expect(ndkTest.MetadataGetProperty(yInOriginalKey)).assertEqual(yInOriginalValue);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_GETPROPERTY_ERROR_0400
     * @tc.name      : testMetadataGetProperty
     * @tc.desc      : Test get fragment metadata property from PictureMetadata that contains all wrong key values
     * @tc.size      : MediumTest
     * @tc.type      : Reliability
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_GETPROPERTY_ERROR_0400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_GETPROPERTY_ERROR_0400`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.FRAGMENT_METADATA);

        expect(ndkTest.MetadataSetProperty(fragmentImageWidthKey, fragmentImageWidthValue))
          .assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(fragmentImageHeightKey, fragmentImageHeightValue))
          .assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(xInOriginalKey, xInOriginalValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);
        expect(ndkTest.MetadataSetProperty(yInOriginalKey, yInOriginalValue)).assertEqual(ErrorCode.IMAGE_SUCCESS);

        expect(ndkTest.MetadataGetPropertyErrorCode(fragmentImageWidthKeyErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataGetPropertyErrorCode(fragmentImageHeightKeyErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataGetPropertyErrorCode(xInOriginalKeyErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        expect(ndkTest.MetadataGetPropertyErrorCode(yInOriginalKeyErr))
          .assertEqual(ErrorCode.IMAGE_UNSUPPORTED_METADATA);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_CLONE_0100
     * @tc.name      : testPictureMetadataClone
     * @tc.desc      : Test clone PictureMetadata
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_CLONE_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTUREMETADATA_CLONE_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        createDefaultExifMetadata();

        expect(ndkTest.MetadataClone() != undefined).assertTrue();
        expect(ndkTest.clonedMetadataGetProperty(imageLengthKey))
          .assertEqual(ndkTest.MetadataGetProperty(imageLengthKey));
        expect(ndkTest.clonedMetadataGetProperty(imageWidthKey))
          .assertEqual(ndkTest.MetadataGetProperty(imageWidthKey));
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0100
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get UNKNOWN pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0100', Level.LEVEL0,
      async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormatByCreateAuxInfo(done, image.PixelMapFormat.UNKNOWN,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0100');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0200
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get RGB_565 pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0200', Level.LEVEL0,
      async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormatByCreateAuxInfo(done, image.PixelMapFormat.RGB_565,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0200');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0300
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get RGBA_8888 pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0300', Level.LEVEL0,
      async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormatByCreateAuxInfo(done, image.PixelMapFormat.RGBA_8888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0300');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0400
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get BGRA_8888 pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0400', Level.LEVEL0,
      async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormatByCreateAuxInfo(done, image.PixelMapFormat.BGRA_8888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0400');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0500
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get RGB_888 pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0500', Level.LEVEL0,
      async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormatByCreateAuxInfo(done, image.PixelMapFormat.RGB_888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0500');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0600
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get ALPHA_8 pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0600', Level.LEVEL0,
      async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormatByCreateAuxInfo(done, image.PixelMapFormat.ALPHA_8,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0600');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0700
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get RGBA_F16 pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0700', Level.LEVEL0,
      async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormatByCreateAuxInfo(done, image.PixelMapFormat.RGBA_F16,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0700');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0800
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get NV21 pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0800', Level.LEVEL0,
      async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormatByCreateAuxInfo(done, image.PixelMapFormat.NV21,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0800');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0900
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get NV12 pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0900', Level.LEVEL0,
      async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormatByCreateAuxInfo(done, image.PixelMapFormat.NV12,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_0900');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1000
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get RGBA_1010102 pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1000', Level.LEVEL0,
      async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormatByCreateAuxInfo(done, image.PixelMapFormat.RGBA_1010102,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1000');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1100
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get YCBCR_P010 pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1100', Level.LEVEL0,
      async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormatByCreateAuxInfo(done, image.PixelMapFormat.YCBCR_P010,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1100');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1200
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get YCRCB_P010 pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1200', Level.LEVEL0,
      async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormatByCreateAuxInfo(done, image.PixelMapFormat.YCRCB_P010,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1200');
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1300
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.FRAGMENT_MAP, image.PixelMapFormat.NV12,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1300')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1400
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.GAINMAP, image.PixelMapFormat.RGB_565,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1400')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1500
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.GAINMAP, image.PixelMapFormat.RGBA_8888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1500')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1600
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.GAINMAP, image.PixelMapFormat.BGRA_8888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1600')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1700
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.GAINMAP, image.PixelMapFormat.RGB_888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1700')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1800
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.GAINMAP, image.PixelMapFormat.RGBA_F16,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1800')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1900
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.GAINMAP, image.PixelMapFormat.RGBA_1010102,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_1900')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2000
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.GAINMAP, image.PixelMapFormat.YCBCR_P010,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2000')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2100
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.GAINMAP, image.PixelMapFormat.YCRCB_P010,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2100')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2200
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.GAINMAP, image.PixelMapFormat.NV21,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2200')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2300
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.GAINMAP, image.PixelMapFormat.NV12,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2300')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2400
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.DEPTH_MAP, image.PixelMapFormat.RGB_565,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2400')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2500
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.DEPTH_MAP, image.PixelMapFormat.RGBA_8888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2500')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2600
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.DEPTH_MAP, image.PixelMapFormat.BGRA_8888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2600')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2700
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.DEPTH_MAP, image.PixelMapFormat.RGB_888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2700')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2800
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.DEPTH_MAP, image.PixelMapFormat.RGBA_F16,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2800')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2900
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.DEPTH_MAP, image.PixelMapFormat.RGBA_1010102,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_2900')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3000
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.DEPTH_MAP, image.PixelMapFormat.YCBCR_P010,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3000')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3100
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.DEPTH_MAP, image.PixelMapFormat.YCRCB_P010,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3100')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3200
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.DEPTH_MAP, image.PixelMapFormat.NV21,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3200')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3300
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.DEPTH_MAP, image.PixelMapFormat.NV12,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3300')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3400
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.UNREFOCUS_MAP, image.PixelMapFormat.RGB_565,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3400')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3500
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.UNREFOCUS_MAP, image.PixelMapFormat.RGBA_8888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3500')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3600
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.UNREFOCUS_MAP, image.PixelMapFormat.BGRA_8888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3600')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3700
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.UNREFOCUS_MAP, image.PixelMapFormat.RGB_888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3700')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3800
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.UNREFOCUS_MAP, image.PixelMapFormat.RGBA_F16,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3800')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3900
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.UNREFOCUS_MAP, image.PixelMapFormat.RGBA_1010102,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_3900')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4000
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.UNREFOCUS_MAP, image.PixelMapFormat.YCBCR_P010,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4000')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4100
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.UNREFOCUS_MAP, image.PixelMapFormat.YCRCB_P010,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4100')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4200
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.UNREFOCUS_MAP, image.PixelMapFormat.NV21,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4200')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4300
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.UNREFOCUS_MAP, image.PixelMapFormat.NV12,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4300')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4400
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.LINEAR_MAP, image.PixelMapFormat.RGB_565,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4400')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4500
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.LINEAR_MAP, image.PixelMapFormat.RGBA_8888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4500')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4600
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.LINEAR_MAP, image.PixelMapFormat.BGRA_8888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4600')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4700
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.LINEAR_MAP, image.PixelMapFormat.RGB_888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4700')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4800
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.LINEAR_MAP, image.PixelMapFormat.RGBA_F16,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4800')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4900
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.LINEAR_MAP, image.PixelMapFormat.RGBA_1010102,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_4900')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5000
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.LINEAR_MAP, image.PixelMapFormat.YCBCR_P010,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5000')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5100
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.LINEAR_MAP, image.PixelMapFormat.YCRCB_P010,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5100')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5200
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.LINEAR_MAP, image.PixelMapFormat.NV21,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5200')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5300
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.LINEAR_MAP, image.PixelMapFormat.NV12,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5300')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5400
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.FRAGMENT_MAP, image.PixelMapFormat.RGB_565,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5400')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5500
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.FRAGMENT_MAP, image.PixelMapFormat.RGBA_8888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5500')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5600
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.FRAGMENT_MAP, image.PixelMapFormat.BGRA_8888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5600')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5700
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.FRAGMENT_MAP, image.PixelMapFormat.RGB_888,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5700')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5800
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.FRAGMENT_MAP, image.PixelMapFormat.RGBA_F16,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5800')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5900
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.FRAGMENT_MAP, image.PixelMapFormat.RGBA_1010102,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_5900')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_6000
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_6000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.FRAGMENT_MAP, image.PixelMapFormat.YCBCR_P010,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_6000')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_6100
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_6100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.FRAGMENT_MAP, image.PixelMapFormat.YCRCB_P010,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_6100')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_6200
     * @tc.name      : testAuxiliaryPictureInfoSetPixelFormatGetPixelFormat
     * @tc.desc      : Test set and get pixelFormat about AuxiliaryPictureInfo.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_6200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testSetAndGetPixelFormat(done, image.AuxiliaryPictureType.FRAGMENT_MAP, image.PixelMapFormat.NV21,
        'SUB_MULTIMEDIA_PICTURE_NDK_AUXILIARYPICTUREINFO_SETPIXELFORMAT_GETPIXELFORMAT_6200')
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0100
     * @tc.name      : testCreatePictureByPixelmap
     * @tc.desc      : Test create picture by pixelmap with different pixelMapFormat.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testPixelMapFormatInPicture(done, testFileJpg, image.PixelMapFormat.NV21,
        `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0100`);
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0200
     * @tc.name      : testCreatePictureByPixelmap
     * @tc.desc      : Test create picture by pixelmap with different pixelMapFormat.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        testPixelMapFormatInPicture(done, testFileHeic, image.PixelMapFormat.NV21,
          `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0200`);
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0300
     * @tc.name      : testCreatePictureByPixelmap
     * @tc.desc      : Test create picture by pixelmap with different pixelMapFormat.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testPixelMapFormatInPicture(done, testFileJpg, image.PixelMapFormat.RGB_565,
        `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0300`);
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0400
     * @tc.name      : testCreatePictureByPixelmap
     * @tc.desc      : Test create picture by pixelmap with different pixelMapFormat.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        testPixelMapFormatInPicture(done, testFileHeic, image.PixelMapFormat.RGB_565,
          `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0400`);
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0500
     * @tc.name      : testCreatePictureByPixelmap
     * @tc.desc      : Test create picture by pixelmap with different pixelMapFormat.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testPixelMapFormatInPicture(done, testFileJpg, image.PixelMapFormat.RGBA_8888,
        `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0500`);
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0600
     * @tc.name      : testCreatePictureByPixelmap
     * @tc.desc      : Test create picture by pixelmap with different pixelMapFormat.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        testPixelMapFormatInPicture(done, testFileHeic, image.PixelMapFormat.RGBA_8888,
          `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0600`);
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0700
     * @tc.name      : testCreatePictureByPixelmap
     * @tc.desc      : Test create picture by pixelmap with different pixelMapFormat.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testPixelMapFormatInPicture(done, testFileJpg, image.PixelMapFormat.BGRA_8888,
        `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0700`);
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0800
     * @tc.name      : testCreatePictureByPixelmap
     * @tc.desc      : Test create picture by pixelmap with different pixelMapFormat.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        testPixelMapFormatInPicture(done, testFileHeic, image.PixelMapFormat.BGRA_8888,
          `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_0800`);
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_1900
     * @tc.name      : testCreatePictureByPixelmap
     * @tc.desc      : Test create picture by pixelmap with different pixelMapFormat.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_1900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      testPixelMapFormatInPicture(done, testFileJpg, image.PixelMapFormat.NV12,
        `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_1900`);
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_2000
     * @tc.name      : testCreatePictureByPixelmap
     * @tc.desc      : Test create picture by pixelmap with different pixelMapFormat.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_2000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        testPixelMapFormatInPicture(done, testFileHeic, image.PixelMapFormat.NV12,
          `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_CREATEPICTUREBYPIXELMAP_2000`);
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_IMAGESOURCE_SETDESIREDAUXILIARYPICTURES_ERROR_0100
     * @tc.name      : testImageSourceGetDesiredAuxiliaryPictures
     * @tc.desc      : Test get desired AuxiliaryPicture from DecodingOptions. 
     *                 Added AuxiliaryPicture type array length abnormal.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_IMAGESOURCE_SETDESIREDAUXILIARYPICTURES_ERROR_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_IMAGESOURCE_SETDESIREDAUXILIARYPICTURES_ERROR_0100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.CreatePictureDecodingOptions();
        let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP];
        expect(ndkTest.SetDesiredAuxiliaryPictures(0, inAuxTpyes)).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_METADATA_SETPROPERTY_ERROR_0900
     * @tc.name      : testMetadataSetProperty
     * @tc.desc      : Test set property to fragment metadata with illegal value.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_METADATA_SETPROPERTY_ERROR_0900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_METADATA_SETPROPERTY_ERROR_0900`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.FRAGMENT_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(fragmentImageWidthKey,fragmentImageWidthValueErr))
          .assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        expect(ndkTest.MetadataSetPropertyErrorCode(xInOriginalKey, xInOriginalValueErr))
          .assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_METADATA_SETPROPERTY_ERROR_1000
     * @tc.name      : testMetadataSetProperty
     * @tc.desc      : Test set property to fragment metadata with illegal value.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_METADATA_SETPROPERTY_ERROR_1000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_METADATA_SETPROPERTY_ERROR_1000`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        ndkTest.MetadataCreate(image.MetadataType.FRAGMENT_METADATA);
        expect(ndkTest.MetadataSetPropertyErrorCode(fragmentImageHeightKeyErr, fragmentImageHeightValueErr))
          .assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        expect(ndkTest.MetadataSetPropertyErrorCode(yInOriginalKeyErr, yInOriginalValueErr))
          .assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0100
     * @tc.name      : testDecodePictureFromJpg
     * @tc.desc      : Test decode picture from jpg.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0100`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP];
          testCreatePictureByImageSource(caseName, testAllAuxiliaryPicturesExcludeFragmentJpg, inAuxTpyes);

          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.UNREFOCUS_MAP) == undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP) == undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.LINEAR_MAP) == undefined).assertTrue();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0200
     * @tc.name      : testDecodePictureFromJpg
     * @tc.desc      : Test decode picture from jpg.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0200`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.DEPTH_MAP];
          testCreatePictureByImageSource(caseName, testAllAuxiliaryPicturesExcludeFragmentJpg, inAuxTpyes);

          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP) == undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.UNREFOCUS_MAP) == undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.LINEAR_MAP) == undefined).assertTrue();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0300
     * @tc.name      : testDecodePictureFromJpg
     * @tc.desc      : Test decode picture from jpg.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0300`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.LINEAR_MAP];
          testCreatePictureByImageSource(caseName, testAllAuxiliaryPicturesExcludeFragmentJpg, inAuxTpyes);

          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP) == undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP) == undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.UNREFOCUS_MAP) == undefined).assertTrue();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0400
     * @tc.name      : testDecodePictureFromJpg
     * @tc.desc      : Test decode picture from jpg.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0400`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.UNREFOCUS_MAP];
          testCreatePictureByImageSource(caseName, testAllAuxiliaryPicturesExcludeFragmentJpg, inAuxTpyes);

          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP) == undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP) == undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.LINEAR_MAP) == undefined).assertTrue();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0500
     * @tc.name      : testDecodePictureFromJpg
     * @tc.desc      : Test decode picture from jpg.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0500`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.FRAGMENT_MAP];
          testCreatePictureByImageSource(caseName, testFileFragmentJpg, inAuxTpyes);

          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0600
     * @tc.name      : testDecodePictureFromJpg
     * @tc.desc      : Test decode picture from jpg.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0600`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP,
            image.AuxiliaryPictureType.DEPTH_MAP, image.AuxiliaryPictureType.LINEAR_MAP,
            image.AuxiliaryPictureType.UNREFOCUS_MAP, image.AuxiliaryPictureType.FRAGMENT_MAP];
          testCreatePictureByImageSource(caseName, testAllAuxiliaryPicturesExcludeFragmentJpg, inAuxTpyes);

          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[1]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[2]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[3]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[4]) == undefined).assertTrue();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0700
     * @tc.name      : testDecodePictureFromJpg
     * @tc.desc      : Test decode picture from jpg.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0700`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP,
            image.AuxiliaryPictureType.DEPTH_MAP, image.AuxiliaryPictureType.LINEAR_MAP,
            image.AuxiliaryPictureType.UNREFOCUS_MAP, image.AuxiliaryPictureType.FRAGMENT_MAP];
          testCreateImageSource(caseName, testAllAuxiliaryPicturesExcludeFragmentJpg)
          ndkTest.CreatePictureDecodingOptions();
          ndkTest.CreatePictureByImageSource();

          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[1]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[2]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[3]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[4]) == undefined).assertTrue();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0800
     * @tc.name      : testDecodePictureFromJpg
     * @tc.desc      : Test decode picture from jpg.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0800`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP,
            image.AuxiliaryPictureType.DEPTH_MAP, image.AuxiliaryPictureType.LINEAR_MAP,
            image.AuxiliaryPictureType.UNREFOCUS_MAP, image.AuxiliaryPictureType.FRAGMENT_MAP];
          testCreatePictureByImageSource(caseName, testFileAllAuxiliaryPictures, inAuxTpyes);

          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[1]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[2]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[3]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[4]) != undefined).assertTrue();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0900
     * @tc.name      : testDecodePictureFromJpg
     * @tc.desc      : Test decode picture from jpg.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      if (!isSupportHdr) {
        hilog.info(domain, testTag, 'device is not support hdr');
        expect(true).assertTrue();
        done();
      } else {
        let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_0900`;
        hilog.info(domain, testTag, caseName + ` begin`);
        try {
          let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP,
            image.AuxiliaryPictureType.DEPTH_MAP, image.AuxiliaryPictureType.LINEAR_MAP,
            image.AuxiliaryPictureType.UNREFOCUS_MAP, image.AuxiliaryPictureType.FRAGMENT_MAP];
          testCreateImageSource(caseName, testFileAllAuxiliaryPictures)
          ndkTest.CreatePictureDecodingOptions();
          ndkTest.CreatePictureByImageSource();

          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[1]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[2]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[3]) != undefined).assertTrue();
          expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[4]) != undefined).assertTrue();
          done();
        } catch (err) {
          hilog.error(domain, testTag, caseName + `: Test exception ` + err);
          expect().assertFalse();
          done();
        }
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_ERROR_1000
     * @tc.name      : testDecodePictureFromJpg
     * @tc.desc      : Test decode picture from jpg.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_ERROR_1000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_ERROR_1000`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreateImageSource(caseName, testAllAuxiliaryPicturesExcludeFragmentJpg);
        expect(ndkTest.CreatePictureByImageSourceWithDecodingOptionIsNull()).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_ERROR_1100
     * @tc.name      : testDecodePictureFromJpg
     * @tc.desc      : Test decode picture from png.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_ERROR_1100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_JPG_ERROR_1100`;
      hilog.info(domain, testTag, caseName + ` begin`);
      try {
        testCreateImageSource(caseName, testFilePng);
        ndkTest.CreatePictureDecodingOptions();
        expect(ndkTest.CreatePictureByImageSourceErrorCode()).assertEqual(ErrorCode.IMAGE_DECODE_FAILED);
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

        /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0100
     * @tc.name      : testDecodePictureFromHeif
     * @tc.desc      : Test decode picture from heif.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
        it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0100', Level.LEVEL0, async (done: Function) => {
          await sleep(1000);
          if (!isSupportHdr) {
            hilog.info(domain, testTag, 'device is not support hdr');
            expect(true).assertTrue();
            done();
          } else {
            let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0100`;
            hilog.info(domain, testTag, caseName + ` begin`);
            try {
              let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP];
              testCreatePictureByImageSource(caseName, testFileWithFiveAuxiliaryPictureHeic, inAuxTpyes);
    
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.UNREFOCUS_MAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.LINEAR_MAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.FRAGMENT_MAP) == undefined).assertTrue();
              done();
            } catch (err) {
              hilog.error(domain, testTag, caseName + `: Test exception ` + err);
              expect().assertFalse();
              done();
            }
          }
        })
    
        /*
         * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0200
         * @tc.name      : testDecodePictureFromHeif
         * @tc.desc      : Test decode picture from heif.
         * @tc.size      : MediumTest
         * @tc.type      : Function
         * @tc.level     : Level 1
         */
        it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0200', Level.LEVEL0, async (done: Function) => {
          await sleep(1000);
          if (!isSupportHdr) {
            hilog.info(domain, testTag, 'device is not support hdr');
            expect(true).assertTrue();
            done();
          } else {
            let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0200`;
            hilog.info(domain, testTag, caseName + ` begin`);
            try {
              let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.DEPTH_MAP];
              testCreatePictureByImageSource(caseName, testFileWithFiveAuxiliaryPictureHeic, inAuxTpyes);
    
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.UNREFOCUS_MAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.LINEAR_MAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.FRAGMENT_MAP) == undefined).assertTrue();
              done();
            } catch (err) {
              hilog.error(domain, testTag, caseName + `: Test exception ` + err);
              expect().assertFalse();
              done();
            }
          }
        })
    
        /*
         * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0300
         * @tc.name      : testDecodePictureFromHeif
         * @tc.desc      : Test decode picture from heif.
         * @tc.size      : MediumTest
         * @tc.type      : Function
         * @tc.level     : Level 1
         */
        it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0300', Level.LEVEL0, async (done: Function) => {
          await sleep(1000);
          if (!isSupportHdr) {
            hilog.info(domain, testTag, 'device is not support hdr');
            expect(true).assertTrue();
            done();
          } else {
            let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0300`;
            hilog.info(domain, testTag, caseName + ` begin`);
            try {
              let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.LINEAR_MAP];
              testCreatePictureByImageSource(caseName, testFileWithFiveAuxiliaryPictureHeic, inAuxTpyes);
    
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.UNREFOCUS_MAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.FRAGMENT_MAP) == undefined).assertTrue();
              done();
            } catch (err) {
              hilog.error(domain, testTag, caseName + `: Test exception ` + err);
              expect().assertFalse();
              done();
            }
          }
        })
    
        /*
         * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0400
         * @tc.name      : testDecodePictureFromHeif
         * @tc.desc      : Test decode picture from heif.
         * @tc.size      : MediumTest
         * @tc.type      : Function
         * @tc.level     : Level 1
         */
        it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0400', Level.LEVEL0, async (done: Function) => {
          await sleep(1000);
          if (!isSupportHdr) {
            hilog.info(domain, testTag, 'device is not support hdr');
            expect(true).assertTrue();
            done();
          } else {
            let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0400`;
            hilog.info(domain, testTag, caseName + ` begin`);
            try {
              let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.UNREFOCUS_MAP];
              testCreatePictureByImageSource(caseName, testFileWithFiveAuxiliaryPictureHeic, inAuxTpyes);
    
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.LINEAR_MAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.FRAGMENT_MAP) == undefined).assertTrue();
              done();
            } catch (err) {
              hilog.error(domain, testTag, caseName + `: Test exception ` + err);
              expect().assertFalse();
              done();
            }
          }
        })
    
        /*
         * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0500
         * @tc.name      : testDecodePictureFromHeif
         * @tc.desc      : Test decode picture from heif.
         * @tc.size      : MediumTest
         * @tc.type      : Function
         * @tc.level     : Level 1
         */
        it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0500', Level.LEVEL0, async (done: Function) => {
          await sleep(1000);
          if (!isSupportHdr) {
            hilog.info(domain, testTag, 'device is not support hdr');
            expect(true).assertTrue();
            done();
          } else {
            let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0500`;
            hilog.info(domain, testTag, caseName + ` begin`);
            try {
              let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.FRAGMENT_MAP];
              testCreatePictureByImageSource(caseName, testFileWithFiveAuxiliaryPictureHeic, inAuxTpyes);
    
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.DEPTH_MAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.LINEAR_MAP) == undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(image.AuxiliaryPictureType.UNREFOCUS_MAP) == undefined).assertTrue();
              done();
            } catch (err) {
              hilog.error(domain, testTag, caseName + `: Test exception ` + err);
              expect().assertFalse();
              done();
            }
          }
        })
    
        /*
         * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0600
         * @tc.name      : testDecodePictureFromHeif
         * @tc.desc      : Test decode picture from heif.
         * @tc.size      : MediumTest
         * @tc.type      : Function
         * @tc.level     : Level 1
         */
        it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0600', Level.LEVEL0, async (done: Function) => {
          await sleep(1000);
          if (!isSupportHdr) {
            hilog.info(domain, testTag, 'device is not support hdr');
            expect(true).assertTrue();
            done();
          } else {
            let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0600`;
            hilog.info(domain, testTag, caseName + ` begin`);
            try {
              let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP,
                image.AuxiliaryPictureType.DEPTH_MAP, image.AuxiliaryPictureType.LINEAR_MAP,
                image.AuxiliaryPictureType.UNREFOCUS_MAP, image.AuxiliaryPictureType.FRAGMENT_MAP];
              testCreatePictureByImageSource(caseName, testFileWithFiveAuxiliaryPictureHeic, inAuxTpyes);
    
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[1]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[2]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[3]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[4]) != undefined).assertTrue();
              done();
            } catch (err) {
              hilog.error(domain, testTag, caseName + `: Test exception ` + err);
              expect().assertFalse();
              done();
            }
          }
        })
    
        /*
         * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0700
         * @tc.name      : testDecodePictureFromHeif
         * @tc.desc      : Test decode picture from heif.
         * @tc.size      : MediumTest
         * @tc.type      : Function
         * @tc.level     : Level 1
         */
        it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0700', Level.LEVEL0, async (done: Function) => {
          await sleep(1000);
          if (!isSupportHdr) {
            hilog.info(domain, testTag, 'device is not support hdr');
            expect(true).assertTrue();
            done();
          } else {
            let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_0700`;
            hilog.info(domain, testTag, caseName + ` begin`);
            try {
              let inAuxTpyes: Array<image.AuxiliaryPictureType> = [image.AuxiliaryPictureType.GAINMAP,
                image.AuxiliaryPictureType.DEPTH_MAP, image.AuxiliaryPictureType.LINEAR_MAP,
                image.AuxiliaryPictureType.UNREFOCUS_MAP, image.AuxiliaryPictureType.FRAGMENT_MAP];
              testCreateImageSource(caseName, testFileWithFiveAuxiliaryPictureHeic)
              ndkTest.CreatePictureDecodingOptions();
              ndkTest.CreatePictureByImageSource();
    
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[0]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[1]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[2]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[3]) != undefined).assertTrue();
              expect(ndkTest.GetAuxiliaryPicture(inAuxTpyes[4]) != undefined).assertTrue();
              done();
            } catch (err) {
              hilog.error(domain, testTag, caseName + `: Test exception ` + err);
              expect().assertFalse();
              done();
            }
          }
        })
    
        /*
         * @tc.number    : SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_ERROR_0800
         * @tc.name      : testDecodePictureFromHeif
         * @tc.desc      : Test decode picture from heif.
         * @tc.size      : MediumTest
         * @tc.type      : Function
         * @tc.level     : Level 2
         */
        it('SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_ERROR_0800', Level.LEVEL0, async (done: Function) => {
          if (!isSupportHdr) {
            hilog.info(domain, testTag, 'device is not support hdr');
            expect(true).assertTrue();
            done();
          } else {
            await sleep(1000);
            let caseName: string = `SUB_MULTIMEDIA_PICTURE_NATIVE_PICTURE_DECODE_HEIF_ERROR_0800`;
            hilog.info(domain, testTag, caseName + ` begin`);
            try {
              testCreateImageSource(caseName, testFileWithFiveAuxiliaryPictureHeic);
              expect(ndkTest.CreatePictureByImageSourceWithDecodingOptionIsNull()).assertEqual(ErrorCode.IMAGE_BAD_PARAMETER);
              done();
            } catch (err) {
              hilog.error(domain, testTag, caseName + `: Test exception ` + err);
              expect().assertFalse();
              done();
            }
          }
        })

    /*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00100`;
      let key = 'NewSubfileType'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "3"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "3"},
        {name: "png", fileName: "test_exif_v2.png", value: "3"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "3"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00200`;
      let key = 'SubfileType'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2"},
        {name: "png", fileName: "test_exif_v2.png", value: "2"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00300`;
      let key = 'ImageWidth'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "3456"},
        {name: "png", fileName: "test_exif_v2.png", value: "0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00400`;
      let key = 'ImageLength'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "4608"},
        {name: "png", fileName: "test_exif_v2.png", value: "0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00500`;
      let key = 'BitsPerSample'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name:"jpeg", fileName:"test_exif_v2.jpg", value:"1, 1, 1"},
        {name:"webp", fileName:"test_exif_v2.webp", value:"8, 8, 8"},
        {name:"png", fileName:"test_exif_v2.png", value:"1, 1, 1"},
        {name:"heif", fileName:"test_exif_v2.heic", value:"1, 1, 1"}
      ]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00600`;
      let key = 'Compression'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Uncompressed"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Uncompressed"},
        {name: "png", fileName: "test_exif_v2.png", value: "Uncompressed"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Uncompressed"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00700`;
      let key = 'PhotometricInterpretation'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Reversed mono"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Reversed mono"},
        {name: "png", fileName: "test_exif_v2.png", value: "Reversed mono"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Reversed mono"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00800`;
      let key = 'ImageDescription'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "_cuva%s\\d"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "cuva"},
        {name: "png", fileName: "test_exif_v2.png", value: "_cuva%s\\d"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "_cuva%s\\d"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_00900`;
      let key = 'Make'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "华为"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "华为"},
        {name: "png", fileName: "test_exif_v2.png", value: "华为"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "华为"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01000`;
      let key = 'Model'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [
        {name: "jpeg", fileName: "test_exif_v2.jpg", value: "TNY-AL00"},
        {name: "webp", fileName: "test_webp_exif.webp", value: "TNY-AL00"},
        {name: "png", fileName: "test_exif_v2.png", value: "TNY-AL00"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "TNY-AL00"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01100`;
      let key = 'StripOffsets'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "123"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "123"},
        {name: "png", fileName: "test_exif_v2.png", value: "123"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "123"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01200`;
      let key = 'Orientation'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Top-left"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Top-right"},
        {name: "png", fileName: "test_exif_v2.png", value: "Top-left"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Top-left"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01300`;
      let key = 'SamplesPerPixel'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "23"},
        {name: "png", fileName: "test_exif_v2.png", value: "0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01400`;
      let key = 'RowsPerStrip'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "123"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "123"},
        {name: "png", fileName: "test_exif_v2.png", value: "123"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "123"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01500`;
      let key = 'StripByteCounts'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "123"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "123"},
        {name: "png", fileName: "test_exif_v2.png", value: "123"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "123"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01600`;
      let key = 'XResolution'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: " 0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "72"},
        {name: "png", fileName: "test_exif_v2.png", value: " 0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: " 0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01700`;
      let key = 'YResolution'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: " 0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "72"},
        {name: "png", fileName: "test_exif_v2.png", value: " 0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: " 0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01800`;
      let key = 'PlanarConfiguration'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Chunky format"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Chunky format"},
        {name: "png", fileName: "test_exif_v2.png", value: "Chunky format"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Chunky format"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_01900`;
      let key = 'ResolutionUnit'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Inch"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Inch"},
        {name: "png", fileName: "test_exif_v2.png", value: "Inch"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Inch"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02000`;
      let key = 'TransferFunction'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [
        {name: "jpeg", fileName: "test_exif_v2.jpg", value: "3 bytes undefined data"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1 bytes undefined data"},
        {name: "png", fileName: "test_exif_v2.png", value: "3 bytes undefined data"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "3 bytes undefined data"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02100`;
      let key = 'Software'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "华为"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "TNY-AL00"},
        {name: "png", fileName: "test_exif_v2.png", value: "华为"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "华为"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02200`;
      let key = 'DateTime'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2024:12:31"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2022:06:02 15:51:35"},
        {name: "png", fileName: "test_exif_v2.png", value: "2024:12:31"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2024:12:31"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02300`;
      let key = 'Artist'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "None"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Bogdan Hrastnik"},
        {name: "png", fileName: "test_exif_v2.png", value: "None"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "None"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02400`;
      let key = 'WhitePoint'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "252, 0/0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "124, 0/0"},
        {name: "png", fileName: "test_exif_v2.png", value: "252, 0/0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "252, 0/0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02500`;
      let key = 'PrimaryChromaticities'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: " 0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "124"},
        {name: "png", fileName: "test_exif_v2.png", value: " 0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: " 0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02600`;
      let key = 'PhotoMode'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [
        {name: "jpeg", fileName: "test_exif_v2.jpg", value: "252"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "123"},
        {name: "png", fileName: "test_exif_v2.png", value: "252"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "252"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02700`;
      let key = 'JPEGInterchangeFormat'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2798"},
        {name: "webp", fileName: "test_webp_exif.webp", value: "1252"},
        {name: "png", fileName: "test_exif_v2.png", value: "2798"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2798"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02800`;
      let key = 'JPEGInterchangeFormatLength'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "4"},
        {name: "webp", fileName: "test_webp_exif.webp", value: "24042"},
        {name: "png", fileName: "test_exif_v2.png", value: "4"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "4"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_02900`;
      let key = 'YCbCrCoefficients'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "0.299, 0.587, 0.114"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "0.299, 0.587, 0.114"},
        {name: "png", fileName: "test_exif_v2.png", value: "0.299, 0.587, 0.114"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "0.299, 0.587, 0.114"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03000`;
      let key = 'YCbCrSubSampling'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "3, 2"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "3, 3"},
        {name: "png", fileName: "test_exif_v2.png", value: "3, 2"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "3, 2"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03100`;
      let key = 'YCbCrPositioning'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Centered"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Centered"},
        {name: "png", fileName: "test_exif_v2.png", value: "Centered"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Centered"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03200`;
      let key = 'ReferenceBlackWhite'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [
        {name: "jpeg", fileName: "test_exif_v2.jpg", value: "222,  0, 1.5,  0, 25.2, 25.2"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "221, 255,  0, 255,  0, 255"},
        {name: "png", fileName: "test_exif_v2.png", value: "222,  0, 1.5,  0, 25.2, 25.2"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "222,  0, 1.5,  0, 25.2, 25.2"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03300`;
      let key = 'Copyright'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [
        {name: "jpeg", fileName: "test_exif_v2.jpg", value: "undefined (Photographer) - [None] (Editor)"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "ABB (Photographer) - [None] (Editor)"},
        {name: "png", fileName: "test_exif_v2.png", value: "undefined (Photographer) - [None] (Editor)"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "undefined (Photographer) - [None] (Editor)"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03400`;
      let key = 'ExposureTime'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1/34 sec."},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1/33 sec."},
        {name: "png", fileName: "test_exif_v2.png", value: "1/34 sec."},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1/34 sec."}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03500`;
      let key = 'FNumber'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "f/1.0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "f/1.8"},
        {name: "png", fileName: "test_exif_v2.png", value: "f/1.0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "f/1.0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03600`;
      let key = 'ExposureProgram'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Not defined"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Normal program"},
        {name: "png", fileName: "test_exif_v2.png", value: "Not defined"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Not defined"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03700`;
      let key = 'SpectralSensitivity'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "abc"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "sensitivity"},
        {name: "png", fileName: "test_exif_v2.png", value: "abc"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "abc"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03800`;
      let key = 'GPSVersionID'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2.2.0.0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2.2.0.0"},
        {name: "png", fileName: "test_exif_v2.png", value: "2.2.0.0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2.2.0.0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_03900`;
      let key = 'GPSLatitudeRef'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "N"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "N"},
        {name: "png", fileName: "test_exif_v2.png", value: "N"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "N"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04000`;
      let key = 'GPSLatitude'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "39, 54, 20"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "39, 54, 20"},
        {name: "png", fileName: "test_exif_v2.png", value: "39, 54, 20"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "39, 54, 20"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04100`;
      let key = 'GPSLongitudeRef'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "E"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "W"},
        {name: "png", fileName: "test_exif_v2.png", value: "E"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "E"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04200`;
      let key = 'GPSLongitude'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "120, 52, 26"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "120, 52, 26"},
        {name: "png", fileName: "test_exif_v2.png", value: "120, 52, 26"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "120, 52, 26"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04300`;
      let key = 'GPSAltitudeRef'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Sea level"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Sea level reference"},
        {name: "png", fileName: "test_exif_v2.png", value: "Sea level"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Sea level"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04400`;
      let key = 'GPSAltitude'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "0.00"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "0.00"},
        {name: "png", fileName: "test_exif_v2.png", value: "0.00"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "0.00"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04500`;
      let key = 'GPSTimeStamp'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "11:37:58.00"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "11:37:58.00"},
        {name: "png", fileName: "test_exif_v2.png", value: "11:37:58.00"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "11:37:58.00"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04600`;
      let key = 'GPSSatellites'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "xxx"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "BBA"},
        {name: "png", fileName: "test_exif_v2.png", value: "xxx"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "xxx"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04700`;
      let key = 'GPSStatus'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "A"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "A"},
        {name: "png", fileName: "test_exif_v2.png", value: "A"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "A"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04800`;
      let key = 'GPSMeasureMode'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2"},
        {name: "png", fileName: "test_exif_v2.png", value: "2"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_04900`;
      let key = 'GPSDOP'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "182"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "182"},
        {name: "png", fileName: "test_exif_v2.png", value: "182"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "182"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05000`;
      let key = 'GPSSpeedRef'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "N"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "K"},
        {name: "png", fileName: "test_exif_v2.png", value: "N"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "N"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05100`;
      let key = 'GPSSpeed'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "150"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "150"},
        {name: "png", fileName: "test_exif_v2.png", value: "150"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "150"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05200`;
      let key = 'GPSTrackRef'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "T"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "T"},
        {name: "png", fileName: "test_exif_v2.png", value: "T"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "T"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05300`;
      let key = 'GPSTrack'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "38.0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "111"},
        {name: "png", fileName: "test_exif_v2.png", value: "38.0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "38.0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05400`;
      let key = 'GPSImgDirectionRef'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "M"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "M"},
        {name: "png", fileName: "test_exif_v2.png", value: "M"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "M"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05500`;
      let key = 'GPSImgDirection'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2.23"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2.23"},
        {name: "png", fileName: "test_exif_v2.png", value: "2.23"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2.23"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05600`;
      let key = 'GPSMapDatum'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "xxx"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "TEST"},
        {name: "png", fileName: "test_exif_v2.png", value: "xxx"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "xxx"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05700`;
      let key = 'GPSDestLatitudeRef'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "N"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "S"},
        {name: "png", fileName: "test_exif_v2.png", value: "N"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "N"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05800`;
      let key = 'GPSDestLatitude'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "33, 22, 11"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "33, 22, 11"},
        {name: "png", fileName: "test_exif_v2.png", value: "33, 22, 11"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "33, 22, 11"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_05900`;
      let key = 'GPSDestLongitudeRef'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "E"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "W"},
        {name: "png", fileName: "test_exif_v2.png", value: "E"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "E"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06000`;
      let key = 'GPSDestLongitude'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "33, 22, 11"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "33, 22, 11"},
        {name: "png", fileName: "test_exif_v2.png", value: "33, 22, 11"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "33, 22, 11"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06100`;
      let key = 'GPSDestBearingRef'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "T"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "M"},
        {name: "png", fileName: "test_exif_v2.png", value: "T"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "T"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06200`;
      let key = 'GPSDestBearing'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2.0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2.0"},
        {name: "png", fileName: "test_exif_v2.png", value: "2.0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2.0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06300`;
      let key = 'GPSDestDistanceRef'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
       const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "N"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "K"},
        {name: "png", fileName: "test_exif_v2.png", value: "N"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "N"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06400`;
      let key = 'GPSDestDistance'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "10"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "10"},
        {name: "png", fileName: "test_exif_v2.png", value: "10"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "10"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06500`;
      let key = 'GPSProcessingMethod'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "CELLID"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "CELLID"},
        {name: "png", fileName: "test_exif_v2.png", value: "CELLID"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "CELLID"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06600`;
      let key = 'GPSAreaInformation'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "arexxx"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "accxx"},
        {name: "png", fileName: "test_exif_v2.png", value: "arexxx"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "arexxx"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06700`;
      let key = 'GPSDateStamp'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2025:01:11"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2022:01:11"},
        {name: "png", fileName: "test_exif_v2.png", value: "2025:01:11"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2025:01:11"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06800`;
      let key = 'GPSDifferential'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1"},
        {name: "png", fileName: "test_exif_v2.png", value: "0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_06900`;
      let key = 'GPSHPositioningError'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2.5"},
        {name: "webp", fileName: "test_exif_v2.webp", value: " 2"},
        {name: "png", fileName: "test_exif_v2.png", value: "2.5"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2.5"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07000`;
      let key = 'ISOSpeedRatings'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "200"},
        {name: "png", fileName: "test_exif_v2.png", value: "1"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07100`;
      let key = 'PhotographicSensitivity'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "200"},
        {name: "png", fileName: "test_exif_v2.png", value: "1"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07200`;
      let key = 'OECF'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "4 bytes undefined data"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2 bytes undefined data"},
        {name: "png", fileName: "test_exif_v2.png", value: "4 bytes undefined data"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "4 bytes undefined data"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07300`;
      let key = 'SensitivityType'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [
        {name: "jpeg", fileName: "test_exif_v2.jpg", value: "Standard output sensitivity (SOS) and ISO speed"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Standard output sensitivity (SOS) and ISO speed"},
        {name: "png", fileName: "test_exif_v2.png", value: "Standard output sensitivity (SOS) and ISO speed"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Standard output sensitivity (SOS) and ISO speed"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07400`;
      let key = 'StandardOutputSensitivity'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "5"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "5"},
        {name: "png", fileName: "test_exif_v2.png", value: "5"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "5"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07500`;
      let key = 'RecommendedExposureIndex'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "123"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "241"},
        {name: "png", fileName: "test_exif_v2.png", value: "123"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "123"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07600`;
      let key = 'ISOSpeedRatings'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "200"},
        {name: "png", fileName: "test_exif_v2.png", value: "1"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07700`;
      let key = 'ISOSpeedLatitudeyyy'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1456"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "3"},
        {name: "png", fileName: "test_exif_v2.png", value: "1456"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1456"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07800`;
      let key = 'ISOSpeedLatitudezzz'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1456"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "3"},
        {name: "png", fileName: "test_exif_v2.png", value: "1456"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1456"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_07900`;
      let key = 'ExifVersion'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Exif Version 1.1"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Exif Version 1.1"},
        {name: "png", fileName: "test_exif_v2.png", value: "Exif Version 1.1"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Exif Version 1.1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08000`;
      let key = 'DateTimeOriginal'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2024:01:25 05:51:34"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2022:06:02 15:51:35"},
        {name: "png", fileName: "test_exif_v2.png", value: "2024:01:25 05:51:34"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2024:01:25 05:51:34"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08100`;
      let key = 'DateTimeDigitized'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2022:06:02 15:51:34"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2022:06:02 15:51:35"},
        {name: "png", fileName: "test_exif_v2.png", value: "2022:06:02 15:51:34"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2022:06:02 15:51:34"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08200`;
      let key = 'OffsetTime'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2024:01:25"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "111"},
        {name: "png", fileName: "test_exif_v2.png", value: "2024:01:25"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2024:01:25"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08300`;
      let key = 'OffsetTimeOriginal'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "chex"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "00xx"},
        {name: "png", fileName: "test_exif_v2.png", value: "chex"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "chex"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08400`;
      let key = 'OffsetTimeDigitized'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "cfh"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "abs"},
        {name: "png", fileName: "test_exif_v2.png", value: "cfh"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "cfh"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08500`;
      let key = 'ComponentsConfiguration'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Y R G B"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Y R G B"},
        {name: "png", fileName: "test_exif_v2.png", value: "Y R G B"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Y R G B"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08600`;
      let key = 'CompressedBitsPerPixel'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "24"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "0.95"},
        {name: "png", fileName: "test_exif_v2.png", value: "24"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "24"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08700`;
      let key = 'ShutterSpeedValue'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2.50 EV (1/6 sec.)"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "14.00 EV (1/16384 sec.)"},
        {name: "png", fileName: "test_exif_v2.png", value: "2.50 EV (1/6 sec.)"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2.50 EV (1/6 sec.)"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08800`;
      let key = 'ApertureValue'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "4.00 EV (f/4.0)"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1.69 EV (f/1.8)"},
        {name: "png", fileName: "test_exif_v2.png", value: "4.00 EV (f/4.0)"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "4.00 EV (f/4.0)"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_08900`;
      let key = 'BrightnessValue'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2.50 EV (19.38 cd/m^2)"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "0.00 EV (3.43 cd/m^2)"},
        {name: "png", fileName: "test_exif_v2.png", value: "2.50 EV (19.38 cd/m^2)"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2.50 EV (19.38 cd/m^2)"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09000`;
      let key = 'ExposureBiasValue'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "23.00 EV"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "0.00 EV"},
        {name: "png", fileName: "test_exif_v2.png", value: "23.00 EV"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "23.00 EV"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09100`;
      let key = 'MaxApertureValue'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2.50 EV (f/2.4)"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1.69 EV (f/1.8)"},
        {name: "png", fileName: "test_exif_v2.png", value: "2.50 EV (f/2.4)"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2.50 EV (f/2.4)"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09200`;
      let key = 'SubjectDistance'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2.5 m"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "25.0 m"},
        {name: "png", fileName: "test_exif_v2.png", value: "2.5 m"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2.5 m"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09300`;
      let key = 'MeteringMode'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Pattern"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Pattern"},
        {name: "png", fileName: "test_exif_v2.png", value: "Pattern"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Pattern"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09400`;
      let key = 'LightSource'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Daylight"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Daylight"},
        {name: "png", fileName: "test_exif_v2.png", value: "Daylight"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Daylight"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09500`;
      let key = 'Flash'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [
        {name: "jpeg", fileName: "test_exif_v2.jpg", value: "Strobe return light not detected"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Flash did not fire, auto mode"},
        {name: "png", fileName: "test_exif_v2.png", value: "Strobe return light not detected"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Strobe return light not detected"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09600`;
      let key = 'FocalLength'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "0.0 mm"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "31.0 mm"},
        {name: "png", fileName: "test_exif_v2.png", value: "0.0 mm"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "0.0 mm"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09700`;
      let key = 'SubjectArea'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [
        {name: "jpeg", fileName: "test_exif_v2.jpg", value: "(x,y) = (12,13)"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "(x,y) = (10,20)"},
        {name: "png", fileName: "test_exif_v2.png", value: "(x,y) = (12,13)"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "(x,y) = (12,13)"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09800`;
      let key = 'MakerNote'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg",
        value: 'HwMnoteCaptureMode:1,HwMnoteBurstNumber:2,HwMnoteFrontCamera:3,HwMnoteRollAngle:4,' +
          'HwMnotePitchAngle:5,HwMnotePhysicalAperture:6,HwMnoteFocusMode:7,HwMnoteFacePointer:122,' +
          'HwMnoteFaceVersion:1,HwMnoteFaceCount:2,HwMnoteFaceConf:3,HwMnoteFaceSmileScore:1 2 3 4 5 6 7 8,' +
          'HwMnoteFaceRect:1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8,HwMnoteFaceLeyeCenter:1 2 3 4,HwMnoteFaceReyeCenter:5 6 7 8,' +
          'HwMnoteFaceMouthCenter:1 2 3 4 5 6 7 8,HwMnoteScenePointer:256,HwMnoteSceneVersion:1,HwMnoteSceneFoodConf:2,' +
          'HwMnoteSceneStageConf:3,HwMnoteSceneBlueSkyConf:4,HwMnoteSceneGreenPlantConf:5,HwMnoteSceneBeachConf:6,' +
          'HwMnoteSceneSnowConf:7,HwMnoteSceneSunsetConf:8,HwMnoteSceneFlowersConf:9,HwMnoteSceneNightConf:10,' +
          'HwMnoteSceneTextConf:11'},
        {name: "webp", fileName: "test_exif_v2.webp", value: "HwMnoteCaptureMode:1,HwMnoteBurstNumber:2," +
          "HwMnoteFrontCamera:3,HwMnoteRollAngle:4,HwMnotePitchAngle:5,HwMnotePhysicalAperture:6,HwMnoteFocusMode:7," +
          "HwMnoteFacePointer:122,HwMnoteFaceVersion:1,HwMnoteFaceCount:2,HwMnoteFaceConf:3," +
          "HwMnoteFaceSmileScore:1 2 3 4 5 6 7 8,HwMnoteFaceRect:1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8," +
          "HwMnoteFaceLeyeCenter:1 2 3 4,HwMnoteFaceReyeCenter:5 6 7 8,HwMnoteFaceMouthCenter:1 2 3 4 5 6 7 8," +
          "HwMnoteScenePointer:256,HwMnoteSceneVersion:1,HwMnoteSceneFoodConf:2,HwMnoteSceneStageConf:3," +
          "HwMnoteSceneBlueSkyConf:4,HwMnoteSceneGreenPlantConf:5,HwMnoteSceneBeachConf:6,HwMnoteSceneSnowConf:7," +
          "HwMnoteSceneSunsetConf:8,HwMnoteSceneFlowersConf:9,HwMnoteSceneNightConf:10,HwMnoteSceneTextConf:11"},
        {name: "png", fileName: "test_exif_v2.png", value: 'HwMnoteCaptureMode:1,HwMnoteBurstNumber:2,' +
          'HwMnoteFrontCamera:3,HwMnoteRollAngle:4,HwMnotePitchAngle:5,HwMnotePhysicalAperture:6,' +
          'HwMnoteFocusMode:7,HwMnoteFacePointer:122,' +
          'HwMnoteFaceVersion:1,HwMnoteFaceCount:2,HwMnoteFaceConf:3,HwMnoteFaceSmileScore:1 2 3 4 5 6 7 8,' +
          'HwMnoteFaceRect:1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8,HwMnoteFaceLeyeCenter:1 2 3 4,' +
          'HwMnoteFaceReyeCenter:5 6 7 8,' + 'HwMnoteFaceMouthCenter:1 2 3 4 5 6 7 8,HwMnoteScenePointer:256,' +
          'HwMnoteSceneVersion:1,HwMnoteSceneFoodConf:2,' +
          'HwMnoteSceneStageConf:3,HwMnoteSceneBlueSkyConf:4,HwMnoteSceneGreenPlantConf:5,HwMnoteSceneBeachConf:6,' +
          'HwMnoteSceneSnowConf:7,HwMnoteSceneSunsetConf:8,HwMnoteSceneFlowersConf:9,HwMnoteSceneNightConf:10,' +
          'HwMnoteSceneTextConf:11'},
        {name: "heif", fileName: "test_exif_v2.heic", value: 'HwMnoteCaptureMode:1,HwMnoteBurstNumber:2,' +
          'HwMnoteFrontCamera:3,HwMnoteRollAngle:4,' + 'HwMnotePitchAngle:5,HwMnotePhysicalAperture:6,' +
          'HwMnoteFocusMode:7,HwMnoteFacePointer:122,' +
          'HwMnoteFaceVersion:1,HwMnoteFaceCount:2,HwMnoteFaceConf:3,HwMnoteFaceSmileScore:1 2 3 4 5 6 7 8,' +
          'HwMnoteFaceRect:1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8,HwMnoteFaceLeyeCenter:1 2 3 4,' +
          'HwMnoteFaceReyeCenter:5 6 7 8,' + 'HwMnoteFaceMouthCenter:1 2 3 4 5 6 7 8,' +
          'HwMnoteScenePointer:256,HwMnoteSceneVersion:1,HwMnoteSceneFoodConf:2,' + 'HwMnoteSceneStageConf:3,' +
          'HwMnoteSceneBlueSkyConf:4,HwMnoteSceneGreenPlantConf:5,HwMnoteSceneBeachConf:6,' +
          'HwMnoteSceneSnowConf:7,HwMnoteSceneSunsetConf:8,HwMnoteSceneFlowersConf:9,HwMnoteSceneNightConf:10,' +
          'HwMnoteSceneTextConf:11'}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_09900`;
      let key = 'HwMnoteScenePointer'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "256"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "256"},
        {name: "png", fileName: "test_exif_v2.png", value: "256"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "256"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10000`;
      let key = 'HwMnoteSceneVersion'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1"},
        {name: "png", fileName: "test_exif_v2.png", value: "1"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10100`;
      let key = 'HwMnoteSceneFoodConf'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2"},
        {name: "png", fileName: "test_exif_v2.png", value: "2"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10200`;
      let key = 'HwMnoteSceneStageConf'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "3"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "3"},
        {name: "png", fileName: "test_exif_v2.png", value: "3"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "3"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10300`;
      let key = 'HwMnoteSceneBlueSkyConf'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "4"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "4"},
        {name: "png", fileName: "test_exif_v2.png", value: "4"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "4"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10400`;
      let key = 'HwMnoteSceneGreenPlantConf'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "5"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "5"},
        {name: "png", fileName: "test_exif_v2.png", value: "5"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "5"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10500`;
      let key = 'HwMnoteSceneBeachConf'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "6"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "6"},
        {name: "png", fileName: "test_exif_v2.png", value: "6"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "6"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10600`;
      let key = 'HwMnoteSceneSnowConf'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "7"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "7"},
        {name: "png", fileName: "test_exif_v2.png", value: "7"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "7"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10700`;
      let key = 'HwMnoteSceneSunsetConf'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "8"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "8"},
        {name: "png", fileName: "test_exif_v2.png", value: "8"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "8"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10800`;
      let key = 'HwMnoteSceneFlowersConf'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "9"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "9"},
        {name: "png", fileName: "test_exif_v2.png", value: "9"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "9"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_10900`;
      let key = 'HwMnoteSceneNightConf'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "10"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "10"},
        {name: "png", fileName: "test_exif_v2.png", value: "10"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "10"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11000`;
      let key = 'HwMnoteSceneTextConf'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "11"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "11"},
        {name: "png", fileName: "test_exif_v2.png", value: "11"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "11"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11100`;
      let key = 'HwMnoteFacePointer'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "122"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "122"},
        {name: "png", fileName: "test_exif_v2.png", value: "122"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "122"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11200`;
      let key = 'HwMnoteFaceVersion'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1"},
        {name: "png", fileName: "test_exif_v2.png", value: "1"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11300`;
      let key = 'HwMnoteFaceCount'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2"},
        {name: "png", fileName: "test_exif_v2.png", value: "2"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11400`;
      let key = 'HwMnoteFaceConf'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "3"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "3"},
        {name: "png", fileName: "test_exif_v2.png", value: "3"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "3"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11500`;
      let key = 'HwMnoteFaceSmileScore'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1 2 3 4 5 6 7 8"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1 2 3 4 5 6 7 8"},
        {name: "png", fileName: "test_exif_v2.png", value: "1 2 3 4 5 6 7 8"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1 2 3 4 5 6 7 8"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11600`;
      let key = 'HwMnoteFaceRect'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [
        {name: "jpeg", fileName: "test_exif_v2.jpg", value: "1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8"},
        {name: "png", fileName: "test_exif_v2.png", value: "1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11700`;
      let key = 'HwMnoteFaceLeyeCenter'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1 2 3 4"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1 2 3 4"},
        {name: "png", fileName: "test_exif_v2.png", value: "1 2 3 4"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1 2 3 4"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11800`;
      let key = 'HwMnoteFaceReyeCenter'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "5 6 7 8"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "5 6 7 8"},
        {name: "png", fileName: "test_exif_v2.png", value: "5 6 7 8"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "5 6 7 8"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_11900`;
      let key = 'HwMnoteFaceMouthCenter'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [
        {name: "jpeg", fileName: "test_exif_v2.jpg", errorData: "987654321", value: '1 2 3 4 5 6 7 8'},
        {name: "png", fileName: "test_exif_v2.png", errorData: "987654321", value: '1 2 3 4 5 6 7 8'},
        {name: "heif", fileName: "test_exif_v2.heic", errorData: "987654321", value: '1 2 3 4 5 6 7 8'},
        {name: "webp", fileName: "test_exif_v2.webp", errorData: "987654321", value: '1 2 3 4 5 6 7 8'}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12000`;
      let key = 'HwMnoteCaptureMode'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1"},
        {name: "png", fileName: "test_exif_v2.png", value: "1"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12100`;
      let key = 'HwMnoteBurstNumber'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
       const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2"},
        {name: "png", fileName: "test_exif_v2.png", value: "2"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12200`;
      let key = 'HwMnoteFrontCamera'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "3"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "3"},
        {name: "png", fileName: "test_exif_v2.png", value: "3"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "3"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12300`;
      let key = 'HwMnoteRollAngle'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "4"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "4"},
        {name: "png", fileName: "test_exif_v2.png", value: "4"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "4"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12400`;
      let key = 'HwMnotePitchAngle'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "5"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "5"},
        {name: "png", fileName: "test_exif_v2.png", value: "5"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "5"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12500`;
      let key = 'HwMnotePhysicalAperture'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "6"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "6"},
        {name: "png", fileName: "test_exif_v2.png", value: "6"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "6"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12600`;
      let key = 'HwMnoteFocusMode'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "7"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "7"},
        {name: "png", fileName: "test_exif_v2.png", value: "7"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "7"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12700`;
      let key = 'UserComment'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "place for user comments"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "place for user comments."},
        {name: "png", fileName: "test_exif_v2.png", value: "place for user comments"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "place for user comments"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12800`;
      let key = 'SubsecTime'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "123456"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "123456"},
        {name: "png", fileName: "test_exif_v2.png", value: "123456"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "123456"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_12900`;
      let key = 'SubsecTimeOriginal'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "427000"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "543792"},
        {name: "png", fileName: "test_exif_v2.png", value: "427000"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "427000"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13000`;
      let key = 'SubsecTimeDigitized'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "427000"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "427000"},
        {name: "png", fileName: "test_exif_v2.png", value: "427000"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "427000"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13100`;
      let key = 'FlashpixVersion'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "FlashPix Version 1.0"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "FlashPix Version 1.0"},
        {name: "png", fileName: "test_exif_v2.png", value: "FlashPix Version 1.0"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "FlashPix Version 1.0"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13200`;
      let key = 'ColorSpace'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "sRGB"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "sRGB"},
        {name: "png", fileName: "test_exif_v2.png", value: "sRGB"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "sRGB"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13300`;
      let key = 'PixelXDimension'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "123"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "123"},
        {name: "png", fileName: "test_exif_v2.png", value: "123"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "123"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13400`;
      let key = 'PixelYDimension'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "234"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "4608"},
        {name: "png", fileName: "test_exif_v2.png", value: "234"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "234"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13500`;
      let key = 'RelatedSoundFile'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "/usr/home"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "abb"},
        {name: "png", fileName: "test_exif_v2.png", value: "/usr/home"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "/usr/home"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13600`;
      let key = 'FlashEnergy'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "832"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "832"},
        {name: "png", fileName: "test_exif_v2.png", value: "832"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "832"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13700`;
      let key = 'SpatialFrequencyResponse'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "13"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "13"},
        {name: "png", fileName: "test_exif_v2.png", value: "13"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "13"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13800`;
      let key = 'FocalPlaneXResolution'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1080"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1080"},
        {name: "png", fileName: "test_exif_v2.png", value: "1080"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1080"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_13900`;
      let key = 'FocalPlaneYResolution'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "880"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "880"},
        {name: "png", fileName: "test_exif_v2.png", value: "880"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "880"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14000`;
      let key = 'FocalPlaneResolutionUnit'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Inch"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Centimeter"},
        {name: "png", fileName: "test_exif_v2.png", value: "Inch"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Inch"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14100`;
      let key = 'SubjectLocation'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "0, 1"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "5, 6"},
        {name: "png", fileName: "test_exif_v2.png", value: "0, 1"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "0, 1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14200`;
      let key = 'ExposureIndex'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1.5"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1.5"},
        {name: "png", fileName: "test_exif_v2.png", value: "1.5"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1.5"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14300`;
      let key = 'SensingMethod'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [
        {name: "jpeg", fileName: "test_exif_v2.jpg", value: "Two-chip color area sensor"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "One-chip color area sensor"},
        {name: "png", fileName: "test_exif_v2.png", value: "Two-chip color area sensor"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Two-chip color area sensor"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14400`;
      let key = 'FileSource'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "DSC"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "DSC"},
        {name: "png", fileName: "test_exif_v2.png", value: "DSC"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "DSC"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14500`;
      let key = 'SceneType'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Directly photographed"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Directly photographed"},
        {name: "png", fileName: "test_exif_v2.png", value: "Directly photographed"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Directly photographed"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14600`;
      let key = 'CFAPattern'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "1 bytes undefined data"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1 bytes undefined data"},
        {name: "png", fileName: "test_exif_v2.png", value: "1 bytes undefined data"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "1 bytes undefined data"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14700`;
      let key = 'CustomRendered'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Normal process"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Custom process"},
        {name: "png", fileName: "test_exif_v2.png", value: "Normal process"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Normal process"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14800`;
      let key = 'ExposureMode'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Auto exposure"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Auto exposure"},
        {name: "png", fileName: "test_exif_v2.png", value: "Auto exposure"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Auto exposure"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_14900`;
      let key = 'WhiteBalance'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Manual white balance"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Auto white balance"},
        {name: "png", fileName: "test_exif_v2.png", value: "Manual white balance"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Manual white balance"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15000`;
      let key = 'DigitalZoomRatio'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "321"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1.00"},
        {name: "png", fileName: "test_exif_v2.png", value: "321"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "321"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15100`;
      let key = 'FocalLengthIn35mmFilm'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "27"},
        {name: "png", fileName: "test_exif_v2.png", value: "2"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15200`;
      let key = 'SceneCaptureType'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Standard"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Standard"},
        {name: "png", fileName: "test_exif_v2.png", value: "Standard"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Standard"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15300`;
      let key = 'GainControl'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Normal"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Normal"},
        {name: "png", fileName: "test_exif_v2.png", value: "Normal"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Normal"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15400`;
      let key = 'Contrast'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Normal"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Normal"},
        {name: "png", fileName: "test_exif_v2.png", value: "Normal"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Normal"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15500`;
      let key = 'Saturation'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Normal"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Normal"},
        {name: "png", fileName: "test_exif_v2.png", value: "Normal"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Normal"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15600`;
      let key = 'Sharpness'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Normal"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Normal"},
        {name: "png", fileName: "test_exif_v2.png", value: "Normal"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Normal"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15700`;
      let key = 'DeviceSettingDescription'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2xxx"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2"},
        {name: "png", fileName: "test_exif_v2.png", value: "2xxx"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2xxx"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15800`;
      let key = 'SubjectDistanceRange'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "Unknown"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "Unknown"},
        {name: "png", fileName: "test_exif_v2.png", value: "Unknown"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "Unknown"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_15900`;
      let key = 'ImageUniqueID'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "FXIC012"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "FXIC012"},
        {name: "png", fileName: "test_exif_v2.png", value: "FXIC012"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "FXIC012"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16000`;
      let key = 'CameraOwnerName'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "c.uec"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2"},
        {name: "png", fileName: "test_exif_v2.png", value: "c.uec"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "c.uec"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16100`;
      let key = 'BodySerialNumber'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "exoch"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "x1"},
        {name: "png", fileName: "test_exif_v2.png", value: "exoch"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "exoch"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16200
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16200
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16200', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16200`;
      let key = 'LensSpecification'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "0.8, 2.5, 1.5, 0.5"},
        {name: "webp", fileName: "test_exif_v2.webp", value: " 1, 2.5,  3,  2"},
        {name: "png", fileName: "test_exif_v2.png", value: "0.8, 2.5, 1.5, 0.5"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "0.8, 2.5, 1.5, 0.5"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16300`;
      let key = 'LensMake'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "xxwx"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "aaa"},
        {name: "png", fileName: "test_exif_v2.png", value: "xxwx"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "xxwx"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16400`;
      let key = 'LensModel'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "txaw"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "xxx"},
        {name: "png", fileName: "test_exif_v2.png", value: "txaw"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "txaw"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16500`;
      let key = 'LensSerialNumber'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "qxhc"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "111"},
        {name: "png", fileName: "test_exif_v2.png", value: "qxhc"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "qxhc"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16600`;
      let key = 'CompositeImage'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "2"},
        {name: "png", fileName: "test_exif_v2.png", value: "2"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16700`;
      let key = 'SourceImageNumberOfCompositeImage'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "23, 34"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1234, 1"},
        {name: "png", fileName: "test_exif_v2.png", value: "23, 34"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "23, 34"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16800`;
      let key = 'SourceExposureTimesOfCompositeImage'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "xxxw"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "123"},
        {name: "png", fileName: "test_exif_v2.png", value: "xxxw"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "xxxw"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_16900`;
      let key = 'Gamma'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2.5"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1.5"},
        {name: "png", fileName: "test_exif_v2.png", value: "2.5"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2.5"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17000`;
      let key = 'DNGVersion'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "2, 2, 3, 1"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "1, 1, 2, 3"},
        {name: "png", fileName: "test_exif_v2.png", value: "2, 2, 3, 1"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "2, 2, 3, 1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17100
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17100
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17100', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17100`;
      let key = 'DefaultCropSize'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v2.jpg", value: "153, 841"},
        {name: "webp", fileName: "test_exif_v2.webp", value: "12, 1"},
        {name: "png", fileName: "test_exif_v2.png", value: "153, 841"},
        {name: "heif", fileName: "test_exif_v2.heic", value: "153, 841"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17300
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17300
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17300', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17300`;
      let key = 'HwMnoteIsXmageSupported'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v4.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v4.webp", value: "1"},
        {name: "png", fileName: "test_exif_v4.png", value: "1"},
        {name: "heif", fileName: "test_exif_v4.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17400
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17400
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17400', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17400`;
      let key = 'HwMnoteXmageMode'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v4.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v4.webp", value: "1"},
        {name: "png", fileName: "test_exif_v4.png", value: "1"},
        {name: "heif", fileName: "test_exif_v4.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17500
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17500
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17500', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17500`;
      let key = 'HwMnoteXmageLeft'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v4.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v4.webp", value: "1"},
        {name: "png", fileName: "test_exif_v4.png", value: "1"},
        {name: "heif", fileName: "test_exif_v4.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17600
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17600
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17600', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17600`;
      let key = 'HwMnoteXmageTop'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v4.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v4.webp", value: "1"},
        {name: "png", fileName: "test_exif_v4.png", value: "1"},
        {name: "heif", fileName: "test_exif_v4.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17700
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17700
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17700', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17700`;
      let key = 'HwMnoteXmageRight'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v4.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v4.webp", value: "1"},
        {name: "png", fileName: "test_exif_v4.png", value: "1"},
        {name: "heif", fileName: "test_exif_v4.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17800
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17800
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17800', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17800`;
      let key = 'HwMnoteXmageBottom'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v4.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v4.webp", value: "1"},
        {name: "png", fileName: "test_exif_v4.png", value: "1"},
        {name: "heif", fileName: "test_exif_v4.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17900
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17900
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17900', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_17900`;
      let key = 'HwMnoteCloudEnhancementMode'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v4.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v4.webp", value: "1"},
        {name: "png", fileName: "test_exif_v4.png", value: "1"},
        {name: "heif", fileName: "test_exif_v4.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })
	
	/*
     * @tc.number    : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_18000
     * @tc.name      : SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_18000
     * @tc.desc      : Test get exif metadata from picture.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_18000', Level.LEVEL0, async (done: Function) => {
      await sleep(1000);
      let caseName: string = `SUB_MULTIMEDIA_PICTURE_NDK_PICTURE_GETMETADATA_18000`;
      let key = 'HwMnoteWindSnapshotMode'
      let exifType: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const values:Array<picTypeValue> = [{name: "jpeg", fileName: "test_exif_v4.jpg", value: "1"},
        {name: "webp", fileName: "test_exif_v4.webp", value: "1"},
        {name: "png", fileName: "test_exif_v4.png", value: "1"},
        {name: "heif", fileName: "test_exif_v4.heic", value: "1"}]
      try {
        for(let i = 0; i < values.length; i++){
            newCreatePictureByPixelMap(caseName, values[i].fileName, done);
            let PropertyValue = ndkTest.PictureMetadataGetPropertyWithNull(exifType, key)
            if(PropertyValue != values[i].value){
                console.log("compare value error file:" + values[i].fileName + "get value: " + PropertyValue + "compare value: " + values[i].value);
                expect.assertFail();
                done()
            }
        }
        expect(true).assertTrue();
        done();
      } catch (err) {
        hilog.error(domain, testTag, caseName + `: Test exception ` + err);
        expect().assertFalse();
        done();
      }
    })

  })
}