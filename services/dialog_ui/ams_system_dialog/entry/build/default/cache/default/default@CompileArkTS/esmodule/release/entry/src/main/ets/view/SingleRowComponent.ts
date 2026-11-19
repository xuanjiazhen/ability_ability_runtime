if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface SingleRowComponent_Params {
    targetInfoArray?: Array<TargetInfo>;
    storage?: LocalStorage;
    singleRowHeight?: number;
}
import type { TargetInfo } from '../data/Model';
import { TargetComponent } from "@bundle:com.ohos.amsdialog/entry/ets/view/TargetComponent";
const TAG = 'SelectorExtensionAbility.SingleRowComponent';
export class SingleRowComponent extends ViewPU {
    constructor(o28, p28, q28, r28 = -1, s28 = undefined, t28) {
        super(o28, q28, r28, t28);
        if (typeof s28 === "function") {
            this.paramsGenerator_ = s28;
        }
        this.targetInfoArray = [];
        this.storage = undefined;
        this.__singleRowHeight = this.initializeConsume("singleRowHeight", "singleRowHeight");
        this.setInitiallyProvidedValue(p28);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(n28: SingleRowComponent_Params) {
        if (n28.targetInfoArray !== undefined) {
            this.targetInfoArray = n28.targetInfoArray;
        }
        if (n28.storage !== undefined) {
            this.storage = n28.storage;
        }
    }
    updateStateVars(m28: SingleRowComponent_Params) {
    }
    purgeVariableDependenciesOnElmtId(l28) {
        this.__singleRowHeight.purgeDependencyOnElmtId(l28);
    }
    aboutToBeDeleted() {
        this.__singleRowHeight.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private targetInfoArray: Array<TargetInfo>;
    private storage?: LocalStorage;
    private __singleRowHeight: ObservedPropertyAbstractPU<number>;
    get singleRowHeight() {
        return this.__singleRowHeight.get();
    }
    set singleRowHeight(k28: number) {
        this.__singleRowHeight.set(k28);
    }
    initialRender() {
        this.observeComponentCreation((g28, h28) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g28);
            Row.create();
            Row.width('100%');
            Row.alignItems(VerticalAlign.Top);
            Row.justifyContent(FlexAlign.SpaceBetween);
            Row.onAreaChange((i28: Area, j28: Area) => {
                if (i28.height !== j28.height) {
                    this.singleRowHeight = Number(j28.height);
                }
            });
            if (!h28) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((u27, v27) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u27);
            ForEach.create();
            const w27 = (x27, y27?: number) => {
                const z27 = x27;
                this.observeComponentCreation((e28, f28) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(e28);
                    __Common__.create();
                    __Common__.visibility(z27.visible ? Visibility.Visible : Visibility.Hidden);
                    if (!f28) {
                        __Common__.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                {
                    this.observeComponentCreation((a28, b28) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(a28);
                        if (b28) {
                            let c28 = new TargetComponent(this, { targetInfo: z27, storage: this.storage }, undefined, a28, () => { }, { page: "entry/src/main/ets/view/SingleRowComponent.ets", line: 32, col: 17 });
                            ViewPU.create(c28);
                            let d28 = () => {
                                return {
                                    targetInfo: z27,
                                    storage: this.storage
                                };
                            };
                            c28.paramsGenerator_ = d28;
                        }
                        else {
                            this.updateStateVarsOfChildByElmtId(a28, {});
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                }
                __Common__.pop();
            };
            this.forEachUpdateFunction(u27, this.targetInfoArray, w27, undefined, true, false);
            if (!v27) {
                ForEach.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        ForEach.pop();
        Row.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
    public __resetStateVarsOnReuse__Internal(t27: Object): void {
        this.__reInitializeConsume__Internal("singleRowHeight", "singleRowHeight");
    }
}
