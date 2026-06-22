if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface Index_Params {
    message?: string;
    myProxy?: UIExtensionProxy | undefined;
}
class Index extends ViewPU {
    constructor(parent, params, __localStorage, elmtId = -1, paramsLambda = undefined, extraInfo) {
        super(parent, __localStorage, elmtId, extraInfo);
        if (typeof paramsLambda === "function") {
            this.paramsGenerator_ = paramsLambda;
        }
        this.__message = new ObservedPropertySimplePU('UIExtension User', this, "message");
        this.myProxy = undefined;
        this.setInitiallyProvidedValue(params);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(params: Index_Params) {
        if (params.message !== undefined) {
            this.message = params.message;
        }
        if (params.myProxy !== undefined) {
            this.myProxy = params.myProxy;
        }
    }
    updateStateVars(params: Index_Params) {
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
    private myProxy: UIExtensionProxy | undefined;
    initialRender() {
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Row.create();
            Row.height('100%');
        }, Row);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create();
            Column.width('100%');
        }, Column);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Text.create(this.message);
            Text.fontSize(30);
            Text.size({ width: '100%', height: '50' });
            Text.fontWeight(FontWeight.Bold);
            Text.textAlign(TextAlign.Center);
        }, Text);
        Text.pop();
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            UIExtensionComponent.create(globalThis.uiExtensionComponentWant);
            UIExtensionComponent.onRemoteReady((proxy) => {
                this.myProxy = proxy;
            });
            UIExtensionComponent.onReceive((data) => {
                this.message = JSON.stringify(data);
            });
            UIExtensionComponent.onResult((data) => {
                this.message = JSON.stringify(data);
            });
            UIExtensionComponent.onRelease((code) => {
                this.message = "release code: " + code;
            });
            UIExtensionComponent.offset({ x: 0, y: 30 });
            UIExtensionComponent.size({ width: 300, height: 300 });
            UIExtensionComponent.border({
                width: 5,
                color: 0x317AF7,
                radius: 10,
                style: BorderStyle.Dotted
            });
        }, UIExtensionComponent);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Button.createWithLabel("sendData");
            Button.type(ButtonType.Capsule);
            Button.offset({ x: 0, y: 60 });
            Button.width('80%');
            Button.type(ButtonType.Capsule);
            Button.margin({
                top: 20
            });
            Button.onClick(() => {
                this.myProxy?.send({
                    "data": 123456,
                    "message": "data from component"
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
        return "Index";
    }
}
registerNamedRoute(() => new Index(undefined, {}), "", { bundleName: "com.ohos.uiextensionuser", moduleName: "entry", pagePath: "pages/Index", pageFullPath: "entry/src/main/ets/pages/Index", integratedHsp: "false", moduleType: "followWithHap" });
