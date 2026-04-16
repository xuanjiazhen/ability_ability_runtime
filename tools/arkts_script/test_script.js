const context = scriptContext;

function hasOwnPropertySafe(target, key) {
    return Object.prototype.hasOwnProperty.call(target, key);
}

function describeValue(value) {
    if (value === null) {
        return {
            type: "null",
            value: null,
        };
    }
    if (value === undefined) {
        return {
            type: "undefined",
            value: "undefined",
        };
    }
    if (Array.isArray(value)) {
        return {
            type: "array",
            length: value.length,
            value,
        };
    }
    if (typeof value === "object") {
        return {
            type: "object",
            keys: Object.keys(value),
            value,
        };
    }
    return {
        type: typeof value,
        value,
    };
}

function getContextSnapshot(target = context) {
    const appInfo = target.applicationInfo || {};
    return {
        applicationInfo: {
            name: appInfo.name ?? null,
            bundleName: appInfo.bundleName ?? null,
            process: appInfo.process ?? null,
        },
        cacheDir: target.cacheDir ?? null,
        tempDir: target.tempDir ?? null,
        filesDir: target.filesDir ?? null,
        databaseDir: target.databaseDir ?? null,
        preferencesDir: target.preferencesDir ?? null,
        bundleCodeDir: target.bundleCodeDir ?? null,
        distributedFilesDir: target.distributedFilesDir ?? null,
        resourceDir: target.resourceDir ?? null,
        cloudFileDir: target.cloudFileDir ?? null,
        logFileDir: target.logFileDir ?? null,
        area: target.area ?? null,
        processName: target.processName ?? null,
    };
}

function verifyScriptContextContract() {
    const includedProperties = [
        "applicationInfo",
        "cacheDir",
        "tempDir",
        "filesDir",
        "databaseDir",
        "preferencesDir",
        "bundleCodeDir",
        "distributedFilesDir",
        "resourceDir",
        "cloudFileDir",
        "logFileDir",
        "area",
        "processName",
    ];
    const includedFunctions = [
        "createBundleContext",
        "getApplicationContext",
    ];
    const excludedMembers = [
        "resourceManager",
        "eventHub",
        "getGroupDir",
        "createModuleContext",
        "createSystemHspModuleResourceManager",
        "createModuleResourceManager",
        "createDisplayContext",
        "createAreaModeContext",
    ];

    const includedState = {};
    includedProperties.forEach((key) => {
        includedState[key] = {
            exists: key in context,
            own: hasOwnPropertySafe(context, key),
            type: typeof context[key],
        };
    });

    const functionState = {};
    includedFunctions.forEach((key) => {
        functionState[key] = typeof context[key] === "function";
    });

    const excludedState = {};
    excludedMembers.forEach((key) => {
        excludedState[key] = key in context;
    });

    return {
        snapshot: getContextSnapshot(),
        includedProperties: includedState,
        includedFunctions: functionState,
        excludedMembers: excludedState,
    };
}

function getApplicationContextInfo() {
    try {
        const applicationContext = context.getApplicationContext();
        return {
            ok: true,
            snapshot: getContextSnapshot(applicationContext),
            methods: {
                getApplicationContext: typeof applicationContext.getApplicationContext === "function",
                createBundleContext: typeof applicationContext.createBundleContext === "function",
            },
        };
    } catch (error) {
        return {
            ok: false,
            error: String(error && error.message ? error.message : error),
        };
    }
}

function probeCreateBundleContext(bundleName) {
    const targetBundleName = bundleName || context.applicationInfo?.bundleName;
    try {
        const bundleContext = context.createBundleContext(targetBundleName);
        return {
            ok: true,
            bundleName: targetBundleName,
            snapshot: getContextSnapshot(bundleContext),
            excludedMembers: {
                resourceManager: "resourceManager" in bundleContext,
                createModuleContext: "createModuleContext" in bundleContext,
                createModuleResourceManager: "createModuleResourceManager" in bundleContext,
            },
        };
    } catch (error) {
        return {
            ok: false,
            bundleName: targetBundleName ?? null,
            error: String(error && error.message ? error.message : error),
        };
    }
}

