/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import hilog from '@ohos.hilog';
import rpc from '@ohos.rpc';
import { BusinessError } from '@ohos.base';
import { Caller, OnReleaseCallback, OnRemoteStateChangeCallback } from '@ohos.app.ability.UIAbility';

const domainID = 0x0000;
const TAG = 'UIAbility';

const EVENT_CALL_NOTIFY = 1;
const REQUEST_SUCCESS = 0;

const ERROR_CODE_INVALID_PARAM = 401;
const ERROR_CODE_CALLER_RELEASED = 16200001;
const ERROR_CODE_CLAAEE_INVALID = 16200002;
const ERROR_CODE_INNER_ERROR = 16000050;

const ERROR_MSG_INVALID_PARAM = 'Invalid input parameter.';
const ERROR_MSG_CALLER_RELEASED = 'Caller released. The caller has been released.';
const ERROR_MSG_CLAAEE_INVALID = 'The callee does not exist.';
const ERROR_MSG_INNER_ERROR = 'Inner Error.';

let errMap = new Map<number, string>();
errMap.set(ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM);
errMap.set(ERROR_CODE_CALLER_RELEASED, ERROR_MSG_CALLER_RELEASED);
errMap.set(ERROR_CODE_CLAAEE_INVALID, ERROR_MSG_CLAAEE_INVALID);
errMap.set(ERROR_CODE_INNER_ERROR, ERROR_MSG_INNER_ERROR);

function buildError(code: number): BusinessError {
  return new BusinessError(code, new Error(errMap.get(code) as string));
}

export class ReleaseCallback {
  private myFun_: OnReleaseCallback = (msg: string) => {}
  constructor(myFun: OnReleaseCallback) {
    this.myFun_ = myFun;
  }
  invoke(msg: string): void {
    this.myFun_(msg);
  }
}

export class CallerImpl implements Caller {
  nativeCaller: long;
  private callee: rpc.IRemoteObject | null = null;
  native nativeReleaseSync(): void;
  native nativeOnReleaseSync(callback: ReleaseCallback): void;
  native nativeOnRemoteStateChangeSync(callback: OnRemoteStateChangeCallback): void;
  private releaseState = false;

  constructor() {
    hilog.info(domainID, TAG, 'Caller::constructor');
  }

  call(method: string, data: rpc.Parcelable): Promise<void> {
    return new Promise<void>((resolve: (data: undefined)=>void, reject: (error: Error) => void): void => {
      hilog.info(domainID, TAG, 'Caller call');
      resolve(undefined);
      return;
    });
  }

  callWithResult(method: string, data: rpc.Parcelable): Promise<rpc.MessageSequence> {
    return new Promise<rpc.MessageSequence>((resolve, reject) => {
      hilog.info(domainID, TAG, 'Caller callWithResult method [' + method + ']');

      const checkError = this.callCheck(method, data);
      if (checkError != null) {
        reject(checkError);
        return;
      }

      hilog.info(domainID, TAG, 'Caller callWithResult msgData rpc.MessageSequence create');
      const msgData: rpc.MessageSequence = this.buildMsgData(method, data);
      const msgReply: rpc.MessageSequence = new rpc.MessageSequence();
      const messageOption: rpc.MessageOption = new rpc.MessageOption();

      let reply: rpc.MessageSequence | undefined = undefined;
      let retData: rpc.RequestResult | undefined = undefined;
      try {
        retData = this.callee.sendMessageRequest(EVENT_CALL_NOTIFY, msgData, msgReply, messageOption);
        hilog.info(domainID, TAG, 'Caller callWithResult msgData rpc.sendMessageRequest called');

        if (retData.errCode !== 0) {
          // msgData.reclaim();
          // msgReply.reclaim();
          hilog.info(domainID, TAG, 'Caller callWithResult return errCode ' + retData.errCode);
          reject(buildError(ERROR_CODE_INNER_ERROR));
          return;
        }
      } catch (e) {
        hilog.info(domainID, TAG, 'Caller call msgData rpc.MessageSequence error ' + e);
        // msgData.reclaim();
        // msgReply.reclaim();
        reject(buildError(ERROR_CODE_INNER_ERROR));
        return;
      }

      try {
        if (!retData || !retData.reply) {
          hilog.info(domainID, TAG, 'Caller callWithResult retData is null');
          // msgData.reclaim();
          // msgReply.reclaim();
          reject(buildError(ERROR_CODE_INNER_ERROR));
          return;
        }

        const retval: number = retData.reply.readInt();
        const str: string = retData.reply.readString();
        if (retval === REQUEST_SUCCESS && str === 'object') {
          reply = retData.reply;
        } else {
          hilog.info(domainID, TAG,
            'Caller callWithResult retval is [' + retval + '], str [' + str + ']');
          // msgData.reclaim();
          // msgReply.reclaim();
          reject(buildError(ERROR_CODE_INNER_ERROR));
          return;
        }
      } catch (e) {
        hilog.info(domainID, TAG, 'Caller callWithResult msgData sendMessageRequest retval error');
        // msgData.reclaim();
        // msgReply.reclaim();
        reject(buildError(ERROR_CODE_INNER_ERROR));
        return;
      }

      hilog.info(domainID, TAG, 'Caller callWithResult msgData sendMessageRequest end');
      resolve(reply);
      return;
    });
  }

