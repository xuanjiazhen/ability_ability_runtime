function toArgs(args) {
    return Array.isArray(args) ? args : [];
}

function isNumericValue(value) {
    if (typeof value === "number") {
        return Number.isFinite(value);
    }
    if (typeof value !== "string" || value.trim() === "") {
        return false;
    }
    return Number.isFinite(Number(value));
}

function toNumberOrNull(value) {
    return isNumericValue(value) ? Number(value) : null;
}

function describeValue(value) {
    if (value === undefined) {
        return {
            type: "undefined",
            value: null,
        };
    }
    if (value === null) {
        return {
            type: "null",
            value: null,
        };
    }
    if (Array.isArray(value)) {
        return {
            type: "array",
            length: value.length,
            values: value.map((item) => describeValue(item)),
        };
    }
    return {
        type: typeof value,
        value,
    };
}

function currentContext() {
    if (globalThis.scriptContext) {
        return globalThis.scriptContext;
    }
    if (globalThis.context) {
        return globalThis.context;
    }
    return null;
}

function snapshotContext(ctx = currentContext()) {
    return {
        hasContext: ctx !== null && ctx !== undefined,
        hasGetApplicationContext: !!ctx && typeof ctx.getApplicationContext === "function",
        hasCreateBundleContext: !!ctx && typeof ctx.createBundleContext === "function",
        applicationInfo: ctx && ctx.applicationInfo ? {
            name: ctx.applicationInfo.name ?? null,
            bundleName: ctx.applicationInfo.bundleName ?? null,
            process: ctx.applicationInfo.process ?? null,
        } : null,
        bundleName: ctx && ctx.bundleName !== undefined ? ctx.bundleName : null,
        processName: ctx && ctx.processName !== undefined ? ctx.processName : null,
    };
}

function completeResult(result) {
    if (typeof globalThis.CompletearktsScript === "function") {
        globalThis.CompletearktsScript({
            code: 0,
            result,
        });
        return "pending";
    }
    return result;
}

function completeErrorResult(error, extra = {}) {
    if (typeof globalThis.CompletearktsScript === "function") {
        globalThis.CompletearktsScript({
            code: -1,
            result: {
                case: "error",
                error,
                ...extra,
            },
        });
        return "pending";
    }
    return {
        case: "error",
        error,
        ...extra,
    };
}

function buildResult(result, targetKind) {
    return {
        result,
        targetKind,
    };
}

function calculateAddResult(args) {
    const list = toArgs(args);
    const left = list[0];
    const right = list[1];
    const leftNumber = toNumberOrNull(left);
    const rightNumber = toNumberOrNull(right);
    return leftNumber !== null && rightNumber !== null ? leftNumber + rightNumber : null;
}

export function add(args) {
    return completeResult(buildResult(calculateAddResult(args), "namedExport"));
}

export function addNumber(args) {
    return completeResult(buildResult(calculateAddResult(args), "namedExport:addNumber"));
}

export function addInvalid(args) {
    const list = toArgs(args);
    const left = list[0];
    const right = list[1];
    const leftNumber = toNumberOrNull(left);
    const rightNumber = toNumberOrNull(right);
    if (leftNumber === null || rightNumber === null) {
        return completeErrorResult("ADD_ARGS_INVALID");
    }
    return completeResult(buildResult(calculateAddResult(args), "namedExport:addInvalid"));
}

export function parseValues(args) {
    const list = toArgs(args);
    const typedValues = list.map((value) => describeValue(value));
    return completeResult(buildResult(typedValues, "namedExport:parseValues"));
}

export function parseEmptyArgs(args) {
    return completeResult(buildResult({
        isEmpty: toArgs(args).length === 0,
    }, "namedExport:parseEmptyArgs"));
}

export function parseArgGap(args) {
    const list = toArgs(args);
    return completeResult(buildResult({
        hasArgGap: list.some((value) => value === undefined),
    }, "namedExport:parseArgGap"));
}

export function throwError(args) {
    const list = toArgs(args);
    throw new Error("throwError expected failure, argc=" + list.length);
}

