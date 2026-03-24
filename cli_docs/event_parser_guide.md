# 事件解析辅助库

## 文档信息

**版本**: v1.0
**创建日期**: 2025-04-22
**状态**: 设计文档

---

## 概述

本文档描述了一个可选的事件解析辅助库，用于简化调用方对工具输出的解析。

**注意**：此辅助库是可选的，调用方可以直接解析，也可以使用此辅助库。

---

## 设计目标

1. **简化调用方开发**：提供统一的解析接口
2. **类型安全**：提供 TypeScript 类型定义
3. **灵活验证**：支持可选的 Schema 验证

---

## API 设计

### TypeScript 接口

```typescript
// frameworks/js/napi/cli/event_parser.ts

/**
 * 解析后的事件
 */
export interface ParsedEvent {
  type: string;
  data: any;
}

/**
 * 事件解析器
 */
export class ToolEventParser {
  /**
   * 解析工具输出的原始行
   * @param rawLine 工具输出的原始文本
   * @param toolInfo 工具信息（包含 eventSchemas）
   * @param options 解析选项
   * @returns 解析后的事件，如果不是自定义事件则返回 null
   */
  static parse(
    rawLine: string,
    toolInfo: ToolInfo,
    options?: ParseOptions
  ): ParsedEvent | null;

  /**
   * 批量解析多行输出
   * @param lines 工具输出的多行文本
   * @param toolInfo 工具信息
   * @param options 解析选项
   * @returns 解析后的事件数组
   */
  static parseBatch(
    lines: string[],
    toolInfo: ToolInfo,
    options?: ParseOptions
  ): ParsedEvent[];
}

/**
 * 解析选项
 */
export interface ParseOptions {
  /**
   * 是否进行 Schema 验证（默认: false）
   */
  validateSchema?: boolean;

  /**
   * 自定义事件解析器
   */
  customParsers?: Record<string, (data: any) => any>;
}
```

---

## 使用示例

### 示例 1：基本使用

```typescript
import { ToolEventParser } from '@ohos.cli/parser';

async function scanNetwork() {
  const toolInfo = await CLI.CLIManager.getToolInfoByName('network-scanner');

  const session = await CLI.CLIManager.execTool('network-scanner', {
    target: '192.168.1.1',
    ports: [80, 443]
  }, { background: true });

  await CLI.CLIManager.subscribeSession(session.sessionId, (event) => {
    if (event.type === 'stdout') {
      const parsed = ToolEventParser.parse(event.data, toolInfo);
      if (parsed) {
        console.log('自定义事件:', parsed.type, parsed.data);
      } else {
        console.log('普通输出:', event.data);
      }
    }
  });
}
```

### 示例 2：批量解析

```typescript
import { ToolEventParser } from '@ohos.cli/parser';

async function parseToolOutput(output: string) {
  const toolInfo = await CLI.CLIManager.getToolInfoByName('network-scanner');
  const lines = output.split('\n');

  const events = ToolEventParser.parseBatch(lines, toolInfo);
  events.forEach(event => {
    console.log('事件类型:', event.type);
    console.log('事件数据:', event.data);
  });
}
```

### 示例 3：自定义解析器

```typescript
const customParsers = {
  'progress': (data) => {
    // 自定义进度解析逻辑
    return {
      percentage: data.percentage,
      status: data.status
    };
  }
};

const options = {
  validateSchema: true,
  customParsers: customParsers
};

const parsed = ToolEventParser.parse(rawLine, toolInfo, options);
```

---

## 实现建议

### C++ 实现

```cpp
// frameworks/js/napi/cli/event_parser_module.cpp

napi_value ToolEventParser::Parse(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];
  napi_value thisVar = nullptr;

  if (napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok) {
    return nullptr;
  }

  // 获取参数
  std::string rawLine = GetStringArg(env, argv[0]);
  napi_value toolInfoObj = argv[1];

  // 解析 JSON
  Json::Value root;
  Json::CharReaderBuilder builder;
  std::string errs;
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

  if (!reader->parse(rawLine.c_str(), rawLine.c_str() + rawLine.length(), &root, &errs)) {
    return nullptr;
  }

  // 检查是否是事件格式
  if (!root.isMember("event") || !root.isMember("data")) {
    return nullptr;
  }

  // 创建返回对象
  napi_value result = CreateObject(env, {
    {"type", root["event"]},
    {"data", root["data"]}
  });

  return result;
}
```

---

## 优势

1. **统一接口**：所有调用方使用相同的解析逻辑
2. **类型安全**：提供 TypeScript 类型定义
3. **性能优化**：批量解析时可以优化性能
4. **易于维护**：解析逻辑集中管理

---

## 注意事项

1. **可选使用**：调用方可以选择不使用此辅助库，自行解析
2. **向后兼容**：不影响现有 API
3. **性能考虑**：解析库会增加一定的性能开销
