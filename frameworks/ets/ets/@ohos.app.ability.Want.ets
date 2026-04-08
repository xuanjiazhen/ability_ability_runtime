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
import { RecordData } from '@ohos.base';

type valueType = Any;
const DOMAIN_ID = 0xD001300;
const TAG = 'RecordSerializeTool';
const int32Max: number = Math.pow(2, 31) - 1;
const int32Min: number = -Math.pow(2, 31);

const PARAM_RESV_WINDOW_MODE = 'ohos.aafwk.param.windowMode';
const PARAM_RESV_DISPLAY_ID = 'ohos.aafwk.param.displayId';

const TYPE_PROPERTY = 'type';
const REMOTE_OBJECT = 'RemoteObject';
const VALUE_PROPERTY = 'value';
const PROPERTIES_SIZE: number = 2;

interface ComplexArrayData {
  intList: Array<int>;
  boolList: Array<boolean>;
  doubleList: Array<double>;
  stringList: Array<string>;
  objectList: Array<Any>;
}

class RecordWriter {
  private buffer = new StringBuilder();
  private store = new Set<Object>();
  private wantParams: NativeWantParams | null = null;
  private static readonly GETTER_PREFIX = '<get>'

  private static asFixedArray<E>(obj: object): FixedArray<E> {
    return obj as FixedArray<E>
  }

  constructor(wantParams: long) {
    this.wantParams = new NativeWantParams(wantParams);
  }

  public write(obj: Object): boolean {
    hilog.info(DOMAIN_ID, TAG, `[DEBUG] RecordWriter.write enter, obj type: %{public}s, value: %{public}s`, typeof obj, JSON.stringify(obj));
    return this.parseRecord(obj);
  }

  private parseRecord(obj: Any): boolean {
    hilog.info(DOMAIN_ID, TAG, `[DEBUG] parseRecord enter, obj instanceof Record: %{public}s, obj type: %{public}s, value: %{public}s`,
      String(obj instanceof Record), typeof obj, JSON.stringify(obj));
    if (obj instanceof Record) {
      this.writeRecordToNative(obj as Object as Record<string, valueType>, this.wantParams);
      return true;
    } else {
      hilog.error(DOMAIN_ID, TAG, `[DEBUG] parseRecord failed: input object not record type, actual type: %{public}s, value: %{public}s`,
        typeof obj, JSON.stringify(obj));
      return false;
    }
  }

