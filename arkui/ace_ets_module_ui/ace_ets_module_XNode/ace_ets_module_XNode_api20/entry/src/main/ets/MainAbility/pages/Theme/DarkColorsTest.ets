import { CustomColors, CustomDarkColors, CustomTheme } from "@ohos.arkui.theme"

class AppColors implements CustomColors {
  fontPrimary?: ResourceColor | undefined = '#FFCCBBFF'
}
class AppDarkColors implements CustomDarkColors {
  fontPrimary?: ResourceColor | undefined = '#FFEEAAFF'
}

class AppTheme implements CustomTheme{
  public colors: AppColors = new AppColors()
  public darkColors?: AppDarkColors = new AppDarkColors()
}

@Entry
@Component
export struct FontPrimary {
  build() {
    WithTheme({ theme: new AppTheme() , colorMode:ThemeColorMode.DARK})
    {
      Row() {
        Column() {
          Text('这是简单文本')
            .id('text_darkColors_id')
            .fontSize(30)
            .fontWeight(FontWeight.Bold)
        }
        .width('100%')
        .height('80%')
      }
      .height('100%')
    }
  }
}