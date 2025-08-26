/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function sleepms(time) {
    return new Promise(resolve => {
        setTimeout(resolve, time);
    });
}

export function testDeepProxy1() {
    const target = { a: 1, nested: { b: 2 } };
    // 递归创建代理的工具函数
    const createProxy = (obj) => new Proxy(obj, {
        get(target, prop) {
            const value = target[prop];
            // 若属性是对象，递归代理
            return typeof value === 'object' && value !== null ? createProxy(value) : value;
        },
        set(target, prop, value) {
            target[prop] = value;
            console.log(`修改属性：${prop}=${value}`);
            return true;
        }
    });
    const proxy = createProxy(target);

    proxy.nested.b = 3; // 触发嵌套对象的set陷阱
    console.log(target.nested.b === 3, "101.1：深度代理嵌套对象修改成功");
    return target.nested.b === 3;
}

export function testNestedProxyThis2() {
    const target = {
        a: 1,
        nested: {
            b: 2,
            getB() {
                return this.b;
            } // 依赖this绑定
        }
    };
    const createProxy = (obj) => new Proxy(obj, {
        get(target, prop) {
            const value = target[prop];
            return typeof value === 'object' && value !== null ? createProxy(value) : value;
        }
    });
    const proxy = createProxy(target);

    console.log(proxy.nested.getB() === 2, "102.1：嵌套代理方法this指向正确");
    return proxy.nested.getB() === 2;
}

export function testProxySetAdd3() {
    const set = new Set();
    const handler = {
        get(target, prop) {
            if (prop === 'add') {
                // 拦截add，只允许偶数
                return (value) => {
                    if (value % 2 === 0) {
                        target.add(value);
                    }
                    return target; // 保持链式调用
                };
            }
            return Reflect.get(target, prop);
        }
    };
    const proxy = new Proxy(set, handler);

    proxy.add(1).add(2).add(3);
    console.log(proxy.size === 1 && proxy.has(2), "103.1：Set.add拦截并过滤值成功");
    return proxy.size === 1 && proxy.has(2);
}

export function testGetInterceptArrayPush6() {
    const arr = [1, 2];
    const handler = {
        get(target, prop) {
            if (prop === 'push') {
                // 拦截push，参数加倍
                return function (...args) {
                    const doubled = args.map(v => v * 2);
                    return target.push(...doubled);
                };
            }
            return Reflect.get(target, prop);
        }
    };
    const proxy = new Proxy(arr, handler);

    proxy.push(3, 4);
    let res = arr.join(',');
    console.log(res === '1,2,6,8', "106.1：数组push方法被拦截修改参数");
    return res === '1,2,6,8';
}

export function testSetArrayLengthLimit7() {
    const arr = [1, 2];
    let flag = 0;
    const handler = {
        set(target, prop, value) {
            if (prop === 'length' && value > 5) {
                throw new RangeError("数组长度不能超过5");
            }
            return Reflect.set(target, prop, value);
        }
    };
    const proxy = new Proxy(arr, handler);

    proxy.length = 3; // 合法修改
    console.log(arr.length === 3, "107.1：合法length修改成功");
    if (arr.length === 3) {
        flag = flag + 1;
    }
    try {
        proxy.length = 6; // 超出限制
        console.error("107.2：未拦截超出限制的length修改");
        flag = flag + 0.1;
    } catch (e) {
        console.log(e instanceof RangeError, "107.2：超出限制应抛RangeError");
        flag = flag + 1;
    }
    return flag;
}

export function testGetLazyInitialization8() {
    const target = { /* 未初始化lazyProp */ };
    const handler = {
        get(target, prop) {
            if (prop === 'lazyProp' && !(prop in target)) {
                // 首次访问时初始化
                target.lazyProp = '初始化值';
                console.log("lazyProp初始化完成");
            }
            return target[prop];
        }
    };
    const proxy = new Proxy(target, handler);
    let res = !('lazyProp' in target);
    console.log(res, "108.1：初始化前目标无属性");
    console.log(proxy.lazyProp === '初始化值', "108.2：首次访问触发初始化");
    console.log('lazyProp' in target, "108.3：初始化后目标有属性");
    return res && proxy.lazyProp === '初始化值' && ('lazyProp' in target);
}

