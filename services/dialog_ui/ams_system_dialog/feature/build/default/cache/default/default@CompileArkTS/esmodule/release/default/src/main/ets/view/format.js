export function format(o4) {
    let p4 = {};
    for (const q4 in o4) {
        if (q4.includes('.')) {
            const r4 = q4.split('.');
            if (q4.includes('.array')) {
                processArrayKey(p4, r4, o4[q4]);
            }
            else {
                processObjectKey(p4, r4, o4[q4]);
            }
        }
        else {
            p4[q4] = o4[q4];
        }
    }
    p4 = formatFilterArrNull(p4);
    return p4;
}
function processObjectKey(i4, j4, k4) {
    let l4 = i4;
    for (let m4 = 0; m4 < j4.length - 1; m4++) {
        const n4 = j4[m4];
        if (!l4[n4] || typeof l4[n4] !== 'object') {
            l4[n4] = {};
        }
        l4 = l4[n4];
    }
    l4[j4[j4.length - 1]] = k4;
}
function processArrayKey(b4, c4, d4) {
    const e4 = parseInt(c4[1].substring(5));
    let f4 = b4;
    for (let g4 = 0; g4 < c4.length - 2; g4++) {
        const h4 = c4[g4];
        if (!f4[h4] || !Array.isArray(f4[h4])) {
            f4[h4] = [];
        }
        f4 = f4[h4];
    }
    if (c4[c4.length - 1] === 'items') {
        f4[e4] = d4;
        return;
    }
    if (!f4[e4] || typeof f4[e4] !== 'object') {
        f4[e4] = {};
    }
    f4[e4][c4[c4.length - 1]] = d4;
}
function formatFilterArrNull(y3) {
    Object.keys(y3).forEach(z3 => {
        if (Array.isArray(y3[z3])) {
            y3[z3] = y3[z3].filter(a4 => a4 !== null);
        }
    });
    return y3;
}
export function jsClone(x3) {
    return JSON.parse(JSON.stringify(x3));
}
export function addArrayParams(p3, q3, r3) {
    let s3 = r3[r3.length - 1];
    const t3 = jsClone(p3[0]);
    t3.forEach((u3, v3) => {
        const w3 = u3.key.split('.');
        t3[v3].key = `${t3[v3].fatherKey}.array${s3}.${w3[2]}`;
        t3[v3].index = s3;
    });
    p3.push(t3);
    return p3;
}
export function removeArrayParams(i3, j3, k3) {
    const l3 = jsClone(i3);
    const m3 = l3.filter((n3, o3) => o3 !== k3);
    return m3;
}
export function removeFormData(e3, f3, g3) {
    Object.keys(e3).forEach(h3 => {
        if (h3.includes(`${f3}.array${g3}.`)) {
            delete e3[h3];
        }
    });
    return jsClone(e3);
}
