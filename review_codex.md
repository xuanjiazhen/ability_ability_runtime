# Review Findings

Review scope:

- `ability_runtime` HEAD commit: `e2e02cc235 add cli-example tools`
- `build` HEAD commit: `62f8ce3d add ohos-cli-executable`

## Findings

- [P1] Restore the `ams_system_dialog_hap` dependency in `dialog_hap` — [services/dialog_ui/ams_system_dialog/BUILD.gn](/data/home/d00838850/oh/foundation/ability/ability_runtime/services/dialog_ui/ams_system_dialog/BUILD.gn:65)
  The `dialog_hap` group is now empty because its only dependency is commented out. Any product or packaging path that still depends on `:dialog_hap` will stop pulling in `ams_system_dialog_hap`, which is a direct packaging regression.

- [P1] `cli_tools.json` and `cli_install_info.json` can go stale in incremental builds — [/data/home/d00838850/oh/build/ohos/cli/cli_profile_merge.py](/data/home/d00838850/oh/build/ohos/cli/cli_profile_merge.py:53)
  `collect_tools_from_metadata()` records every per-tool `install_info_file` as an input dependency, but `main()` writes the depfile with only `src_cli_install_info_file` as the dependency at [/data/home/d00838850/oh/build/ohos/cli/cli_profile_merge.py](/data/home/d00838850/oh/build/ohos/cli/cli_profile_merge.py:177). When a tool config changes and regenerates its `*_info.json`, the merge step may not rerun, leaving `cli_tools.json` and `cli_install_info.json` stale in incremental builds.

- [P2] The install/registry pipeline ignores the actual executable filename once `output_name` differs from `name` — [/data/home/d00838850/oh/build/ohos/cli/ohos_cli_executable.gni](/data/home/d00838850/oh/build/ohos/cli/ohos_cli_executable.gni:102)
  The template explicitly distinguishes `target_name` from `_executable_name` and records the real executable path in metadata, but later stages fall back to `config.name`. In [/data/home/d00838850/oh/build/ohos/cli/cli_profile_merge.py](/data/home/d00838850/oh/build/ohos/cli/cli_profile_merge.py:109), the install source/destination are rebuilt from `config.get('name', ...)`, and [/data/home/d00838850/oh/build/ohos/cli/cli_info_process/merge_cli_info.py](/data/home/d00838850/oh/build/ohos/cli/cli_info_process/merge_cli_info.py:80) hardcodes the registry executable path as `/system/bin/cli_tool/tools/{config.name}`. The current sample tools keep `name == output_name`, so they work today, but the template contract is already broken for any future target that sets a different `output_name`.
