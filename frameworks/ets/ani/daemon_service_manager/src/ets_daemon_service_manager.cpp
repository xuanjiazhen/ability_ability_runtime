
void StartDaemon(ani_env *env, ani_string aniBundleName, ani_string aniDaemonName, ani_object callback)
{
    InvokeDaemonCall(env, aniBundleName, aniDaemonName, callback,
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->StartDaemon(bundleName, daemonName); });
}

void StopDaemon(ani_env *env, ani_string aniBundleName, ani_string aniDaemonName, ani_object callback)
{
    InvokeDaemonCall(env, aniBundleName, aniDaemonName, callback,
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->StopDaemon(bundleName, daemonName); });
}
