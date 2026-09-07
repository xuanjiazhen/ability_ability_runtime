import mediaQuery from "@ohos:mediaquery";
const TAG = 'BreakpointSystem';
type triggerFunc = (columnType: ColumnType) => void;
export enum ColumnType {
    BREAKPOINT_4_COLUMN = "sm",
    BREAKPOINT_8_COLUMN = "md",
    BREAKPOINT_12_COLUMN = "lg"
}
export const BREAKPOINT_VALUE = [0, 520, 840];
export default class BreakpointSystem {
    private currentBreakpoint: string = '';
    private smListener: mediaQuery.MediaQueryListener | null = null;
    private mdListener: mediaQuery.MediaQueryListener | null = null;
    private lgListener: mediaQuery.MediaQueryListener | null = null;
    private callback: triggerFunc | null = null;
    constructor() {
        console.info(TAG, 'BreakpointSystem construct.');
    }
    private updateCurrentBreakpoint(x23: ColumnType) {
        if (this.currentBreakpoint !== x23) {
            this.currentBreakpoint = x23;
            if (this.callback) {
                this.callback(x23);
            }
        }
    }
    private isBreakpointSM = (w23: mediaQuery.MediaQueryResult) => {
        if (w23.matches) {
            this.updateCurrentBreakpoint(ColumnType.BREAKPOINT_4_COLUMN);
        }
    };
    private isBreakpointMD = (v23: mediaQuery.MediaQueryResult) => {
        if (v23.matches) {
            this.updateCurrentBreakpoint(ColumnType.BREAKPOINT_8_COLUMN);
        }
    };
    private isBreakpointLG = (u23: mediaQuery.MediaQueryResult) => {
        if (u23.matches) {
            this.updateCurrentBreakpoint(ColumnType.BREAKPOINT_12_COLUMN);
        }
    };
    public register(t23: triggerFunc) {
        this.callback = t23;
        console.info(TAG, `register.currentBreakpoint: ${this.currentBreakpoint}`);
        if (BREAKPOINT_VALUE === null || BREAKPOINT_VALUE.length !== 3) {
            console.info(TAG, 'breakpoint system register error');
        }
        this.smListener = mediaQuery.matchMediaSync('(width<' + BREAKPOINT_VALUE[1] + 'vp)');
        this.smListener.on('change', this.isBreakpointSM);
        this.mdListener = mediaQuery.matchMediaSync('(' + BREAKPOINT_VALUE[1] + 'vp<=width<' + BREAKPOINT_VALUE[2] + 'vp)');
        this.mdListener.on('change', this.isBreakpointMD);
        this.lgListener = mediaQuery.matchMediaSync('(' + BREAKPOINT_VALUE[1] + 'vp<=width)');
        this.lgListener.on('change', this.isBreakpointLG);
    }
    public unregister() {
        if (this.smListener !== null) {
            this.smListener.off('change', this.isBreakpointSM);
        }
        if (this.mdListener !== null) {
            this.mdListener.off('change', this.isBreakpointMD);
        }
        if (this.lgListener !== null) {
            this.lgListener.off('change', this.isBreakpointLG);
        }
    }
}
