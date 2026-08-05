import deviceInfo from "@ohos:deviceInfo";
export class DeviceInfoUtil {
    public static isTable(): boolean {
        return deviceInfo.deviceType === 'tablet' || deviceInfo.deviceType === '2in1';
    }
}
