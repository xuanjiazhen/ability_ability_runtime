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
    constructor(x1, y1, z1, a2 = -1, b2 = undefined, c2) {
        super(x1, z1, a2, c2);
        if (typeof b2 === "function") {
            this.paramsGenerator_ = b2;
        }
        this.__articlePathStack = new ObservedPropertyObjectPU(new NavPathStack(), this, "articlePathStack");
        this.addProvidedVar("articlePathStack", this.__articlePathStack, false);
        this.scroller = new Scroller();
        this.setInitiallyProvidedValue(y1);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(w1: Index_Params) {
        if (w1.articlePathStack !== undefined) {
            this.articlePathStack = w1.articlePathStack;
        }
        if (w1.scroller !== undefined) {
            this.scroller = w1.scroller;
        }
    }
    updateStateVars(v1: Index_Params) {
    }
    purgeVariableDependenciesOnElmtId(u1) {
        this.__articlePathStack.purgeDependencyOnElmtId(u1);
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
    set articlePathStack(t1: NavPathStack) {
        this.__articlePathStack.set(t1);
    }
    private scroller: Scroller;
    onBackPress() {
        try {
            let s1 = this.getUIContext().getHostContext() as common.UIAbilityContext;
            if (s1) {
                s1.terminateSelf();
            }
        }
        catch (r1) {
            hilog.error(DOMAIN, TAG, 'Failed to context.terminateSelf. Cause: %{public}s', JSON.stringify(r1));
        }
    }
    intentRouter(e1: string, f1?: IntentInfo, g1 = null) {
        this.observeComponentCreation((h1, i1) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h1);
            If.create();
            if (e1 === 'intentDetail') {
                this.ifElseBranchUpdateFunction(0, () => {
                    {
                        this.observeComponentCreation((n1, o1) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(n1);
                            if (o1) {
                                let p1 = new IntentDetailPage(this, {}, undefined, n1, () => { }, { page: "feature/src/main/ets/pages/Index.ets", line: 43, col: 7 });
                                ViewPU.create(p1);
                                let q1 = () => {
                                    return {};
                                };
                                p1.paramsGenerator_ = q1;
                            }
                            else {
                                this.updateStateVarsOfChildByElmtId(n1, {});
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                    }
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                    {
                        this.observeComponentCreation((j1, k1) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(j1);
                            if (k1) {
                                let l1 = new IntentContent(this, {}, undefined, j1, () => { }, { page: "feature/src/main/ets/pages/Index.ets", line: 45, col: 7 });
                                ViewPU.create(l1);
                                let m1 = () => {
                                    return {};
                                };
                                l1.paramsGenerator_ = m1;
                            }
                            else {
                                this.updateStateVarsOfChildByElmtId(j1, {});
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                    }
                });
            }
            if (!i1) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
    }
    initialRender() {
        this.observeComponentCreation((c1, d1) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c1);
            Navigation.create(this.articlePathStack, { moduleName: "default", pagePath: "feature/src/main/ets/pages/Index", isUserCreateStack: true });
            Navigation.navDestination({ builder: this.intentRouter.bind(this) });
            Navigation.hideTitleBar(true);
            Navigation.mode(NavigationMode.Stack);
            if (!d1) {
                Navigation.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        {
            this.observeComponentCreation((y, z) => {
                ViewStackProcessor.StartGetAccessRecordingFor(y);
                if (z) {
                    let a1 = new IntentContent(this, {}, undefined, y, () => { }, { page: "feature/src/main/ets/pages/Index.ets", line: 51, col: 7 });
                    ViewPU.create(a1);
                    let b1 = () => {
                        return {};
                    };
                    a1.paramsGenerator_ = b1;
                }
                else {
                    this.updateStateVarsOfChildByElmtId(y, {});
                }
                ViewStackProcessor.StopGetAccessRecording();
            });
        }
        Navigation.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
    public __resetStateVarsOnReuse__Internal(x: Object): void {
    }
}
ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new Index(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
