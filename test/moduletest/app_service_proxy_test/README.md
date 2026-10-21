# AppService 代理连接模块测试

本套由开发维护：native 测试进程模拟 SA，使用已安装应用的真实 token/UID，经 AbilityManagerClient 调用设备上的 AMS，验证 AppService 冷加载、连接回调、RPC 和后台用户实例。它不实现正式 EDM 业务 API，不替代 MDM Kit/EDM 端到端联调。

## 构建

在配套 OpenHarmony 源码根目录运行：

```sh
./build.sh --product-name <product> --build-target app_service_proxy_tests --ccache
```

产品名按测试设备选择。构建从现有 SDK 创建独立视图，并同步当前源码的权限目录；不修改共享 SDK。产物位于 `out/<product>/tests/moduletest/ability_runtime/ability_runtime/app_service/`，包含测试程序和 `resource/` 下的 HAP、脚本。HAP 由源码生成，不提交编译后的 HAP。signature/ 中保存测试签名 profile 及其 JSON；可用 generate_profile.py 和仓内公开演示密钥重新生成。

## 设备与运行

使用允许 native 测试进程设置测试 token 的 OpenHarmony 工程设备。镜像须含配套 AMS、新权限定义及基础 AppService 产品配置（`const.abilityms.support_app_service=true`）；完整能力配置 `const.abilityms.app_service_full_capability` 按产品能力设置。测试不修改生产配置，也不会伪造权限服务的授权结果。测试程序、HAP 与设备的系统库须来自配套源码/SDK；不支持把当前测试程序直接放到任意旧镜像运行。SDK 26.0.1 需预先按工程预编译流程准备。

测试 profile 使用仓内公开演示证书，发行类型为 enterprise_mdm，供认可该测试根证书的工程镜像使用。正式产品必须使用其认可的测试签名；替换签名时保持 caller/denied 的 appIdentifier 与目标允许名单一致。不能通过删除 MDM 授予约束来使测试通过。

```sh
hdc file send out/<product>/tests/moduletest/ability_runtime/ability_runtime/app_service /data/local/tmp/
hdc shell 'cd /data/local/tmp/app_service && sh resource/run.sh'
```

测试程序查询前台用户，以具有企业包安装权限的模拟 SA 安装四个专用包，结束后卸载；跨用户用例创建独立测试用户，在准备阶段激活该用户以初始化系统服务，再切回原用户、设置并核验测试用户的解锁状态。随后向后台测试用户安装目标 HAP，连接期间检查前台用户不变，结束后删除测试用户。仅在允许创建、激活和解锁普通测试用户的专用工程设备执行。已有同名测试包时拒绝覆盖，须先检查并清理上次运行遗留包。

fixture 安装、产品能力、权限授权、账户准备失败均会明确失败，不作为测试通过。连接及 RPC 结果轮询最多等待 15 秒，安装回调最多等待 45 秒；这些是测试超时保护，不是产品性能指标。结果读取 `app_service_results.xml` 及终端断言；组件日志标签为 `AppServiceTest`。成功连接必须有成功回调、非空对象、匹配本次请求的 RPC 回复与正确用户 UID。

## 测试映射

| 用例 | 验证内容 |
| --- | --- |
| Connect_001 | 新许可目标冷加载、回调与通信 |
| ComponentPermission_001 | 无目标许可拒绝 |
| CallerPermission_001 | 真实 A 的访问权限正反例 |
| Visibility_001 | 不对外开放的目标拒绝 |
| ColdAndActive_001 | 冷组件的允许名单、已运行组件连接 |
| InvalidIdentity_001 | token/UID 不一致拒绝 |
| BackgroundUi_001 | 根据镜像完整能力配置验证后台 startAbility 的公开 Promise 结果 |
| BackgroundUser_001 | 两用户同包目标，后台实例的正确路由 |

`legacy` HAP 供支持旧许可的产品回归使用。前台应用直连另做应用集成验证；本模块测试只代表当前镜像配置的结果，不自动代表其他产品配置或完整产品验收通过。异常断电导致清理未执行时，删除本套专用测试用户和 `com.ohos.appservice.proxy.*` 测试包后再运行。
