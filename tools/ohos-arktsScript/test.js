import scriptManager from "@ohos.app.ability.scriptManager";

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
        cacheDir: ctx && ctx.cacheDir !== undefined ? ctx.cacheDir : null,
        tempDir: ctx && ctx.tempDir !== undefined ? ctx.tempDir : null,
        filesDir: ctx && ctx.filesDir !== undefined ? ctx.filesDir : null,
        databaseDir: ctx && ctx.databaseDir !== undefined ? ctx.databaseDir : null,
        preferencesDir: ctx && ctx.preferencesDir !== undefined ? ctx.preferencesDir : null,
        bundleCodeDir: ctx && ctx.bundleCodeDir !== undefined ? ctx.bundleCodeDir : null,
        distributedFilesDir: ctx && ctx.distributedFilesDir !== undefined ? ctx.distributedFilesDir : null,
        resourceDir: ctx && ctx.resourceDir !== undefined ? ctx.resourceDir : null,
        cloudFileDir: ctx && ctx.cloudFileDir !== undefined ? ctx.cloudFileDir : null,
        logFileDir: ctx && ctx.logFileDir !== undefined ? ctx.logFileDir : null,
    };
}

function buildExpectedResult(caseName, args, extra = {}) {
    const list = toArgs(args);
    return {
        case: caseName,
        argc: list.length,
        args: list,
        ...extra,
    };
}

function buildCaseResult(caseName, args, extra = {}) {
    return {
        ...buildExpectedResult(caseName, args),
        context: snapshotContext(),
        ...extra,
    };
}

