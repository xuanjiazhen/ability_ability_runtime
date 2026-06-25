import type { processDataCallback } from './i_idl_service_ext';
import type { insertDataToMapCallback } from './i_idl_service_ext';
import IdlServiceExtStub from "@normalized:N&&&entry/src/main/ets/IdlServiceExt/idl_service_ext_stub&";
const ERR_OK = 0;
const TAG: string = "[IdlServiceExtImpl]";
// 开发者需要在这个类型里对接口进行实现
export default class ServiceExtImpl extends IdlServiceExtStub {
    processData(data: number, callback: processDataCallback): void {
        // 开发者自行实现业务逻辑
        console.info(TAG, `processData: ${data}`);
        callback(ERR_OK, data + 1);
    }
    insertDataToMap(key: string, val: number, callback: insertDataToMapCallback): void {
        // 开发者自行实现业务逻辑
        console.info(TAG, `insertDataToMap, key: ${key}  val: ${val}`);
        callback(ERR_OK);
    }
}
