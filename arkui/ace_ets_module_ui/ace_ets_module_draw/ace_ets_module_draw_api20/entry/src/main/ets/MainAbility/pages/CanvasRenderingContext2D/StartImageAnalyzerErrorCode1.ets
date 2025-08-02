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

 import { BusinessError } from '@kit.BasicServicesKit';
@Entry
@Component
struct StartImageAnalyzerErrorCode1 {
  @State str : string = '0'
  private settings: RenderingContextSettings = new RenderingContextSettings(true)
  private context: CanvasRenderingContext2D = new CanvasRenderingContext2D(this.settings)
  private config: ImageAnalyzerConfig = {
    types: [ImageAnalyzerType.SUBJECT, ImageAnalyzerType.TEXT]
  }
  private img = new ImageBitmap('resources/base/media/icon.png')
  private aiController: ImageAnalyzerController = new ImageAnalyzerController()
  private options: ImageAIOptions = {
    types: [ImageAnalyzerType.SUBJECT, ImageAnalyzerType.TEXT],
    aiController: this.aiController
  }

  build() {
    Column({ space:5 }) {
      Text(this.str).id('ErrorCodeStopImageAnalyzer')
      Row(){
        Button('start')
          .id('ButtonStopImageAnalyzerStart')
          .width(70)
          .height(50)
          .margin(5)
          .onClick(() => {
            this.context.startImageAnalyzer(this.config)
              .then(() => {
                console.log('analysis complete')
              })
              .catch((error: BusinessError) => {
                console.log('error code: ' + error.code)
                this.str = error.code.toString()
              })
            this.context.stopImageAnalyzer()
          })
        Button('stop')
          .width(70)
          .height(50)
          .margin(5)
          .onClick(() => {
            this.context.stopImageAnalyzer()
          })
      }
      Canvas(this.context, this.options)
        .width(100)
        .height(100)
        .enableAnalyzer(true)
        .onReady(() => {
          this.context.drawImage(this.img, 0, 0, 100, 100)
        })
    }
    .width('100%')
  }
}