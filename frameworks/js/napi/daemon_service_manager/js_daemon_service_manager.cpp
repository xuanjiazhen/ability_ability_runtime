
napi_value StartDaemon(napi_env env, napi_callback_info info)
{
    return InvokeDaemonCall(env, info, "JsDaemonServiceManager::StartDaemon",
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->StartDaemon(bundleName, daemonName); });
}

napi_value StopDaemon(napi_env env, napi_callback_info info)
{
    return InvokeDaemonCall(env, info, "JsDaemonServiceManager::StopDaemon",
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->StopDaemon(bundleName, daemonName); });
}
