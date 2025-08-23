/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class A {
  public static new_A(): A {
    return new A();
  }

  public static new_A_array(): Array<A> {
    return [];
  }

  boolean_method(a0: int, a1: int): boolean {
    if (a0 + a1 > 10) {
      return true;
    } else {
      return false;
    }
  }
}

class B extends A {
  public static new_B(): B {
    return new B();
  }

  boolean_method(a0: int, a1: int): boolean {
    if (a0*a1 > 10) {
      return true;
    } else {
      return false;
    }
  }
}

class C extends B {
  public static new_C(): C {
    return new C();
  }

  boolean_method(a0: int, a1: int): boolean {
    if (a0 - a1 > 10) {
      return true;
    } else {
      return false;
    }
  }
}

enum ToFind {
  RED,
  GREEN,
  BLUE
}


class MobilePhone {
  constructor(model: String, weight: int) {
    this.model = model;
    this.weight = weight;
  }
  public model: String;
  public weight: int;
}

class Animal {
  constructor(name: string, value: boolean) {
    this.name = name;
    this.value = value;
  }
  public name: string;
  public value: boolean;
}

function newAnimalObject() {
  return new Animal('Cat', true);
}

class Test {
  constructor(a: int, tag: string, animal: Animal) {
    this.a = a;
    this.tag = tag;
    this.animal = animal;
  }

  checkValue(): boolean {
    return this.a == 200 && this.tag == 'Pure P60' && this.animal.name == 'Cat' && this.animal.value == true;
  }

  setValue(a: int, tag: string, animal: Animal) {
    this.a = a;
    this.tag = tag;
    this.animal = animal;
  }

  newTestObject(a: int, tag: string, animal: Animal): Test {
    this.setValue(a, tag, animal);
    return this;
  }

  private a: int;
  private tag: string;
  private animal: Animal;
};

function getNull(): null {
  return null
}