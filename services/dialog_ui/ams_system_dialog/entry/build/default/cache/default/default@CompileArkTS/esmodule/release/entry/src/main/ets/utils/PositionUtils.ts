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
        let c25 = {
            width: UI_TIPS_DIALOG_WIDTH,
            height: UI_TIPS_DIALOG_HEIGHT,
            offsetX: 0,
            offsetY: 0,
            oversizeHeight: false
        };
        let d25: display.Display | null = this.getDefaultDisplay();
        if (d25) {
            let e25 = (deviceInfo.deviceType === STR_PHONE) || (deviceInfo.deviceType === STR_DEFAULT);
            if (e25) {
                c25.width = d25.width * WIDTH_MULTIPLE;
                c25.height = d25.height * HEIGHT_MULTIPLE;
                c25.offsetX = d25.width * SETX_WIDTH_MULTIPLE;
                c25.offsetY = d25.height - c25.height - UI_DEFAULT_BUTTOM_CLIP;
            }
            else {
                c25.offsetX = (d25.width - c25.width) / UI_HALF;
                c25.offsetY = (d25.height - c25.height) / UI_HALF;
            }
        }
        else {
            c25.offsetX = (UI_DEFAULT_WIDTH - c25.width) / UI_HALF;
            c25.offsetY = UI_DEFAULT_HEIGHT - c25.height - UI_DEFAULT_BUTTOM_CLIP;
        }
        return c25;
    }
    public static getSelectorDialogPosition(a25): Position {
        let b25 = {
            width: 0,
            height: 0,
            offsetX: 0,
            offsetY: 0,
            oversizeHeight: false
        };
        if (deviceInfo.deviceType === STR_PHONE || deviceInfo.deviceType === STR_DEFAULT) {
            this.getPhoneSelectorDialogPosition(b25, a25);
        }
        else {
            this.getPcSelectorDialogPosition(b25, a25);
        }
        return b25;
    }
    private static getPhoneSelectorDialogPosition(u24, v24): void {
        let w24: display.Display | null = this.getDefaultDisplay();
        if (!w24) {
            return;
        }
        let x24 = w24.densityPixels;
        let y24 = w24.width;
        let z24 = w24.height;
        if (w24.orientation === display.Orientation.PORTRAIT ||
            w24.orientation === display.Orientation.PORTRAIT_INVERTED) {
            this.getPhoneSelectorPortraitPosition(u24, y24, z24, v24, x24);
        }
        else {
            this.getPhoneSelectorLandscapePosition(u24, y24, z24, v24, x24);
        }
    }
    private static getPcSelectorDialogPosition(r24, s24): void {
        r24.width = UI_SELECTOR_DIALOG_WIDTH;
        r24.height = UI_SELECTOR_DIALOG_HEIGHT;
        let t24: display.Display | null = this.getDefaultDisplay();
        this.pcSelectorPositionAdaptive(r24, s24);
        if (t24) {
            r24.offsetX = (t24.width - r24.width) / UI_HALF;
            r24.offsetY = (t24.height - r24.height) / UI_HALF;
        }
        else {
            r24.offsetX = (UI_DEFAULT_WIDTH - r24.width) / UI_HALF;
            r24.offsetY = UI_DEFAULT_HEIGHT - r24.height - UI_DEFAULT_BUTTOM_CLIP;
        }
    }
    private static getPhoneSelectorPortraitPosition(l24, m24, n24, o24, p24): void {
        l24.width = Math.floor(m24 * UI_SELECTOR_PORTRAIT_WIDTH_RATIO);
        l24.height = Math.floor(UI_SELECTOR_DIALOG_HEIGHT * p24);
        this.phoneSelectorPositionAdaptive(l24, p24, o24);
        let q24 = Math.floor(n24 * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO);
        if (q24 < l24.height) {
            l24.oversizeHeight = true;
            l24.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H1 * p24);
        }
        l24.offsetX = Math.floor(m24 * UI_SELECTOR_PORTRAIT_WIDTH_EDGE_RATIO);
        l24.offsetY = Math.floor((n24 * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO - l24.height));
    }
    private static getPhoneSelectorLandscapePosition(f24, g24, h24, i24, j24): void {
        f24.width = Math.floor(g24 * (UI_SELECTOR_LANDSCAPE_GRILLE_LARGE * UI_SELECTOR_LANDSCAPE_COUNT_FOUR +
            UI_SELECTOR_LANDSCAPE_GRILLE_SAMLL * UI_SELECTOR_LANDSCAPE_COUNT_THREE));
        f24.height = Math.floor((UI_SELECTOR_LANDSCAPE_HEIGHT) * j24);
        this.phoneSelectorPositionAdaptive(f24, j24, i24);
        let k24 = Math.floor((h24 - UI_SELECTOR_LANDSCAPE_SIGNAL_BAR * j24) *
            UI_SELECTOR_LANDSCAPE_MAX_RATIO);
        if (f24.height > k24) {
            f24.oversizeHeight = true;
            f24.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H1 * j24);
        }
        f24.offsetX = Math.floor((g24 - f24.width) / UI_HALF);
        f24.offsetY = Math.floor((h24 * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO - f24.height));
    }
    private static phoneSelectorPositionAdaptive(c24, d24, e24): void {
        if (e24 > LINE_NUMS_EIGHT) {
            c24.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H3 * d24);
            return;
        }
        else if (e24 > LINE_NUMS_FOUR) {
            c24.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H2 * d24);
            return;
        }
        else if (e24 > LINE_NUMS_ZERO) {
            c24.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H1 * d24);
            return;
        }
    }
    private static pcSelectorPositionAdaptive(a24, b24): void {
        if (b24 <= LINE_NUMS_TWO) {
            a24.height = UI_SELECTOR_DIALOG_PC_H2;
        }
        else if (b24 === LINE_NUMS_THREE) {
            a24.height = UI_SELECTOR_DIALOG_PC_H3;
        }
        else if (b24 === LINE_NUMS_FOUR) {
            a24.height = UI_SELECTOR_DIALOG_PC_H4;
        }
        else if (b24 > LINE_NUMS_FOUR) {
            a24.height = UI_SELECTOR_DIALOG_PC_H5;
        }
        else {
            a24.height = UI_SELECTOR_DIALOG_PC_H0;
        }
    }
    private static getDefaultDisplay(): display.Display | null {
        let y23: display.Display | null = null;
        try {
            y23 = display.getDefaultDisplaySync();
        }
        catch (z23) {
            console.error(TAG, 'getDefaultDisplaySync failed.');
        }
        return y23;
    }
}
;
