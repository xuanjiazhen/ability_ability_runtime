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

import hilog from '@ohos.hilog'
import rpc from '@ohos.rpc';

type valueType = NullishType;
const DOMAIN_ID = 0xD001300;
const TAG = 'WantSerializeTool';
const int32Max: number = Math.pow(2, 31) - 1;
const int32Min: number = -Math.pow(2, 31);

interface ComplexArrayData {
  intlist: Array<int>;
  boolList: Array<boolean>;
  doubleList: Array<double>;
  stringList: Array<string>;
  objectList: Array<NullishType>;
}

class RecordWriter {
  private buffer = new StringBuilder();
  private store = new Set<Object>();
  private wantParams: NativeWantParams | null = null;

  constructor(wantParams: long) {
    this.wantParams = new NativeWantParams(wantParams);
  }

  public write(obj: Object): boolean {
    return this.parseRecord(obj);
    // return this.buffer.toString();
  }

  private parseRecord(obj: NullishType): boolean {
    if (obj instanceof Record) {
      return this.writeRecord(obj as Object as Record<string, valueType>);
    } else {
      hilog.error(DOMAIN_ID, TAG, `input object not record type`);
      return false;
    }
  }

  private writeObject(obj: NullishType): void {
    if (obj === null) {
      this.buffer.append('null');
    } else if (obj === undefined) {
      this.buffer.append('undefined');
    } else if (obj instanceof String) {
      this.buffer.append(JSON.stringify(obj as String));
    } else if (this.writeValueType(obj as Object)) {
      // nothing to do
    } else if (obj instanceof Array) {
      this.writeArray(obj as Object as Array<valueType>);
    } else if (obj instanceof Record) {
      this.writeRecord(obj as Object as Record<string, valueType>);
    } else {
      const objType = Type.of(obj);
      if (objType instanceof ArrayType) {
        this.writeBuildArray(obj as Object, reflect.Value.of(obj) as ArrayValue);
      } else {
        this.buffer.append('null');
      }
    }
  }

  private writeObjectToNative(key: string, obj: NullishType, wantParams?: NativeWantParams): boolean {
    let localWantParams = (wantParams === undefined) ? this.wantParams : wantParams;
    if (obj === null) {
      //this.buffer.append('null');
      //localWantParams?.setParam(key, 'null'); //1.1不收集null
      return true;
    } else if (obj === undefined) {
      //this.buffer.append('undefined');
      //localWantParams?.setParam(key, 'undefined'); //1.1不收集undefined
      return true;
    } else if (obj instanceof String) {
      //this.buffer.append(JSON.stringify(obj as String));
      localWantParams?.setParam(key, obj as String);
      return true;
    } else if (this.writeValueTypeToNative(localWantParams, key, obj as Object)) {
      // nothing to do
      return true;
    } else if (obj instanceof Array) {
      this.writeArrayToNative(key, obj as Object as Array<valueType>, localWantParams);
      return true;
    } else if (obj instanceof Record) {
      //this.writeRecord(obj as Object as Record<string, valueType>);
      let newWantParams = new NativeWantParams();
      if (this.writeRecordToNative(obj as Object as Record<string, valueType>, newWantParams) != true) {
        hilog.error(DOMAIN_ID, TAG, `write record to native failed`);
        return false;
      }
      localWantParams?.setParam(key, newWantParams);
      return true;
    } else {
      const objType = Type.of(obj);
      if (objType instanceof ClassType) {
        let newWantParams = new NativeWantParams();
        if (this.writeClassValueToNative(obj as Object, objType as ClassType, newWantParams) != true) {
          localWantParams?.setParam(key, '{}');
        } else {
          localWantParams?.setParam(key, newWantParams);
        }
      } else {
        //this.buffer.append('null');
        //localWantParams?.setParam(key, 'null'); // 1.1不识别类型不收集
        hilog.error(DOMAIN_ID, TAG, `object type not support parse, key: %{public}s`, key);
      }
      return true;
    }
    return false;
  }

