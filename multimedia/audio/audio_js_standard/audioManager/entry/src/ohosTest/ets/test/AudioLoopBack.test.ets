/**
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


import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect, Level} from '@ohos/hypium';
import audio from '@ohos.multimedia.audio';
import { BusinessError } from '@ohos.base';
import { Driver } from '@ohos.UiTest';



export default function audioLoopBack() {

  const TAG = "[AudioLoopBackJsTest]";

  describe("AudioLoopBackJsTest", () => {

    beforeAll(async () => {
      console.info(TAG + "beforeAll called");
    })

    afterAll(() => {
      console.info(TAG + 'afterAll called')
    })

    beforeEach(() => {
      console.info(TAG + 'beforeEach called')
    })

    afterEach(() => {
      console.info(TAG + 'afterEach called')
    })


    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_ISAUDIOLOOPBACKSUPPORTED_JS_0100
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_ISAUDIOLOOPBACKSUPPORTED_0100
     * @tc.desc      : isAudioLoopbackSupported
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_ISAUDIOLOOPBACKSUPPORTED_0100", Level.LEVEL2, async () => {
      try {
        let ret: boolean = audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(-1);
        expect(ret).assertFalse();
      } catch (err) {
        console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ISAUDIOLOOPBACKSUPPORTED_0100 ERROR: ${err}`);
        expect(Number(err.code)).assertEqual(6800101)
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_ISAUDIOLOOPBACKSUPPORTED_JS_0200
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_ISAUDIOLOOPBACKSUPPORTED_0200
     * @tc.desc      : isAudioLoopbackSupported
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_ISAUDIOLOOPBACKSUPPORTED_0200", Level.LEVEL2, async () => {
      try {
        let ret: boolean = audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(null);
        expect(ret).assertFalse();
      } catch (err) {
        console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ISAUDIOLOOPBACKSUPPORTED_0200 ERROR: ${err}`);
        expect(Number(err.code)).assertEqual(401)
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_ISAUDIOLOOPBACKSUPPORTED_JS_0300
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_ISAUDIOLOOPBACKSUPPORTED_0300
     * @tc.desc      : isAudioLoopbackSupported
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_ISAUDIOLOOPBACKSUPPORTED_0300", Level.LEVEL2, async () => {
      try {
        let ret: boolean = audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(undefined);
        expect(ret).assertFalse();
      } catch (err) {
        console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ISAUDIOLOOPBACKSUPPORTED_0300 ERROR: ${err}`);
        expect(Number(err.code)).assertEqual(401)
      }
    })

	  /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_CREATEAUDIOLOOPBACK_JS_0100
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_CREATEAUDIOLOOPBACK_0100
     * @tc.desc      : createAudioLoopback
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_CREATEAUDIOLOOPBACK_0100", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopBack: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          if (null == audioLoopBack) {
            expect().assertFail();
          }
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_CREATEAUDIOLOOPBACK_0100 ERROR: ${err}`);
          if (201 == err.code) {
            expect(Number(err.code)).assertEqual(201)
          } else {
            expect().assertFail();
          }
        }
      }
    })


    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_CREATEAUDIOLOOPBACK_JS_0200
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_CREATEAUDIOLOOPBACK_0200
     * @tc.desc      : createAudioLoopback
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_CREATEAUDIOLOOPBACK_0200", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          await audio.createAudioLoopback(-1);
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_CREATEAUDIOLOOPBACK_0200 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(6800101);
        }
      }

    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETVOLUME_JS_0100
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETVOLUME_0100
     * @tc.desc      : createAudioLoopback
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETVOLUME_0100", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopBack: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopBack.setVolume(0);
        } catch(err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETVOLUME_0100 ERROR: ${err}`);
          if (201 == err.code) {
            expect(Number(err.code)).assertEqual(201)
          } else {
            expect().assertFail();
          }
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETVOLUME_JS_0200
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETVOLUME_0200
     * @tc.desc      : setVolume
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETVOLUME_0200", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopBack: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopBack.setVolume(0.5);
        } catch(err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETVOLUME_0200 ERROR: ${err}`);
          if (201 == err.code) {
            expect(Number(err.code)).assertEqual(201)
          } else {
            expect().assertFail();
          }
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETVOLUME_JS_0300
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETVOLUME_0300
     * @tc.desc      : setVolume
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETVOLUME_0300", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopBack: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopBack.setVolume(1);
        } catch(err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETVOLUME_0300 ERROR: ${err}`);
          if (201 == err.code) {
            expect(Number(err.code)).assertEqual(201)
          } else {
            expect().assertFail();
          }
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETVOLUME_JS_0400
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETVOLUME_0400
     * @tc.desc      : setVolume
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETVOLUME_0400", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopBack: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopBack.setVolume(-0.1);
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETVOLUME_0400 ERROR: ${err}`);
          if (201 == err.code) {
            expect(Number(err.code)).assertEqual(201);
          } else {
            expect(Number(err.code)).assertEqual(6800101);
          }
        }
      }
    })


    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETVOLUME_JS_0500
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETVOLUME_0500
     * @tc.desc      : setVolume
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETVOLUME_0500", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopBack: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopBack.setVolume(1.1);
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETVOLUME_0500 ERROR: ${err}`);
          if (201 == err.code) {
            expect(Number(err.code)).assertEqual(201);
          } else {
            expect(Number(err.code)).assertEqual(6800101);
          }
        }
      }
    })


    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETVOLUME_JS_0600
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETVOLUME_0600
     * @tc.desc      : setVolume
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETVOLUME_0600", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopBack: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopBack.setVolume(null);
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETVOLUME_0600 ERROR: ${err}`);
          if (201 == err.code) {
            expect(Number(err.code)).assertEqual(201);
          } else {
            expect(Number(err.code)).assertEqual(6800101);
          }
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_GETSTATUS_JS_0100
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_GETSTATUS_0100
     * @tc.desc      : getStatus
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_GETSTATUS_0100", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopBack: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          let res = await audioLoopBack.getStatus();
          await expect(res).assertEqual(-2);
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETSTATUS_0100 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_ENABLE_JS_0100
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_ENABLE_0100
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_ENABLE_0100", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
            let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
            expect(null != audioLoopBack);
            audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
              if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
                console.info('audio loopback status is: UNAVAILABLE_DEVICE');
              } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
                console.info('audio loopback status is: UNAVAILABLE_SCENE');
              } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
                console.info('audio loopback status is: AVAILABLE_IDLE');
              } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
                console.info('audio loopback status is: AVAILABLE_RUNNING');
              }
            });
            audioLoopback.enable(undefined).then((isSuccess) => {
            if (isSuccess) {
              console.info('audio loopback enable success');
            } else {
              console.info('audio loopback enable fail');
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0100 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(6800101);
          });
          audioLoopback.off('statusChange');
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0100 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_ENABLE_JS_0200
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_ENABLE_0200
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_ENABLE_0200", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(null).then((isSuccess) => {
            if (isSuccess) {
              console.info('audio loopback enable success');
            } else {
              console.info('audio loopback enable fail');
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0200 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(6800101);
          });
          audioLoopback.off('statusChange');
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0200 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_ENABLE_JS_0300
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_ENABLE_0300
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_ENABLE_0300", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', undefined)
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0300 ERROR: ${err}`);
          if (201 == err.code) {
            expect(Number(err.code)).assertEqual(201);
          } else {
            expect(Number(err.code)).assertEqual(401);
          }
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_ENABLE_JS_0400
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_ENABLE_0400
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_ENABLE_0400", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.off('statusChange', undefined);
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0400 ERROR: ${err}`);
          if (201 == err.code) {
            expect(Number(err.code)).assertEqual(201);
          } else {
            expect(Number(err.code)).assertEqual(401);
          }
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_ENABLE_JS_0500
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_ENABLE_0500
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_ENABLE_0500", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0500 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0500 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0500 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0500 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0500 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0500 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_ENABLE_0500 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

/**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_GETREVERBPRESET_JS_0100
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_GETREVERBPRESET_0100
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_GETREVERBPRESET_0100", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETREVERBPRESET_0100 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETREVERBPRESET_0100 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETREVERBPRESET_0100 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          let reverb: audio.AudioLoopbackReverbPreset = audioLoopback.getReverbPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_GETREVERBPRESET_0100 equalizer: ${reverb}`);
          expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.THEATER);

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETREVERBPRESET_0100 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETREVERBPRESET_0100 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETREVERBPRESET_0100 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETREVERBPRESET_0100 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_GETEQUALIZERPRESET_JS_0100
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_GETEQUALIZERPRESET_0100
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_GETEQUALIZERPRESET_0100", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETEQUALIZERPRESET_0100 audio loopback enable true success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETEQUALIZERPRESET_0100 audio loopback enable true fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETEQUALIZERPRESET_0100 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          let equalizer: audio.AudioLoopbackEqualizerPreset = audioLoopback.getEqualizerPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_GETEQUALIZERPRESET_0100 equalizer: ${equalizer}`);
          expect(equalizer).assertEqual(audio.AudioLoopbackEqualizerPreset.FULL);

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETEQUALIZERPRESET_0100 audio loopback enable false success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETEQUALIZERPRESET_0100 audio loopback enable false fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETEQUALIZERPRESET_0100 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_GETEQUALIZERPRESET_0100 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0100
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0100
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0100", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0100 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0100 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0100 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          let res: boolean = audioLoopback.setReverbPreset(audio.AudioLoopbackReverbPreset.ORIGINAL);
          expect(res).assertTrue();
          let reverb: audio.AudioLoopbackReverbPreset = audioLoopback.getReverbPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0100 reverb: ${reverb}`);
          expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.ORIGINAL);

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0100 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0100 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0100 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0100 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0200
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0200
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0200", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0200 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0200 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0200 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          let res: boolean = audioLoopback.setReverbPreset(audio.AudioLoopbackReverbPreset.KTV);
          expect(res).assertTrue();
          let reverb: audio.AudioLoopbackReverbPreset = audioLoopback.getReverbPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0200 reverb: ${reverb}`);
          expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.KTV);

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0200 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0200 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0200 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0200 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0300
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0300
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0300", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0300 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0300 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0300 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          let res: boolean = audioLoopback.setReverbPreset(audio.AudioLoopbackReverbPreset.THEATER);
          expect(res).assertTrue();
          let reverb: audio.AudioLoopbackReverbPreset = audioLoopback.getReverbPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0300 reverb: ${reverb}`);
          expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.THEATER);

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0300 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0300 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0300 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0300 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0400
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0400
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0400", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0400 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0400 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0400 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          let res: boolean = audioLoopback.setReverbPreset(audio.AudioLoopbackReverbPreset.CONCERT);
          expect(res).assertTrue();
          let reverb: audio.AudioLoopbackReverbPreset = audioLoopback.getReverbPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0400 reverb: ${reverb}`);
          expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.CONCERT);

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0400 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0400 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0400 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0400 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0500
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0500
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0500", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0500 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0500 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0500 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          try {
            audioLoopback.setReverbPreset(null);
          } catch (err) {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0500 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(401);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0500 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0500 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0500 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0500 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0600
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0600
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0600", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0600 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0600 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0600 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          try {
            audioLoopback.setReverbPreset(undefined);
          } catch (err) {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0600 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(401);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0600 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0600 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0600 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0600 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0700
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0700
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0700", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0700 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0700 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0700 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          try {
            audioLoopback.setReverbPreset(-1);
          } catch (err) {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0700 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(6800101);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0700 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0700 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0700 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0700 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0800
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0800
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0800", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0800 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0800 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0800 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          try {
            audioLoopback.setReverbPreset(0);
          } catch (err) {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0800 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(6800101);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0800 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0800 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0800 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0800 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0900
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0900
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0900", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0900 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0900 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0900 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          try {
            audioLoopback.setReverbPreset(5);
          } catch (err) {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0900 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(6800101);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0900 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0900 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0900 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0900 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0010
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0010
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0010", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0010 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0010 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0010 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          for (let i = 0; i < 2000; ++i) {
            let res: boolean = audioLoopback.setReverbPreset(audio.AudioLoopbackReverbPreset.ORIGINAL);
            expect(res).assertTrue();
            let reverb: audio.AudioLoopbackReverbPreset = audioLoopback.getReverbPreset();
            console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0010 reverb: ${reverb}`);
            expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.ORIGINAL);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0010 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0010 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0010 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0010 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0011
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0011
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0011", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0011 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0011 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0011 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          for (let i = 0; i < 2000; ++i) {
            let res: boolean = audioLoopback.setReverbPreset(audio.AudioLoopbackReverbPreset.KTV);
            expect(res).assertTrue();
            let reverb: audio.AudioLoopbackReverbPreset = audioLoopback.getReverbPreset();
            console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0011 reverb: ${reverb}`);
            expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.KTV);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0011 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0011 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0011 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0011 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0012
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0012
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0012", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0012 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0012 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0012 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          for (let i = 0; i < 2000; ++i) {
            let res: boolean = audioLoopback.setReverbPreset(audio.AudioLoopbackReverbPreset.THEATER);
            expect(res).assertTrue();
            let reverb: audio.AudioLoopbackReverbPreset = audioLoopback.getReverbPreset();
            console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0012 reverb: ${reverb}`);
            expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.THEATER);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0012 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0012 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0012 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0012 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0013
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0013
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0013", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0013 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0013 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0013 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          for (let i = 0; i < 2000; ++i) {
            let res: boolean = audioLoopback.setReverbPreset(audio.AudioLoopbackReverbPreset.CONCERT);
            expect(res).assertTrue();
            let reverb: audio.AudioLoopbackReverbPreset = audioLoopback.getReverbPreset();
            console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0013 reverb: ${reverb}`);
            expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.CONCERT);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0013 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0013 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0013 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0013 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_JS_0014
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          let res: boolean = audioLoopback.setReverbPreset(audio.AudioLoopbackReverbPreset.ORIGINAL);
          expect(res).assertTrue();
          let reverb: audio.AudioLoopbackReverbPreset = audioLoopback.getReverbPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014 reverb: ${reverb}`);
          expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.ORIGINAL);

          res = audioLoopback.setReverbPreset(audio.AudioLoopbackReverbPreset.KTV);
          expect(res).assertTrue();
          reverb = audioLoopback.getReverbPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014 reverb: ${reverb}`);
          expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.KTV);

          res = audioLoopback.setReverbPreset(audio.AudioLoopbackReverbPreset.THEATER);
          expect(res).assertTrue();
          reverb = audioLoopback.getReverbPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014 reverb: ${reverb}`);
          expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.THEATER);

          res = audioLoopback.setReverbPreset(audio.AudioLoopbackReverbPreset.CONCERT);
          expect(res).assertTrue();
          reverb = audioLoopback.getReverbPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014 reverb: ${reverb}`);
          expect(reverb).assertEqual(audio.AudioLoopbackReverbPreset.CONCERT);

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETREVERBPRESET_0014 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_JS_0100
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0100
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0100", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0100 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0100 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0100 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          let res: boolean = audioLoopback.setEqualizerPreset(audio.AudioLoopbackEqualizerPreset.FLAT);
          expect(res).assertTrue();
          let eaualizer: audio.AudioLoopbackEqualizerPreset = audioLoopback.getEqualizerPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0100 eaualizer: ${eaualizer}`);
          expect(eaualizer).assertEqual(audio.AudioLoopbackEqualizerPreset.FLAT);

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0100 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0100 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0100 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0100 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_JS_0200
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0200
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0200", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0200 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0200 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0200 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          let res: boolean = audioLoopback.setEqualizerPreset(audio.AudioLoopbackEqualizerPreset.FULL);
          expect(res).assertTrue();
          let eaualizer: audio.AudioLoopbackEqualizerPreset = audioLoopback.getEqualizerPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0200 eaualizer: ${eaualizer}`);
          expect(eaualizer).assertEqual(audio.AudioLoopbackEqualizerPreset.FULL);

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0200 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0200 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0200 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0200 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_JS_0300
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0300
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0300", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0300 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0300 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0300 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          let res: boolean = audioLoopback.setEqualizerPreset(audio.AudioLoopbackEqualizerPreset.BRIGHT);
          expect(res).assertTrue();
          let eaualizer: audio.AudioLoopbackEqualizerPreset = audioLoopback.getEqualizerPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0300 eaualizer: ${eaualizer}`);
          expect(eaualizer).assertEqual(audio.AudioLoopbackEqualizerPreset.BRIGHT);

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0300 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0300 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0300 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0300 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_JS_0400
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0400
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0400", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0400 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0400 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0400 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          try {
            audioLoopback.setEqualizerPreset(null);
          } catch (err) {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0400 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(401);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0400 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0400 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0400 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0400 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_JS_0500
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0500
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0500", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0500 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0500 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0500 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          try {
            audioLoopback.setEqualizerPreset(undefined);
          } catch (err) {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0500 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(401);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0500 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0500 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0500 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0500 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_JS_0600
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0600
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0600", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0600 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0600 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0600 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          try {
            audioLoopback.setEqualizerPreset(-1);
          } catch (err) {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0600 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(6800101);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0600 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0600 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0600 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0600 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_JS_0700
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0700
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0700", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0700 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0700 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0700 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          try {
            audioLoopback.setEqualizerPreset(0);
          } catch (err) {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0700 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(6800101);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0700 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0700 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0700 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0700 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_JS_0800
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0800
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0800", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0800 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0800 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0800 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          try {
            audioLoopback.setEqualizerPreset(4);
          } catch (err) {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0800 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(6800101);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0800 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0800 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0800 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0800 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_JS_0900
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0900
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0900", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0900 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0900 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0900 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          try {
            audioLoopback.setEqualizerPreset(5);
          } catch (err) {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0900 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(6800101);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0900 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0900 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0900 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0900 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_JS_0010
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0010
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0010", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0010 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0010 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0010 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          for (let i = 0; i < 2000; ++i) {
            let res: boolean = audioLoopback.setEqualizerPreset(audio.AudioLoopbackEqualizerPreset.FLAT);
            expect(res).assertTrue();
            let eaualizer: audio.AudioLoopbackEqualizerPreset = audioLoopback.getEqualizerPreset();
            console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0010 eaualizer: ${eaualizer}`);
            expect(eaualizer).assertEqual(audio.AudioLoopbackEqualizerPreset.FLAT);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0010 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0010 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0010 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0010 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_JS_0011
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0011
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0011", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0011 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0011 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0011 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          for (let i = 0; i < 2000; ++i) {
            let res: boolean = audioLoopback.setEqualizerPreset(audio.AudioLoopbackEqualizerPreset.BRIGHT);
            expect(res).assertTrue();
            let eaualizer: audio.AudioLoopbackEqualizerPreset = audioLoopback.getEqualizerPreset();
            console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0011 eaualizer: ${eaualizer}`);
            expect(eaualizer).assertEqual(audio.AudioLoopbackEqualizerPreset.BRIGHT);
          }

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0011 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0011 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0011 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0011 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })

    /**
     * @tc.number    : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_JS_0012
     * @tc.name      : SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0012
     * @tc.desc      : enable
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 2
     */
    it("SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0012", Level.LEVEL2, async () => {
      if (audio.getAudioManager().getStreamManager().isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE)) {
        try {
          let audioLoopback: audio.AudioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
          expect(null != audioLoopBack);
          audioLoopback.on('statusChange', (status: audio.AudioLoopbackStatus) => {
            if (status == audio.AudioLoopbackStatus.UNAVAILABLE_DEVICE) {
              console.info('audio loopback status is: UNAVAILABLE_DEVICE');
            } else if (status == audio.AudioLoopbackStatus.UNAVAILABLE_SCENE) {
              console.info('audio loopback status is: UNAVAILABLE_SCENE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_IDLE) {
              console.info('audio loopback status is: AVAILABLE_IDLE');
            } else if (status == audio.AudioLoopbackStatus.AVAILABLE_RUNNING) {
              console.info('audio loopback status is: AVAILABLE_RUNNING');
            }
          });
          audioLoopback.enable(true).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0012 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0012 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0012 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });

          let res: boolean = audioLoopback.setEqualizerPreset(audio.AudioLoopbackEqualizerPreset.FLAT);
          expect(res).assertTrue();
          let eaualizer: audio.AudioLoopbackEqualizerPreset = audioLoopback.getEqualizerPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0012 eaualizer: ${eaualizer}`);
          expect(eaualizer).assertEqual(audio.AudioLoopbackEqualizerPreset.FLAT);

          res = audioLoopback.setEqualizerPreset(audio.AudioLoopbackEqualizerPreset.FULL);
          expect(res).assertTrue();
          eaualizer = audioLoopback.getEqualizerPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0012 eaualizer: ${eaualizer}`);
          expect(eaualizer).assertEqual(audio.AudioLoopbackEqualizerPreset.FULL);

          res = audioLoopback.setEqualizerPreset(audio.AudioLoopbackEqualizerPreset.BRIGHT);
          expect(res).assertTrue();
          eaualizer = audioLoopback.getEqualizerPreset();
          console.info(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0012 eaualizer: ${eaualizer}`);
          expect(eaualizer).assertEqual(audio.AudioLoopbackEqualizerPreset.BRIGHT);

          audioLoopback.enable(false).then((isSuccess) => {
            if (isSuccess) {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0012 audio loopback enable success`);
            } else {
              console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0012 audio loopback enable fail`);
            }
          }).catch((err: BusinessError) => {
            console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0012 ERROR: ${err}`);
            expect(Number(err.code)).assertEqual(201);
          });
        } catch (err) {
          console.error(`${TAG} SUB_MULTIMEDIA_AUDIO_SETEQUALIZERPRESET_0012 ERROR: ${err}`);
          expect(Number(err.code)).assertEqual(201);
        }
      }
    })
  })
}