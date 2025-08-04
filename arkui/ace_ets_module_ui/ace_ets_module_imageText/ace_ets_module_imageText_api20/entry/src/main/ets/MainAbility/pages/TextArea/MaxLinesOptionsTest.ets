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

@Entry
@Component
struct MaxLinesOptionsTest {

  @State testText:string = 'this\n'+
    'is\n'+
    'long\n'+
    'long\n'+
    'long\n'+
    'long\n'+
    'test text'
  build() {
    Column() {
      TextArea({text:this.testText})
        .maxLines(4,{overflowMode:MaxLinesMode.CLIP})
        .textOverflow(TextOverflow.Clip)
        .id('TextAreaMaxLinesModeCLIP')
        .height('15%')
      TextArea({text:this.testText})
        .maxLines(4,{overflowMode:MaxLinesMode.SCROLL})
        .textOverflow(TextOverflow.Clip)
        .id('TextAreaMaxLinesModeSCROLL')
        .height('15%')
      TextArea({text:this.testText})
        .maxLines(4)
        .textOverflow(TextOverflow.Clip)
        .id('TextAreaMaxLinesModeNoSet')
        .height('15%')
      TextArea({text:this.testText})
        .maxLines(4,{overflowMode:undefined})
        .textOverflow(TextOverflow.Clip)
        .id('TextAreaMaxLinesModeUndefined')
        .height('15%')
    }
  }
}