  private writeValueType(obj: Object): boolean {
    if (obj instanceof Boolean) {
      this.buffer.append(JSON.stringify(obj.unboxed()));
      return true;
    } else if (obj instanceof Byte) {
      this.buffer.append(JSON.stringify(obj.unboxed()));
      return true;
    } else if (obj instanceof Char) {
      this.buffer.append(JSON.stringify(obj.unboxed()));
      return true;
    } else if (obj instanceof Short) {
      this.buffer.append(JSON.stringify(obj.unboxed()));
      return true;
    } else if (obj instanceof Int) {
      this.buffer.append(JSON.stringify(obj.unboxed()));
      return true;
    } else if (obj instanceof Long) {
      this.buffer.append(JSON.stringify(obj.unboxed()));
      return true;
    } else if (obj instanceof Float) {
      this.buffer.append(JSON.stringify(obj.unboxed()));
      return true;
    } else if (obj instanceof Double) {
      this.buffer.append(JSON.stringify(obj.unboxed()));
      return true;
    } else if (obj instanceof BigInt) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else {
      return false;
    }
  }

  private writeValueTypeToNative(wantParams: NativeWantParams | null, key: string, obj: Object): boolean {
    if (obj instanceof Boolean) {
      //this.buffer.append(JSON.stringify(obj.unboxed()));
      wantParams?.setParam(key, obj.unboxed());
      return true;
    } else if (obj instanceof Byte) {
      //this.buffer.append(JSON.stringify(obj.unboxed()));
      wantParams?.setParam(key, obj.unboxed());
      return true;
    } else if (obj instanceof Char) {
      //this.buffer.append(JSON.stringify(obj.unboxed()));
      wantParams?.setParam(key, obj.unboxed());
      return true;
    } else if (obj instanceof Short) {
      //this.buffer.append(JSON.stringify(obj.unboxed()));
      wantParams?.setParam(key, obj.unboxed());
      return true;
    } else if (obj instanceof Int) {
      //this.buffer.append(JSON.stringify(obj.unboxed()));
      wantParams?.setParam(key, obj.unboxed());
      return true;
    } else if (obj instanceof Long) {
      //this.buffer.append(JSON.stringify(obj.unboxed()));
      wantParams?.setParam(key, obj.unboxed());
      return true;
    } else if (obj instanceof Float) {
      //this.buffer.append(JSON.stringify(obj.unboxed()));
      wantParams?.setParam(key, obj.unboxed());
      return true;
    } else if (obj instanceof Double) {
      //this.buffer.append(JSON.stringify(obj.unboxed()));
      wantParams?.setParam(key, obj.unboxed());
      return true;
    } else if (obj instanceof BigInt) {
      //this.buffer.append(JSON.stringify(obj));
      //wantParams?.setParam(key, obj.toString()); // 1.1不收集BigInt类型
      return true;
    } else {
      return false;
    }
  }

  private writeArray(arr: Array<valueType>): void {
    this.buffer.append('[');
    const length = arr.length as int;
    this.checkReferencesCycle(arr);
    this.store.add(arr);
    for (let idx = 0; idx < length; idx++) {
      if (arr[idx] == null) {
        this.buffer.append('null');
      } else {
        this.writeObject(arr[idx]);
      }
      if (idx < length - 1) {
        this.buffer.append(',');
      }
    }
    this.store.delete(arr);
    this.buffer.append(']');
  }