  release(): void {
    hilog.info(domainID, TAG, 'Caller release called.');
    if (this.releaseState === true) {
      hilog.info(domainID, TAG, 'Caller release remoteObj releaseState is true');
      throw buildError(ERROR_CODE_CALLER_RELEASED);
    }

    if (this.callee == null) {
      hilog.info(domainID, TAG, 'Caller release call remoteObj is released');
      throw buildError(ERROR_CODE_CLAAEE_INVALID);
    }

    this.releaseState = true;
    this.callee = null;
    this.nativeReleaseSync();
  }

  onRelease(callback: OnReleaseCallback): void {
    hilog.info(domainID, TAG, 'onRelease called');
    this.nativeOnReleaseSync(new ReleaseCallback(callback));
  }

  onRemoteStateChange(callback: OnRemoteStateChangeCallback): void {
    hilog.info(domainID, TAG, 'Caller onRemoteStateChange jscallback called.');

    if (typeof callback !== 'function') {
      hilog.info(domainID, TAG, 'Caller onRemoteStateChange ' + typeof callback);
    }
    if (this.releaseState === true) {
      hilog.info(domainID, TAG, 'Caller onRemoteStateChange remoteObj releaseState is true');
      throw buildError(ERROR_CODE_CALLER_RELEASED);
    }
    this.nativeOnRemoteStateChangeSync((msg: string) => { callback(msg);});
  }

  on(type: string, callback: OnReleaseCallback): void {
    hilog.info(domainID, TAG, 'Caller onRelease jscallback called.');

    if (typeof type !== 'string' || type !== 'release') {
      hilog.info(domainID, TAG, 'Caller onRelease error, input [type] is invalid.');
    }
    if (typeof callback !== 'function') {
      hilog.info(domainID, TAG, 'Caller onRelease error ' + typeof callback);
    }
    if (this.releaseState === true) {
      hilog.info(domainID, TAG, 'Caller onRelease error, remoteObj releaseState is true');
      throw buildError(ERROR_CODE_CALLER_RELEASED);
    }
    this.nativeOnReleaseSync(new ReleaseCallback(callback));
  }

  off(type: string, callback: OnReleaseCallback): void {
    if (typeof type !== 'string' || type !== 'release') {
      hilog.info(domainID, TAG, 'Caller onRelease error, input [type] is invalid.');
    }
    if (callback && typeof callback !== 'function') {
      hilog.info(domainID, TAG, 'Caller onRelease error ' + typeof callback);
    }
    // Empty
  }

  callCheck(method: string, data: Object): BusinessError | null {
    if (typeof method !== 'string' || typeof data !== 'object') {
      hilog.info(domainID, TAG, 'Caller callCheck ' + typeof method + ' ' + typeof data);
    }
    if (method === '' || data == null) {
      hilog.info(domainID, TAG, 'Caller callCheck ' + method + ', ' + data);
    }
    if (this.releaseState === true) {
      hilog.info(domainID, TAG, 'Caller callCheck remoteObj releaseState is true');
      return buildError(ERROR_CODE_CALLER_RELEASED);
    }
    if (this.callee == null) {
      hilog.info(domainID, TAG, 'Caller callCheck this.callee is nullptr');
      throw buildError(ERROR_CODE_CLAAEE_INVALID);
    }
    return null;
  }

  private buildMsgData(method: string, data: rpc.Parcelable): rpc.MessageSequence {
    const msg = new rpc.MessageSequence();
    msg.writeString(method);
    // msg.writeParcelable(data);
    return msg;
  }
}
