
  export native function nativeStartDaemon(bundleName: string, daemonName: string,
    callback: AsyncCallbackWrapper<void>): void;

  export native function nativeStopDaemon(bundleName: string, daemonName: string,
    callback: AsyncCallbackWrapper<void>): void;

  /**
   * Starts a registered and enabled daemon process.
   *
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   */
  export function start(bundleName: string, daemonName: string): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void,
      reject: (err: BusinessError) => void): void => {
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null): void => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        daemonServiceManager.nativeStartDaemon(bundleName, daemonName, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  /**
   * Stops a running daemon process.
   *
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   */
  export function stop(bundleName: string, daemonName: string): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void,
      reject: (err: BusinessError) => void): void => {
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null): void => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        daemonServiceManager.nativeStopDaemon(bundleName, daemonName, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }
}
