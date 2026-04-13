function add(a, b) {
    return a + b;
}

function echoTypes(a, b, c, d, e) {
    return JSON.stringify([
        typeof a,
        typeof b,
        typeof c,
        Array.isArray(d),
        typeof e,
    ]);
}

globalThis.add = add;
globalThis.echoTypes = echoTypes;
