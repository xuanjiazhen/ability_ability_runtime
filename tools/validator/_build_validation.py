from __future__ import annotations

import sys
from pathlib import Path


def ensure_build_validation_import_path() -> Path:
    # 在build仓使用
    build_validation_dir = (
        Path(__file__).resolve().parents[5] / "build" / "ohos" / "cli" / "cli_info_process"
    )
    # 复制到本地的时候进行使用
    # build_validation_dir = (
    #     Path(__file__).resolve().parents[0] / "cli_info_process"
    # )
    if str(build_validation_dir) not in sys.path:
        sys.path.insert(0, str(build_validation_dir))
    return build_validation_dir
