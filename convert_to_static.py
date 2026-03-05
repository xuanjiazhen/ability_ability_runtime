#!/usr/bin/env python3
"""
Convert test BUILD.gn files to use static library references.
"""

import os
import glob
from pathlib import Path

# Base directory
BASE_DIR = "D:/code/26-1-13ability_runtime/ability_ability_runtime"
TEST_DIR = os.path.join(BASE_DIR, "test/unittest")

# Mappings from dynamic to static libraries
REPLACEMENTS = {
    '"${ability_runtime_services_path}/abilitymgr:abilityms"':
        '"${ability_runtime_services_path}/abilitymgr:abilityms_static"',
    '"${ability_runtime_services_path}/abilitymgr:wantagent_manager"':
        '"${ability_runtime_services_path}/abilitymgr:wantagent_manager_static"',
    '"${ability_runtime_services_path}/abilitymgr:mission_list"':
        '"${ability_runtime_services_path}/abilitymgr:mission_list_static"',
    '"${ability_runtime_services_path}/common:app_util"':
        '"${ability_runtime_services_path}/common:app_util_static"',
    '"${ability_runtime_services_path}/common:hisysevent_report"':
        '"${ability_runtime_services_path}/common:hisysevent_report_static"',
    '"${ability_runtime_services_path}/common:task_handler_wrap"':
        '"${ability_runtime_services_path}/common:task_handler_wrap_static"',
    '"${ability_runtime_services_path}/common:user_controller"':
        '"${ability_runtime_services_path}/common:user_controller_static"',
    '"${ability_runtime_services_path}/common:perm_verification"':
        '"${ability_runtime_services_path}/common:perm_verification_static"',
    '"${ability_runtime_services_path}/appmgr:libappms"':
        '"${ability_runtime_services_path}/appmgr:libappms_static"',
    '"${ability_runtime_services_path}/uripermmgr:libupms"':
        '"${ability_runtime_services_path}/uripermmgr:libupms_static"',
    '"${ability_runtime_services_path}/quickfixmgr:quickfixms"':
        '"${ability_runtime_services_path}/quickfixmgr:quickfixms_static"',
}

def process_file(filepath):
    """Process a single BUILD.gn file."""
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    original_content = content

    # Apply all replacements
    for old, new in REPLACEMENTS.items():
        if old in content:
            content = content.replace(old, new)
            print(f"  Converted: {old} -> {new}")

    # Only write if content changed
    if content != original_content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    """Main function."""
    # Find all BUILD.gn files in test/unittest subdirectories
    build_files = []
    for root, dirs, files in os.walk(TEST_DIR):
        for f in files:
            if f == "BUILD.gn":
                build_files.append(os.path.join(root, f))

    print(f"Found {len(build_files)} BUILD.gn files in {TEST_DIR}")

    modified_count = 0
    for filepath in build_files:
        rel_path = os.path.relpath(filepath, BASE_DIR)
        if process_file(filepath):
            print(f"Modified: {rel_path}")
            modified_count += 1

    print(f"\nTotal files modified: {modified_count} / {len(build_files)}")

if __name__ == "__main__":
    main()
