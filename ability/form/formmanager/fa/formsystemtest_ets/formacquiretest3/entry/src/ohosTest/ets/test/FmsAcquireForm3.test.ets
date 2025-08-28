/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import featureAbility from "@ohos.ability.featureAbility";
import commonEvent from '@ohos.commonEvent';
import { beforeEach, afterEach, describe, expect, it } from '@ohos/hypium';
import CommonEventManager from '@ohos.commonEventManager';
import { BusinessError } from '@ohos.base';

interface onFormEvent {
    events: string[];
}

let options:CommonEventManager.CommonEventPublishData = {
    code: 1,
    data: "initial data",
    isOrdered: true
}

let onErrorForm_Event: onFormEvent = {
    events: ["FMS_FormOnError_commonEvent"],
};
let onAcquiredForm_Event: onFormEvent = {
    events: ["FMS_FormOnAcquired_commonEvent"],
};
let subscriberFormCastEvent: onFormEvent = {
    events: ["FMS_FormCast_commonEvent"],
};
let onSupplyEvent: onFormEvent = {
    events: ["FMS_FormSupply_commonEvent"],
};

const unsubscribeCallback = (tcNumber: string) => {
    console.info(`====>${tcNumber} unsubscribeCallback====>`);
}

let deleteForm_Event: string = "FMS_FormDelete_commonEvent";

let subscriberOnError: CommonEventManager.CommonEventSubscriber;
let subscriberOnAcquired: CommonEventManager.CommonEventSubscriber;
let subscriberCast: CommonEventManager.CommonEventSubscriber;
let subscriberSupply: CommonEventManager.CommonEventSubscriber;

