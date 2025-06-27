export function formate(obj) {
    const nestedObj = {};
    for (const key in obj) {
        if (obj.hasOwnProperty(key)) {
            // 检查key是否包含点号，表示这是一个嵌套属性
            if (key.includes('.')) {
                // if(key.startsWith("array")){
                if (key.includes(".array")) {
                    // 分割键为各个部分
                    const parts = key.split('.');
                    let current = nestedObj;
                    const index = parseInt(parts[1].substring(5));

                    // 遍历除最后一部分外的所有键部分
                    // wayPoints.array0.poiId
                    for (let i = 0; i < parts.length - 2; i++) {
                        const part = parts[i];

                        // 如果当前部分不存在于对象中，则创建一个空对象
                        if (!current[part] || !Array.isArray(current[part])) {
                            current[part] = [];
                        }
                        if (current.length < index) {
                            current.length = index + 1
                        }

                        // 移动到下一层级
                        current = current[part]
                        // current = current[part][index];
                    }
                    // console.log(`formateJS的current=${JSON.stringify(current)}===${index}===${parts[parts.length - 1]}====${JSON.stringify(obj[key])}`)

                    // 如果当前部分不存在于对象中，则创建一个空对象
                    if (!current[index] || typeof current[index] !== 'object') {
                        current[index] = {};
                    }
                    // 设置最终的属性值
                    current[index][parts[parts.length - 1]] = obj[key];
                } else {
                    // 分割键为各个部分
                    const parts = key.split('.');
                    let current = nestedObj;

                    // 遍历除最后一部分外的所有键部分
                    for (let i = 0; i < parts.length - 1; i++) {
                        const part = parts[i];

                        // 如果当前部分不存在于对象中，则创建一个空对象
                        if (!current[part] || typeof current[part] !== 'object') {
                            current[part] = {};
                        }

                        // 移动到下一层级
                        current = current[part];
                    }

                    // 设置最终的属性值
                    current[parts[parts.length - 1]] = obj[key];

                }
            } else {
                // 如果不是嵌套属性，直接赋值
                nestedObj[key] = obj[key];
            }
        }
    }

    return nestedObj;
}

export function jsClone(objArr) {
    return JSON.parse(JSON.stringify(objArr));

    //方法一：浅拷贝
    // let objArrClone=[]
    // objArr.forEach((item,index)=>{
    //     objArrClone[index] = Object.assign({},item)
    //
    //     // const itemObj = JSON.parse(JSON.stringify(item))
    //     // objArrClone[index] = Object.assign({},itemObj)
    //     console.log(`fieldSubProperties.length()3 = ${JSON.stringify(objArrClone[index])}`)
    // })
    // return objArrClone

    // 方法2：structuredClone（现代环境）
    // return structuredClone(objArr);

    // 方法3：Lodash _.cloneDeep（需要安装 lodash）
    // return _.cloneDeep(objArr);
}

export function addArrayParams(fieldSubProperties, fieldName) {
    console.log(`fieldSubProperties.length()1 = ${JSON.stringify(fieldSubProperties)}`)
    console.log(`fieldSubProperties.length()2 = ${JSON.stringify(fieldSubProperties.length)}`)
    let len = fieldSubProperties.length;

    const arr = jsClone(fieldSubProperties[0])
    arr.forEach((item, index) => {
        const path = item.key.split(".")
        arr[index].key = `${arr[index].fatherKey}.array${len}.${path[2]}`
        arr[index].index = len
    })
    fieldSubProperties.push(arr)
    return fieldSubProperties
    // return arr
}

export function removeArrayParams(fieldSubProperties, fieldName, arrIndex) {
    console.log(`fieldSubProperties.length()1 = ${JSON.stringify(fieldSubProperties)}`)
    console.log(`fieldSubProperties.length()2 = ${JSON.stringify(fieldSubProperties.length)}`)

    // delete fieldSubProperties[arrIndex];
    const arr = jsClone(fieldSubProperties);
    const newArr = arr.filter((item,index) => index != arrIndex);
    return newArr
}