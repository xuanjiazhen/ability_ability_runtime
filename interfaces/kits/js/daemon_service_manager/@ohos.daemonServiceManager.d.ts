
  /**
   * Starts a registered and enabled daemon process.
   *
   * @param bundleName Bundle that owns the daemon.
   * @param daemonName Name used when the daemon was registered.
   * @returns Promise resolved after the daemon process is started.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 2930003 - Daemon is not registered.
   * @throws { BusinessError } 2930004 - Daemon is disabled.
   * @throws { BusinessError } 2930006 - Device does not support daemon service.
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   */
  function start(bundleName: string, daemonName: string): Promise<void>;

  /**
   * Stops a running daemon process.
   *
   * @param bundleName Bundle that owns the daemon.
   * @param daemonName Name used when the daemon was registered.
   * @returns Promise resolved after the daemon process is stopped.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 2930003 - Daemon is not registered.
   * @throws { BusinessError } 2930005 - Daemon is not running.
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   */
  function stop(bundleName: string, daemonName: string): Promise<void>;
}

export default daemonServiceManager;
