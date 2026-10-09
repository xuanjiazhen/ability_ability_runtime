
int32_t DaemonRegistry::GetStatus(
    const std::string &bundleName, const std::string &daemonName, DaemonRecord &record) const
{
    std::shared_lock lock(mutex_);
    auto iter = records_.find(MakeKey(bundleName, daemonName));
    if (iter == records_.end()) {
        return ERR_DAEMON_NOT_REGISTERED;
    }
    record = iter->second;
    return ERR_OK;
}

void DaemonRegistry::SetPid(const std::string &bundleName, const std::string &daemonName, int32_t pid)
{
    std::unique_lock lock(mutex_);
    auto iter = records_.find(MakeKey(bundleName, daemonName));
    if (iter == records_.end()) {
        return;
    }
    iter->second.pid = pid;
}