function inspectAvailableFunctions() {
    return [
        // 基础同步函数
        "main",
        "add",
        "subtract",
        "multiply",
        "divide",
        "greet",
        "configure",
        "process",
        "update",
        "toggle",
        "calculate",
        "getContextInfo",
        "verifyScriptContextContract",
        "getApplicationContextInfo",
        "probeCreateBundleContext",
        "testTypes",
        "roundTripObject",
        "returnSample",
        "runSelfCheck",
        "failWithError",
        "notFunction",
        // 异步执行测试函数
        "asyncResolve",
        "asyncReject",
        "asyncCalculate",
        "asyncChain",
        "asyncApiCall",
        "asyncImmediate",
        "mixedReturn",
        "nestedPromise",
    ];
}

function main(...args) {
    return {
        function: "main",
        argc: args.length,
        args: args.map((arg, index) => ({
            index,
            ...describeValue(arg),
        })),
        context: {
            bundleName: context.applicationInfo?.bundleName ?? null,
            processName: context.processName ?? null,
        },
    };
}

function add(a, b) {
    if (typeof a !== "number" || typeof b !== "number") {
        return null;
    }
    return a + b;
}

function subtract(a, b) {
    if (typeof a !== "number" || typeof b !== "number") {
        return null;
    }
    return a - b;
}

function multiply(a, b) {
    if (typeof a !== "number" || typeof b !== "number") {
        return null;
    }
    return a * b;
}

function divide(a, b) {
    if (typeof a !== "number" || typeof b !== "number" || b === 0) {
        return null;
    }
    return a / b;
}

function greet(name, greeting) {
    const actualName = name || "World";
    const actualGreeting = greeting || "Hello";
    return actualGreeting + ", " + actualName + "!";
}

function configure(environment, port, debug, timeout) {
    return {
        environment,
        port,
        debug,
        timeout,
        types: {
            environment: typeof environment,
            port: typeof port,
            debug: typeof debug,
            timeout: typeof timeout,
        },
    };
}

function process(data) {
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
        return {
            message: "received object",
            keys: Object.keys(data),
            value: data,
        };
    }
    return "received unknown type";
}

function update(config) {
    if (config === null || config === undefined || typeof config !== "object") {
        return null;
    }
    return {
        ...config,
        updated: true,
        updateMarker: "arkTSScript",
    };
}

function toggle(enable, verbose) {
    const actualEnable = enable === true;
    const actualVerbose = verbose === true;
    return {
        enabled: actualEnable,
        verbose: actualVerbose,
        status: actualEnable ? "active" : "inactive",
    };
}

function calculate(operation, a, b) {
    switch (operation) {
        case "add":
            return add(a, b);
        case "subtract":
            return subtract(a, b);
        case "multiply":
            return multiply(a, b);
        case "divide":
            return divide(a, b);
        default:
            return null;
    }
}

function getContextInfo() {
    return getContextSnapshot();
}

function testTypes(strArg, numArg, boolArg, nullArg, undefinedArg, objArg) {
    return {
        string: describeValue(strArg),
        number: describeValue(numArg),
        boolean: describeValue(boolArg),
        nullValue: describeValue(nullArg),
        undefinedValue: describeValue(undefinedArg),
        object: describeValue(objArg),
    };
}

function roundTripObject(payload) {
    return {
        input: describeValue(payload),
        output: payload,
    };
}

