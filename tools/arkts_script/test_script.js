const context = globalThis.scriptContext;

function snapshot(ctx = context) {
    return {
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

function verifyScriptContextContract() {
    return {
        hasScriptContext: context !== undefined && context !== null,
        hasGetApplicationContext: !!context && typeof context.getApplicationContext === "function",
        hasCreateBundleContext: !!context && typeof context.createBundleContext === "function",
        snapshot: snapshot(),
    };
}

function getApplicationContextInfo() {
    if (!context || typeof context.getApplicationContext !== "function") {
        return null;
    }
    const applicationContext = context.getApplicationContext();
    return snapshot(applicationContext);
}

function probeCreateBundleContext(bundleName) {
    if (!context || typeof context.createBundleContext !== "function") {
        return null;
    }
    const bundleContext = context.createBundleContext(bundleName || (context.applicationInfo && context.applicationInfo.bundleName));
    return snapshot(bundleContext);
}

function echoArgs(args) {
    return {
        argc: Array.isArray(args) ? args.length : 0,
        args: Array.isArray(args) ? args : [],
        context: snapshot(),
    };
}

function add(args) {
    return echoArgs(args);
}

function process(args) {
    return {
        function: "process",
        args: Array.isArray(args) ? args : [],
        context: snapshot(),
    };
}

function asyncResolve(args) {
    return Promise.resolve({
        function: "asyncResolve",
        args: Array.isArray(args) ? args : [],
        context: snapshot(),
    });
}

function asyncReject(args) {
    return Promise.reject(new Error("asyncReject: " + JSON.stringify(args || [])));
}

function runSelfCheck() {
    return {
        availableFunctions: [
            "add",
            "process",
            "asyncResolve",
            "asyncReject",
            "verifyScriptContextContract",
            "getApplicationContextInfo",
            "probeCreateBundleContext",
            "manualComplete",
        ],
        context: verifyScriptContextContract(),
    };
}

function manualComplete(args) {
    if (typeof globalThis.completeArkTsTool === "function") {
        globalThis.completeArkTsTool(0, {
            ok: true,
            args: Array.isArray(args) ? args : [],
            context: snapshot(),
        });
    }
    return "pending";
}

globalThis.add = add;
globalThis.process = process;
globalThis.asyncResolve = asyncResolve;
globalThis.asyncReject = asyncReject;
globalThis.verifyScriptContextContract = verifyScriptContextContract;
globalThis.getApplicationContextInfo = getApplicationContextInfo;
globalThis.probeCreateBundleContext = probeCreateBundleContext;
globalThis.runSelfCheck = runSelfCheck;
globalThis.manualComplete = manualComplete;

if (typeof console !== "undefined" && console !== null) {
    console.log("=== ArkTS script test loaded ===");
    console.log("Context ready: " + (context !== undefined && context !== null));
}