export function testReflectEnumerateLike9() {
    const obj = { a: 1 };
    Object.defineProperty(obj, 'b', { value: 2, enumerable: false });
    Object.setPrototypeOf(obj, { c: 3 }); // 继承属性

    // 模拟for...in枚举（自有可枚举+继承可枚举）
    const enumerate = (obj) => {
        const keys = [];
        let current = obj;
        while (current !== null) {
            Reflect.ownKeys(current).forEach(key => {
                if (typeof key === 'string' && Reflect.getOwnPropertyDescriptor(current, key).enumerable) {
                    keys.push(key);
                }
            });
            current = Reflect.getPrototypeOf(current);
        }
        return keys;
    };

    const keys = enumerate(obj);
    console.log(keys.join(',') === 'a,c', "109.1：模拟枚举行为成功");
    return keys.join(',') === 'a,c';
}

export function testReflectPreventExtensions10() {
    const obj1 = {};
    let flag = 0;
    const success1 = Reflect.preventExtensions(obj1);
    console.log(success1 && !Reflect.isExtensible(obj1), "110.1：首次阻止扩展成功");
    if (success1 && !Reflect.isExtensible(obj1)) {
        flag = flag + 1;
    }
    const success2 = Reflect.preventExtensions(obj1); // 重复调用
    console.log(success2, "110.2：重复阻止成功");
    if (success2) {
        flag = flag + 1;
    }
    return flag;
}

export function testReflectGetPrototypeNull11() {
    const obj = Object.create(null); // 无原型
    console.log(Reflect.getPrototypeOf(obj) === null, "111.1：null原型对象返回null");
    return Reflect.getPrototypeOf(obj) === null;
}

export async function testProxyAsyncFunction12() {
    let flag = 0;
    const asyncFunc = async () => 10;

    const handler = {
        get(target, prop) {
            if (prop === 'then') {
                // 拦截then，结果加倍
                return (onFulfilled) => {
                    return target.then(value => onFulfilled(value * 2));
                };
            }
            return Reflect.get(target, prop);
        }
    };
    const proxy = new Proxy(asyncFunc, handler);

    proxy().then(result => {
        console.log(result === 20, "112.1：异步函数结果被拦截修改");
        flag = flag + 1;
    });

    await sleepms(2000);
    return flag;
}

export function testProxyGenerator13() {
    let flag = 0;

    function* generator() {
        yield 1;
        yield 2;
    }

    const handler = {
        get(target, prop) {
            if (prop === 'next') {
                // 拦截next，值加倍
                return () => {
                    const result = target.next();
                    return result.done ? result : { ...result, value: result.value * 2 };
                };
            }
            return Reflect.get(target, prop);
        }
    };
    const gen = generator();
    const proxy = new Proxy(gen, handler);
    let res1 = proxy.next().value
    if (res1 === 2) {
        flag = flag + 1;
    }
    let res2 = proxy.next().value
    if (res2 === 4) {
        flag = flag + 1;
    }
    console.log(res1 === 2, "113.1：生成器next值被加倍");
    console.log(res2 === 4, "113.2：第二次next值被加倍");
    return flag;

}

export function testOwnKeysJsonStringify14() {
    const target = { a: 1, b: 2, secret: 'hidden' };
    const handler = {
        ownKeys(target) {
            // 过滤secret键
            return Reflect.ownKeys(target).filter(prop => prop !== 'secret');
        }
    };
    const proxy = new Proxy(target, handler);

    const json = JSON.stringify(proxy);
    console.log(json === '{"a":1,"b":2}', "114.1：JSON序列化过滤secret键");
    return json === '{"a":1,"b":2}';
}

export function testProxyStructuredClone15() {
    const target = { a: 1, b: [2, 3] };
    let flag = 0;
    const proxy = new Proxy(target, {
        get: Reflect.get,
        set: Reflect.set
    }); // 透明代理

    try {
        const clone = structuredClone(proxy);
        console.log(clone.a === 1 && clone.b[0] === 2, "115.1：代理对象克隆成功");
        flag = flag + 0.1;
    } catch (e) {
        console.error("115.2：代理对象克隆失败", e);
        flag = flag + 1;
    }
    return flag;
}


export function testProxyDataValidator16() {
    let flag = 0;
    // 验证规则：age为数字且>0，name为字符串
    const schema = {
        age: (v) => typeof v === 'number' && v > 0,
        name: (v) => typeof v === 'string' && v.length > 0
    };
    const target = { age: 20, name: 'Tom' };
    const handler = {
        set(target, prop, value) {
            if (schema[prop] && !schema[prop](value)) {
                throw new TypeError(`属性${prop}验证失败`);
            }
            target[prop] = value;
            return true;
        }
    };
    const proxy = new Proxy(target, handler);

    proxy.age = 25; // 合法
    try {
        proxy.age = -5; // 非法
        console.error("116.1：未拦截非法age值");
        flag = flag + 0.1;
    } catch (e) {
        console.log(e instanceof TypeError, "116.1：非法值应抛TypeError");
        flag = flag + 1;
    }
    return flag;
}

