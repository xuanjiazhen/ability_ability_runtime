export function format(k4) {
    let l4 = {};
    for (const m4 in k4) {
        if (m4.includes('.')) {
            const n4 = m4.split('.');
            if (m4.includes('.array')) {
                processArrayKey(l4, n4, k4[m4]);
            }
            else {
                processObjectKey(l4, n4, k4[m4]);
            }
        }
        else {
            l4[m4] = k4[m4];
        }
    }
    l4 = formatFilterArrNull(l4);
    return l4;
}
function processObjectKey(e4, f4, g4) {
    let h4 = e4;
    for (let i4 = 0; i4 < f4.length - 1; i4++) {
        const j4 = f4[i4];
        if (!h4[j4] || typeof h4[j4] !== 'object') {
            h4[j4] = {};
        }
        h4 = h4[j4];
    }
    h4[f4[f4.length - 1]] = g4;
}
function processArrayKey(x3, y3, z3) {
    const a4 = parseInt(y3[1].substring(5));
    let b4 = x3;
    for (let c4 = 0; c4 < y3.length - 2; c4++) {
        const d4 = y3[c4];
        if (!b4[d4] || !Array.isArray(b4[d4])) {
            b4[d4] = [];
        }
        b4 = b4[d4];
    }
    if (y3[y3.length - 1] === 'items') {
        b4[a4] = z3;
        return;
    }
    if (!b4[a4] || typeof b4[a4] !== 'object') {
        b4[a4] = {};
    }
    b4[a4][y3[y3.length - 1]] = z3;
}
function formatFilterArrNull(u3) {
    Object.keys(u3).forEach(v3 => {
        if (Array.isArray(u3[v3])) {
            u3[v3] = u3[v3].filter(w3 => w3 !== null);
        }
    });
    return u3;
}
export function jsClone(t3) {
    return JSON.parse(JSON.stringify(t3));
}
export function addArrayParams(l3, m3, n3) {
    let o3 = n3[n3.length - 1];
    const p3 = jsClone(l3[0]);
    p3.forEach((q3, r3) => {
        const s3 = q3.key.split('.');
        p3[r3].key = `${p3[r3].fatherKey}.array${o3}.${s3[2]}`;
        p3[r3].index = o3;
    });
    l3.push(p3);
    return l3;
}
export function removeArrayParams(e3, f3, g3) {
    const h3 = jsClone(e3);
    const i3 = h3.filter((j3, k3) => k3 !== g3);
    return i3;
}
export function removeFormData(a3, b3, c3) {
    Object.keys(a3).forEach(d3 => {
        if (d3.includes(`${b3}.array${c3}.`)) {
            delete a3[d3];
        }
    });
    return jsClone(a3);
}
