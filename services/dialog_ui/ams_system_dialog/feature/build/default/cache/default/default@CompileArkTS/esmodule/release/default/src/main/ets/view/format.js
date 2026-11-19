export function format(q4) {
    let r4 = {};
    for (const s4 in q4) {
        if (s4.includes('.')) {
            const t4 = s4.split('.');
            if (s4.includes('.array')) {
                processArrayKey(r4, t4, q4[s4]);
            }
            else {
                processObjectKey(r4, t4, q4[s4]);
            }
        }
        else {
            r4[s4] = q4[s4];
        }
    }
    r4 = formatFilterArrNull(r4);
    return r4;
}
function processObjectKey(k4, l4, m4) {
    let n4 = k4;
    for (let o4 = 0; o4 < l4.length - 1; o4++) {
        const p4 = l4[o4];
        if (!n4[p4] || typeof n4[p4] !== 'object') {
            n4[p4] = {};
        }
        n4 = n4[p4];
    }
    n4[l4[l4.length - 1]] = m4;
}
function processArrayKey(d4, e4, f4) {
    const g4 = parseInt(e4[1].substring(5));
    let h4 = d4;
    for (let i4 = 0; i4 < e4.length - 2; i4++) {
        const j4 = e4[i4];
        if (!h4[j4] || !Array.isArray(h4[j4])) {
            h4[j4] = [];
        }
        h4 = h4[j4];
    }
    if (e4[e4.length - 1] === 'items') {
        h4[g4] = f4;
        return;
    }
    if (!h4[g4] || typeof h4[g4] !== 'object') {
        h4[g4] = {};
    }
    h4[g4][e4[e4.length - 1]] = f4;
}
function formatFilterArrNull(a4) {
    Object.keys(a4).forEach(b4 => {
        if (Array.isArray(a4[b4])) {
            a4[b4] = a4[b4].filter(c4 => c4 !== null);
        }
    });
    return a4;
}
export function jsClone(z3) {
    return JSON.parse(JSON.stringify(z3));
}
export function addArrayParams(r3, s3, t3) {
    let u3 = t3[t3.length - 1];
    const v3 = jsClone(r3[0]);
    v3.forEach((w3, x3) => {
        const y3 = w3.key.split('.');
        v3[x3].key = `${v3[x3].fatherKey}.array${u3}.${y3[2]}`;
        v3[x3].index = u3;
    });
    r3.push(v3);
    return r3;
}
export function removeArrayParams(k3, l3, m3) {
    const n3 = jsClone(k3);
    const o3 = n3.filter((p3, q3) => q3 !== m3);
    return o3;
}
export function removeFormData(g3, h3, i3) {
    Object.keys(g3).forEach(j3 => {
        if (j3.includes(`${h3}.array${i3}.`)) {
            delete g3[j3];
        }
    });
    return jsClone(g3);
}