export function testProxyObserver17() {
    const target = { value: 0 };
    const observers = [];
    // 注册观察者
    const observe = (callback) => observers.push(callback);
    // 代理：属性变化时通知所有观察者
    const proxy = new Proxy(target, {
        set(target, prop, value) {
            const oldValue = target[prop];
            target[prop] = value;
            observers.forEach(cb => cb(prop, oldValue, value));
            return true;
        }
    });

    let lastChange;
    observe((prop, old, newValue) => {
        lastChange = { prop, old, newValue };
    });
    proxy.value = 1;
    console.log(lastChange.prop === 'value' && lastChange.newValue === 1, "117.1：观察者收到变化通知");
    return lastChange.prop === 'value' && lastChange.newValue === 1;
}

export function testProxyReadOnlyView18() {
    let flag = 0;
    const target = { a: 1, b: { c: 2 } };
    // 递归创建只读代理
    const createReadOnlyProxy = (obj) => new Proxy(obj, {
        set: () => {
            throw new TypeError("只读对象不可修改");
        },
        deleteProperty: () => {
            throw new TypeError("只读对象不可删除属性");
        },
        defineProperty: () => {
            throw new TypeError("只读对象不可定义属性");
        },
        get(target, prop) {
            const value = target[prop];
            return typeof value === 'object' && value !== null ? createReadOnlyProxy(value) : value;
        }
    });
    const readOnly = createReadOnlyProxy(target);

    try {
        readOnly.a = 2;
        console.error("118.1：未阻止修改只读属性");
        flag = flag + 0.1;
    } catch (e) {
        console.log(e instanceof TypeError, "118.1：修改应抛TypeError");
        flag = flag + 1;
    }
    try {
        readOnly.b.c = 3;
        console.error("118.2：未阻止修改嵌套只读属性");
        flag = flag + 0.1;
    } catch (e) {
        console.log(e instanceof TypeError, "118.2：嵌套修改应抛TypeError");
        flag = flag + 1;
    }
    return flag;
}

export function testGetReturnThis19() {
    const target = { self: null };
    const handler = {
        get(target, prop) {
            if (prop === 'self') {
                return proxy;
            } // 返回代理自身
            return target[prop];
        }
    };
    const proxy = new Proxy(target, handler);

    console.log(proxy.self === proxy, "119.1：get陷阱返回代理自身成功（无死循环）");
    return proxy.self === proxy;
}

export function testSetPrototypeProperty20() {

    function Target() {
    }

    const newProto = {};
    const handler = {
        set(target, prop, value) {
            if (prop === 'prototype') {
                // 拦截prototype修改，添加标记
                value.__proxyMark = true;
            }
            return Reflect.set(target, prop, value);
        }
    };
    const proxy = new Proxy(Target, handler);

    proxy.prototype = newProto;
    console.log(Target.prototype.__proxyMark === true, "120.1：prototype修改被拦截处理");
    return Target.prototype.__proxyMark === true;
}

export function testHasForInLoop21() {
    const target = { a: 1, b: 2, c: 3 };
    const handler = {
        has(target, prop) {
            // 只允许偶数键名通过（b是第2个字母）
            return ['b'].includes(prop);
        }
    };
    const proxy = new Proxy(target, handler);

    const enumerated = [];
    for (const prop in proxy) {
        enumerated.push(prop);
    }
    let res = enumerated.join(',') === 'a,b,c';
    console.log(res, "121.1：for...in循环受has陷阱影响");
    return res;
}

export function testApplyValidateArgs22() {
    const sum = (a, b) => a + b;
    let flag = 0;
    const handler = {
        apply(target, thisArg, args) {
            // 验证参数为数字
            if (!args.every(v => typeof v === 'number')) {
                throw new TypeError("参数必须为数字");
            }
            return target.apply(thisArg, args);
        }
    };
    const proxy = new Proxy(sum, handler);

    console.log(proxy(1, 2) === 3, "122.1：合法参数调用成功");
    try {
        proxy('1', 2); // 非法参数
        console.error("122.2：未拦截非法参数");
        flag = flag + 0.1;
    } catch (e) {
        console.log(e instanceof TypeError, "122.2：非法参数应抛TypeError");
        flag = flag + 1;
    }
    return flag;
}

export function testConstructModifyInstance23() {
    class User {
        constructor(name) {
            this.name = name;
        }
    }

    const handler = {
        construct(target, args, newTarget) {
            const instance = new target(...args);
            // 添加创建时间
            instance.createdAt = new Date().toISOString().slice(0, 10);
            return instance;
        }
    };
    const ProxyUser = new Proxy(User, handler);

    const user = new ProxyUser('Alice');
    console.log(user.name === 'Alice' && user.createdAt === "2025-08-25", "123.1：construct陷阱修改实例成功");
    return user.name === 'Alice' && user.createdAt === "2025-08-25";
}