  private unwrapArray(arr: Array<valueType>, complexArray: ComplexArrayData): void {
    const length = arr.length as int;
    this.checkReferencesCycle(arr);
    this.store.add(arr);
    for (let idx = 0; idx < length; idx++) {
      const element = arr[idx];
      if (element === null || element === undefined) {
        continue;
      }
      const valueType = typeof element;
      switch (valueType) {
        case 'string':
          complexArray.stringList.push(element as string);
          break;
        case 'boolean':
          complexArray.boolList.push((element as Boolean).unboxed());
          break;
        case 'number':
          if (element instanceof Int) {
            const num = element as number;
            //if (num > int32Min && num < int32Max) {
              complexArray.intlist.push((element as int).unboxed());
              if (complexArray.doubleList.length > 0) {
                complexArray.doubleList.push(...complexArray.intlist.map(num => Number(num)));
                complexArray.intlist = [];
              }
            //} else {
            //  if (complexArray.intlist.length > 0) {
            //    complexArray.doubleList.push(...complexArray.intlist.map(num => Number(num)));
            //    complexArray.intlist = [];
            //  }
            //  complexArray.doubleList.push((element as number).unboxed());
            //}
          } else if (element instanceof Long) {
            // consistent with napi behavior
            const doubleValue = Number(element.unboxed());
            if (complexArray.intlist.length > 0) {
              complexArray.doubleList.push(...complexArray.intlist.map(num => Number(num)));
              complexArray.intlist = [];
            }
            complexArray.doubleList.push(doubleValue);
          } else {
            if (complexArray.intlist.length > 0) {
              complexArray.doubleList.push(...complexArray.intlist.map(num => Number(num)));
              complexArray.intlist = [];
            }
            complexArray.doubleList.push((element as number).unboxed());
          }
          break;
        case 'object':
          complexArray.objectList.push(element);
          break;
        default:
          hilog.error(DOMAIN_ID, TAG, `unknown array value type: %{public}s`, JSON.stringify(arr[idx]));
          break;
      }
    }
    this.store.delete(arr);
  }

  private handleArrayObject(obj: object, wantParams: NativeWantParams): boolean {
    if (Array.isArray(obj)) {
      let localArr: Array<valueType> = obj as Array<valueType>;
      for (let i = 0; i < localArr.length; i++) {
        this.writeObjectToNative(i.toString(), localArr[i] as Object, wantParams);
      }
      // consistent with napi behavior
      wantParams.setParam("length", localArr.length as int);
    } else if (obj instanceof Record) {
      if (this.writeRecordToNative(obj as Object as Record<string, valueType>, wantParams)!= true) {
        hilog.error(DOMAIN_ID, TAG, `write array member To native failed`);
        return false;
      }
    } else {
      const objType = Type.of(obj);
      if (objType instanceof ClassType) {
        let newWantParams = new NativeWantParams();
        return this.writeClassValueToNative(obj, objType as ClassType, wantParams);
      } else {
        return false;
      }
    }
    return true;
  }

  private writeArrayToNative(key: string, arr: Array<valueType>, wantParams: NativeWantParams | null): void {
    let complexArray: ComplexArrayData = {
        intlist: [],
        boolList: [],
        doubleList: [],
        stringList: [],
        objectList: []
      }
    this.unwrapArray(arr, complexArray);
    if (complexArray.objectList.length > 0) {
      let wantParamsArray: Array<NativeWantParams> = [];
      let newWantParams: NativeWantParams;
      for (let i = 0; i < complexArray.objectList.length; i++) {
        newWantParams = new NativeWantParams();
        if (this.handleArrayObject(complexArray.objectList[i] as object, newWantParams)!= true) {
          hilog.error(DOMAIN_ID, TAG, `write array member To native failed, index: %{public}d`, i);
          continue;
        }
        wantParamsArray.push(newWantParams);
      }
      if (wantParamsArray.length > 0) {
        wantParams?.SetArrayWantParams(key, wantParamsArray);
      }
    } else if (complexArray.stringList.length > 0) {
      wantParams?.SetArrayStringParam(key, complexArray.stringList);
    } else if (complexArray.intlist.length > 0) {
      wantParams?.SetArrayIntParam(key, complexArray.intlist);
    } else if (complexArray.boolList.length > 0) {
      wantParams?.SetArrayBooleanParam(key, complexArray.boolList);
    } else if (complexArray.doubleList.length > 0) {
      wantParams?.SetArrayDoubleParam(key, complexArray.doubleList);
    }
  }

  private writeBuildArray(arr: Object, arrayValue: ArrayValue): void {
    this.buffer.append('[');
    const length = arrayValue.getLength() as int;
    this.checkReferencesCycle(arr);
    this.store.add(arr);
    for (let idx = 0; idx < length; idx++) {
      let member = arrayValue.getElement(idx).getData();
      if (member == null) {
        this.buffer.append('null');
      } else {
        this.writeObject(member);
      }
      if (idx < length - 1) {
        this.buffer.append(',');
      }
    }
    this.store.delete(arr);
    this.buffer.append(']');
  }

