// 测试脚本 - test_script.js
// 这个脚本会被 arkTSScript.bin 加载和执行
// 支持多种参数类型：string、number、boolean、null、undefined、object

// 获取绑定的 scriptContext
const context = scriptContext;

console.log("=== Test Script Loaded ===");
console.log("BundleName: " + context.bundleName);
console.log("CacheDir: " + context.cacheDir);
console.log("FilesDir: " + context.filesDir);
console.log("TempDir: " + context.tempDir);
console.log("");

// 默认入口函数 - 接收任意参数并打印类型信息
function main(...args) {
    console.log("=== Main function called ===");
    console.log("Arguments count: " + args.length);
    
    args.forEach((arg, index) => {
        const type = typeof arg;
        let valueStr;
        
        if (arg === null) {
            valueStr = "null";
        } else if (arg === undefined) {
            valueStr = "undefined";
        } else if (type === "object") {
            valueStr = JSON.stringify(arg);
        } else if (type === "string") {
            valueStr = '"' + arg + '"';
        } else {
            valueStr = String(arg);
        }
        
        console.log("  Arg[" + index + "]: " + valueStr + " (type: " + type + ")");
    });
    
    // 返回所有参数
    return args;
}

// 加法函数 - 接收数字
function add(a, b) {
    console.log("=== Add function ===");
    console.log("a = " + a + " (" + typeof a + ")");
    console.log("b = " + b + " (" + typeof b + ")");
    
    if (typeof a !== "number" || typeof b !== "number") {
        console.error("Error: Arguments must be numbers");
        return null;
    }
    
    const result = a + b;
    console.log("Result: " + result);
    return result;
}

// 减法函数
function subtract(a, b) {
    console.log("=== Subtract function ===");
    if (typeof a !== "number" || typeof b !== "number") {
        console.error("Error: Arguments must be numbers");
        return null;
    }
    return a - b;
}

// 乘法函数
function multiply(a, b) {
    console.log("=== Multiply function ===");
    if (typeof a !== "number" || typeof b !== "number") {
        console.error("Error: Arguments must be numbers");
        return null;
    }
    return a * b;
}

// 除法函数
function divide(a, b) {
    console.log("=== Divide function ===");
    if (typeof a !== "number" || typeof b !== "number") {
        console.error("Error: Arguments must be numbers");
        return null;
    }
    if (b === 0) {
        console.error("Error: Division by zero");
        return null;
    }
    return a / b;
}

// 问候函数 - 接收字符串
function greet(name, greeting) {
    console.log("=== Greet function ===");
    
    const actualName = name || "World";
    const actualGreeting = greeting || "Hello";
    
    console.log("name = " + JSON.stringify(actualName) + " (" + typeof actualName + ")");
    console.log("greeting = " + JSON.stringify(actualGreeting) + " (" + typeof actualGreeting + ")");
    
    const msg = actualGreeting + ", " + actualName + "!";
    console.log("Message: " + msg);
    return msg;
}

// 配置函数 - 接收混合类型参数
function configure(environment, port, debug, timeout) {
    console.log("=== Configure function ===");
    
    const config = {
        environment: environment,
        port: port,
        debug: debug,
        timeout: timeout
    };
    
    console.log("Configuration:");
    console.log("  environment: " + JSON.stringify(config.environment) + " (" + typeof config.environment + ")");
    console.log("  port: " + config.port + " (" + typeof config.port + ")");
    console.log("  debug: " + config.debug + " (" + typeof config.debug + ")");
    console.log("  timeout: " + config.timeout + " (" + typeof config.timeout + ")");
    
    return config;
}

// 处理函数 - 接收任意类型并返回描述
function process(data) {
    console.log("=== Process function ===");
    console.log("Input: " + JSON.stringify(data) + " (" + typeof data + ")");
    
    if (data === null) {
        return "received null";
    }
    if (data === undefined) {
        return "received undefined";
    }
    if (typeof data === "boolean") {
        return "received boolean: " + data;
    }
    if (typeof data === "number") {
        return "received number: " + data;
    }
    if (typeof data === "string") {
        return "received string: " + data;
    }
    if (typeof data === "object") {
        return "received object with keys: " + Object.keys(data).join(", ");
    }
    
    return "received unknown type";
}

