# hyperSnapManager.d.ts 新增声明片段

> 本文件描述需要添加到 `interface_sdk-js/api/@ohos.app.ability.hyperSnapManager.d.ts` 中的新增声明。
> 本特性**无需新增独立 .d.ts 类型文件**（无新接口类型/数据结构），仅需在现有 `hyperSnapManager` namespace 中追加一个函数声明。

---

## 插入位置

在现有 `getLastError` 函数声明之后（约 line 234）、namespace 闭合 `}` 之前（约 line 235）插入以下函数声明。

---

## 新增函数声明

```typescript
    /**
     * Checks whether the current process is undergoing Hyper Snap initialization.
     *
     * During the Hyper Snap template process initialization (including preloading and image creation phases),
     * this method returns **true**. Once the snapshot is complete, fails, or the process is not a template
     * process, this method returns **false**.
     *
     * This is useful for applications to distinguish between a normal cold start and a Hyper Snap template
     * process initialization, allowing them to adjust their initialization logic accordingly.
     *
     * @returns { Promise<boolean> } Promise used to return the result.
     *     - **true**: The current process is undergoing Hyper Snap initialization.
     *     - **false**: The current process is not undergoing Hyper Snap initialization
     *       (snapshot completed, failed, or the process is not a template process).
     * @throws { BusinessError } 16000050 - Connect to system service failed.
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 27 dynamic&static
     */
    function isHyperSnapInitializing(): Promise<boolean>;
```

---

## 完整文件示意（变更后尾部）

```typescript
    // ... existing getLastError declaration ...

    function getLastError(errType: HyperSnapErrorType): Promise<HyperSnapErrorInfo>;

    /**
     * Checks whether the current process is undergoing Hyper Snap initialization.
     *
     * During the Hyper Snap template process initialization (including preloading and image creation phases),
     * this method returns **true**. Once the snapshot is complete, fails, or the process is not a template
     * process, this method returns **false**.
     *
     * This is useful for applications to distinguish between a normal cold start and a Hyper Snap template
     * process initialization, allowing them to adjust their initialization logic accordingly.
     *
     * @returns { Promise<boolean> } Promise used to return the result.
     *     - **true**: The current process is undergoing Hyper Snap initialization.
     *     - **false**: The current process is not undergoing Hyper Snap initialization
     *       (snapshot completed, failed, or the process is not a template process).
     * @throws { BusinessError } 16000050 - Connect to system service failed.
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 27 dynamic&static
     */
    function isHyperSnapInitializing(): Promise<boolean>;
}
export default hyperSnapManager;
```

---

## 使用示例

```typescript
import hyperSnapManager from '@ohos.app.ability.hyperSnapManager';

// 查询当前进程是否正在快照初始化
const isInitializing: boolean = await hyperSnapManager.isHyperSnapInitializing();

if (isInitializing) {
  // 当前进程是 HyperSnap 模板进程，正在快照初始化
  // 走快照模板进程的初始化流程
  console.info('Hyper Snap is initializing, using template init flow.');
} else {
  // 普通冷启动或快照已完成/失败
  // 走标准初始化流程
  console.info('Hyper Snap is not initializing, using standard init flow.');
}
```

### 错误处理

```typescript
try {
  const isInitializing = await hyperSnapManager.isHyperSnapInitializing();
  console.info(`HyperSnap initializing: ${isInitializing}`);
} catch (err) {
  // SA 连接失败
  console.error(`Failed to query HyperSnap state: ${err.message}`);
}
```

### 与现有 API 配合使用

```typescript
import hyperSnapManager from '@ohos.app.ability.hyperSnapManager';

// 启用 HyperSnap
hyperSnapManager.setHyperSnapEnabled(true);

// 查询当前是否在快照初始化中
const isInitializing = await hyperSnapManager.isHyperSnapInitializing();

if (!isInitializing) {
  // 快照不在初始化中，可能是已完成或未启用
  // 如遇兼容性问题，可请求重建快照
  hyperSnapManager.requestRebuildHyperSnap();
}

// 检查上次错误信息
try {
  const errorInfo = await hyperSnapManager.getLastError(
    hyperSnapManager.HyperSnapErrorType.CREATE_SNAPSHOT
  );
  if (errorInfo.code !== hyperSnapManager.HyperSnapErrorCode.ERR_OK) {
    console.error(`Last HyperSnap error: ${errorInfo.msg}`);
  }
} catch (err) {
  console.error('Failed to get last error info.');
}
```