function returnSample(kind) {
    switch (kind) {
        case "undefined":
            return undefined;
        case "null":
            return null;
        case "boolean":
            return true;
        case "number":
            return 123.5;
        case "string":
            return "arkTSScript";
        case "object":
            return {
                ok: true,
                bundleName: context.applicationInfo?.bundleName ?? null,
            };
        case "array":
            return [1, "two", false, null];
        default:
            return {
                supportedKinds: ["undefined", "null", "boolean", "number", "string", "object", "array"],
            };
    }
}

function failWithError(message) {
    throw new Error(message || "intentional test error");
}

// ==================== 异步执行测试用例 ====================

// 返回 resolved Promise 的异步函数
function asyncResolve(value, delayMs) {
    const actualDelay = delayMs || 100;
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve({
                type: "async_resolve",
                input: value,
                delay: actualDelay,
                timestamp: Date.now(),
            });
        }, actualDelay);
    });
}

// 返回 rejected Promise 的异步函数
function asyncReject(errorMessage, delayMs) {
    const actualDelay = delayMs || 100;
    const message = errorMessage || "async rejection";
    return new Promise((_, reject) => {
        setTimeout(() => {
            reject(new Error(message));
        }, actualDelay);
    });
}

// 异步计算 - 模拟耗时操作
function asyncCalculate(operation, a, b, delayMs) {
    const actualDelay = delayMs || 50;
    return new Promise((resolve, reject) => {
        setTimeout(() => {
            if (typeof a !== "number" || typeof b !== "number") {
                reject(new Error("Invalid arguments: expected numbers"));
                return;
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
                    if (b === 0) {
                        reject(new Error("Division by zero"));
                        return;
                    }
                    result = a / b;
                    break;
                default:
                    reject(new Error("Unknown operation: " + operation));
                    return;
            }
            resolve({
                operation,
                a,
                b,
                result,
                async: true,
            });
        }, actualDelay);
    });
}

// 链式异步操作
function asyncChain(steps, delayMs) {
    const actualDelay = delayMs || 50;
    const stepCount = steps || 3;
    
    return new Promise((resolve) => {
        let currentStep = 0;
        const results = [];
        
        function nextStep() {
            currentStep++;
            if (currentStep > stepCount) {
                resolve({
                    type: "async_chain",
                    totalSteps: stepCount,
                    results: results,
                });
                return;
            }
            
            setTimeout(() => {
                results.push({
                    step: currentStep,
                    timestamp: Date.now(),
                });
                nextStep();
            }, actualDelay);
        }
        
        nextStep();
    });
}

// 模拟异步 API 调用
function asyncApiCall(endpoint, data, shouldFail) {
    return new Promise((resolve, reject) => {
        setTimeout(() => {
            if (shouldFail) {
                reject({
                    code: 500,
                    message: "Internal Server Error",
                    endpoint: endpoint,
                });
            } else {
                resolve({
                    code: 200,
                    data: {
                        endpoint: endpoint,
                        received: data,
                        processed: true,
                        timestamp: Date.now(),
                    },
                });
            }
        }, 100);
    });
}

// 立即 resolve 的 Promise（测试微任务）
function asyncImmediate(value) {
    return Promise.resolve({
        type: "immediate_promise",
        value: value,
        note: "resolved immediately",
    });
}

// 测试混合返回类型（有时同步，有时异步）
function mixedReturn(useAsync, value) {
    if (useAsync) {
        return Promise.resolve({
            mode: "async",
            value: value,
        });
    }
    return {
        mode: "sync",
        value: value,
    };
}

// 返回嵌套 Promise（Promise 的 Promise）
function nestedPromise(depth, value) {
    const actualDepth = depth || 2;
    let promise = Promise.resolve(value);
    for (let i = 0; i < actualDepth; i++) {
        promise = promise.then(v => ({ 
            nestedLevel: i + 1, 
            value: v,
            isPromise: true,
        }));
    }
    return promise;
}

