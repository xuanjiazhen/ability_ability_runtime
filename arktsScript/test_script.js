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
        errorCases: {
            missingFunction: "invoke with -f missingFunction",
            notAFunction: "invoke with -f notFunction",
            thrownError: "invoke with -f failWithError",
        },
    };
}

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

console.log("=== ArkTSScript test script loaded ===");
console.log("BundleName: " + (context.applicationInfo?.bundleName ?? "unknown"));
console.log("Available functions: " + inspectAvailableFunctions().join(", "));