export default function test() {
    describe(`FmsAcquiredFormTest3`, () => {

        beforeEach(async () => {
            subscriberOnError = await commonEvent.createSubscriber(onErrorForm_Event);
            subscriberOnAcquired = await commonEvent.createSubscriber(onAcquiredForm_Event);
            subscriberCast = await commonEvent.createSubscriber(subscriberFormCastEvent);
            subscriberSupply = await commonEvent.createSubscriber(onSupplyEvent);
            await sleepTime(1000);
        })

        afterEach(async() => {
            commonEvent.unsubscribe(subscriberOnError, () => unsubscribeCallback("afterEach unsubscribe subscriberOnError"));
            commonEvent.unsubscribe(subscriberOnAcquired, () => unsubscribeCallback("afterEach unsubscribe subscriberOnAcquired"));
            commonEvent.unsubscribe(subscriberCast, () => unsubscribeCallback("afterEach unsubscribe subscriberCast"));
            commonEvent.unsubscribe(subscriberSupply, () => unsubscribeCallback("afterEach unsubscribe subscriberSupply"));
        })

        /**
         * @tc.number: FMS_acquireForm_1600
         * @tc.name: A single form host (using a single thread) continuously creates 2 normal and 2 temporary forms.
         * @tc.desc: Verify that a single host (using a single thread) can successfully create multiple normal
         *           and temporary forms continuously.
         */
        it(`FMS_acquireForm_1600`, 0, async (done: () => void) => {
            console.info(`FMS_acquireForm_1600 start`);

            let FMS_acquireForm_1600_Temp2 = async () => {
                console.info(`FMS_acquireForm_1600_Temp2 start`);
                await featureAbility.startAbility({
                    want: {
                        bundleName: "com.ohos.st.formsystemhost",
                        abilityName: "com.ohos.st.formsystemhost.MainAbility",

                        parameters: {
                            "formId": "0",
                            "name": "Form_Js001",
                            "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                            "ability": "FormAbility",
                            "moduleName": "entry",
                            "temporary": true,
                            "isCreate": true
                        }
                    }
                }).then((res) => {
                    console.info(`FMS_acquireForm_1600_Temp2 then res: ${JSON.stringify(res)}`);
                    expect(res).assertEqual(0);
                    commonEvent.publish(deleteForm_Event, ()=>{});
                    done();
                }).catch((err: BusinessError) => {
                    console.info(`FMS_acquireForm_1600_Temp2 catch error: ${JSON.stringify(err)}`);
                });
                console.info(`FMS_acquireForm_1600_Temp2 end`);
            }

            let delPublishCallBack_Temp1 = () => {
                console.info("FMS_acquireForm_1600 delPublishCallBack_Temp1 start");
                setTimeout( () => {
                    console.info('FMS_acquireForm_1600 delPublishCallBack_Temp1 end');
                    FMS_acquireForm_1600_Temp2();
                }, 50);
            }

            let FMS_acquireForm_1600_Temp1 = async () => {
                console.info(`FMS_acquireForm_1600_Temp1 start`);
                await featureAbility.startAbility({
                    want: {
                        bundleName: "com.ohos.st.formsystemhost",
                        abilityName: "com.ohos.st.formsystemhost.MainAbility",

                        parameters: {
                            "formId": "0",
                            "name": "Form_Js001",
                            "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                            "ability": "FormAbility",
                            "moduleName": "entry",
                            "temporary": true,
                            "isCreate": true
                        }
                    }
                }).then((res) => {
                    console.info(`FMS_acquireForm_1600_Temp1 then res: ${JSON.stringify(res)}`);
                    expect(res).assertEqual(0);
                    commonEvent.publish(deleteForm_Event, delPublishCallBack_Temp1);
                }).catch((err: BusinessError) => {
                    console.info(`FMS_acquireForm_1600_Temp1 catch error: ${JSON.stringify(err)}`);
                });
                console.info(`FMS_acquireForm_1600_Temp1 end`);
            }

            let delPublishCallBack_Normal2 = () => {
                console.info("FMS_acquireForm_1600 delPublishCallBack_Normal2 start");
                setTimeout(() => {
                    console.info('FMS_acquireForm_1600 delPublishCallBack_Normal2 end');
                    FMS_acquireForm_1600_Temp1();
                }, 50);
            }

            let FMS_acquireForm_1600_normal2 = async () => {
                console.info(`FMS_acquireForm_1600_normal2 start`);
                await featureAbility.startAbility({
                    want: {
                        bundleName: "com.ohos.st.formsystemhost",
                        abilityName: "com.ohos.st.formsystemhost.MainAbility",
                        parameters: {
                            "formId": "0",
                            "name": "Form_Js001",
                            "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                            "ability": "FormAbility",
                            "moduleName": "entry",
                            "temporary": false,
                            "isCreate": true
                        }
                    }
                }).then((res) => {
                    console.info(`FMS_acquireForm_1600_normal2 then res: ${JSON.stringify(res)}`);
                    expect((res)).assertEqual(0);
                    commonEvent.publish(deleteForm_Event, delPublishCallBack_Normal2);
                }).catch((err: BusinessError) => {
                    console.info(`FMS_acquireForm_1600_normal2 catch error: ${JSON.stringify(err)}`);
                });
                console.info(`FMS_acquireForm_1600_normal2 end`);
            }

            let delPublishCallBack_Normal1 = () => {
                console.info("FMS_acquireForm_1600 delPublishCallBack_Normal1 start");
                setTimeout( () => {
                    console.info('FMS_acquireForm_1600 delPublishCallBack_Normal1 end');
                    FMS_acquireForm_1600_normal2();
                }, 50);
            }

            let FMS_acquireForm_1600_normal1 = async () => {
                console.info(`FMS_acquireForm_1600_normal1 start`);
                await featureAbility.startAbility({
                    want: {
                        bundleName: "com.ohos.st.formsystemhost",
                        abilityName: "com.ohos.st.formsystemhost.MainAbility",
                        parameters: {
                            "formId": "0",
                            "name": "Form_Js001",
                            "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                            "ability": "FormAbility",
                            "moduleName": "entry",
                            "temporary": false,
                            "isCreate": true
                        }
                    }
                }).then((res) => {
                    console.info(`FMS_acquireForm_1600_normal1 then res: ${JSON.stringify(res)}`);
                    expect(res).assertEqual(0);
                    commonEvent.publish(deleteForm_Event, delPublishCallBack_Normal1);
                }).catch((err: BusinessError) => {
                    console.info(`FMS_acquireForm_1600_normal1 catch error: ${JSON.stringify(err)}`);
                });
                console.info(`FMS_acquireForm_1600_normal1 end`);
            }
            FMS_acquireForm_1600_normal1();
            console.info(`FMS_acquireForm_1600 end`);
        });

        /**
         * @tc.number: FMS_acquireForm_1800
         * @tc.name: Multiple form hosts create one normal form and one temporary form respectively.
         * @tc.desc: Verify that multiple hosts can successfully create multiple normal and temporary forms respectively.
         */
        it(`FMS_acquireForm_1800`, 0, async (done: () => void) => {
            console.info(`FMS_acquireForm_1800 start`);
            let FMS_acquireForm_1800_Temp2 = async () => {
                console.info(`FMS_acquireForm_1800_Temp2 start`);
                await featureAbility.startAbility({
                    want: {
                        bundleName: "com.ohos.st.formsystemhostb",
                        abilityName: "com.ohos.st.formsystemhostb.MainAbility",

                        parameters: {
                            "formId": "0",
                            "name": "Form_Js001",
                            "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                            "ability": "FormAbility",
                            "moduleName": "entry",
                            "temporary": true,
                            "isCreate": true
                        }
                    }
                }).then((res) => {
                    console.info(`FMS_acquireForm_1800_Temp2 then res: ${JSON.stringify(res)}`);
                    expect(res).assertEqual(0);
                    commonEvent.publish(deleteForm_Event, ()=>{});
                    done();
                }).catch((err: BusinessError) => {
                    console.info(`FMS_acquireForm_1800_Temp2 catch error: ${JSON.stringify(err)}`);
                });
                console.info(`FMS_acquireForm_1800_Temp2 end`);
            }

            let delPublishCallBack_Temp1 = () => {
                console.info("FMS_acquireForm_1800 delPublishCallBack_Temp1 start");
                setTimeout( () => {
                    console.info('FMS_acquireForm_1800 delPublishCallBack_Temp1 end');
                    FMS_acquireForm_1800_Temp2();
                }, 50);
            }

            let FMS_acquireForm_1800_Temp1 = async () => {
                console.info(`FMS_acquireForm_1800_Temp1 start`);
                await featureAbility.startAbility({
                    want: {
                        bundleName: "com.ohos.st.formsystemhost",
                        abilityName: "com.ohos.st.formsystemhost.MainAbility",
                        parameters: {
                            "formId": "0",
                            "name": "Form_Js001",
                            "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                            "ability": "FormAbility",
                            "moduleName": "entry",
                            "temporary": true,
                            "isCreate": true
                        }
                    }
                }).then((res) => {
                    console.info(`FMS_acquireForm_1800_Temp1 then res: ${JSON.stringify(res)}`);
                    options.code = res;
                    options.data = 'FMS_FormOnAcquired_commonEvent';
                    expect(res).assertEqual(0);
                    commonEvent.publish(deleteForm_Event, options, delPublishCallBack_Temp1);
                }).catch((err: BusinessError) => {
                    console.info(`FMS_acquireForm_1800_Temp1 catch error: ${JSON.stringify(err)}`);
                });
                console.info(`FMS_acquireForm_1800_Temp1 end`);
            }

            let delPublishCallBack_Normal2 = () => {
                console.info("FMS_acquireForm_1800 delPublishCallBack_Normal2 start");
                setTimeout( () => {
                    console.info('FMS_acquireForm_1800 delPublishCallBack_Normal2 end');
                    FMS_acquireForm_1800_Temp1();
                }, 50);
            }

            let FMS_acquireForm_1800_normal2 = async () => {
                console.info(`FMS_acquireForm_1800_normal2 start`);
                await featureAbility.startAbility({
                    want: {
                        bundleName: "com.ohos.st.formsystemhostb",
                        abilityName: "com.ohos.st.formsystemhostb.MainAbility",
                        parameters: {
                            "formId": "0",
                            "name": "Form_Js001",
                            "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                            "ability": "FormAbility",
                            "moduleName": "entry",
                            "temporary": false,
                            "isCreate": true
                        }
                    }
                }).then((res) => {
                    console.info(`FMS_acquireForm_1800_normal2 then res: ${JSON.stringify(res)}`);
                    expect(res).assertEqual(0);
                    commonEvent.publish(deleteForm_Event, delPublishCallBack_Normal2);
                }).catch((err: BusinessError) => {
                    console.info(`FMS_acquireForm_1800_normal2 catch error: ${JSON.stringify(err)}`);
                });
                console.info(`FMS_acquireForm_1800_normal2 end`);
            }

            let delPublishCallBack_Normal1 = () => {
                console.info("FMS_acquireForm_1800 delPublishCallBack_Normal1 start");
                setTimeout(async () => {
                    console.info('FMS_acquireForm_1800 delPublishCallBack_Normal1 end');
                    FMS_acquireForm_1800_normal2();
                }, 50);
            }

            let FMS_acquireForm_1800_normal1 = async() => {
                console.info(`FMS_acquireForm_1800_normal1 start`);
                await featureAbility.startAbility({
                    want: {
                        bundleName: "com.ohos.st.formsystemhost",
                        abilityName: "com.ohos.st.formsystemhost.MainAbility",
                        parameters: {
                            "formId": "0",
                            "name": "Form_Js001",
                            "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                            "ability": "FormAbility",
                            "moduleName": "entry",
                            "temporary": false,
                            "isCreate": true
                        }
                    }
                }).then((res) => {
                    console.info(`FMS_acquireForm_1800_normal1 then res: ${JSON.stringify(res)}`);
                    expect(res).assertEqual(0);
                    commonEvent.publish(deleteForm_Event, delPublishCallBack_Normal1);
                }).catch((err: BusinessError) => {
                    console.info(`FMS_acquireForm_1800_normal1 catch error: ${JSON.stringify(err)}`);
                });
                console.info(`FMS_acquireForm_1800_normal1 end`);
            }
            await FMS_acquireForm_1800_normal1();
            console.info(`FMS_acquireForm_1800 end`);
        });

        /**
         * @tc.number: FMS_acquireForm_1900
         * @tc.name: Add a temporary form when the formId is set to 0.
         * @tc.desc: When the verification formId is set to 0, the temporary form is created successfully.
         */
        it(`FMS_acquireForm_1900`, 0, async (done: () => void) => {
            console.info(`FMS_acquireForm_1900 start`);
            await featureAbility.startAbility({
                want: {
                    bundleName: "com.ohos.st.formsystemhost",
                    abilityName: "com.ohos.st.formsystemhost.MainAbility",
                    parameters: {
                        "formId": "0",
                        "name": "Form_Js001",
                        "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                        "ability": "FormAbility",
                        "moduleName": "entry",
                        "temporary": true,
                        "isCreate": true
                    }
                }
            }).then((res) => {
                console.info(`FMS_acquireForm_1900 then res: ${JSON.stringify(res)}`);
                expect(res).assertEqual(0);
                options.code = res;
                options.data = 'FMS_FormSupply_commonEvent';
                commonEvent.publish(deleteForm_Event, options, (err) => {
                    console.info('FMS_acquireForm_1900 publish deleteEvent:' + JSON.stringify(err));
                    done();
                });
            }).catch((err: BusinessError) => {
                console.info(`FMS_acquireForm_1900 catch error: ${JSON.stringify(err)}`);
                commonEvent.publish("FMS_FormOnError_commonEvent", () => {
                    console.info(`FMS_acquireForm_1900 publish FMS_FormOnError_commonEvent`)})
            });

            console.info(`FMS_acquireForm_1900 end`);
        });

        /**
         * @tc.number: FMS_acquireForm_2100
         * @tc.name: When creating a temporary form, the formId is specified as the created normal formId.
         * @tc.desc: Failed to create a temporary form when verifying that the formId is set to create a normal formId.
         */
        it(`FMS_acquireForm_2100`, 0, async (done: () => void) => {
            console.info(`FMS_acquireForm_2100 start`);
            let formId: string = 'FMS_FormOnAcquired_commonEvent';
            let onAcquiredCallBack = async () => {
                console.info(`FMS_acquireForm_2100_CallBack startAbilityhost`);
                await featureAbility.startAbility({
                    want: {
                        bundleName: "com.ohos.st.formsystemhost",
                        abilityName: "com.ohos.st.formsystemhost.MainAbility",
                        parameters: {
                            "formId": formId,
                            "name": "Form_Js001",
                            "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                            "ability": "FormAbility",
                            "moduleName": "entry",
                            "temporary": true,
                            "isCreate": true
                        }
                    }
                }).then((res) => {
                    console.info(`FMS_acquireForm_2100_CallBack then res: ${JSON.stringify(res)}`);
                    expect(res).assertEqual(0);
                    done();
                }).catch((err: BusinessError) => {
                    console.info(`FMS_acquireForm_2100_CallBack catch error: ${JSON.stringify(err)}`);
                    commonEvent.publish(deleteForm_Event, () => {
                        console.info('FMS_acquireForm_2100_CallBack catch publish deleteForm_Event');
                    });
                });
            }

            console.info(`FMS_acquireForm_2100 startAbilityhost`);
            await featureAbility.startAbility({
                want: {
                    bundleName: "com.ohos.st.formsystemhost",
                    abilityName: "com.ohos.st.formsystemhost.MainAbility",
                    parameters: {
                        "formId": "0",
                        "name": "Form_Js001",
                        "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                        "ability": "FormAbility",
                        "moduleName": "entry",
                        "temporary": false,
                        "isCreate": true
                    }
                }
            }).then((res) => {
                console.info(`FMS_acquireForm_2100 then res: ${JSON.stringify(res)}`);
                expect(res).assertEqual(0);
                onAcquiredCallBack();
            }).catch((err: BusinessError) => {
                console.info(`FMS_acquireForm_2100 catch error: ${JSON.stringify(err)}`);
            });
            console.info(`FMS_acquireForm_2100 end`);
        });

        /**
         * @tc.number: FMS_acquireForm_2200
         * @tc.name: Cast to normal form when the temporary formId is set to negative.
         * @tc.desc: Failed to Cast to normal form when the temporary formId is set to negative
         */
        it(`FMS_acquireForm_2200`, 0, async (done: () => void) => {
            console.info(`FMS_acquireForm_2200 start`);
            await featureAbility.startAbility({
                want: {
                    bundleName: "com.ohos.st.formsystemhost",
                    abilityName: "com.ohos.st.formsystemhost.MainAbility",
                    parameters: {
                        "formId": "0",
                        "name": "Form_Js001",
                        "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                        "ability": "FormAbility",
                        "moduleName": "entry",
                        "temporary": true,
                        "temporaryId": "-1",
                        "castForm": true,
                        "isCreate": true
                    }
                }
            }).then((res) => {
                console.info(`FMS_acquireForm_2200 then res: ${JSON.stringify(res)}`);
                expect(res).assertEqual(0);
                options.code = res;
                options.data = 'FMS_FormCast_commonEvent';
                commonEvent.publish(deleteForm_Event,  async (err) => {
                    console.info('FMS_acquireForm_2200 then publish deleteForm_Event' + JSON.stringify(err));
                    done();
                });
            }).catch((err: BusinessError) => {
                console.info(`FMS_acquireForm_2200 catch error: ${JSON.stringify(err)}`);

            });
            console.info(`FMS_acquireForm_2200 end`);
        });

        /**
         * @tc.number: FMS_acquireForm_2300
         * @tc.name: Cast to normal form when the temporary formId is set to 0.
         * @tc.desc: Failed to convert to normal form when verifying that the temporary formIdis set to 0
         */
        it(`FMS_acquireForm_2300`, 0, async (done: () => void) => {
            console.info(`FMS_acquireForm_2300 start`);
            await featureAbility.startAbility({
                want: {
                    bundleName: "com.ohos.st.formsystemhost",
                    abilityName: "com.ohos.st.formsystemhost.MainAbility",
                    parameters: {
                        "formId": "0",
                        "name": "Form_Js001",
                        "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                        "ability": "FormAbility",
                        "moduleName": "entry",
                        "temporary": true,
                        "temporaryId": "0",
                        "castForm": true,
                        "isCreate": true
                    }
                }
            }).then((res) => {
                console.info(`FMS_acquireForm_2300 then res: ${JSON.stringify(res)}`);
                expect(res).assertEqual(0);
                commonEvent.publish(deleteForm_Event, async (err) => {
                    console.info('FMS_acquireForm_2300 then publish deleteForm_Event' + JSON.stringify(err));
                    done();
                });
            }).catch((err: BusinessError) => {
                console.info(`FMS_acquireForm_2300 catch error: ${JSON.stringify(err)}`);
            });
            console.info(`FMS_acquireForm_2300 end`);
        });

        /**
         * @tc.number: FMS_acquireForm_2400
         * @tc.name: Non affiliated temporary form ID to normal form.
         * @tc.desc: Failed to convert non affiliated temporary form ID to normal form.
         */
        it(`FMS_acquireForm_2400`, 0, async (done: () => void) => {
            console.info(`FMS_acquireForm_2400 start`);
            let onAcquiredCallBack = async () => {
                console.info(`FMS_acquireForm_2400_callback start`);
                await featureAbility.startAbility({
                    want: {
                        bundleName: "com.ohos.st.formsystemhostb",
                        abilityName: "com.ohos.st.formsystemhostb.MainAbility",
                        parameters: {
                            "formId": "0",
                            "name": "Form_Js001",
                            "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                            "ability": "FormAbility",
                            "moduleName": "entry",
                            "temporary": false,
                            "temporaryId": "FMS_FormOnAcquired_commonEvent",
                            "castForm": true,
                            "isCreate": true
                        }
                    }
                }).then((res) => {
                    console.info(`FMS_acquireForm_2400_callback then res: ${JSON.stringify(res)}`);
                    expect(res).assertEqual(0);
                    commonEvent.publish(deleteForm_Event, async (err) => {
                        console.info('FMS_acquireForm_2400_callback then publish deleteForm_Event' + JSON.stringify(err));
                        done();
                    });
                }).catch((err: BusinessError) => {
                    console.info(`FMS_acquireForm_2400_callback catch error: ${JSON.stringify(err)}`);
                });
                console.info(`FMS_acquireForm_2400_callback end`);
            };
            console.info(`FMS_acquireForm_2400 startAbilityhost`);
            await featureAbility.startAbility({
                want: {
                    bundleName: "com.ohos.st.formsystemhost",
                    abilityName: "com.ohos.st.formsystemhost.MainAbility",
                    parameters: {
                        "formId": "0",
                        "name": "Form_Js001",
                        "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                        "ability": "FormAbility",
                        "moduleName": "entry",
                        "temporary": true,
                        "isCreate": true
                    }
                }
            }).then((res) => {
                console.info(`FMS_acquireForm_2400 then res: ${JSON.stringify(res)}`);
                expect(res).assertEqual(0);
                onAcquiredCallBack();
            }).catch((err: BusinessError) => {
                console.info(`FMS_acquireForm_2400 catch error: ${JSON.stringify(err)}`);
            });
            console.info(`FMS_acquireForm_2400 end`);
        });

        /**
         * @tc.number: FMS_acquireForm_2500
         * @tc.name: cast non-existent temporary formId to normal form.
         * @tc.desc: Failed to verify that the temporary formId that does not exist is casted to normal form.
         */
        it(`FMS_acquireForm_2500`, 0, async (done: () => void) => {
            console.info(`FMS_acquireForm_2500 start`);
            await featureAbility.startAbility({
                want: {
                    bundleName: "com.ohos.st.formsystemhost",
                    abilityName: "com.ohos.st.formsystemhost.MainAbility",
                    parameters: {
                        "formId": "0",
                        "name": "Form_Js001",
                        "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                        "ability": "FormAbility",
                        "moduleName": "entry",
                        "temporary": true,
                        "temporaryId": "123456789",
                        "castForm": true,
                        "isCreate": true
                    }
                }
            }).then((res) => {
                console.info(`FMS_acquireForm_2500 then res: ${JSON.stringify(res)}`);
                expect(res).assertEqual(0);
                commonEvent.publish(deleteForm_Event, async () => {
                    console.info('FMS_acquireForm_2500 then publish deleteForm_Event');
                    done();
                });
            }).catch((err: BusinessError) => {
                console.info(`FMS_acquireForm_2500 catch error: ${JSON.stringify(err)}`);
            });
            console.info(`FMS_acquireForm_2500 end`);
        });

       /**
         * @tc.number: FMS_acquireForm_2600
         * @tc.name: When the normal form does not reach the maximum value created by the form, the temporary form changes to the normal form.
         * @tc.desc: Verify that when the normal form does not reach the maximum value created by the form.
         *           the temporary form can be successfully casted to the normal form.
         */
        it(`FMS_acquireForm_2600`, 0, async (done: () => void) => {
            console.info(`FMS_acquireForm_2600 start`);
            let onAcquiredCallBack = async () => {
                console.info("FMS_acquireForm_2600_callback startAbilityhost");
                await featureAbility.startAbility({
                    want: {
                        bundleName: "com.ohos.st.formsystemhost",
                        abilityName: "com.ohos.st.formsystemhost.MainAbility",
                        parameters: {
                            "formId": "0",
                            "name": "Form_Js001",
                            "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                            "ability": "FormAbility",
                            "moduleName": "entry",
                            "temporary": false,
                            "temporaryId": "FMS_FormCast_commonEvent",
                            "castForm": true,
                            "isCreate": false
                        }
                    }
                }).then((res) => {
                    console.info(`FMS_acquireForm_2600_callback then res: ${JSON.stringify(res)}`);
                    expect(res).assertEqual(0);
                    commonEvent.publish(deleteForm_Event, async () => {
                        console.info("FMS_acquireForm_2600_callback then publish deleteForm_Event");
                        done();
                    });
                }).catch((err: BusinessError) => {
                    console.info(`FMS_acquireForm_2600_callback catch error: ${JSON.stringify(err)}`);
                });
                console.info(`FMS_acquireForm_2600_callback end`);
            }

            console.info(`FMS_acquireForm_2600 startAbilityhost`);
            await featureAbility.startAbility({
                want: {
                    bundleName: "com.ohos.st.formsystemhost",
                    abilityName: "com.ohos.st.formsystemhost.MainAbility",
                    parameters: {
                        "formId": "0",
                        "name": "Form_Js001",
                        "bundle": "com.acts.form.formsystemtestservicea.hmservice",
                        "ability": "FormAbility",
                        "moduleName": "entry",
                        "temporary": true,
                        "isCreate": true
                    }
                }
            }).then((res) => {
                console.info(`FMS_acquireForm_2600 then res: ${JSON.stringify(res)}`);
                expect(res).assertEqual(0);
                onAcquiredCallBack();
            }).catch((err: BusinessError) => {
                console.info(`FMS_acquireForm_2600 catch error: ${JSON.stringify(err)}`);
            });
            console.info(`FMS_acquireForm_2600 end`);
        });
    });
}

function sleepTime(time: number): Promise<void> {
    return new Promise((resolve, reject) => {
        try{
            setTimeout(resolve,time);
        } catch(e) {
            console.error(`sleepTime error: ${JSON.stringify(e)}`);
			reject();
        }
    });
}