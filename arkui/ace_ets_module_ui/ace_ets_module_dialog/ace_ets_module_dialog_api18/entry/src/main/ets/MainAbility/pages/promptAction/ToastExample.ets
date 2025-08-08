import { PromptAction } from '@kit.ArkUI';
import { BusinessError } from '@kit.BasicServicesKit';

@Entry
@Component
struct ToastExample {
  @State toastId: number = 0;
  promptAction: PromptAction = this.getUIContext().getPromptAction();

  build() {
    Column() {
      Button('openToast')
        .height('10%')
        .onClick(() => {
          this.promptAction.openToast({
            message: 'Toast Massage',
            duration: 10000,
          }).then((toastId: number) => {
            this.toastId = toastId;
          })
            .catch((error: BusinessError) => {
              console.error(`openToast error code is ${error.code}, message is ${error.message}`);
            })
        })
      Blank().height('30%')
      Button('closeToast')
        .height('10%')
        .onClick(() => {
          try {
            this.promptAction.closeToast(this.toastId);
          } catch (error) {
            let message = (error as BusinessError).message;
            let code = (error as BusinessError).code;
            console.error(`CloseToast error code is ${code}, message is ${message}`);
          };
        })
    }.height('100%').width('100%').justifyContent(FlexAlign.Center)
  }
}