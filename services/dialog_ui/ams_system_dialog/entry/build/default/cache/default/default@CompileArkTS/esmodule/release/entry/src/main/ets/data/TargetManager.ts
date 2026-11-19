import HashMap from "@ohos:util.HashMap";
import type Want from "@ohos:app.ability.Want";
import { TargetAction, TargetInfo, TargetType, UIExtensionInfo } from "@bundle:com.ohos.amsdialog/entry/ets/data/Model";
import bundleManager from "@ohos:bundle.bundleManager";
const TAG = 'SelectorExtensionAbility.TargetManager';
export namespace TargetManager {
    interface o {
        bundleName: string;
        abilityName: string;
        uri?: string;
        targetType: number;
    }
    const p: HashMap<Want, Array<TargetInfo>> = new HashMap();
    export function getAvailableTargets(w: Want): Array<TargetInfo> {
        if (p.hasKey(w)) {
            console.info(TAG, `Get targets from cache.`);
            return p.get(w);
        }
        let x: bundleManager.ExtensionAbilityInfo[] = [];
        if (!(w?.parameters?.['ability.want.params.uiExtensionTargetType'])) {
            return [];
        }
        try {
            x =
                bundleManager.queryExtensionAbilityInfoSync(w.parameters['ability.want.params.uiExtensionTargetType'] as string, bundleManager.ExtensionAbilityFlag.GET_EXTENSION_ABILITY_INFO_DEFAULT);
        }
        catch (z) {
            console.info(TAG, `there are no target uiextension: ${z}`);
        }
        return x.map(y => {
            return new TargetInfo(y, n(w, {
                bundleName: y.bundleName,
                abilityName: y.name,
                targetType: TargetType.UI_EXTENSION,
                uri: ''
            }));
        });
    }
    function n(t: Want, u: o): TargetAction {
        if (t.parameters) {
            t.parameters['ability.want.params.uiExtensionType'] = t.parameters['ability.want.params.uiExtensionTargetType'];
            t.parameters['ability.want.params.uiExtensionTargetType'] = '';
        }
        return new TargetAction(TargetType.UI_EXTENSION, (v?: LocalStorage) => {
            return new UIExtensionInfo(u.bundleName, u.abilityName, t.parameters);
        });
    }
    export function removePartialTargetListIfExceedsMax(q: Array<TargetInfo>, r: number): Array<TargetInfo> {
        if (q.length <= r) {
            return q;
        }
        const s: TargetInfo[] = q.slice(0, r - 1);
        s.push(new TargetInfo(null, new TargetAction(TargetType.SHOW_ALL_TARGET, null)));
        return s;
    }
}
