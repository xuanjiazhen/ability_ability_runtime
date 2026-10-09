    int32_t GetStatus(const std::string &bundleName, const std::string &daemonName, DaemonRecord &record) const;

    // FEAT-DAEMON-03 start/stop. Tracks the daemon process pid reported by appspawn.
    void SetPid(const std::string &bundleName, const std::string &daemonName, int32_t pid);