function runSelfCheck() {
    return {
        availableFunctions: inspectAvailableFunctions(),
        arithmetic: {
            add: add(10, 20),
            subtract: subtract(10, 3),
            multiply: multiply(6, 7),
            divide: divide(20, 5),
            divideByZero: divide(20, 0),
        },
        typeRoundTrip: testTypes("hello", 42, true, null, undefined, { key: "value" }),
        contextContract: verifyScriptContextContract(),
        applicationContext: getApplicationContextInfo(),
        bundleContext: probeCreateBundleContext(context.applicationInfo?.bundleName),
        returnSamples: {
            undefined: describeValue(returnSample("undefined")),
            null: describeValue(returnSample("null")),
            boolean: describeValue(returnSample("boolean")),
            number: describeValue(returnSample("number")),
            string: describeValue(returnSample("string")),
            object: describeValue(returnSample("object")),
            array: describeValue(returnSample("array")),
        },
        asyncTests: {
            // 注意：这些测试需要在 CLI 中实际调用异步函数来验证
            // 新格式：--abcPath=xxx --functionName=yyy --params={"argv1": value1, "argv2": value2}
            asyncResolve: 'arktsScript --abcPath=/data/scripts/test.abc --functionName=asyncResolve --params={"argv1":"testValue"}',
            asyncReject: 'arktsScript --abcPath=/data/scripts/test.abc --functionName=asyncReject --params={"argv1":"errorMessage"}',
            asyncCalculate: 'arktsScript --abcPath=/data/scripts/test.abc --functionName=asyncCalculate --params={"argv1":"add","argv2":10,"argv3":20}',
            asyncChain: 'arktsScript --abcPath=/data/scripts/test.abc --functionName=asyncChain --params={"argv1":5}',
            asyncApiCall: 'arktsScript --abcPath=/data/scripts/test.abc --functionName=asyncApiCall --params={"argv1":"/api/test","argv2":{"key":"value"}}',
            asyncImmediate: 'arktsScript --abcPath=/data/scripts/test.abc --functionName=asyncImmediate --params={"argv1":"quickValue"}',
            mixedReturn: 'arktsScript --abcPath=/data/scripts/test.abc --functionName=mixedReturn --params={"argv1":true,"argv2":"test"}',
            nestedPromise: 'arktsScript --abcPath=/data/scripts/test.abc --functionName=nestedPromise --params={"argv1":3,"argv2":"nestedValue"}',
        },
        errorCases: {
            missingFunction: "arktsScript --abcPath=/data/scripts/test.abc --functionName=missingFunction",
            notAFunction: "arktsScript --abcPath=/data/scripts/test.abc --functionName=notFunction",
            thrownError: 'arktsScript --abcPath=/data/scripts/test.abc --functionName=failWithError --params={"argv1":"Test error"}',
        },
    };
}

// ==================== 导出所有函数到 globalThis ====================

// 基础同步函数
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
globalThis.verifyScriptContextContract = verifyScriptContextContract;
globalThis.getApplicationContextInfo = getApplicationContextInfo;
globalThis.probeCreateBundleContext = probeCreateBundleContext;
globalThis.testTypes = testTypes;
globalThis.roundTripObject = roundTripObject;
globalThis.returnSample = returnSample;
globalThis.runSelfCheck = runSelfCheck;
globalThis.failWithError = failWithError;
globalThis.notFunction = 123;

// 异步执行测试函数
globalThis.asyncResolve = asyncResolve;
globalThis.asyncReject = asyncReject;
globalThis.asyncCalculate = asyncCalculate;
globalThis.asyncChain = asyncChain;
globalThis.asyncApiCall = asyncApiCall;
globalThis.asyncImmediate = asyncImmediate;
globalThis.mixedReturn = mixedReturn;
globalThis.nestedPromise = nestedPromise;

console.log("=== ArkTSScript test script loaded ===");
console.log("BundleName: " + (context.applicationInfo?.bundleName ?? "unknown"));
console.log("Available functions: " + inspectAvailableFunctions().join(", "));