export function testRevocableReleaseResource24() {
    let resource = { inUse: true };
    const releaseResource = () => {
        resource.inUse = false;
    };
    // 创建可撤销代理时绑定释放逻辑
    const revocable = Proxy.revocable({}, {
        get: () => {
        }
    });
    // 包装撤销方法，添加资源释放
    const originalRevoke = revocable.revoke;
    revocable.revoke = () => {
        originalRevoke();
        releaseResource();
    };

    revocable.revoke();
    console.log(!resource.inUse, "124.1：撤销代理触发资源释放");
    return !resource.inUse;
}

export function testProxyArrayPop26() {
    const arr = [1, 2, 3];
    const deleted = [];
    const handler = {
        get(target, prop) {
            if (prop === 'pop') {
                return () => {
                    const last = target.pop();
                    deleted.push(last);
                    return last;
                };
            }
            return Reflect.get(target, prop);
        }
    };
    const proxy = new Proxy(arr, handler);

    proxy.pop();
    console.log(deleted[0] === 3 && arr.length === 2, "126.1：数组pop拦截并记录删除元素");
    return deleted[0] === 3 && arr.length === 2;
}

export function testProxyArraySplice27() {
    const arr = [1, 2, 3, 4];
    const handler = {
        get(target, prop) {
            if (prop === 'splice') {
                return (start, deleteCount, ...items) => {
                    // 限制最多删除2个元素
                    deleteCount = Math.min(deleteCount, 2);
                    return target.splice(start, deleteCount, ...items);
                };
            }
            return Reflect.get(target, prop);
        }
    };
    const proxy = new Proxy(arr, handler);

    const removed = proxy.splice(0, 3); // 尝试删除3个，实际删除2个
    console.log(removed.length === 2 && arr.length === 2, "127.1：splice拦截限制删除数量");
    return removed.length === 2 && arr.length === 2;
}

export function testProxyErrorStack28() {
    const error = new Error("原始错误");
    const handler = {
        get(target, prop) {
            if (prop === 'stack') {
                // 隐藏原始堆栈，返回自定义信息
                return "自定义错误堆栈（已拦截）";
            }
            return Reflect.get(target, prop);
        }
    };
    const proxy = new Proxy(error, handler);

    console.log(proxy.message === "原始错误", "128.1：错误消息正常访问");
    console.log(proxy.stack === "自定义错误堆栈（已拦截）", "128.2：stack属性被拦截修改");
    return (proxy.message === "原始错误") && (proxy.stack === "自定义错误堆栈（已拦截）");
}

export function testProxyIntlFormatter29() {
    const formatter = new Intl.DateTimeFormat('zh-CN');
    const handler = {
        get(target, prop) {
            if (prop === 'format') {
                return (date) => {
                    // 格式化前添加前缀
                    return `日期：${target.format(date)}`;
                };
            }
            return Reflect.get(target, prop);
        }
    };
    const proxy = new Proxy(formatter, handler);

    const result = proxy.format(new Date('2023-01-01'));
    console.log(result.startsWith('日期：'), "129.1：Intl格式化方法被拦截");
    return result.startsWith('日期：');
}

export function testSetToPrimitive32() {
    const target = { value: 0 };
    const handler = {
        set(target, prop, value) {
            // 转换值为数字后赋值
            target[prop] = Number(value);
            return true;
        }
    };
    const proxy = new Proxy(target, handler);
    const obj = { [Symbol.toPrimitive]: () => '123' }; // 可转换为原始值的对象

    proxy.value = obj;
    console.log(target.value === 123, "132.1：set陷阱处理Symbol.toPrimitive转换");
    return target.value === 123;
}

export function testGetSymbolIterator33() {
    const target = { data: [1, 2, 3] };
    const handler = {
        get(target, prop) {
            if (prop === Symbol.iterator) {
                // 自定义迭代器：返回加倍后的值
                let index = 0;
                return () => ({
                    next: () => {
                        if (index < target.data.length) {
                            return { value: target.data[index++] * 2, done: false };
                        }
                        return { done: true };
                    }
                });
            }
            return Reflect.get(target, prop);
        }
    };
    const proxy = new Proxy(target, handler);

    const result = [...proxy]; // 触发迭代器
    let res = result.join(',') === '2,4,6';
    console.log(res, "133.1：自定义迭代器生效");
    return res;
}