  private writeObject(obj: Any): void {
    hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObject enter, obj type: %{public}s, value: %{public}s`, typeof obj, JSON.stringify(obj));
    if (obj === null) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObject branch: null`);
      this.buffer.append('null');
    } else if (obj === undefined) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObject branch: undefined`);
      this.buffer.append('undefined');
    } else if (obj instanceof String) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObject branch: String, value: %{public}s`, JSON.stringify(obj as String));
      this.buffer.append(JSON.stringify(obj as String));
    } else if (this.writeValueType(obj as Object)) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObject branch: valueType, value: %{public}s`, JSON.stringify(obj));
      // nothing to do
    } else if (obj instanceof Array) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObject branch: Array, length: %{public}d`, (obj as Array<valueType>).length);
      this.writeArray(obj as Object as Array<valueType>);
    } else if (obj instanceof Record) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObject branch: Record, value: %{public}s`, JSON.stringify(obj));
      this.writeRecord(obj as Object as Record<string, valueType>);
    } else if (obj instanceof Object) {
      const objCls = Class.of(obj);
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObject branch: Object, isFixedArray: %{public}s, isEnum: %{public}s, isInterface: %{public}s, isUnion: %{public}s, isPrimitive: %{public}s, value: %{public}s`,
        String(objCls.isFixedArray()), String(objCls.isEnum()), String(objCls.isInterface()),
        String(objCls.isUnion()), String(objCls.isPrimitive()), JSON.stringify(obj));
      if (objCls.isFixedArray()) {
        this.writeBuildArray(RecordWriter.asFixedArray<Any>(obj));
      }
    } else {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObject branch: fallback null, value: %{public}s`, JSON.stringify(obj));
      this.buffer.append('null');
    }
  }

  private tupleToArray(tuple: Tuple): Array<valueType> {
    let arr: Array<valueType> = [];

    const obj = tuple as Object;
    const objClass = Class.of(obj);
    const writableFields = this.getWritableFields(objClass, obj);
    for (let fieldIdx = 0; fieldIdx < writableFields.length; fieldIdx++) {
      const fieldValue = writableFields[fieldIdx][1];
      arr.push(fieldValue);
    }
    return arr;
  }

  private isClassType(cls: Class): boolean {
    return (!cls.isEnum()
             && !cls.isInterface()
             && !cls.isFixedArray()
             && !cls.isUnion()
             && !cls.isPrimitive())
  }

  private writeObjectToNative(key: string, obj: Any, wantParams?: NativeWantParams | null): boolean {
    let localWantParams = (wantParams === undefined) ? this.wantParams : wantParams;
    hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative enter, key: %{public}s, value type: %{public}s, value: %{public}s`,
      JSON.stringify(key), typeof obj, JSON.stringify(obj));
    // skip reserved param
    if (this.blackListFilter(key)) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative branch: blackListFilter, key: %{public}s`, JSON.stringify(key));
      return true;
    }
    if (obj === null) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative branch: null, key: %{public}s`, JSON.stringify(key));
      return true;
    } else if (obj === undefined) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative branch: undefined, key: %{public}s`, JSON.stringify(key));
      return true;
    } else if (obj instanceof String) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative branch: String, key: %{public}s, value: %{public}s`,
        JSON.stringify(key), JSON.stringify(obj as String));
      localWantParams?.setParam(key, obj as String);
      return true;
    } else if (this.writeValueTypeToNative(localWantParams, key, obj as Object)) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative branch: valueType, key: %{public}s, value: %{public}s`,
        JSON.stringify(key), JSON.stringify(obj));
      // nothing to do
      return true;
    } else if (obj instanceof Array) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative branch: Array, key: %{public}s, length: %{public}d`,
        JSON.stringify(key), (obj as Array<valueType>).length);
      this.writeArrayToNative(key, obj as Object as Array<valueType>, localWantParams);
      return true;
    } else if (obj instanceof Tuple) {
      let tupleAsArray = this.tupleToArray(obj as Tuple);
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative branch: Tuple, key: %{public}s, tupleAsArray length: %{public}d`,
        JSON.stringify(key), tupleAsArray.length);
      this.writeArrayToNative(key, tupleAsArray as Object as Array<valueType>, localWantParams);
      return true;
    } else if (this.isRemoteObjectFormat(obj)) { // consistent with napi behavior
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative branch: RemoteObject, key: %{public}s`, JSON.stringify(key));
      let newWantParams = new NativeWantParams();
       if (this.writeRemoteObjectToNative(obj as Object as Record<string, valueType>, newWantParams) === true) {
          localWantParams?.setParam(key, newWantParams);
       }
      return true;
    } else if (this.isRemoteProxyFormat(obj)) { // consistent with napi behavior
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative branch: RemoteProxy, key: %{public}s`, JSON.stringify(key));
      let newWantParams = new NativeWantParams();
       if (this.writeRemoteProxyToNative(obj as Object as Record<string, valueType>, newWantParams) === true) {
          localWantParams?.setParam(key, newWantParams);
       }
      return true;
    } else if (obj instanceof Record) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative branch: Record, key: %{public}s, value: %{public}s`,
        JSON.stringify(key), JSON.stringify(obj));
      let newWantParams = new NativeWantParams();
      this.writeRecordToNative(obj as Object as Record<string, valueType>, newWantParams);
      localWantParams?.setParam(key, newWantParams);
      return true;
    } else {
      const objCls = Class.of(obj as Object);
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative branch: else(Object), key: %{public}s, isClassType: %{public}s, isEnum: %{public}s, isInterface: %{public}s, isFixedArray: %{public}s, isUnion: %{public}s, isPrimitive: %{public}s, typeof obj: %{public}s, value: %{public}s`,
        JSON.stringify(key), String(this.isClassType(objCls)),
        String(objCls.isEnum()), String(objCls.isInterface()),
        String(objCls.isFixedArray()), String(objCls.isUnion()),
        String(objCls.isPrimitive()), typeof obj, JSON.stringify(obj));
      if (this.isClassType(objCls)) {
        // enum value are string
        if (typeof obj === 'string') {
          hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative sub-branch: Class string, key: %{public}s, value: %{public}s`,
            JSON.stringify(key), String(obj as Object));
          let value: string = String(obj as Object);
          localWantParams?.setParam(key, value);
          return true;
        }
        // enum value are number
        if (typeof obj === 'number') {
          hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative sub-branch: Class number, key: %{public}s, value: %{public}s`,
            JSON.stringify(key), String(obj as Object));
          let value: string = String(obj as Object);
          let num: number = Number(value);
          if (Number.isInteger(num)) {
            if (num >= int32Min && num <= int32Max) {
              localWantParams?.setParam(key, num as int);
            } else {
              localWantParams?.setParam(key, num as double);
            }
          } else {
            localWantParams?.setParam(key, num as double);
          }
          return true;
        }
        hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative sub-branch: Class object, key: %{public}s, value: %{public}s`,
          JSON.stringify(key), JSON.stringify(obj));
        let newWantParams = new NativeWantParams();
        this.writeClassValueToNative(obj as Object, objCls, newWantParams);
        localWantParams?.setParam(key, newWantParams);
        return true;
      } else {
        hilog.error(DOMAIN_ID, TAG, `[DEBUG] writeObjectToNative sub-branch: NOT class type, key: %{public}s, value: %{public}s`,
          JSON.stringify(key), JSON.stringify(obj));
      }
      return true;
    }
    return false;
  }

  private blackListFilter(key: string): boolean {
    if (key === PARAM_RESV_WINDOW_MODE) {
      return true;
    }
    if (key === PARAM_RESV_DISPLAY_ID) {
      return true;
    }
    return false;
  }

  private writeValueType(obj: Object): boolean {
    if (obj instanceof Boolean) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Byte) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Char) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Short) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Int) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Long) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Float) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Double) {
      this.buffer.append(JSON.stringify(obj));
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
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeValueTypeToNative: Boolean, key: %{public}s, value: %{public}s`, key, JSON.stringify(obj));
      wantParams?.setParam(key, obj);
      return true;
    } else if (obj instanceof Byte) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeValueTypeToNative: Byte, key: %{public}s, value: %{public}s`, key, JSON.stringify(obj));
      wantParams?.setParam(key, obj);
      return true;
    } else if (obj instanceof Char) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeValueTypeToNative: Char, key: %{public}s, value: %{public}s`, key, JSON.stringify(obj));
      wantParams?.setParam(key, Char.toByte(obj));
      return true;
    } else if (obj instanceof Short) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeValueTypeToNative: Short, key: %{public}s, value: %{public}s`, key, JSON.stringify(obj));
      wantParams?.setParam(key, obj);
      return true;
    } else if (obj instanceof Int) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeValueTypeToNative: Int, key: %{public}s, value: %{public}s`, key, JSON.stringify(obj));
      wantParams?.setParam(key, obj);
      return true;
    } else if (obj instanceof Long) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeValueTypeToNative: Long, key: %{public}s, value: %{public}s`, key, JSON.stringify(obj));
      wantParams?.setParam(key, obj);
      return true;
    } else if (obj instanceof Float) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeValueTypeToNative: Float, key: %{public}s, value: %{public}s`, key, JSON.stringify(obj));
      wantParams?.setParam(key, obj);
      return true;
    } else if (obj instanceof Double) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeValueTypeToNative: Double, key: %{public}s, value: %{public}s`, key, JSON.stringify(obj));
      // consistent with napi behavior
      if (key === VALUE_PROPERTY) {
        let value: double = obj;
        if (Number.isInteger(value) && value >= int32Min && value <= int32Max) {
          wantParams?.setParam(key, value as int);
        } else {
          wantParams?.setParam(key, value as double);
        }
      } else {
        wantParams?.setParam(key, obj);
      }
      return true;
    } else if (obj instanceof BigInt) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeValueTypeToNative: BigInt, key: %{public}s, value: %{public}s`, key, JSON.stringify(obj));
      // consistent with napi behavior
      return true;
    } else {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeValueTypeToNative: not matched, key: %{public}s, value: %{public}s`, key, JSON.stringify(obj));
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

  private unwrapArrayElementNumber(element: valueType, complexArray: ComplexArrayData): void {
    if (element instanceof Int) {
      hilog.debug(DOMAIN_ID, TAG, `array value type Int`);
      if (complexArray.doubleList.length > 0) {
        complexArray.intList.forEach(num => {
          complexArray.doubleList.push(Number(num));
        });
        complexArray.intList = [];
        complexArray.doubleList.push((element as number));
      } else {
        complexArray.intList.push((element as int));
      }
    } else if (element instanceof Long) {
      hilog.debug(DOMAIN_ID, TAG, `array value type Long`);
      // consistent with napi behavior
      const doubleValue = Number(element);
      if (complexArray.intList.length > 0) {
        complexArray.intList.forEach(num => {
          complexArray.doubleList.push(Number(num));
        });
        complexArray.intList = [];
      }
      complexArray.doubleList.push(doubleValue);
    } else if (element instanceof Double) {
      hilog.debug(DOMAIN_ID, TAG, `array value type Double`);
      if (complexArray.intList.length > 0) {
        complexArray.intList.forEach(num => {
          complexArray.doubleList.push(Number(num));
        });
        complexArray.intList = [];
      }
      complexArray.doubleList.push((element as number));
    } else {
      const objCls = Class.of(element as Object);
      // enum value are number
      if (this.isClassType(objCls)) {
        let value: string = String(element as Object);
        let enumNum: number = Number(value);
        if (Number.isInteger(enumNum) && enumNum >= int32Min && enumNum <= int32Max) {
          hilog.debug(DOMAIN_ID, TAG, `array value type Class Int`);
          if (complexArray.doubleList.length > 0) {
            complexArray.intList.forEach(num => {
              complexArray.doubleList.push(Number(num));
            });
            complexArray.intList = [];
            complexArray.doubleList.push(enumNum as double);
          } else {
            complexArray.intList.push(enumNum as int);
          }
        } else {
          hilog.debug(DOMAIN_ID, TAG, `array value type Class double`);
          if (complexArray.intList.length > 0) {
            complexArray.intList.forEach(num => {
              complexArray.doubleList.push(Number(num));
            });
            complexArray.intList = [];
          }
          complexArray.doubleList.push(enumNum as double);
        }
      }
    }
  }

  private unwrapArray(arr: Array<valueType>, complexArray: ComplexArrayData): void {
    const length = arr.length as int;
    this.checkReferencesCycle(arr);
    this.store.add(arr);
    hilog.info(DOMAIN_ID, TAG, `[DEBUG] unwrapArray enter, array length: %{public}d`, length);
    for (let idx = 0; idx < length; idx++) {
      const element = arr[idx];
      if (element === null || element === undefined) {
        hilog.info(DOMAIN_ID, TAG, `[DEBUG] unwrapArray[%{public}d]: null or undefined, skipping`, idx);
        continue;
      }
      const valueType = typeof element;
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] unwrapArray[%{public}d]: type=%{public}s, value=%{public}s, instanceof String=%{public}s, instanceof Record=%{public}s, instanceof Array=%{public}s`,
        idx, valueType, JSON.stringify(element),
        String(element instanceof String), String(element instanceof Record), String(element instanceof Array));
      switch (valueType) {
        case 'string':
          if (element instanceof String) {
            complexArray.stringList.push(element as string);
          } else {
            const objCls = Class.of(element as Object);
            if (this.isClassType(objCls)) {
              let value: string = String(element as Object);
              complexArray.stringList.push(value as string);
            }
          }
          break;
        case 'boolean':
          complexArray.boolList.push((element as Boolean));
          break;
        case 'number':
        case 'int':
        case 'long':
        case 'float':
          try {
            this.unwrapArrayElementNumber(element, complexArray);
          } catch (err) {
            hilog.error(DOMAIN_ID, TAG, `unwrapArray number error: ${err}`);
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
    if (Array.isArray(obj)) { // consistent with napi behavior
      let localArr: Array<valueType> = obj as Array<valueType>;
      hilog.debug(DOMAIN_ID, TAG, `array object element type array, length: %{public}d`, localArr.length);
      for (let i = 0; i < localArr.length; i++) {
        this.writeObjectToNative(i.toString(), localArr[i] as Object, wantParams);
      }
      wantParams.setParam("length", localArr.length as int);
    } else if (obj instanceof Tuple) { // consistent with napi behavior
      let tupleAsArray = this.tupleToArray(obj as Tuple);
      hilog.debug(DOMAIN_ID, TAG, `array object element type tuple, length: %{public}d`, tupleAsArray.length);
      for (let i = 0; i < tupleAsArray.length; i++) {
        this.writeObjectToNative(i.toString(), tupleAsArray[i] as Object, wantParams);
      }
      wantParams.setParam("length", tupleAsArray.length as int);
    } else if (obj instanceof Record) {
      hilog.debug(DOMAIN_ID, TAG, `array object element type Record`);
      this.writeRecordToNative(obj as Object as Record<string, valueType>, wantParams);
    } else {
      const objCls = Class.of(obj);
      if (this.isClassType(objCls)) {
        hilog.debug(DOMAIN_ID, TAG, `array object element type class`);
        this.writeClassValueToNative(obj, objCls, wantParams);
      } else {
        hilog.error(DOMAIN_ID, TAG, `array member object type invalid`);
        return false;
      }
    }
    return true;
  }

  private writeArrayToNative(key: string, arr: Array<valueType>, wantParams: NativeWantParams | null): void {
    hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeArrayToNative enter, key: %{public}s, array length: %{public}d, value: %{public}s`,
      key, arr.length, JSON.stringify(arr));
    let complexArray: ComplexArrayData = {
      intList: [],
      boolList: [],
      doubleList: [],
      stringList: [],
      objectList: []
    }
    this.unwrapArray(arr, complexArray);
    hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeArrayToNative unwrap result, key: %{public}s, intList: %{public}d, boolList: %{public}d, doubleList: %{public}d, stringList: %{public}d, objectList: %{public}d`,
      key, complexArray.intList.length, complexArray.boolList.length,
      complexArray.doubleList.length, complexArray.stringList.length, complexArray.objectList.length);
    if (complexArray.objectList.length > 0) {
      hilog.debug(DOMAIN_ID, TAG, `array %{public}s objectList size: %{public}d`, key, complexArray.objectList.length);
      let wantParamsArray: Array<NativeWantParams> = [];
      let newWantParams: NativeWantParams;
      for (let i = 0; i < complexArray.objectList.length; i++) {
        newWantParams = new NativeWantParams();
        if (this.handleArrayObject(complexArray.objectList[i] as object, newWantParams) != true) {
          hilog.error(DOMAIN_ID, TAG, `parse array member failed, index: %{public}d`, i);
        }
        wantParamsArray.push(newWantParams);
      }
      wantParams?.SetArrayWantParams(key, wantParamsArray);
    } else if (complexArray.stringList.length > 0) {
      hilog.debug(DOMAIN_ID, TAG, `array %{public}s stringList size: %{public}d`, key, complexArray.stringList.length);
      wantParams?.SetArrayStringParam(key, complexArray.stringList);
    } else if (complexArray.intList.length > 0) {
      hilog.debug(DOMAIN_ID, TAG, `array %{public}s intList size: %{public}d`, key, complexArray.intList.length);
      wantParams?.SetArrayIntParam(key, complexArray.intList);
    } else if (complexArray.boolList.length > 0) {
      hilog.debug(DOMAIN_ID, TAG, `array %{public}s boolList size: %{public}d`, key, complexArray.boolList.length);
      wantParams?.SetArrayBooleanParam(key, complexArray.boolList);
    } else if (complexArray.doubleList.length > 0) {
      hilog.debug(DOMAIN_ID, TAG, `array %{public}s doubleList size: %{public}d`, key, complexArray.doubleList.length);
      wantParams?.SetArrayDoubleParam(key, complexArray.doubleList);
    }
  }

  private writeBuildArray<E>(array: FixedArray<E>): void {
    this.buffer.append('[');
    const length = array.length;
    this.checkReferencesCycle(array);
    this.store.add(array);
    for (let idx = 0; idx < length; idx++) {
      let member = array[idx];
      if (member == null) {
        this.buffer.append('null');
      } else {
        this.writeObject(member);
      }
      if (idx < length - 1) {
        this.buffer.append(',');
      }
    }
    this.store.delete(array);
    this.buffer.append(']');
  }

  private writeRecord(rec: Record<string, valueType>): void {
    this.buffer.append('{');
    this.checkReferencesCycle(rec);
    this.store.add(rec);
    let isFirst = true;
    for (let key of rec.keys()) {
      if (rec[key] !== undefined) {
        if (!isFirst) {
          this.buffer.append(',');
        } else {
          isFirst = false;
        }
        this.buffer.append(JSON.stringify(key as String));
        this.buffer.append(':');
        this.writeObject(rec[key]);
      }
    }
    this.store.delete(rec);
    this.buffer.append('}');
  }

  private isRemoteObjectFormat(obj: valueType): boolean {
    if (!(obj instanceof Record)) {
      return false;
    }
    let hasType = false;
    let hasValue = false;
    let typeValue: valueType = undefined;
    let valueValue: valueType = undefined;
    let count = 0;
    let rec: Record<string, valueType> = obj as Object as Record<string, valueType>;
    for (let key of rec.keys()) {
      if (rec[key] === undefined || rec[key] === null) {
        return false;
      }
      if (key === TYPE_PROPERTY) {
        hasType = true;
        typeValue = rec[key];
      } else if (key === VALUE_PROPERTY) {
        hasValue = true;
        valueValue = rec[key];
      }
      count++;
    }

    if (count !== PROPERTIES_SIZE) {
      return false;
    }
    if (!hasType || !hasValue) {
      return false;
    }
    if (typeof typeValue !== 'string' || typeValue !== REMOTE_OBJECT) {
      return false;
    }
    if (!(valueValue instanceof rpc.RemoteObject)) {
      return false;
    }

    return true;
  }

  private isRemoteProxyFormat(obj: valueType): boolean {
    if (!(obj instanceof Record)) {
      return false;
    }
    let hasType = false;
    let hasValue = false;
    let typeValue: valueType = undefined;
    let valueValue: valueType = undefined;
    let count = 0;
    let rec: Record<string, valueType> = obj as Object as Record<string, valueType>;
    for (let key of rec.keys()) {
      if (rec[key] === undefined || rec[key] === null) {
        return false;
      }
      if (key === TYPE_PROPERTY) {
        hasType = true;
        typeValue = rec[key];
      } else if (key === VALUE_PROPERTY) {
        hasValue = true;
        valueValue = rec[key];
      }
      count++;
    }

    if (count !== PROPERTIES_SIZE) {
      return false;
    }
    if (!hasType || !hasValue) {
      return false;
    }
    if (typeof typeValue !== 'string' || typeValue !== REMOTE_OBJECT) {
      return false;
    }
    if (!(valueValue instanceof rpc.RemoteProxy)) {
      return false;
    }

    return true;
  }

  private writeRemoteObjectToNative(obj: Record<string, valueType>, wantParams: NativeWantParams | null): boolean {
    for (let key of obj.keys()) {
      if (key === TYPE_PROPERTY) {
        wantParams?.setParam(TYPE_PROPERTY, REMOTE_OBJECT);
      } else if (key === VALUE_PROPERTY) {
        if (wantParams === null || wantParams.setParam(VALUE_PROPERTY, obj[key] as rpc.RemoteObject) != true) {
          hilog.error(DOMAIN_ID, TAG, `write RemoteObject to native failed`);
          return false;
        }
      }
    }
    return true;
  }

  private writeRemoteProxyToNative(obj: Record<string, valueType>, wantParams: NativeWantParams | null): boolean {
    for (let key of obj.keys()) {
      if (key === TYPE_PROPERTY) {
        wantParams?.setParam(TYPE_PROPERTY, REMOTE_OBJECT);
      } else if (key === VALUE_PROPERTY) {
        if (wantParams === null || wantParams.setParam(VALUE_PROPERTY, obj[key] as rpc.RemoteProxy) != true) {
          hilog.error(DOMAIN_ID, TAG, `write RemoteProxy to native failed`);
          return false;
        }
      }
    }
    return true;
  }

  private writeRecordToNative(rec: Record<string, valueType>, wantParams: NativeWantParams | null): boolean {
    this.checkReferencesCycle(rec);
    this.store.add(rec);
    let isSuccess = false;
    hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeRecordToNative enter, record value: %{public}s`, JSON.stringify(rec));
    for (let key of rec.keys()) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeRecordToNative iterating key: %{public}s, value type: %{public}s, value: %{public}s`,
        JSON.stringify(key as String), typeof rec[key], JSON.stringify(rec[key]));
      if (rec[key] === undefined || rec[key] === null) {
        hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeRecordToNative skip null/undefined key: %{public}s`, JSON.stringify(key as String));
        continue;
      }
      if (this.writeObjectToNative(key as String, rec[key] as Object, wantParams) != true) {
        hilog.error(DOMAIN_ID, TAG, `[DEBUG] writeRecordToNative writeObjectToNative failed, key: %{public}s, value: %{public}s`,
          JSON.stringify(key as String), JSON.stringify(rec[key]));
      }
      isSuccess = true;
    }
    this.store.delete(rec);
    if (isSuccess != true) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeRecordToNative null record, no keys written`);
    }
    return isSuccess;
  }

  private writeClassValueToNative(obj: Object, objCls: Class, wantParams: NativeWantParams): boolean {
    const writableFields = this.getWritableFields(objCls, obj);
    hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeClassValueToNative enter, writableFields count: %{public}d, obj: %{public}s`,
      writableFields.length, JSON.stringify(obj));

    let isSuccess = false;
    if (writableFields.length > 0) {
      this.checkReferencesCycle(obj);
      this.store.add(obj);
      isSuccess = this.writeClassFields(writableFields, wantParams);
      this.store.delete(obj);
    } else {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeClassValueToNative no writableFields found, obj: %{public}s`, JSON.stringify(obj));
    }

    return isSuccess;
  }

  private writeField(fieldName: string, fieldVal: Any, wantParams: NativeWantParams): boolean {
    hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeField enter, fieldName: %{public}s, fieldVal type: %{public}s, fieldVal: %{public}s`,
      fieldName, typeof fieldVal, JSON.stringify(fieldVal));
    if (fieldVal === undefined) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeField skip: fieldVal is undefined, fieldName: %{public}s`, fieldName);
      return false;
    }

    const fieldValType = Class.ofAny(fieldVal);
    if (fieldValType instanceof Function) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeField skip: fieldValType is Function, fieldName: %{public}s`, fieldName);
      return false;
    }

    return this.writeObjectToNative(fieldName, fieldVal, wantParams);
  }

  private writeClassFields(writableFields: Array<[string, Any]>, wantParams: NativeWantParams): boolean {
    const FIELD_NAME = 0
    const FIELD_VALUE = 1

    hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeClassFields enter, fields count: %{public}d`, writableFields.length);
    let isSuccess = false;
    for (let fieldIdx = 0; fieldIdx < writableFields.length; fieldIdx++) {
      const fieldTypeValuePair = writableFields[fieldIdx];

      const objFieldName = fieldTypeValuePair[FIELD_NAME];
      const objFieldValue = fieldTypeValuePair[FIELD_VALUE];
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeClassFields field[%{public}d]: name=%{public}s, value type=%{public}s, value=%{public}s`,
        fieldIdx, objFieldName, typeof objFieldValue, JSON.stringify(objFieldValue));

      if (this.writeField(objFieldName, objFieldValue, wantParams) == true) {
        isSuccess= true;
      }
    }

    hilog.info(DOMAIN_ID, TAG, `[DEBUG] writeClassFields done, isSuccess: %{public}s`, String(isSuccess));
    return isSuccess;
  }

  private findKeyIndex(fieldName: string, fields: Array<[string, Any]>): number {
    return fields.findIndex((writableField) => writableField[0]  == fieldName);
  }

  private addWritableProperty(cls: Class, obj: object, propName: string, writableFields: Array<[string, Any]>): void {
      const getterName = RecordWriter.GETTER_PREFIX + propName
      const getter = cls.getInstanceMethod(getterName)
      if (getter === undefined) {
          return
      }

      const propVal = getter.invoke(obj)

      const propNameValuePair: [string, Any] = [propName, propVal]
      writableFields.push(propNameValuePair)
  }

  private addWritableProperties(cls: Class, obj: object, writableFields: Array<[string, Any]>): void {
      for (const method of cls.getInstanceMethods()) {
          const methodName = method.getName()
          if (methodName.startsWith(RecordWriter.GETTER_PREFIX)) {
              const propName = methodName.substring(RecordWriter.GETTER_PREFIX.length)
              const propValue = method.invoke(obj)
              const fieldNameValuePair: [string, Any] = [propName, propValue]
              writableFields.push(fieldNameValuePair)
          }
      }
  }

  private getWritableFields(cls: Class, obj: object): Array<[string, Any]> {
    const writableFields = new Array<[string, Any]>();
    let fieldRename : string | undefined = undefined;
    const instanceFields = cls.getInstanceFields();
    hilog.info(DOMAIN_ID, TAG, `[DEBUG] getWritableFields enter, instanceFields count: %{public}d, obj: %{public}s`,
      instanceFields.length, JSON.stringify(obj));

    for (const field of instanceFields) {
      const index: int = this.findKeyIndex(field.getName(), writableFields).toInt();
      const fieldValue = field.getValue(obj);
      const fieldNameValuePair: [string, Any] = [field.getName(), fieldValue];
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] getWritableFields field[%{public}s]: name=%{public}s, value type=%{public}s, value=%{public}s`,
        String(index), field.getName(), typeof fieldValue, JSON.stringify(fieldValue));
      if (fieldRename != undefined || index == -1) {
        writableFields.push(fieldNameValuePair);
      } else {
        writableFields[index] = fieldNameValuePair;
      }
      if (reflect.isLiteralInitializedInterface(obj)) {
        this.addWritableProperties(cls, obj, writableFields)
      }
    }

    hilog.info(DOMAIN_ID, TAG, `[DEBUG] getWritableFields result, total writableFields: %{public}d`, writableFields.length);
    for (let i = 0; i < writableFields.length; i++) {
      hilog.info(DOMAIN_ID, TAG, `[DEBUG] getWritableFields result[%{public}d]: name=%{public}s, value=%{public}s`,
        i, writableFields[i][0], JSON.stringify(writableFields[i][1]));
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
  public static unwrapRecordNoThrow(obj: Record<string, Object>, nativeWantParams: long): boolean {
    try {
      return RecordSerializeTool.unwrapRecord(obj as Object as Record<string, Any>, nativeWantParams);
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

  public static unwrapRecord(obj: Record<string, valueType>, nativeWantParams: long): boolean {
    return new RecordWriter(nativeWantParams).write(obj);
  }

  public static parse(text: string): Record<string, valueType> {
    let jsonValue = JSON.parseJsonElement(text);
    let res = RecordSerializeTool.jsonValue2Object(jsonValue);
    if (!(res instanceof Record)) {
      throw new TypeError('RecordSerializeTool parse only used for Record');
    }
    return res as Record<string, valueType>;
  }

  private static jsonValue2Object(value: jsonx.JsonElement): string | number | boolean | null |
    Array<valueType> | Record<string, valueType> {
    let vType = value.jsonType;
    if (vType === jsonx.JsonType.JsonString) {
      return value.asString();
    } else if (vType === jsonx.JsonType.JsonNumber) {
      return value.asDouble();
    } else if (vType === jsonx.JsonType.JsonTrue) {
      return new Boolean(true);
    } else if (vType === jsonx.JsonType.JsonFalse) {
      return new Boolean(false);
    } else if (vType === jsonx.JsonType.JsonNull) {
      return null;
    } else if (vType === jsonx.JsonType.JsonArray) {
      let values: Array<jsonx.JsonElement> = value.asArray();
      let result: Array<valueType> = new Array<valueType>();
      for (let i: int = 0; i < values.length; i++) {
        result.push(RecordSerializeTool.jsonValue2Object(values[i]));
      }
      return result;
    } else if (vType === jsonx.JsonType.JsonObject) {
      let iterVal: IterableIterator<[string, jsonx.JsonElement]> = value.$_iterator();
      let result: Record<string, valueType> = new Record<string, valueType>();
      for (let t of iterVal) {
        result[t[0]] = RecordSerializeTool.jsonValue2Object(t[1]);
      }
      return result;
    } else {
      throw new TypeError('unknown JSONElement type');
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
  private static native nativeSetRemoteProxyParam(
    nativeWantParams: long, key: string, value: rpc.RemoteProxy): boolean;

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

  public setParam(key: string, value: double): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetDoubleParam(this.nativeWantParams, key, value);
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

  public SetArrayStringParam(key: string, value: Array<string>): boolean {
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

  public setParam(key: string, value: rpc.RemoteProxy): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetRemoteProxyParam(this.nativeWantParams, key, value);
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
  parameters?: Record<string, RecordData>;
  entities?: Array<string>;
  moduleName?: string;
  readonly fds?: Record<string, int>;
}
