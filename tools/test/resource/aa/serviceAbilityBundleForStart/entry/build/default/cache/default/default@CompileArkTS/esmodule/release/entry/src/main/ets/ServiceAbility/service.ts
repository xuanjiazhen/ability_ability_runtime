import type Want from "@ohos:app.ability.Want";
import rpc from "@ohos:rpc";
import hilog from "@ohos:hilog";
const TAG: string = '[ServiceAbility_Mode]';
const domain: number = 0xFF00;
class FirstServiceAbilityStub extends rpc.RemoteObject {
    constructor(des: Object) {
        if (typeof des === 'string') {
            super(des);
        }
        else {
            return;
        }
    }
    onRemoteMessageRequest(code: number, data: rpc.MessageSequence, reply: rpc.MessageSequence, option: rpc.MessageOption): boolean {
        hilog.info(domain, TAG, 'ServiceAbility onRemoteRequest called');
        if (code === 1) {
            let string = data.readString();
            hilog.info(domain, TAG, `ServiceAbility string=${string}`);
            let result = Array.from(string).sort().join('');
            hilog.info(domain, TAG, `ServiceAbility result=${result}`);
            reply.writeString(result);
        }
        else {
            hilog.info(domain, TAG, 'ServiceAbility unknown request code');
        }
        return true;
    }
}
class ServiceAbility {
    onStart(): void {
        hilog.info(domain, TAG, 'ServiceAbility onStart');
    }
    onStop(): void {
        hilog.info(domain, TAG, 'ServiceAbility onStop');
    }
    onCommand(want: Want, startId: number): void {
        hilog.info(domain, TAG, 'ServiceAbility onCommand');
    }
    onConnect(want: Want): rpc.RemoteObject {
        hilog.info(domain, TAG, 'ServiceAbility onDisconnect' + want);
        return new FirstServiceAbilityStub('test');
    }
    onDisconnect(want: Want): void {
        hilog.info(domain, TAG, 'ServiceAbility onDisconnect' + want);
    }
}
export default new ServiceAbility();
