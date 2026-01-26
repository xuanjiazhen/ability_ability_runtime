## gitcode的基本配置
1.  git token是： sNyUor41XC2DLoHhVzg1C6RR
2.  gitcode账号为： dsz2025
3.  ability_runtime代码仓地址： https://gitcode.com/openharmony/ability_ability_runtime, git config对应的配置为 upstream
4.  issue地址： https://gitcode.com/openharmony/ability_ability_runtime/issues
5.  个人仓地址：https://gitcode.com/dsz2025/ability_ability_runtime，git config中对应的配置为origin


## 门禁检查：
static-check 静态检查
start build 门禁编译（注意：需要pr绑定了issue并且static-check通过的情况下才能触发）

## AI检视工具：
`
BASE_URL = "https://openharmonyinsight.cn/api/v1/review"

response = requests.post(
        url=BASE_URL,
        json={
            "gitcode_pr_url": pr_url,
            "gitcode_token": token,
            "review_type": "deep"
        },
        headers={"Content-Type": "application/json"},
    )
其中BASE_URL是API地址，pr_url是需要检视PR
`

## 代码提交完整流程
1. 提交代码并提交到个人仓中
2. 在origin仓中创建PR和issue
3. 将PR与issue绑定
4. AI检视：利用AI检视工具对PR进行分析，过滤不合理的意见
5. 静态检查
6. 门禁编译：以上所有步骤完成后可以进行门禁编译