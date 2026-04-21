/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_AAFWK_WANT_H
#define MOCK_AAFWK_WANT_H

#include "operation.h"

namespace OHOS {
namespace AAFwk {

class Want {
public:
    Operation GetOperation() const { return operation_; }
    void SetOperation(const Operation &operation) { operation_ = operation; }

private:
    Operation operation_;
};

} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_AAFWK_WANT_H
