// 应用 Skill 测试用例
// 编译命令：ets_frontend --output main.abc main.ets

// 同步函数：简单计算
export function calculate(args: string[]): string {
    const operation = args[0] || "add";
    const a = parseFloat(args[1] || "0");
    const b = parseFloat(args[2] || "0");
    
    let result = 0;
    switch (operation) {
        case "add":
            result = a + b;
            break;
        case "subtract":
            result = a - b;
            break;
        case "multiply":
            result = a * b;
            break;
        case "divide":
            if (b === 0) {
                return JSON.stringify({
                    success: false,
                    error: "Division by zero"
                });
            }
            result = a / b;
            break;
        default:
            return JSON.stringify({
                success: false,
                error: "Unknown operation: " + operation
            });
    }
    
    return JSON.stringify({
        success: true,
        operation: operation,
        result: result,
        inputs: [a, b]
    });
}

// 异步函数：延迟计算
export function asyncCalculate(args: string[]): Promise<string> {
    return new Promise((resolve) => {
        const delay = parseInt(args[0] || "100");
        const value = args[1] || "test";
        
        setTimeout(() => {
            resolve(JSON.stringify({
                success: true,
                operation: "asyncCalculate",
                delay: delay,
                value: value,
                timestamp: Date.now()
            }));
        }, delay);
    });
}

// 字符串处理
export function processString(args: string[]): string {
    const operation = args[0] || "upper";
    const input = args[1] || "";
    
    let result = input;
    switch (operation) {
        case "upper":
            result = input.toUpperCase();
            break;
        case "lower":
            result = input.toLowerCase();
            break;
        case "reverse":
            result = input.split("").reverse().join("");
            break;
        case "length":
            return JSON.stringify({
                success: true,
                operation: operation,
                length: input.length
            });
    }
    
    return JSON.stringify({
        success: true,
        operation: operation,
        input: input,
        result: result
    });
}

// 数组操作
export function arrayOperations(args: string[]): string {
    const operation = args[0] || "join";
    const items = args.slice(1);
    
    let result: any;
    switch (operation) {
        case "join":
            result = items.join(",");
            break;
        case "count":
            result = items.length;
            break;
        case "first":
            result = items[0] || null;
            break;
        case "last":
            result = items[items.length - 1] || null;
            break;
        default:
            result = items;
    }
    
    return JSON.stringify({
        success: true,
        operation: operation,
        result: result,
        items: items
    });
}

// 使用 completeArkTsTool 的手动完成
export function manualProcess(args: string[]): void {
    const step = args[0] || "step1";
    
    // 模拟多步处理
    setTimeout(() => {
        const result = JSON.stringify({
            success: true,
            step: step,
            message: "Processing completed",
            mode: "manual"
        });
        completeArkTsTool(0, result);
    }, 50);
}

// 错误测试：抛出异常
export function throwError(args: string[]): string {
    const errorType = args[0] || "generic";
    
    if (errorType === "generic") {
        throw new Error("Generic error thrown");
    } else if (errorType === "range") {
        throw new RangeError("Range error thrown");
    } else if (errorType === "type") {
        throw new TypeError("Type error thrown");
    }
    
    return JSON.stringify({ success: true });
}

// 返回 Promise 并 reject
export function asyncError(args: string[]): Promise<string> {
    return new Promise((resolve, reject) => {
        const errorMessage = args[0] || "Async error occurred";
        
        setTimeout(() => {
            reject(JSON.stringify({
                success: false,
                error: errorMessage
            }));
        }, 50);
    });
}