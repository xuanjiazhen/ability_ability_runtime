function toArgs(args) {
    return Array.isArray(args) ? args : [];
}

function parseNumber(value, fallback = null) {
    const parsed = Number(value);
    return Number.isFinite(parsed) ? parsed : fallback;
}

function parseBoolean(value, fallback = null) {
    if (value === "true") {
        return true;
    }
    if (value === "false") {
        return false;
    }
    return fallback;
}

function parseJson(value, fallback = null) {
    if (typeof value !== "string" || value.length === 0) {
        return fallback;
    }
    try {
        return JSON.parse(value);
    } catch (err) {
        return fallback;
    }
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

function buildCaseResult(caseName, args, extra = {}) {
    const list = toArgs(args);
    return {
        case: caseName,
        argc: list.length,
        args: list,
        context: snapshotContext(),
        ...extra,
    };
}

function add(args) {
    const list = toArgs(args);
    const left = parseNumber(list[0], 0);
    const right = parseNumber(list[1], 0);
    return buildCaseResult("add", args, {
        left,
        right,
        sum: left + right,
    });
}

function parseValues(args) {
    const list = toArgs(args);
    return buildCaseResult("parseValues", args, {
        parsedValues: [
            parseNumber(list[0]),
            parseBoolean(list[1]),
            parseJson(list[2]),
        ],
    });
}

function describeContext(args) {
    const list = toArgs(args);
    const ctx = currentContext();
    const appCtx = ctx && typeof ctx.getApplicationContext === "function" ? ctx.getApplicationContext() : null;
    const bundleName = list.length > 0 ? list[0] : (ctx && ctx.applicationInfo && ctx.applicationInfo.bundleName) || "";
    const bundleCtx = ctx && typeof ctx.createBundleContext === "function" ? ctx.createBundleContext(bundleName) : null;

    return buildCaseResult("describeContext", args, {
        applicationContext: snapshotContext(appCtx),
        bundleContext: snapshotContext(bundleCtx),
    });
}

function completeWithResult(args) {
    const list = toArgs(args);
    const code = parseNumber(list[0], 0);
    const resultPayload = parseJson(list[1], null);
    const executeResult = {
        code,
    };
    if (resultPayload !== null) {
        executeResult.result = resultPayload;
    }

    if (typeof globalThis.completeArkTsTool === "function") {
        globalThis.completeArkTsTool(executeResult);
        return "pending";
    }

    return buildCaseResult("completeWithResult", args, {
        executeResult,
    });
}

function selfCheck(args) {
    return buildCaseResult("selfCheck", args, {
        exports: [
            "add",
            "parseValues",
            "describeContext",
            "completeWithResult",
            "completeArkTsTool",
            "selfCheck",
            "ArktsScriptSuite",
        ],
    });
}

class ArktsScriptSuite {
    add(args) {
        return add(args);
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

globalThis.add = add;
globalThis.parseValues = parseValues;
globalThis.describeContext = describeContext;
globalThis.completeWithResult = completeWithResult;
globalThis.selfCheck = selfCheck;
globalThis.ArktsScriptSuite = ArktsScriptSuite;

if (typeof console !== "undefined" && console !== null) {
    console.log("=== ArkTS script test loaded ===");
    console.log("Context ready: " + (currentContext() !== null));
}
