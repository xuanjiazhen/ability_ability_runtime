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

import { NodeContent, router } from '@kit.ArkUI';
import entry from 'libentry.so';

@Component
struct CAPIComponent {
  private rootSlot = new NodeContent();
  @State postType: number = 0;

  aboutToAppear(): void {
    if (this.postType == 0) {
      entry.CreateNodeArkUIPostAsyncUITask(this.rootSlot, this.getUIContext());
    } else if (this.postType == 1) {
      entry.CreateNodeArkUIPostUITask(this.rootSlot, this.getUIContext());
    } else if (this.postType == 2) {
      entry.CreateNodeArkUIPostUITaskAndWait(this.rootSlot, this.getUIContext());
    }
  }

  aboutToDisappear(): void {
    // 释放已创建的C-API组件
    entry.DisposeNodeTree(this.rootSlot);
  }

  build() {
    Column() {
      // C-API组件挂载点
      ContentSlot(this.rootSlot)
    }
    .width('100%')
  }
}

@Component
struct TitleComponent {
  @Link message: string;

  build() {
    Column() {
      Text(this.message)
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .width('100%')
        .textAlign(TextAlign.Center)
    }
    .margin(5)
    .align(Alignment.Center)
    .width('95%')
    .height('8%')
    .borderRadius(10)
    .backgroundColor('#FFFFFF')
  }
}

interface paramsObj{
  postType: number
}

@Entry
@Component
struct Page {
  postType: number = 0;
  @State message: string = 'CreateNodeTreeOnMultiThread';

  aboutToAppear(): void {
    this.postType = 2;
    if (this.postType) {
      this.message = 'ArkUIPostAsyncUITask';
    } else if (this.postType == 1) {
      this.message = 'ArkUIPostUITask'
    } else if (this.postType == 2) {
      this.message = 'ArkUIPostUITaskAndWait'
    }
  }

  build() {
    Flex() {
      Column() {
        TitleComponent({message : this.message})
          .width('100%')
          .align(Alignment.Center)
        CAPIComponent({postType : this.postType})
          .width('100%')
      }
      .backgroundColor('#EEEEEE')
      .width('100%')
      .height('100%')
    }
    .width('100%')
    .height('100%')
  }
}