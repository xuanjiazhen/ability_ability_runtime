/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <iostream>

/**
 * @brief DCO签名测试示例
 *
 * 这个文件演示了如何使用gitcode-automation skill创建
 * 带DCO (Developer Certificate of Origin) 签名的commit。
 *
 * DCO要求每个commit都必须包含:
 * Signed-off-by: Name <email>
 *
 * 这样可以确保代码贡献者认证了原始开发者证书。
 */
void TestDCOSignature() {
    std::cout << "DCO签名测试" << std::endl;
    std::cout << "每个commit都必须包含Signed-off-by签名" << std::endl;
}

int main() {
    TestDCOSignature();
    return 0;
}
