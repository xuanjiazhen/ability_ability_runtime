if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface Index_Params {
    articlePathStack?: NavPathStack;
    scroller?: Scroller;
}
import { IntentContent } from "@bundle:com.ohos.amsdialog/default/ets/view/IntentContent";
import type { IntentInfo } from "@bundle:com.ohos.amsdialog/default/ets/view/IntentContent";
import { IntentDetailPage } from "@bundle:com.ohos.amsdialog/default/ets/view/IntentDetailPage";
import type common from "@ohos:app.ability.common";
import hilog from "@ohos:hilog";
export const DOMAIN = 0x1323;
export const TAG = 'InsightIntents';
class Index extends ViewPU {
    constructor(s1, t1, u1, v1 = -1, w1 = undefined, x1) {
        super(s1, u1, v1, x1);
        if (typeof w1 === "function") {
            this.paramsGenerator_ = w1;
        }
        this.__articlePathStack = new ObservedPropertyObjectPU(new NavPathStack(), this, "articlePathStack");
        this.addProvidedVar("articlePathStack", this.__articlePathStack, false);
        this.scroller = new Scroller();
        this.setInitiallyProvidedValue(t1);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(r1: Index_Params) {
        if (r1.articlePathStack !== undefined) {
            this.articlePathStack = r1.articlePathStack;
        }
        if (r1.scroller !== undefined) {
            this.scroller = r1.scroller;
        }
    }
    updateStateVars(q1: Index_Params) {
    }
    purgeVariableDependenciesOnElmtId(p1) {
        this.__articlePathStack.purgeDependencyOnElmtId(p1);
    }
    aboutToBeDeleted() {
        this.__articlePathStack.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __articlePathStack: ObservedPropertyObjectPU<NavPathStack>;
    get articlePathStack() {
        return this.__articlePathStack.get();
    }
    set articlePathStack(o1: NavPathStack) {
        this.__articlePathStack.set(o1);
    }
    private scroller: Scroller;
    onBackPress() {
        try {
            let n1 = this.getUIContext().getHostContext() as common.UIAbilityContext;
            n1.terminateSelf();
        }
        catch (m1) {
            hilog.error(DOMAIN, TAG, 'Failed to context.terminateSelf. Cause: %{public}s', JSON.stringify(m1));
        }
    }
    intentRouter(z: string, a1?: IntentInfo, b1 = null) {
        this.observeComponentCreation((c1, d1) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c1);
            If.create();
            if (z === 'intentDetail') {
                this.ifElseBranchUpdateFunction(0, () => {
                    {
                        this.observeComponentCreation((i1, j1) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(i1);
                            if (j1) {
                                let k1 = new IntentDetailPage(this, {}, undefined, i1, () => { }, { page: "feature/src/main/ets/pages/Index.ets", line: 41, col: 7 });
                                ViewPU.create(k1);
                                let l1 = () => {
                                    return {};
                                };
                                k1.paramsGenerator_ = l1;
                            }
                            else {
                                this.updateStateVarsOfChildByElmtId(i1, {});
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                    }
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                    {
                        this.observeComponentCreation((e1, f1) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(e1);
                            if (f1) {
                                let g1 = new IntentContent(this, {}, undefined, e1, () => { }, { page: "feature/src/main/ets/pages/Index.ets", line: 43, col: 7 });
                                ViewPU.create(g1);
                                let h1 = () => {
                                    return {};
                                };
                                g1.paramsGenerator_ = h1;
                            }
                            else {
                                this.updateStateVarsOfChildByElmtId(e1, {});
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                    }
                });
            }
            if (!d1) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
    }
    initialRender() {
        this.observeComponentCreation((x, y) => {
            ViewStackProcessor.StartGetAccessRecordingFor(x);
            Navigation.create(this.articlePathStack, { moduleName: "default", pagePath: "feature/src/main/ets/pages/Index", isUserCreateStack: true });
            Navigation.navDestination({ builder: this.intentRouter.bind(this) });
            Navigation.hideTitleBar(true);
            Navigation.mode(NavigationMode.Stack);
            if (!y) {
                Navigation.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        {
            this.observeComponentCreation((t, u) => {
                ViewStackProcessor.StartGetAccessRecordingFor(t);
                if (u) {
                    let v = new IntentContent(this, {}, undefined, t, () => { }, { page: "feature/src/main/ets/pages/Index.ets", line: 49, col: 7 });
                    ViewPU.create(v);
                    let w = () => {
                        return {};
                    };
                    v.paramsGenerator_ = w;
                }
                else {
                    this.updateStateVarsOfChildByElmtId(t, {});
                }
                ViewStackProcessor.StopGetAccessRecording();
            });
        }
        Navigation.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
}
ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new Index(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