  private writeRecord(rec: Record<string, valueType>): boolean {
    //this.buffer.append('{');
    this.checkReferencesCycle(rec);
    this.store.add(rec);
    let isSuccess = false;
    for (let key of rec.keys()) {
      if (rec[key] !== undefined) {
        //if (!isFirst) {
        //  this.buffer.append(',');
        //} else {
        //  isFirst = false;
        //}
        //this.buffer.append(JSON.stringify(key as String));
        //this.buffer.append(':');
        //this.writeObject(rec[key]);

        isSuccess = this.writeObjectToNative(key as String, rec[key] as Object);
        if (isSuccess != true) {
          hilog.error(DOMAIN_ID, TAG, `write object to native failed, key: %{public}s`, JSON.stringify(key as String));
          return false;
        }
      }
    }
    this.store.delete(rec);
    //this.buffer.append('}');
    return isSuccess;
  }

  private writeRecordToNative(rec: Record<string, valueType>, wantParams: NativeWantParams): boolean {
    this.checkReferencesCycle(rec);
    this.store.add(rec);
    let isSuccess = false;
    for (let key of rec.keys()) {
      if (rec[key] == undefined) {
        continue;
      }
      isSuccess = this.writeObjectToNative(key as String, rec[key] as Object, wantParams);
      if (isSuccess != true) {
        hilog.error(DOMAIN_ID, TAG, `write object to native failed, key: %{public}s`, JSON.stringify(key as String));
        return false;
      }
    }
    this.store.delete(rec);
    if (isSuccess != true) {
      hilog.error(DOMAIN_ID, TAG, `null record`);
    }
    return isSuccess;
  }

  private writeClassValueToNative(obj: Object, objType: ClassType, wantParams: NativeWantParams): boolean {
    //const currentBuffer = this.buffer
    // we need this local buffer to discard empty object prolog: '{','\n',<space> and replace it with '{}'
    //this.buffer = new StringBuilder("{")

    const writableFields = this.getWritableFields(objType, reflect.Value.of(obj) as ClassValue);

    let isSuccess = false;
    if (writableFields.length > 0) {
        //this.path.add(obj)
        this.checkReferencesCycle(obj);
        this.store.add(obj);
        isSuccess = this.writeClassFields(writableFields, wantParams);
        //this.path.delete(obj);
        this.store.delete(obj);
    }

    return isSuccess;
  }

  private writeField(fieldName: string, fieldVal: NullishType, wantParams: NativeWantParams): boolean {
    if (fieldVal === undefined) {
      return false;
    }

    const fieldValType = Type.of(fieldVal);
    if (fieldValType instanceof LambdaType) {
      return false;
    }

    //this.checkReferencesCycle(fieldVal);
    //this.writeObject(fieldVal)
    return this.writeObjectToNative(fieldName, fieldVal, wantParams);
  }

  private writeClassFields(writableFields: Array<[Field, reflect.Value]>, wantParams: NativeWantParams): boolean {
    const FIELD_TYPE = 0
    const FIELD_VALUE = 1

    let isSuccess = false;
    for (let fieldIdx = 0; fieldIdx < writableFields.length; fieldIdx++) {
        const fieldTypeValuePair = writableFields[fieldIdx];

        const objField = fieldTypeValuePair[0] as Field;
        const objFieldValue = fieldTypeValuePair[1] as reflect.Value;

        //isSuccess = this.writeField(objField.getName(), objFieldValue.getData(), wantParams, isSuccess);
        if (this.writeField(objField.getName(), objFieldValue.getData(), wantParams) == true) {
          isSuccess= true;
        }
    }

    return isSuccess;
  }

  private findKeyIndex(field: Field, fields: Array<[Field, reflect.Value]>): number {
    return fields.findIndex((value: [Field, reflect.Value], index: number, array:Array<[Field, reflect.Value]>)
      => {return value[0].getName() == field.getName()});
  }