// 更新函数 - 接收 JSON 对象
function update(config) {
    console.log("=== Update function ===");
    
    if (config === null || config === undefined) {
        console.error("Error: config cannot be null or undefined");
        return null;
    }
    
    if (typeof config !== "object") {
        console.error("Error: config must be an object");
        return null;
    }
    
    console.log("Config keys: " + Object.keys(config).join(", "));
    console.log("Config: " + JSON.stringify(config));
    
    // 模拟更新操作
    config.updated = true;
    config.timestamp = Date.now();
    
    return config;
}

// 条件函数 - 接收布尔值
function toggle(enable, verbose) {
    console.log("=== Toggle function ===");
    
    const actualEnable = enable === true;
    const actualVerbose = verbose === true;
    
    console.log("enable = " + actualEnable + " (" + typeof actualEnable + ")");
    console.log("verbose = " + actualVerbose + " (" + typeof actualVerbose + ")");
    
    return {
        enabled: actualEnable,
        verbose: actualVerbose,
        status: actualEnable ? "active" : "inactive"
    };
}

// 计算函数 - 综合示例
function calculate(operation, a, b) {
    console.log("=== Calculate function ===");
    console.log("Operation: " + operation);
    console.log("a = " + a + ", b = " + b);
    
    if (typeof a !== "number" || typeof b !== "number") {
        console.error("Error: a and b must be numbers");
        return null;
    }
    
    let result;
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
            result = b !== 0 ? a / b : null;
            break;
        default:
            console.error("Unknown operation: " + operation);
            return null;
    }
    
    console.log("Result: " + result);
    return result;
}

// 获取上下文信息
function getContextInfo() {
    console.log("=== Context Info ===");
    const info = {
        bundleName: context.bundleName,
        cacheDir: context.cacheDir,
        filesDir: context.filesDir,
        databaseDir: context.databaseDir,
        preferencesDir: context.preferencesDir,
        tempDir: context.tempDir,
        bundleCodeDir: context.bundleCodeDir
    };
    console.log(JSON.stringify(info, null, 2));
    return info;
}

// 测试所有类型
function testTypes(strArg, numArg, boolArg, nullArg, undefinedArg, objArg) {
    console.log("=== Test Types ===");
    
    console.log("String: " + JSON.stringify(strArg) + " (" + typeof strArg + ")");
    console.log("Number: " + numArg + " (" + typeof numArg + ")");
    console.log("Boolean: " + boolArg + " (" + typeof boolArg + ")");
    console.log("Null: " + nullArg + " (" + typeof nullArg + ")");
    console.log("Undefined: " + undefinedArg + " (" + typeof undefinedArg + ")");
    console.log("Object: " + JSON.stringify(objArg) + " (" + typeof objArg + ")");
    
    return {
        string: { value: strArg, type: typeof strArg },
        number: { value: numArg, type: typeof numArg },
        boolean: { value: boolArg, type: typeof boolArg },
        null: { value: nullArg, type: typeof nullArg },
        undefined: { value: undefinedArg, type: typeof undefinedArg },
        object: { value: objArg, type: typeof objArg }
    };
}

// 确保所有函数在 global 上
if (typeof globalThis !== 'undefined') {
    globalThis.main = main;
    globalThis.add = add;
    globalThis.subtract = subtract;
    globalThis.multiply = multiply;
    globalThis.divide = divide;
    globalThis.greet = greet;
    globalThis.configure = configure;
    globalThis.process = process;
    globalThis.update = update;
    globalThis.toggle = toggle;
    globalThis.calculate = calculate;
    globalThis.getContextInfo = getContextInfo;
    globalThis.testTypes = testTypes;
}

console.log("=== Test Script Ready ===");
console.log("Available functions: main, add, subtract, multiply, divide, greet, configure, process, update, toggle, calculate, getContextInfo, testTypes");
console.log("");
