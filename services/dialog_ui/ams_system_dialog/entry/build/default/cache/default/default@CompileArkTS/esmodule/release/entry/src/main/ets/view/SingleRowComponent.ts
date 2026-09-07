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
    constructor(e28, f28, g28, h28 = -1, i28 = undefined, j28) {
        super(e28, g28, h28, j28);
        if (typeof i28 === "function") {
            this.paramsGenerator_ = i28;
        }
        this.targetInfoArray = [];
        this.storage = undefined;
        this.__singleRowHeight = this.initializeConsume("singleRowHeight", "singleRowHeight");
        this.setInitiallyProvidedValue(f28);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(d28: SingleRowComponent_Params) {
        if (d28.targetInfoArray !== undefined) {
            this.targetInfoArray = d28.targetInfoArray;
        }
        if (d28.storage !== undefined) {
            this.storage = d28.storage;
        }
    }
    updateStateVars(c28: SingleRowComponent_Params) {
    }
    purgeVariableDependenciesOnElmtId(b28) {
        this.__singleRowHeight.purgeDependencyOnElmtId(b28);
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
    set singleRowHeight(a28: number) {
        this.__singleRowHeight.set(a28);
    }
    initialRender() {
        this.observeComponentCreation((w27, x27) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w27);
            Row.create();
            Row.width('100%');
            Row.alignItems(VerticalAlign.Top);
            Row.justifyContent(FlexAlign.SpaceBetween);
            Row.onAreaChange((y27: Area, z27: Area) => {
                if (y27.height !== z27.height) {
                    this.singleRowHeight = Number(z27.height);
                }
            });
            if (!x27) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((k27, l27) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k27);
            ForEach.create();
            const m27 = (n27, o27?: number) => {
                const p27 = n27;
                this.observeComponentCreation((u27, v27) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(u27);
                    __Common__.create();
                    __Common__.visibility(p27.visible ? Visibility.Visible : Visibility.Hidden);
                    if (!v27) {
                        __Common__.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                {
                    this.observeComponentCreation((q27, r27) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(q27);
                        if (r27) {
                            let s27 = new TargetComponent(this, { targetInfo: p27, storage: this.storage }, undefined, q27, () => { }, { page: "entry/src/main/ets/view/SingleRowComponent.ets", line: 32, col: 17 });
                            ViewPU.create(s27);
                            let t27 = () => {
                                return {
                                    targetInfo: p27,
                                    storage: this.storage
                                };
                            };
                            s27.paramsGenerator_ = t27;
                        }
                        else {
                            this.updateStateVarsOfChildByElmtId(q27, {});
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                }
                __Common__.pop();
            };
            this.forEachUpdateFunction(k27, this.targetInfoArray, m27, undefined, true, false);
            if (!l27) {
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
}
