# appManager.d.ts 新增声明片段

> 本文件描述需要添加到 `interface_sdk-js/api/@ohos.app.ability.appManager.d.ts` 中的新增声明。
> 本特性**无需新增独立 .d.ts 类型文件**（无新接口类型/数据结构），仅需在 `appManager.d.ts` 中新增函数声明。

---

## 插入位置

在现有 `killProcessWithAccount` 的三个重载声明之后（约 line 1142，callback 版之后）、`isRunningInStabilityTest` 声明之前（约 line 1145）插入以下函数声明。

---

## 新增函数声明

```typescript
  /**
   * Kills a process by bundle name and account ID, with keep-alive protection. If the target process is a keep-alive
   * application, the process is not killed and an error code is returned. This API uses a promise to return the result.
   *
   * > **NOTE**
   * >
   * > - The ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS permission is not required when **accountId** specifies the
   * > current user.
   * >
   * > - Unlike [killProcessWithAccount]{@link appManager.killProcessWithAccount(bundleName: string, accountId: int): Promise<void>}
   * > , this API rejects the kill request if the target process is a keep-alive application. Use
   * > killProcessWithAccount if you need to forcibly kill a keep-alive process.
   *
   * @permission ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS and ohos.permission.KILL_APP_PROCESSES
   *     or ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS and ohos.permission.CLEAN_BACKGROUND_PROCESSES
   *     (INTERACT_ACROSS_LOCAL_ACCOUNTS is not required when accountId specifies the current user)
   * @param { string } bundleName - Bundle name.
   * @param { int } accountId - ID of a system account. For details, see
   *     [getOsAccountLocalId]{@link @ohos.account.osAccount:osAccount.AccountManager.getOsAccountLocalId(callback: AsyncCallback<int>)}
   *     .
   * @returns { Promise<void> } Promise that returns no value. If the target process is a keep-alive application, the
   *     promise is rejected with error code 2097184.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 401 - Parameter error. Possible causes: 1. Mandatory parameters are left unspecified;
   *     2. Incorrect parameter types; 3. Parameter verification failed.
   * @throws { BusinessError } 16000050 - Internal error.
   * @throws { BusinessError } 2097184 - The target process is a keep-alive application and cannot be killed.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @systemapi
   * @since 27 dynamic
   * @since 27 static
   */
  function killProcessWithAccountSafely(bundleName: string, accountId: int): Promise<void>;
```

---

## 使用示例

```typescript
import appManager from '@ohos.app.ability.appManager';

try {
  // 非 keepalive 应用：查杀成功
  await appManager.killProcessWithAccountSafely('com.example.target', 100);
  console.log('Process killed successfully.');
} catch (err) {
  if (err.code === 2097184) {
    // keepalive 应用被拒绝
    console.warn('Cannot kill keep-alive application.');
  } else if (err.code === 201) {
    console.error('Permission denied.');
  } else {
    console.error(`Failed to kill process: ${err.message}`);
  }
}
```

### 与旧接口对比

```typescript
// 旧接口：直接杀，不校验 keepalive（强杀场景）
await appManager.killProcessWithAccount('com.example.keepalive', 100);
// → 进程被杀掉

// 新接口：校验 keepalive，拒绝杀 keepalive 应用
await appManager.killProcessWithAccountSafely('com.example.keepalive', 100);
// → Promise reject, err.code === 2097184
```
