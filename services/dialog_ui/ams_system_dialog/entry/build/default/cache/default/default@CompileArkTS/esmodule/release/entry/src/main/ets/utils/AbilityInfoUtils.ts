import type { BusinessError } from "@ohos:base";
import type common from "@ohos:app.ability.common";
const TAG = 'SelectorExtensionAbility.AbilityInfoUtils';
export namespace AbilityInfoUtils {
    interface e23 {
        bundleName: string;
        moduleName: string;
        abilityName?: string;
        abilityIconId: number;
        abilityLabelId: number;
    }
    export function getAbilityLabelAndIcon(n23: common.UIExtensionContext, o23: e23, p23: (data: Object[] | null) => void) {
        let q23: Context | undefined = n23?.createModuleContext(o23.bundleName, o23.moduleName);
        if (!q23) {
            throw { code: 1000, data: 'Get module context error.' } as BusinessError<string> as Error;
        }
        const r23 = q23.resourceManager;
        Promise.all<Object>([r23.getDrawableDescriptor(o23.abilityIconId),
            r23.getStringValue(o23.abilityLabelId)]).then((s23: Object[]) => {
            p23(s23);
        });
    }
    export function getAbilityLabelAndIconSync(f23: common.UIExtensionContext, g23: e23, h23: (data: Object[] | null) => void) {
        let i23: Context | undefined = f23.createModuleContext(g23.bundleName, g23.moduleName);
        if (!i23) {
            throw { code: 1000, data: 'Get module context error.' } as BusinessError<string> as Error;
        }
        const j23 = i23.resourceManager;
        try {
            let l23 = j23.getDrawableDescriptor(g23.abilityIconId);
            let m23 = j23.getStringSync(g23.abilityLabelId);
            h23([l23, m23]);
        }
        catch (k23) {
            console.error(TAG, `Failed to getAbilityLabelAndIconSync, error: ${JSON.stringify(k23)}`);
            h23(null);
        }
    }
}
