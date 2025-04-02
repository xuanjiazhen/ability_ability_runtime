export interface AbilityFirstFrameStateData {
  bundleName: string;

  moduleName: string;

  abilityName: string;

  appIndex: number;

  isColdStart: boolean;
}

class AbilityFirstFrameStateDataInner implements AbilityFirstFrameStateData {
  bundleName: string = '';

  moduleName: string = '';

  abilityName: string = '';

  appIndex: number = -1;

  isColdStart: boolean = false;
}