  private getWritableFields(classType: ClassType, classValue: ClassValue): Array<[Field, reflect.Value]> {
    const writableFields = new Array<[Field, reflect.Value]>();
    let fieldRename : string | undefined = undefined;

    const fieldsCount = classValue.getFieldsNum();
    for (let fieldIdx = 0; fieldIdx < fieldsCount; fieldIdx++) {
      const field = classType.getField(fieldIdx);
      if (field.isStatic()) {
        continue;
      }
      const index: number = this.findKeyIndex(field, writableFields);
      const fieldValue = classValue.getField(fieldIdx);

      const fieldTypeValuePair: [Field, reflect.Value] = [field, fieldValue];
      if (fieldRename != undefined || index == -1) {
          writableFields.push(fieldTypeValuePair);
      } else {
          writableFields[index] = fieldTypeValuePair;
      }
    }

    return writableFields;
  }

  private checkReferencesCycle(obj: Object): void {
    if (this.store.has(obj)) {
      throw new TypeError('cyclic object value');
    }
  }
}

export class RecordSerializeTool {
  public static stringifyNoThrow(obj: Record<string, Object>, nativeWantParams: long): boolean {
    try {
      return RecordSerializeTool.stringify(obj as Object as Record<string, NullishType>, nativeWantParams);
    } catch (err) {
      hilog.error(DOMAIN_ID, TAG, `RecordSerializeTool.stringify error: ${err}`);
      return false;
    }
  }

  public static parseNoThrow(text: string): Record<string, Object> {
    try {
      return RecordSerializeTool.parse(text) as Object as Record<string, Object>;
    } catch (err) {
      hilog.error(DOMAIN_ID, TAG, `RecordSerializeTool.parse error: ${err}`);
      return new Record<string, Object>();
    }
  }

  public static stringify(obj: Record<string, valueType>, nativeWantParams: long): boolean {
    return new RecordWriter(nativeWantParams).write(obj);
  }

  public static parse(text: string): Record<string, valueType> {
    let jsonValue = JSONParser.parse(text);
    let res = RecordSerializeTool.jsonValue2Object(jsonValue);
    if (!(res instanceof Record)) {
      throw new TypeError('RecordSerializeTool parse only used for Record');
    }
    return res as Record<string, valueType>;
  }

  private static jsonValue2Object(value: JSONValue): string | number | boolean | null |
  Array<valueType> | Record<string, valueType> {
    if (value instanceof JSONString) {
      return value.value;
    } else if (value instanceof JSONNumber) {
      return new Double(value.value);
    } else if (value instanceof JSONTrue) {
      return new Boolean(true);
    } else if (value instanceof JSONFalse) {
      return new Boolean(false);
    } else if (value instanceof JSONNull) {
      return null;
    } else if (value instanceof JSONArray) {
      let obj = value as JSONArray;
      let values = obj.values;
      let result: Array<valueType> = new Array<valueType>();
      for (let i: int = 0; i < values.length; i++) {
        result.push(RecordSerializeTool.jsonValue2Object(values[i]));
      }
      return result;
    } else if (value instanceof JSONObject) {
      let obj = value as JSONObject;
      let keys: Array<JSONString> = obj.keys_;
      let values: Array<JSONValue> = obj.values;
      let result: Record<string, valueType> = new Record<string, valueType>();
      for (let i: int = 0; i < keys.length; i++) {
        result[keys[i].value] = RecordSerializeTool.jsonValue2Object(values[i]);
      }
      return result;
    } else {
      throw new TypeError('unknown JSONValue');
    }
  }
}

class NativeWantParamsCleaner {
  public nativeWantParams: long = 0;

  constructor(nativeWantParams: long) {
    this.nativeWantParams = nativeWantParams;
  }

  private static native nativeDestroy(nativeWantParams: long): void;

  public clean(): void {
    NativeWantParamsCleaner.nativeDestroy(this.nativeWantParams);
    this.nativeWantParams = 0;
  }
}

function nativeWantParamsCleanerCallback(cleaner: NativeWantParamsCleaner): void {
  cleaner.clean();
}

let destroyRegister = new FinalizationRegistry<NativeWantParamsCleaner>(nativeWantParamsCleanerCallback);

