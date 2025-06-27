
import { hilog } from '@kit.PerformanceAnalysisKit';
import { map } from '@kit.ConnectivityKit';
import { collections } from '@kit.ArkTS';

interface propertiesInfo{
  name: string
  type: string
  description?: string
  enum?: Array<string | number>
  subItem?: propertiesInfo
  maxLength?: number
  default?: number | Resource
}

interface propertiesRecord{
  string: propertiesInfo
}

type ObjVal = string | number | null;
type ObjType = Record<string,object>
type ArrType = ObjType[]

type FormDataType = Record<string, ObjVal | ObjType | ArrType>

@Component
export struct FormPage{
  setNestedProperty (obj: FormDataType, path: string[],value: string|number|null|undefined): object {
    let current: FormDataType | Record<string, string| number> | object[] = obj;
    for (let i = 0; i < path.length - 1; i++) {
      current = current[path[i]];
    }
    current[path[path.length - 1]] = value;
    return current
  }


  build() {
    Column(){

    }
    .width('100%')
    .height('100%')
    .padding({left:5,right:5,bottom:70})
    .margin(15)
  }
}

