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
    constructor(w1, x1, y1, z1 = -1, a2 = undefined, b2) {
        super(w1, y1, z1, b2);
        if (typeof a2 === "function") {
            this.paramsGenerator_ = a2;
        }
        this.__articlePathStack = new ObservedPropertyObjectPU(new NavPathStack(), this, "articlePathStack");
        this.addProvidedVar("articlePathStack", this.__articlePathStack, false);
        this.scroller = new Scroller();
        this.setInitiallyProvidedValue(x1);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(v1: Index_Params) {
        if (v1.articlePathStack !== undefined) {
            this.articlePathStack = v1.articlePathStack;
        }
        if (v1.scroller !== undefined) {
            this.scroller = v1.scroller;
        }
    }
    updateStateVars(u1: Index_Params) {
    }
    purgeVariableDependenciesOnElmtId(t1) {
        this.__articlePathStack.purgeDependencyOnElmtId(t1);
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
    set articlePathStack(s1: NavPathStack) {
        this.__articlePathStack.set(s1);
    }
    private scroller: Scroller;
    onBackPress() {
        try {
            let r1 = this.getUIContext().getHostContext() as common.UIAbilityContext;
            if (r1) {
                r1.terminateSelf();
            }
        }
        catch (q1) {
            hilog.error(DOMAIN, TAG, 'Failed to context.terminateSelf. Cause: %{public}s', JSON.stringify(q1));
        }
    }
    intentRouter(d1: string, e1?: IntentInfo, f1 = null) {
        this.observeComponentCreation((g1, h1) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g1);
            If.create();
            if (d1 === 'intentDetail') {
                this.ifElseBranchUpdateFunction(0, () => {
                    {
                        this.observeComponentCreation((m1, n1) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(m1);
                            if (n1) {
                                let o1 = new IntentDetailPage(this, {}, undefined, m1, () => { }, { page: "feature/src/main/ets/pages/Index.ets", line: 43, col: 7 });
                                ViewPU.create(o1);
                                let p1 = () => {
                                    return {};
                                };
                                o1.paramsGenerator_ = p1;
                            }
                            else {
                                this.updateStateVarsOfChildByElmtId(m1, {});
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                    }
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                    {
                        this.observeComponentCreation((i1, j1) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(i1);
                            if (j1) {
                                let k1 = new IntentContent(this, {}, undefined, i1, () => { }, { page: "feature/src/main/ets/pages/Index.ets", line: 45, col: 7 });
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
            if (!h1) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
    }
    initialRender() {
        this.observeComponentCreation((b1, c1) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b1);
            Navigation.create(this.articlePathStack, { moduleName: "default", pagePath: "feature/src/main/ets/pages/Index", isUserCreateStack: true });
            Navigation.navDestination({ builder: this.intentRouter.bind(this) });
            Navigation.hideTitleBar(true);
            Navigation.mode(NavigationMode.Stack);
            if (!c1) {
                Navigation.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        {
            this.observeComponentCreation((x, y) => {
                ViewStackProcessor.StartGetAccessRecordingFor(x);
                if (y) {
                    let z = new IntentContent(this, {}, undefined, x, () => { }, { page: "feature/src/main/ets/pages/Index.ets", line: 51, col: 7 });
                    ViewPU.create(z);
                    let a1 = () => {
                        return {};
                    };
                    z.paramsGenerator_ = a1;
                }
                else {
                    this.updateStateVarsOfChildByElmtId(x, {});
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