class NativeWantParams {
  static {
    loadLibrary("want_ani_kit.z");
  }

  private nativeWantParams: long = 0;
  private cleaner: NativeWantParamsCleaner | null = null;

  constructor(wantParams?: long) {
    if (wantParams === undefined) {
      this.nativeWantParams = NativeWantParams.nativeCreate();
      this.registerCleaner(this.nativeWantParams)
      return;
    }
    this.nativeWantParams = wantParams;
  }

  private static native nativeCreate(): long;

  private static native nativeSetStringParam(nativeWantParams: long, key: string, value: string): boolean;
  private static native nativeSetDoubleParam(nativeWantParams: long, key: string, value: double): boolean;
  private static native nativeSetIntParam(nativeWantParams: long, key: string, value: int): boolean;
  private static native nativeSetLongParam(nativeWantParams: long, key: string, value: long): boolean;
  private static native nativeSetBooleanParam(nativeWantParams: long, key: string, value: boolean): boolean;
  private static native nativeSetWantParams(nativeWantParams: long, key: string, value: long): boolean;

  private static native nativeSetArrayStringParam(nativeWantParams: long, key: string, value: Array<string>): boolean;
  private static native nativeSetArrayDoubleParam(nativeWantParams: long, key: string, value: Array<double>): boolean;
  private static native nativeSetArrayIntParam(nativeWantParams: long, key: string, value: Array<int>): boolean;
  private static native nativeSetArrayLongParam(nativeWantParams: long, key: string, value: Array<long>): boolean;
  private static native nativeSetArrayBooleanParam(nativeWantParams: long, key: string, value: Array<boolean>): boolean;
  private static native nativeSetArrayWantParams(nativeWantParams: long, key: string, value: Array<long>): boolean;

  private static native nativeSetRemoteObjectParam(
    nativeWantParams: long, key: string, value: rpc.RemoteObject): boolean;

  private registerCleaner(nativeWantParams: long): void {
    this.cleaner = new NativeWantParamsCleaner(nativeWantParams);
    destroyRegister.register(this, this.cleaner!, this);
  }

  public setParam(key: string, value: string): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetStringParam(this.nativeWantParams, key, value);
  }

  public setParam(key: string, value: double): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetDoubleParam(this.nativeWantParams, key, value);
  }

  public setParam(key: string, value: int): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetIntParam(this.nativeWantParams, key, value);
  }

  public setParam(key: string, value: long): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetLongParam(this.nativeWantParams, key, value);
  }

  public setParam(key: string, value: boolean): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetBooleanParam(this.nativeWantParams, key, value);
  }

  public setParam(key: string, value: NativeWantParams): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetWantParams(this.nativeWantParams, key, value.nativeWantParams);
  }

  public SetParam(key: string, value: Array<string>): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetArrayStringParam(this.nativeWantParams, key, value);
  }

  public SetArrayDoubleParam(key: string, value: Array<double>): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetArrayDoubleParam(this.nativeWantParams, key, value);
  }

  public SetArrayIntParam(key: string, value: Array<int>): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetArrayIntParam(this.nativeWantParams, key, value);
  }

  public SetArrayLongParam(key: string, value: Array<long>): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetArrayLongParam(this.nativeWantParams, key, value);
  }

  public SetArrayBooleanParam(key: string, value: Array<boolean>): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetArrayBooleanParam(this.nativeWantParams, key, value);
  }

  public SetArrayWantParams(key: string, value: Array<NativeWantParams>): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    let nativeWantParams: Array<long> = new Array<long>();
    for (let item of value) {
      nativeWantParams.push(item.nativeWantParams);
    }
    return NativeWantParams.nativeSetArrayWantParams(this.nativeWantParams, key, nativeWantParams);
  }

  public setParam(key: string, value: rpc.RemoteObject): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetRemoteObjectParam(this.nativeWantParams, key, value);
  }
}

export default class Want {
  bundleName?: string;
  abilityName?: string;
  deviceId?: string;
  uri?: string;
  type?: string;
  flags?: int;
  action?: string;
  parameters?: Record<string, Object>;
  entities?: Array<string>;
  moduleName?: string;
}
