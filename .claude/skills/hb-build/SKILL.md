---
name: hb-build
description: 使用 hb build 独立编译命令对 ability_runtime 代码仓进行增量或全量编译
license: Apache-2.0
metadata:
  version: "3.1.0"
  author: agno-team
  tags: ["独立编译", "构建", "hb"]
---

## 概述

本技能用于对 `ability_runtime` 代码仓进行独立编译，支持多种编译模式和目标选择。

**编译方式**：独立编译（使用 `hb build` 命令）

**目标部件**：ability_runtime

## 编译模式

用户可选择以下编译模式：

1. **全量编译本仓**：编译整个 ability_runtime 组件
2. **仅编译修改相关的目标**：基于代码变更智能选择编译目标
3. **编译指定目标**：明确指定一个或多个编译目标

## 编译选项

### 是否首次编译

- **首次编译**：执行完整的编译流程
- **非首次编译**：编译命令后添加 `--fast-rebuild` 参数以加速增量编译

### 其他常用参数

- `--skip-download`：跳过下载步骤（适用于非首次编译）
- `-t` 或 `--test`：编译测试目标
- `-i` 或 `--install`：编译并安装目标

## 使用示例

### 1. 测试用例代码编译

#### 首次编译或 GN 文件修改后
```bash
hb build ability_runtime -t --build-target //foundation/ability/ability_runtime/test/new_test/examples/oh_mock_utils_unit_test:unittest
```

#### 编译单个 TDD 用例
```bash
hb build ability_runtime -t --build-target //foundation/ability/ability_runtime/test/new_test/examples/oh_mock_utils_unit_test:oh_mock_utils_basic_return_test
```

### 2. 业务代码编译

#### 编译单个业务目标
```bash
hb build ability_runtime -i --build-target //foundation/ability/ability_runtime/services/uripermmgr:libupms
```

#### 编译核心服务目标
```bash
# AbilityManagerService
hb build ability_runtime -i --build-target //foundation/ability/ability_runtime/services/abilitymgr:abilityms

# AppManagerService
hb build ability_runtime -i --build-target //foundation/ability/ability_runtime/services/appmgr:appms

# DataObserverManager
hb build ability_runtime -i --build-target //foundation/ability/ability_runtime/services/dataobsmgr:dataobsms
```

### 3. 非首次编译（增量编译）

#### 使用快速重建模式
```bash
hb build ability_runtime -i --build-target //foundation/ability/ability_runtime/services/uripermmgr:libupms --fast-rebuild --skip-download
```

### 4. 多个目标编译

#### 同时编译多个目标
```bash
hb build ability_runtime -i --build-target \
  //foundation/ability/ability_runtime/services/abilitymgr:abilityms \
  //foundation/ability/ability_runtime/services/uripermmgr:libupms
```

### 5. 全量编译

#### 编译整个 ability_runtime 组件
```bash
hb build ability_runtime
```

## 常用编译目标路径

### 测试目标
```
//foundation/ability/ability_runtime/test/unittest:...
//foundation/ability/ability_runtime/test/moduletest:...
//foundation/ability/ability_runtime/test/fuzztest:...
```

### 服务端目标
```
//foundation/ability/ability_runtime/services/abilitymgr:abilityms
//foundation/ability/ability_runtime/services/appmgr:appms
//foundation/ability/ability_runtime/services/uripermmgr:libupms
//foundation/ability/ability_runtime/services/dataobsmgr:dataobsms
//foundation/ability/ability_runtime/services/quickfixmgr:...
```

### 框架层目标
```
//foundation/ability/ability_runtime/frameworks/native:...
//foundation/ability/ability_runtime/interfaces/kits/native:...
```

## 注意事项

1. **GN 文件变更后**：需要执行完整编译，不要使用 `--fast-rebuild`
2. **依赖变更**：如果修改了公共头文件或接口，建议全量编译相关模块
3. **编译环境**：确保已正确配置 OpenHarmony 编译环境和产品参数
4. **磁盘空间**：完整编译需要足够的磁盘空间，建议定期清理构建产物
5. **并行编译**：`hb build` 默认使用多核并行编译，可根据机器配置调整并发数
6. **不编译 HAP 包**：编译 ability_runtime 时默认不编译 hap 包（如 services/dialog_ui），hap 包需通过单独的构建命令或产品级构建生成