function withInterfaceInfo(result, calledInterface, jsCall, expectedResult) {
    return {
        ...result,
        calledInterface,
        jsCall,
        expectedResult,
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

function buildAddResult(args, targetKind, calledInterface, jsCall) {
    const list = toArgs(args);
    const left = list[0];
    const right = list[1];
    const leftNumber = toNumberOrNull(left);
    const rightNumber = toNumberOrNull(right);
    const canAdd = leftNumber !== null && rightNumber !== null;
    const expectedResult = buildExpectedResult("add", args, {
        targetKind,
        left,
        right,
        leftNumber,
        rightNumber,
        sum: canAdd ? leftNumber + rightNumber : null,
    });
    return withInterfaceInfo(buildCaseResult("add", args, {
        targetKind,
        left,
        right,
        leftNumber,
        rightNumber,
        sum: expectedResult.sum,
    }), calledInterface, jsCall, expectedResult);
}

export async function add(args) {
    await scriptManager.completeArkTSScript({
        code: 0,
        result: buildAddResult(args, "namedExport", "export async function add(args)",
            "await scriptManager.completeArkTSScript({ code: 0, result: buildAddResult(args, 'namedExport') })"),
    });
    return "pending";
}

export function addNumber(args) {
    return completeResult(buildAddResult(args, "namedExport:addNumber", "export function addNumber(args)",
        "CompletearktsScript({ code: 0, result: buildAddResult(args, 'namedExport:addNumber') })"));
}

export function addInvalid(args) {
    const list = toArgs(args);
    const left = list[0];
    const right = list[1];
    const leftNumber = toNumberOrNull(left);
    const rightNumber = toNumberOrNull(right);
    const expectedResult = {
        case: "addInvalid",
        argc: list.length,
        args: list,
        error: "ADD_ARGS_INVALID",
        reason: "arg0 and arg1 must be numeric values",
        leftNumber,
        rightNumber,
    };
    if (leftNumber === null || rightNumber === null) {
        return completeErrorResult("ADD_ARGS_INVALID", withInterfaceInfo({
            reason: "arg0 and arg1 must be numeric values",
            argc: list.length,
            args: list,
            leftNumber,
            rightNumber,
        }, "export function addInvalid(args)",
            "CompletearktsScript({ code: -1, result: { case: 'error', error: 'ADD_ARGS_INVALID' } })",
            expectedResult));
    }
    return completeResult(buildAddResult(args, "namedExport:addInvalid", "export function addInvalid(args)",
        "CompletearktsScript({ code: 0, result: buildAddResult(args, 'namedExport:addInvalid') })"));
}

export function parseValues(args) {
    const list = toArgs(args);
    const typedValues = list.map((value) => describeValue(value));
    return completeResult(withInterfaceInfo(buildCaseResult("parseValues", args, {
        typedValues,
    }), "export function parseValues(args)",
        "return { case: 'parseValues', typedValues: args.map(describeValue) }",
        buildExpectedResult("parseValues", args, {
            typedValues,
        })));
}

export function parseEmptyArgs(args) {
    return completeResult(withInterfaceInfo(buildCaseResult("parseEmptyArgs", args, {
        isEmpty: toArgs(args).length === 0,
    }), "export function parseEmptyArgs(args)",
        "return { case: 'parseEmptyArgs', isEmpty: args.length === 0 }",
        buildExpectedResult("parseEmptyArgs", args, {
            isEmpty: toArgs(args).length === 0,
        })));
}

export function parseArgGap(args) {
    const list = toArgs(args);
    const typedValues = list.map((value) => describeValue(value));
    return completeResult(withInterfaceInfo(buildCaseResult("parseArgGap", args, {
        typedValues,
        hasArgGap: list.some((value) => value === undefined),
    }), "export function parseArgGap(args)",
        "return { case: 'parseArgGap', typedValues, hasArgGap }",
        buildExpectedResult("parseArgGap", args, {
            typedValues,
            hasArgGap: list.some((value) => value === undefined),
        })));
}

export function throwError(args) {
    const list = toArgs(args);
    throw new Error("throwError expected failure, argc=" + list.length);
}

export function completeWithoutArgument(args) {
    if (typeof globalThis.CompletearktsScript === "function") {
        globalThis.CompletearktsScript();
        return "pending";
    }
    return buildCaseResult("completeWithoutArgument", args);
}

export function completeWithNonObject(args) {
    if (typeof globalThis.CompletearktsScript === "function") {
        globalThis.CompletearktsScript("invalid");
        return "pending";
    }
    return buildCaseResult("completeWithNonObject", args);
}

export function completeWithoutCode(args) {
    if (typeof globalThis.CompletearktsScript === "function") {
        globalThis.CompletearktsScript({
            result: {
                case: "completeWithoutCode",
            },
        });
        return "pending";
    }
    return buildCaseResult("completeWithoutCode", args);
}

export function completeWithInvalidResult(args) {
    if (typeof globalThis.CompletearktsScript === "function") {
        globalThis.CompletearktsScript({
            code: 0,
            result: "invalid",
        });
        return "pending";
    }
    return buildCaseResult("completeWithInvalidResult", args);
}

export function completeWithInvalidUri(args) {
    if (typeof globalThis.CompletearktsScript === "function") {
        globalThis.CompletearktsScript({
            code: 0,
            result: {
                case: "completeWithInvalidUri",
            },
            uri: 100,
        });
        return "pending";
    }
    return buildCaseResult("completeWithInvalidUri", args);
}

export function completeWithInvalidFlag(args) {
    if (typeof globalThis.CompletearktsScript === "function") {
        globalThis.CompletearktsScript({
            code: 0,
            result: {
                case: "completeWithInvalidFlag",
            },
            flag: "invalid",
        });
        return "pending";
    }
    return buildCaseResult("completeWithInvalidFlag", args);
}

export function testCatalog(args) {
    return completeResult(buildCaseResult("testCatalog", args, {
        cases: [
            // Expected: code=0, result.case=add, result.sum=20.
            {
                functionName: "add",
                scene: "named export add, numeric strings are converted before summing",
                args: "{\"arg0\":\"10\",\"arg1\":\"10\"}",
                expected: {
                    code: 0,
                    result: {
                        case: "add",
                        targetKind: "namedExport",
                        left: "10",
                        right: "10",
                        leftNumber: 10,
                        rightNumber: 10,
                        sum: 20,
                    },
                },
            },
            // Expected: code=0, result.case=add, result.targetKind=defaultClassMethod:addByDefaultClass, result.sum=30.
            {
                functionName: "addByDefaultClass",
                scene: "default class method dispatch",
                args: "{\"arg0\":10,\"arg1\":20}",
                expected: {
                    code: 0,
                    result: {
                        case: "add",
                        targetKind: "defaultClassMethod:addByDefaultClass",
                        sum: 30,
                    },
                },
            },
            // Expected: code=0, result.case=add, result.sum=20.
            {
                functionName: "addNumber",
                scene: "named export add with numeric JSON arguments",
                args: "{\"arg0\":10,\"arg1\":10}",
                expected: {
                    code: 0,
                    result: {
                        case: "add",
                        targetKind: "namedExport:addNumber",
                        leftNumber: 10,
                        rightNumber: 10,
                        sum: 20,
                    },
                },
            },
            // Expected: code=-1, result.case=error, result.error=ADD_ARGS_INVALID.
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
            // Expected: code=0, result.case=parseValues, result.argc=5.
            {
                functionName: "parseValues",
                scene: "string, number, boolean, object and array argument conversion",
                args: "{\"arg0\":\"abc\",\"arg1\":1,\"arg2\":true,\"arg3\":{\"name\":\"demo\"},\"arg4\":[1,2]}",
                expected: {
                    code: 0,
                    result: {
                        case: "parseValues",
                        argc: 5,
                    },
                },
            },
            // Expected: code=0, result.case=parseEmptyArgs, result.isEmpty=true.
            {
                functionName: "parseEmptyArgs",
                scene: "empty --args or no --args produces an empty argument array",
                args: "{}",
                expected: {
                    code: 0,
                    result: {
                        case: "parseEmptyArgs",
                        argc: 0,
                        isEmpty: true,
                    },
                },
            },
            // Expected: code=0, result.case=parseArgGap, result.argc=2, result.hasArgGap=true.
            {
                functionName: "parseArgGap",
                scene: "argument index gap fills missing slots as undefined",
                args: "{\"arg1\":\"second\"}",
                expected: {
                    code: 0,
                    result: {
                        case: "parseArgGap",
                        argc: 2,
                        hasArgGap: true,
                    },
                },
            },
            // Expected: code=0, result.case=describeContext, context/applicationContext/bundleContext are available.
            {
                functionName: "describeContext",
                scene: "script context, application context and bundle context availability",
                args: "{}",
                expected: {
                    code: 0,
                    result: {
                        case: "describeContext",
                        context: {
                            hasContext: true,
                        },
                    },
                },
            },
            // Expected: code=0, result={ ok: true }, uri=file://demo, flag=1.
            {
                functionName: "completeWithResult",
                scene: "CompletearktsScript success payload with code/result/uri/flag",
                args: "{\"arg0\":0,\"arg1\":{\"ok\":true},\"arg2\":\"file://demo\",\"arg3\":1}",
                expected: {
                    code: 0,
                    result: {
                        ok: true,
                    },
                    uri: "file://demo",
                    flag: 1,
                },
            },
            // Expected error: CompletearktsScript requires a result argument.
            {
                functionName: "completeWithoutArgument",
                scene: "CompletearktsScript abnormal branch: missing result argument",
                expectedError: "CompletearktsScript requires a result argument",
            },
            // Expected error: CompletearktsScript requires an object argument.
            {
                functionName: "completeWithNonObject",
                scene: "CompletearktsScript abnormal branch: argument is not object",
                expectedError: "CompletearktsScript requires an object argument",
            },
            // Expected error: CompletearktsScript requires a numeric code.
            {
                functionName: "completeWithoutCode",
                scene: "CompletearktsScript abnormal branch: missing numeric code",
                expectedError: "CompletearktsScript requires a numeric code",
            },
            // Expected: code=0, result=\"invalid\".
            {
                functionName: "completeWithInvalidResult",
                scene: "CompletearktsScript compatibility branch: non-object result is forwarded",
                expected: {
                    code: 0,
                    result: "invalid",
                },
            },
            // Expected error: CompletearktsScript failed to read uri.
            {
                functionName: "completeWithInvalidUri",
                scene: "CompletearktsScript abnormal branch: uri is not string",
                expectedError: "CompletearktsScript failed to read uri",
            },
            // Expected error: CompletearktsScript failed to read flag.
            {
                functionName: "completeWithInvalidFlag",
                scene: "CompletearktsScript abnormal branch: flag is not int",
                expectedError: "CompletearktsScript failed to read flag",
            },
            // Expected error: failed to call function.
            {
                functionName: "throwError",
                scene: "target JS function throws",
                expectedError: "failed to call function",
            },
            // Expected error: failed to resolve function.
            {
                functionName: "notExists",
                scene: "function resolve failure",
                expectedError: "failed to resolve function",
            },
        ],
    }));
}

export function selfCheck(args) {
    return completeResult(buildCaseResult("selfCheck", args, {
        exports: [
            "add",
            "addNumber",
            "addInvalid",
            "parseValues",
            "parseEmptyArgs",
            "parseArgGap",
            "describeContext",
            "completeWithResult",
            "throwError",
            "completeWithoutArgument",
            "completeWithNonObject",
            "completeWithoutCode",
            "completeWithInvalidResult",
            "completeWithInvalidUri",
            "completeWithInvalidFlag",
            "testCatalog",
            "selfCheck",
            "default",
            "ArktsScriptSuite",
        ],
    }));
}

export function describeContext(args) {
    const list = toArgs(args);
    const ctx = currentContext();
    const appCtx = ctx && typeof ctx.getApplicationContext === "function" ? ctx.getApplicationContext() : null;
    const bundleName = list.length > 0 ? list[0] : (ctx && ctx.applicationInfo && ctx.applicationInfo.bundleName) || "";
    const bundleCtx = ctx && typeof ctx.createBundleContext === "function" ? ctx.createBundleContext(bundleName) : null;

    return completeResult(withInterfaceInfo(buildCaseResult("describeContext", args, {
        applicationContext: snapshotContext(appCtx),
        bundleContext: snapshotContext(bundleCtx),
    }), "export function describeContext(args)",
        "return { context: snapshotContext(), applicationContext: snapshotContext(appCtx), bundleContext: snapshotContext(bundleCtx) }",
        buildExpectedResult("describeContext", args, {
            context: {
                hasContext: true,
            },
            applicationContext: {
                hasContext: true,
            },
            bundleContext: {
                hasContext: true,
            },
        })));
}

export function completeWithResult(args) {
    const list = toArgs(args);
    const code = typeof list[0] === "number" ? list[0] : 0;
    const resultPayload = list[1] !== null && typeof list[1] === "object" && !Array.isArray(list[1]) ? list[1] : null;
    const uri = typeof list[2] === "string" ? list[2] : null;
    const flag = typeof list[3] === "number" ? list[3] : null;
    const executeResult = {
        code,
    };
    if (resultPayload !== null) {
        executeResult.result = resultPayload;
    }
    if (uri !== null) {
        executeResult.uri = uri;
    }
    if (flag !== null) {
        executeResult.flag = flag;
    }

    if (typeof globalThis.CompletearktsScript === "function") {
        globalThis.CompletearktsScript(executeResult);
        return "pending";
    }

    return withInterfaceInfo(buildCaseResult("completeWithResult", args, {
        executeResult,
    }), "export function completeWithResult(args)",
        "CompletearktsScript({ code, result, uri, flag })",
        executeResult);
}

export default class ArktsScriptSuite {
    add(args) {
        return completeResult(buildAddResult(args, "defaultClassMethod:add", "default class ArktsScriptSuite.add(args)",
            "CompletearktsScript({ code: 0, result: buildAddResult(args, 'defaultClassMethod:add') })"));
    }

    addByDefaultClass(args) {
        return completeResult(buildAddResult(args, "defaultClassMethod:addByDefaultClass",
            "default class ArktsScriptSuite.addByDefaultClass(args)",
            "CompletearktsScript({ code: 0, result: buildAddResult(args, 'defaultClassMethod:addByDefaultClass') })"));
    }

    parseValues(args) {
        return parseValues(args);
    }

    describeContext(args) {
        return describeContext(args);
    }

    completeWithResult(args) {
        return completeWithResult(args);
    }

    selfCheck(args) {
        return selfCheck(args);
    }
}

globalThis.add = function add(args) {
    return completeResult(buildAddResult(args, "globalFallback:add", "globalThis.add(args)",
        "CompletearktsScript({ code: 0, result: buildAddResult(args, 'globalFallback:add') })"));
};
globalThis.parseValues = parseValues;
globalThis.describeContext = describeContext;
globalThis.completeWithResult = completeWithResult;
globalThis.selfCheck = selfCheck;
globalThis.ArktsScriptSuite = ArktsScriptSuite;

if (typeof console !== "undefined" && console !== null) {
    console.log("=== ArkTS script test loaded ===");
    console.log("Context ready: " + (currentContext() !== null));
}
