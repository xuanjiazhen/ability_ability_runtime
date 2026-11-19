import deviceInfo from "@ohos:deviceInfo";
import display from "@ohos:display";
const TAG = '[PositionUtils]';
const UI_SELECTOR_DIALOG_WIDTH = 656;
const UI_SELECTOR_DIALOG_HEIGHT = 700;
const UI_SELECTOR_DIALOG_PC_H0 = 1;
const UI_SELECTOR_DIALOG_PC_H2 = 800;
const UI_SELECTOR_DIALOG_PC_H3 = 928;
const UI_SELECTOR_DIALOG_PC_H4 = 1056;
const UI_SELECTOR_DIALOG_PC_H5 = 1172;
const UI_SELECTOR_DIALOG_PHONE_H1 = 280;
const UI_SELECTOR_DIALOG_PHONE_H2 = 400;
const UI_SELECTOR_DIALOG_PHONE_H3 = 410;
const UI_SELECTOR_LANDSCAPE_SIGNAL_BAR = 24;
const UI_SELECTOR_LANDSCAPE_HEIGHT = 350;
const UI_SELECTOR_LANDSCAPE_COUNT_THREE = 3;
const UI_SELECTOR_LANDSCAPE_COUNT_FOUR = 4;
const UI_SELECTOR_LANDSCAPE_GRILLE_LARGE = 0.107692;
const UI_SELECTOR_LANDSCAPE_GRILLE_SAMLL = 0.015385;
const UI_SELECTOR_LANDSCAPE_MAX_RATIO = 0.9;
const UI_SELECTOR_PORTRAIT_WIDTH_RATIO = 0.8;
const UI_SELECTOR_PORTRAIT_WIDTH_EDGE_RATIO = 0.1;
const UI_SELECTOR_PORTRAIT_HEIGHT_RATIO = 0.98;
const UI_TIPS_DIALOG_WIDTH = 656;
const UI_TIPS_DIALOG_HEIGHT = 270;
const UI_HALF = 2;
const UI_DEFAULT_BUTTOM_CLIP = 100;
const UI_DEFAULT_WIDTH = 2560;
const UI_DEFAULT_HEIGHT = 1600;
const STR_PHONE = 'phone';
const STR_DEFAULT = 'default';
const LINE_NUMS_ZERO = 0;
const LINE_NUMS_TWO = 2;
const LINE_NUMS_THREE = 3;
const LINE_NUMS_FOUR = 4;
const LINE_NUMS_EIGHT = 8;
const WIDTH_MULTIPLE = 0.8;
const HEIGHT_MULTIPLE = 0.3;
const SETX_WIDTH_MULTIPLE = 0.1;
export interface Position {
    width: number;
    height: number;
    offsetX: number;
    offsetY: number;
    oversizeHeight: boolean;
}
export default class PositionUtils {
    public static getTipsDialogPosition(): Position {
        let k25 = {
            width: UI_TIPS_DIALOG_WIDTH,
            height: UI_TIPS_DIALOG_HEIGHT,
            offsetX: 0,
            offsetY: 0,
            oversizeHeight: false
        };
        let l25: display.Display | null = this.getDefaultDisplay();
        if (l25) {
            let m25 = (deviceInfo.deviceType === STR_PHONE) || (deviceInfo.deviceType === STR_DEFAULT);
            if (m25) {
                k25.width = l25.width * WIDTH_MULTIPLE;
                k25.height = l25.height * HEIGHT_MULTIPLE;
                k25.offsetX = l25.width * SETX_WIDTH_MULTIPLE;
                k25.offsetY = l25.height - k25.height - UI_DEFAULT_BUTTOM_CLIP;
            }
            else {
                k25.offsetX = (l25.width - k25.width) / UI_HALF;
                k25.offsetY = (l25.height - k25.height) / UI_HALF;
            }
        }
        else {
            k25.offsetX = (UI_DEFAULT_WIDTH - k25.width) / UI_HALF;
            k25.offsetY = UI_DEFAULT_HEIGHT - k25.height - UI_DEFAULT_BUTTOM_CLIP;
        }
        return k25;
    }
    public static getSelectorDialogPosition(i25): Position {
        let j25 = {
            width: 0,
            height: 0,
            offsetX: 0,
            offsetY: 0,
            oversizeHeight: false
        };
        if (deviceInfo.deviceType === STR_PHONE || deviceInfo.deviceType === STR_DEFAULT) {
            this.getPhoneSelectorDialogPosition(j25, i25);
        }
        else {
            this.getPcSelectorDialogPosition(j25, i25);
        }
        return j25;
    }
    private static getPhoneSelectorDialogPosition(c25, d25): void {
        let e25: display.Display | null = this.getDefaultDisplay();
        if (!e25) {
            return;
        }
        let f25 = e25.densityPixels;
        let g25 = e25.width;
        let h25 = e25.height;
        if (e25.orientation === display.Orientation.PORTRAIT ||
            e25.orientation === display.Orientation.PORTRAIT_INVERTED) {
            this.getPhoneSelectorPortraitPosition(c25, g25, h25, d25, f25);
        }
        else {
            this.getPhoneSelectorLandscapePosition(c25, g25, h25, d25, f25);
        }
    }
    private static getPcSelectorDialogPosition(z24, a25): void {
        z24.width = UI_SELECTOR_DIALOG_WIDTH;
        z24.height = UI_SELECTOR_DIALOG_HEIGHT;
        let b25: display.Display | null = this.getDefaultDisplay();
        this.pcSelectorPositionAdaptive(z24, a25);
        if (b25) {
            z24.offsetX = (b25.width - z24.width) / UI_HALF;
            z24.offsetY = (b25.height - z24.height) / UI_HALF;
        }
        else {
            z24.offsetX = (UI_DEFAULT_WIDTH - z24.width) / UI_HALF;
            z24.offsetY = UI_DEFAULT_HEIGHT - z24.height - UI_DEFAULT_BUTTOM_CLIP;
        }
    }
    private static getPhoneSelectorPortraitPosition(t24, u24, v24, w24, x24): void {
        t24.width = Math.floor(u24 * UI_SELECTOR_PORTRAIT_WIDTH_RATIO);
        t24.height = Math.floor(UI_SELECTOR_DIALOG_HEIGHT * x24);
        this.phoneSelectorPositionAdaptive(t24, x24, w24);
        let y24 = Math.floor(v24 * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO);
        if (y24 < t24.height) {
            t24.oversizeHeight = true;
            t24.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H1 * x24);
        }
        t24.offsetX = Math.floor(u24 * UI_SELECTOR_PORTRAIT_WIDTH_EDGE_RATIO);
        t24.offsetY = Math.floor((v24 * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO - t24.height));
    }
    private static getPhoneSelectorLandscapePosition(n24, o24, p24, q24, r24): void {
        n24.width = Math.floor(o24 * (UI_SELECTOR_LANDSCAPE_GRILLE_LARGE * UI_SELECTOR_LANDSCAPE_COUNT_FOUR +
            UI_SELECTOR_LANDSCAPE_GRILLE_SAMLL * UI_SELECTOR_LANDSCAPE_COUNT_THREE));
        n24.height = Math.floor((UI_SELECTOR_LANDSCAPE_HEIGHT) * r24);
        this.phoneSelectorPositionAdaptive(n24, r24, q24);
        let s24 = Math.floor((p24 - UI_SELECTOR_LANDSCAPE_SIGNAL_BAR * r24) *
            UI_SELECTOR_LANDSCAPE_MAX_RATIO);
        if (n24.height > s24) {
            n24.oversizeHeight = true;
            n24.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H1 * r24);
        }
        n24.offsetX = Math.floor((o24 - n24.width) / UI_HALF);
        n24.offsetY = Math.floor((p24 * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO - n24.height));
    }
    private static phoneSelectorPositionAdaptive(k24, l24, m24): void {
        if (m24 > LINE_NUMS_EIGHT) {
            k24.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H3 * l24);
            return;
        }
        else if (m24 > LINE_NUMS_FOUR) {
            k24.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H2 * l24);
            return;
        }
        else if (m24 > LINE_NUMS_ZERO) {
            k24.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H1 * l24);
            return;
        }
    }
    private static pcSelectorPositionAdaptive(i24, j24): void {
        if (j24 <= LINE_NUMS_TWO) {
            i24.height = UI_SELECTOR_DIALOG_PC_H2;
        }
        else if (j24 === LINE_NUMS_THREE) {
            i24.height = UI_SELECTOR_DIALOG_PC_H3;
        }
        else if (j24 === LINE_NUMS_FOUR) {
            i24.height = UI_SELECTOR_DIALOG_PC_H4;
        }
        else if (j24 > LINE_NUMS_FOUR) {
            i24.height = UI_SELECTOR_DIALOG_PC_H5;
        }
        else {
            i24.height = UI_SELECTOR_DIALOG_PC_H0;
        }
    }
    private static getDefaultDisplay(): display.Display | null {
        let g24: display.Display | null = null;
        try {
            g24 = display.getDefaultDisplaySync();
        }
        catch (h24) {
            console.error(TAG, 'getDefaultDisplaySync failed.');
        }
        return g24;
    }
}
;
