if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface Extension_Params {
    message?: string;
    session?: UIExtensionContentSession | undefined;
}
import type UIExtensionContentSession from "@ohos:app.ability.UIExtensionContentSession";
let storage = LocalStorage.getShared();
const TAG: string = '[testTag] ExtensionPage';
class Extension extends ViewPU {
    constructor(parent, params, __localStorage, elmtId = -1, paramsLambda = undefined, extraInfo) {
        super(parent, __localStorage, elmtId, extraInfo);
        if (typeof paramsLambda === "function") {
            this.paramsGenerator_ = paramsLambda;
        }
        this.__message = new ObservedPropertySimplePU('UIExtension Provider', this, "message");
        this.session = storage.get<UIExtensionContentSession>('session');
        this.setInitiallyProvidedValue(params);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(params: Extension_Params) {
        if (params.message !== undefined) {
            this.message = params.message;
        }
        if (params.session !== undefined) {
            this.session = params.session;
        }
    }
    updateStateVars(params: Extension_Params) {
    }
    purgeVariableDependenciesOnElmtId(rmElmtId) {
        this.__message.purgeDependencyOnElmtId(rmElmtId);
    }
    aboutToBeDeleted() {
        this.__message.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __message: ObservedPropertySimplePU<string>;
    get message() {
        return this.__message.get();
    }
    set message(newValue: string) {
        this.__message.set(newValue);
    }
    private session: UIExtensionContentSession | undefined;
    onPageShow() {
        console.info(TAG, 'show');
        // if u wanna use setReceiveDataCallback, you should be a system app.
        // this.session?.setReceiveDataCallback((data) => {
        //   this.message = JSON.stringify(data);
        // })
    }
    initialRender() {
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Row.create();
            Row.height('100%');
        }, Row);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create();
        }, Column);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Text.create(this.message);
            Text.fontSize(20);
            Text.fontWeight(FontWeight.Bold);
            Text.textAlign(TextAlign.Center);
        }, Text);
        Text.pop();
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Button.createWithLabel("send data");
            Button.width('80%');
            Button.type(ButtonType.Capsule);
            Button.margin({
                top: 20
            });
            Button.onClick(() => {
                this.session?.sendData({ "data": 543321 });
            });
        }, Button);
        Button.pop();
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Button.createWithLabel("terminate self");
            Button.width('80%');
            Button.type(ButtonType.Capsule);
            Button.margin({
                top: 20
            });
            Button.onClick(() => {
                this.session?.terminateSelf();
                storage.clear();
            });
        }, Button);
        Button.pop();
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Button.createWithLabel("terminate self with result");
            Button.width('80%');
            Button.type(ButtonType.Capsule);
            Button.margin({
                top: 20
            });
            Button.onClick(() => {
                this.session?.terminateSelfWithResult({
                    resultCode: 0,
                    want: {
                        bundleName: "com.ohos.uiextensionprovider",
                        parameters: { "result": 123456 }
                    }
                });
            });
        }, Button);
        Button.pop();
        Column.pop();
        Row.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
    static getEntryName(): string {
        return "Extension";
    }
}
if (storage && storage.routeName != undefined && storage.storage != undefined) {
    registerNamedRoute(() => new Extension(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : storage.storage), storage.routeName, { bundleName: "com.ohos.uiextensionprovider", moduleName: "entry", pagePath: "pages/Extension", pageFullPath: "entry/src/main/ets/pages/Extension", integratedHsp: "false", moduleType: "followWithHap" });
}
else if (storage && storage.routeName != undefined && storage.storage == undefined) {
    registerNamedRoute(() => new Extension(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : storage.storage), storage.routeName, { bundleName: "com.ohos.uiextensionprovider", moduleName: "entry", pagePath: "pages/Extension", pageFullPath: "entry/src/main/ets/pages/Extension", integratedHsp: "false", moduleType: "followWithHap" });
}
else if (storage && storage.routeName == undefined && storage.storage != undefined) {
    registerNamedRoute(() => new Extension(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : storage.storage), "", { bundleName: "com.ohos.uiextensionprovider", moduleName: "entry", pagePath: "pages/Extension", pageFullPath: "entry/src/main/ets/pages/Extension", integratedHsp: "false", moduleType: "followWithHap" });
}
else if (storage && storage.useSharedStorage != undefined) {
    registerNamedRoute(() => new Extension(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : undefined), "", { bundleName: "com.ohos.uiextensionprovider", moduleName: "entry", pagePath: "pages/Extension", pageFullPath: "entry/src/main/ets/pages/Extension", integratedHsp: "false", moduleType: "followWithHap" });
}
else {
    registerNamedRoute(() => new Extension(undefined, {}, storage), "", { bundleName: "com.ohos.uiextensionprovider", moduleName: "entry", pagePath: "pages/Extension", pageFullPath: "entry/src/main/ets/pages/Extension", integratedHsp: "false", moduleType: "followWithHap" });
}
