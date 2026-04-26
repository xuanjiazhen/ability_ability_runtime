from __future__ import annotations

import sys
from pathlib import Path


def _resolve_build_validation_dir() -> Path:
    current = Path(__file__).resolve()
    for parent in current.parents:
        candidate = parent / "build" / "ohos" / "cli" / "cli_info_process"
        if candidate.is_dir():
            return candidate
    raise FileNotFoundError("cannot locate build/ohos/cli/cli_info_process")


def ensure_build_validation_import_path() -> Path:
    # 在源码仓中优先定位 build 侧校验实现。
    build_validation_dir = _resolve_build_validation_dir()
    # 复制到本地的时候进行使用: 从build/ohos/cli/cli_info_process下载最新的校验工具
    # build_validation_dir = (
    #     Path(__file__).resolve().parents[0] / "cli_info_process"
    # )
    if str(build_validation_dir) not in sys.path:
        sys.path.insert(0, str(build_validation_dir))
    return build_validation_dir
