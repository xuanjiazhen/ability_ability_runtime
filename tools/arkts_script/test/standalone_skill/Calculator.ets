// 独立 Skill 测试用例：Calculator
// 编译命令：ets_frontend --output Calculator.abc Calculator.ets

export default class Calculator {
    // 同步加法
    add(args: string[]): string {
        const a = parseFloat(args[0] || "0");
        const b = parseFloat(args[1] || "0");
        const result = a + b;
        return JSON.stringify({
            success: true,
            operation: "add",
            result: result,
            inputs: [a, b]
        });
    }

    // 同步减法
    subtract(args: string[]): string {
        const a = parseFloat(args[0] || "0");
        const b = parseFloat(args[1] || "0");
        const result = a - b;
        return JSON.stringify({
            success: true,
            operation: "subtract",
            result: result,
            inputs: [a, b]
        });
    }

    // 异步乘法（Promise）
    multiply(args: string[]): Promise<string> {
        return new Promise((resolve) => {
            const a = parseFloat(args[0] || "0");
            const b = parseFloat(args[1] || "0");
            const result = a * b;
            
            // 模拟异步操作
            setTimeout(() => {
                resolve(JSON.stringify({
                    success: true,
                    operation: "multiply",
                    result: result,
                    inputs: [a, b]
                }));
            }, 100);
        });
    }

    // 异步除法（Promise，带错误处理）
    divide(args: string[]): Promise<string> {
        return new Promise((resolve, reject) => {
            const a = parseFloat(args[0] || "0");
            const b = parseFloat(args[1] || "0");
            
            if (b === 0) {
                reject(JSON.stringify({
                    success: false,
                    operation: "divide",
                    error: "Division by zero",
                    inputs: [a, b]
                }));
                return;
            }
            
            const result = a / b;
            setTimeout(() => {
                resolve(JSON.stringify({
                    success: true,
                    operation: "divide",
                    result: result,
                    inputs: [a, b]
                }));
            }, 100);
        });
    }

    // 使用 completeArkTsTool 的手动完成模式
    manualComplete(args: string[]): void {
        const operation = args[0] || "unknown";
        const result = args[1] || "no result";
        
        // 模拟一些处理
        const output = JSON.stringify({
            success: true,
            operation: operation,
            result: result,
            mode: "manual"
        });
        
        // 调用全局 completeArkTsTool 函数通知完成
        completeArkTsTool(0, output);
    }

    // 字符串拼接测试
    concat(args: string[]): string {
        const result = args.join("-");
        return JSON.stringify({
            success: true,
            operation: "concat",
            result: result,
            count: args.length
        });
    }

    // 空参数测试
    noArgs(): string {
        return JSON.stringify({
            success: true,
            operation: "noArgs",
            message: "No arguments provided"
        });
    }
}