export function testCatalog(args) {
    return completeResult(buildResult({
        cases: [
            // 1. Expected: exitCode=0, stdout contains help usage and parameter descriptions.
            {
                functionName: "--help",
                scene: "cli help output",
                command: "ohos-arktsScript --help",
                expected: {
                    exitCode: 0,
                    stdoutIncludes: [
                        "ohos-arktsScript - ArkTS script execution tool for loading ABC files and invoking target functions",
                        "Usage:",
                        "ohos-arktsScript [options]",
                        "Parameters:",
                        "--abcPath <path>",
                        "--scriptPath <path>",
                        "--functionName <name>",
                        "--args <jsonObject>",
                        "--help",
                        "Examples:",
                    ],
                },
            },
            // 2. Expected: code=0, result.result=20, result.targetKind=namedExport.
            {
                functionName: "add",
                scene: "named export add, numeric strings are converted before summing",
                args: "{\"arg0\":\"10\",\"arg1\":\"10\"}",
                expected: {
                    code: 0,
                    result: {
                        result: 20,
                        targetKind: "namedExport",
                    },
                },
            },
            // 3. Expected: code=0, result.result=30, result.targetKind=defaultClassMethod:addByDefaultClass.
            {
                functionName: "addByDefaultClass",
                scene: "default class method dispatch",
                args: "{\"arg0\":10,\"arg1\":20}",
                expected: {
                    code: 0,
                    result: {
                        result: 30,
                        targetKind: "defaultClassMethod:addByDefaultClass",
                    },
                },
            },
            // 4. Expected: code=0, result.result=20, result.targetKind=namedExport:addNumber.
            {
                functionName: "addNumber",
                scene: "named export add with numeric JSON arguments",
                args: "{\"arg0\":10,\"arg1\":10}",
                expected: {
                    code: 0,
                    result: {
                        result: 20,
                        targetKind: "namedExport:addNumber",
                    },
                },
            },
            // 5. Expected: code=-1, result.case=error, result.error=ADD_ARGS_INVALID.
            {
                functionName: "addInvalid",
                scene: "add abnormal branch: non-numeric argument",
                args: "{\"arg0\":\"abc\",\"arg1\":10}",
                expected: {
                    code: -1,
                    result: {
                        case: "error",
                        error: "ADD_ARGS_INVALID",
                    },
                },
            },
            // 6. Expected: code=0, result.targetKind=namedExport:parseValues, result.result length=5.
            {
                functionName: "parseValues",
                scene: "string, number, boolean, object and array argument conversion",
                args: "{\"arg0\":\"abc\",\"arg1\":1,\"arg2\":true,\"arg3\":{\"name\":\"demo\"},\"arg4\":[1,2]}",
                expected: {
                    code: 0,
                    result: {
                        targetKind: "namedExport:parseValues",
                    },
                },
            },
            // 7. Expected: code=0, result.result.isEmpty=true, result.targetKind=namedExport:parseEmptyArgs.
            {
                functionName: "parseEmptyArgs",
                scene: "empty --args or no --args produces an empty argument array",
                args: "{}",
                expected: {
                    code: 0,
                    result: {
                        result: {
                            isEmpty: true,
                        },
                        targetKind: "namedExport:parseEmptyArgs",
                    },
                },
            },
            // 8. Expected: code=0, result.result.hasArgGap=true, result.targetKind=namedExport:parseArgGap.
            {
                functionName: "parseArgGap",
                scene: "argument index gap fills missing slots as undefined",
                args: "{\"arg1\":\"second\"}",
                expected: {
                    code: 0,
                    result: {
                        result: {
                            hasArgGap: true,
                        },
                        targetKind: "namedExport:parseArgGap",
                    },
                },
            },
            // 9. Expected: code=0, result.targetKind=namedExport:describeContext.
            {
                functionName: "describeContext",
                scene: "script context, application context and bundle context availability",
                args: "{}",
                expected: {
                    code: 0,
                    result: {
                        targetKind: "namedExport:describeContext",
                    },
                },
            },
            // 10. Expected error: failed to parse arguments.
            {
                functionName: "add",
                scene: "abcPath is empty",
                command: "ohos-arktsScript --abcPath '' --functionName add",
                expectedError: "failed to parse arguments",
            },
            // 11. Expected error: failed to parse arguments.
            {
                functionName: "add",
                scene: "functionName is empty",
                command: "ohos-arktsScript --abcPath /data/test/module.abc --functionName ''",
                expectedError: "failed to parse arguments",
            },
            // 12. Expected error: failed to resolve function.
            {
                functionName: "notExists",
                scene: "abcPath is valid and functionName is fake",
                command: "ohos-arktsScript --abcPath /data/test/module.abc --functionName notExists",
                expectedError: "failed to resolve function",
            },
            // 13. Expected error: failed to parse arguments.
            {
                functionName: "add",
                scene: "abcPath is valid, functionName is valid and args is invalid",
                command: "ohos-arktsScript --abcPath /data/test/module.abc --functionName add --args '{\"invalid\":10}'",
                expectedError: "failed to parse arguments",
            },
            // 14. Expected error: failed to call function.
            {
                functionName: "throwError",
                scene: "target JS function throws",
                expectedError: "failed to call function",
            },
        ],
    }, "namedExport:testCatalog"));
}

export function selfCheck(args) {
    return completeResult(buildResult({
        exports: [
            "add",
            "addNumber",
            "addInvalid",
            "parseValues",
            "parseEmptyArgs",
            "parseArgGap",
            "describeContext",
            "throwError",
            "testCatalog",
            "selfCheck",
            "default",
            "ArktsScriptSuite",
        ],
    }, "namedExport:selfCheck"));
}

export function describeContext(args) {
    const list = toArgs(args);
    const ctx = currentContext();
    const appCtx = ctx && typeof ctx.getApplicationContext === "function" ? ctx.getApplicationContext() : null;
    const bundleName = list.length > 0 ? list[0] : (ctx && ctx.applicationInfo && ctx.applicationInfo.bundleName) || "";
    const bundleCtx = ctx && typeof ctx.createBundleContext === "function" ? ctx.createBundleContext(bundleName) : null;

    return completeResult(buildResult({
        context: snapshotContext(ctx),
        applicationContext: snapshotContext(appCtx),
        bundleContext: snapshotContext(bundleCtx),
    }, "namedExport:describeContext"));
}

export default class ArktsScriptSuite {
    addByDefaultClass(args) {
        return completeResult(buildResult(calculateAddResult(args), "defaultClassMethod:addByDefaultClass"));
    }
}
globalThis.ArktsScriptSuite = ArktsScriptSuite;

if (typeof console !== "undefined" && console !== null) {
    console.log("=== ArkTS script test loaded ===");
    console.log("Context ready: " + (currentContext() !== null));
}
