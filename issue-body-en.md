## Problem Description

When `GetMemLeakStringFunc` returns char* data containing null bytes (\0), the `std::string` assignment `dumpResult = buf` and `strlen(buf)` in `GetSnapshot` truncate the data at the first null byte, causing incomplete dump results.

## Solution

1. Update `GetMemLeakStringFunc` signature to add `int *` output parameter for actual data length
2. Use `dumpResult.assign(buf, outLen)` instead of `dumpResult = buf` in `GetDumpResult`
3. Use `write(fd, buf, outLen)` instead of `strlen(buf)` in `GetSnapshot`

## Affected Modules

- ability_runtime/frameworks/native/appkit/app/
- ability_runtime/interfaces/kits/native/appkit/app/