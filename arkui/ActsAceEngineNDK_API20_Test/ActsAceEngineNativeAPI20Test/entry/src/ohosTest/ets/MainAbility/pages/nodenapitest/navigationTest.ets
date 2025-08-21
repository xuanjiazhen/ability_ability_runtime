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

import render from 'libnoderenderndk.so'
import { NodeContent } from '@kit.ArkUI';
import { NavDestinationTestBuilder } from './navdestinationTest';

@Entry
@Component
struct NavigationTest {
  PAGE_TAG: string = 'NavigationTest';
  private nodeContent: NodeContent = new NodeContent();
  @Provide pathStack: NavPathStack = new NavPathStack()
  aboutToAppear():void{
    // 通过C-API创建节点，并添加到管理器nodeContent上
    if (render) {
      render.CreateNodeContent(this.nodeContent, 'TestAPP');
    }
  }
  @Builder
  pageMap(pageName: string) {
    if (pageName === 'NavDestinationTest') {
      NavDestinationTestBuilder()
    }
  }

  build() {
    Row() {
      Navigation(this.pathStack) {
        ContentSlot(this.nodeContent);
      }.title('Main')
      .navDestination(this.pageMap)
      .width('100%')
    }
    .height('100%')
  }
}