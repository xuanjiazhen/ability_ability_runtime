import type { BusinessError } from "@ohos:base";
import type common from "@ohos:app.ability.common";
const TAG = 'SelectorExtensionAbility.AbilityInfoUtils';
export namespace AbilityInfoUtils {
    interface m23 {
        bundleName: string;
        moduleName: string;
        abilityName?: string;
        abilityIconId: number;
        abilityLabelId: number;
    }
    export function getAbilityLabelAndIcon(v23: common.UIExtensionContext, w23: m23, x23: (data: Object[] | null) => void) {
        let y23: Context | undefined = v23?.createModuleContext(w23.bundleName, w23.moduleName);
        if (!y23) {
            throw { code: 1000, data: 'Get module context error.' } as BusinessError<string> as Error;
        }
        const z23 = y23.resourceManager;
        Promise.all<Object>([z23.getDrawableDescriptor(w23.abilityIconId),
            z23.getStringValue(w23.abilityLabelId)]).then((a24: Object[]) => {
            x23(a24);
        });
    }
    export function getAbilityLabelAndIconSync(n23: common.UIExtensionContext, o23: m23, p23: (data: Object[] | null) => void) {
        let q23: Context | undefined = n23.createModuleContext(o23.bundleName, o23.moduleName);
        if (!q23) {
            throw { code: 1000, data: 'Get module context error.' } as BusinessError<string> as Error;
        }
        const r23 = q23.resourceManager;
        try {
            let t23 = r23.getDrawableDescriptor(o23.abilityIconId);
            let u23 = r23.getStringSync(o23.abilityLabelId);
            p23([t23, u23]);
        }
        catch (s23) {
            console.error(TAG, `Failed to getAbilityLabelAndIconSync, error: ${JSON.stringify(s23)}`);
            p23(null);
        }
    }
}
