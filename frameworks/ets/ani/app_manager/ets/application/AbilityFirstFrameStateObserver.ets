import { AbilityFirstFrameStateData } from 'application.AbilityFirstFrameStateData';
export default interface AbilityFirstFrameStateObserver {
    onAbilityFirstFrameDrawn(data: AbilityFirstFrameStateData): void;
}

export class AbilityFirstFrameStateObserverInner implements AbilityFirstFrameStateObserver {
    onAbilityFirstFrameDrawn(data: AbilityFirstFrameStateData): void {
    }
}