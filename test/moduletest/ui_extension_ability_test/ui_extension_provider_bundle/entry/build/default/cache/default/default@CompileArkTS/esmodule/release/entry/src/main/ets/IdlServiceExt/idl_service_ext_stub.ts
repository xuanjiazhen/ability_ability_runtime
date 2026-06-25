import type { processDataCallback } from "./i_idl_service_ext";
import type { insertDataToMapCallback } from "./i_idl_service_ext";
import type IIdlServiceExt from "./i_idl_service_ext";
import rpc from "@ohos:rpc";
export default class IdlServiceExtStub extends rpc.RemoteObject implements IIdlServiceExt {
    constructor(des: string) {
        super(des);
    }
    async onRemoteMessageRequest(code: number, data: rpc.MessageSequence, reply: rpc.MessageSequence, option: rpc.MessageOption): Promise<boolean> {
        console.log("onRemoteMessageRequest called, code = " + code);
        switch (code) {
            case IdlServiceExtStub.COMMAND_PROCESS_DATA: {
                let _data = data.readInt();
                this.processData(_data, (errCode, returnValue) => {
                    reply.writeInt(errCode);
                    if (errCode == 0) {
                        reply.writeInt(returnValue);
                    }
                });
                return true;
            }
            case IdlServiceExtStub.COMMAND_INSERT_DATA_TO_MAP: {
                let _key = data.readString();
                let _val = data.readInt();
                this.insertDataToMap(_key, _val, (errCode) => {
                    reply.writeInt(errCode);
                });
                return true;
            }
            default: {
                console.log("invalid request code" + code);
                break;
            }
        }
        return false;
    }
    processData(data: number, callback: processDataCallback): void {
    }
    insertDataToMap(key: string, val: number, callback: insertDataToMapCallback): void {
    }
    static readonly COMMAND_PROCESS_DATA = 1;
    static readonly COMMAND_INSERT_DATA_TO_MAP = 2;
}
