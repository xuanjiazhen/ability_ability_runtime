#!/system/bin/sh
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0.
set -eu
TEST_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$TEST_DIR"
export APP_SERVICE_TEST_HAP_DIR="$TEST_DIR/resource"
chmod 755 ./app_service_proxy_test
./app_service_proxy_test --gtest_output=xml:./app_service_results.xml
