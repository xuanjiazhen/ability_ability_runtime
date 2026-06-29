/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "file_page_releaser.h"

#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bundle_mgr_helper.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "madvise_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace FilePageReleaser {
namespace {
using AppExecFwk::BundleInfo;
using AppExecFwk::BundleMgrHelper;
using AppExecFwk::HapModuleInfo;
using MadviseUtil::FileOffsetSpan;

// BundleMgrHelper::GetBundleInfo flag: GET_BUNDLE_INFO_WITH_HAP_MODULE.
// (GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE == 0x2 in bundle_info.h)
constexpr int32_t FLAG_WITH_HAP_MODULE = 0x00000002;

constexpr const char* RAWFILE_DIR_PREFIX = "resources/rawfile/";

// ---- ZIP minimal parser (central directory only) ----
// All multi-byte fields are little-endian.
constexpr uint32_t ZIP_EOCD_SIG = 0x06054b50;
constexpr uint32_t ZIP_CENTRAL_SIG = 0x02014b50;
constexpr uint32_t ZIP_LOCAL_SIG = 0x04034b50;
constexpr size_t EOCD_SEARCH_BACK_MAX = 65557; // 64K + 22 (EOCD) + margin
constexpr size_t EOCD_SIZE = 22;
constexpr size_t CENTRAL_FIXED_SIZE = 46;
constexpr size_t LOCAL_FIXED_SIZE = 30;

struct ZipEntry {
    std::string name;
    uint16_t compressMethod = 0;
    uint32_t compressedSize = 0;
    uint32_t localHeaderOffset = 0;
};

uint16_t ReadLe16(const uint8_t* p)
{
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

uint32_t ReadLe32(const uint8_t* p)
{
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

bool ReadBytes(int fd, off_t offset, size_t len, std::vector<uint8_t>& out)
{
    out.resize(len);
    ssize_t n = pread(fd, out.data(), len, offset);
    if (n < 0 || static_cast<size_t>(n) != len) {
        return false;
    }
    return true;
}

// Find End of Central Directory record. Returns its file offset or -1.
off_t FindEocd(int fd, off_t fileSize)
{
    size_t searchLen = static_cast<size_t>(std::min<off_t>(EOCD_SEARCH_BACK_MAX, fileSize));
    if (searchLen < EOCD_SIZE) {
        return -1;
    }
    off_t start = fileSize - static_cast<off_t>(searchLen);
    std::vector<uint8_t> buf;
    if (!ReadBytes(fd, start, searchLen, buf)) {
        return -1;
    }
    // scan backwards for EOCD signature
    for (ssize_t i = static_cast<ssize_t>(searchLen) - static_cast<ssize_t>(EOCD_SIZE); i >= 0; i--) {
        uint32_t sig = ReadLe32(buf.data() + i);
        if (sig == ZIP_EOCD_SIG) {
            return start + i;
        }
    }
    return -1;
}

// Parse all central directory entries.
bool ParseZipEntries(const std::string& path, std::vector<ZipEntry>& entries)
{
    int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "zip open failed: %{public}s, errno=%{public}d", path.c_str(), errno);
        return false;
    }
    struct stat st;
    if (fstat(fd, &st) != 0 || st.st_size <= 0) {
        close(fd);
        return false;
    }
    off_t fileSize = st.st_size;
    off_t eocdOff = FindEocd(fd, fileSize);
    if (eocdOff < 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "zip EOCD not found: %{public}s", path.c_str());
        close(fd);
        return false;
    }
    std::vector<uint8_t> eocd;
    if (!ReadBytes(fd, eocdOff, EOCD_SIZE, eocd)) {
        close(fd);
        return false;
    }
    uint16_t totalEntries = ReadLe16(eocd.data() + 10);
    uint32_t cdSize = ReadLe32(eocd.data() + 12);
    uint32_t cdOffset = ReadLe32(eocd.data() + 16);
    if (cdOffset == 0xFFFFFFFF || cdSize == 0xFFFFFFFF) {
        // ZIP64 not supported in this minimal parser
        TAG_LOGE(AAFwkTag::ABILITY, "zip64 not supported: %{public}s", path.c_str());
        close(fd);
        return false;
    }
    std::vector<uint8_t> cd;
    if (!ReadBytes(fd, static_cast<off_t>(cdOffset), cdSize, cd)) {
        close(fd);
        return false;
    }
    size_t pos = 0;
    for (uint16_t i = 0; i < totalEntries; i++) {
        if (pos + CENTRAL_FIXED_SIZE > cd.size()) {
            break;
        }
        const uint8_t* p = cd.data() + pos;
        if (ReadLe32(p) != ZIP_CENTRAL_SIG) {
            break;
        }
        ZipEntry e;
        e.compressMethod = ReadLe16(p + 10);
        e.compressedSize = ReadLe32(p + 20);
        uint16_t nameLen = ReadLe16(p + 28);
        uint16_t extraLen = ReadLe16(p + 30);
        uint16_t commentLen = ReadLe16(p + 32);
        e.localHeaderOffset = ReadLe32(p + 42);
        pos += CENTRAL_FIXED_SIZE;
        if (pos + nameLen > cd.size()) {
            break;
        }
        e.name.assign(reinterpret_cast<const char*>(cd.data() + pos), nameLen);
        entries.push_back(e);
        pos += nameLen + extraLen + commentLen;
    }
    close(fd);
    return true;
}

// Compute the on-disk byte span [start,end) of an entry's file data by reading
// its local file header (local header's name/extra length may differ from
// central directory's). Returns false if anything looks inconsistent.
bool ResolveEntryDataSpan(int fd, const ZipEntry& e, FileOffsetSpan& span)
{
    uint8_t hdr[LOCAL_FIXED_SIZE];
    ssize_t n = pread(fd, hdr, LOCAL_FIXED_SIZE, static_cast<off_t>(e.localHeaderOffset));
    if (n != static_cast<ssize_t>(LOCAL_FIXED_SIZE)) {
        return false;
    }
    if (ReadLe32(hdr) != ZIP_LOCAL_SIG) {
        return false;
    }
    uint16_t nameLen = ReadLe16(hdr + 26);
    uint16_t extraLen = ReadLe16(hdr + 28);
    uint64_t dataOff = static_cast<uint64_t>(e.localHeaderOffset) + LOCAL_FIXED_SIZE + nameLen + extraLen;
    span.start = dataOff;
    span.end = dataOff + e.compressedSize;
    return true;
}

bool IsPlainFileName(const std::string& s)
{
    if (s.empty()) {
        return false;
    }
    if (s.find('/') != std::string::npos || s.find('\\') != std::string::npos) {
        return false;
    }
    if (s == "." || s == "..") {
        return false;
    }
    return true;
}

std::string BaseName(const std::string& p)
{
    auto pos = p.find_last_of('/');
    return pos == std::string::npos ? p : p.substr(pos + 1);
}

bool HasSuffix(const std::string& s, const std::string& suf)
{
    return s.size() >= suf.size() &&
           s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

// Query the caller's own bundle info (hap module level) — no bundle name
// needed, the bundle manager resolves "self" from the calling token.
bool QueryOwnBundleInfo(BundleInfo& info)
{
    auto helper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (helper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "BundleMgrHelper null");
        return false;
    }
    ErrCode ret = helper->GetBundleInfoForSelf(FLAG_WITH_HAP_MODULE, info);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "GetBundleInfoForSelf failed ret=%{public}d", ret);
        return false;
    }
    return true;
}

const HapModuleInfo* FindModule(const BundleInfo& info, const std::string& moduleName)
{
    for (const auto& m : info.hapModuleInfos) {
        if (m.moduleName == moduleName) {
            return &m;
        }
    }
    return nullptr;
}
} // namespace

int32_t ReleaseByFileNames(const std::vector<std::string>& fileNames)
{
    if (fileNames.empty()) {
        TAG_LOGW(AAFwkTag::ABILITY, "ReleaseByFileNames: empty file list");
        return ERR_INVALID_VALUE;
    }
    // 1. validate all inputs (atomic)
    for (const auto& f : fileNames) {
        if (!IsPlainFileName(f)) {
            TAG_LOGW(AAFwkTag::ABILITY, "ReleaseByFileNames: illegal name '%{public}s'", f.c_str());
            return ERR_INVALID_VALUE;
        }
        bool so = HasSuffix(f, ".so");
        bool hap = HasSuffix(f, ".hap");
        bool hsp = HasSuffix(f, ".hsp");
        if (!so && !hap && !hsp) {
            TAG_LOGW(AAFwkTag::ABILITY, "ReleaseByFileNames: unsupported suffix '%{public}s'", f.c_str());
            return ERR_INVALID_VALUE;
        }
    }

    BundleInfo info;
    if (!QueryOwnBundleInfo(info)) {
        return ERR_INTERNAL_ERROR;
    }

    // 2. for hap/hsp, ensure each maps to a hap of this bundle; collect names
    std::vector<std::string> soNames;
    std::vector<std::string> hapBases; // basenames used to match /proc/self/maps
    for (const auto& f : fileNames) {
        if (HasSuffix(f, ".so")) {
            soNames.push_back(f);
            continue;
        }
        // hap/hsp: must match a hap path of this bundle
        bool found = false;
        for (const auto& m : info.hapModuleInfos) {
            if (!m.hapPath.empty() && BaseName(m.hapPath) == f) {
                found = true;
                break;
            }
        }
        if (!found) {
            TAG_LOGW(AAFwkTag::ABILITY, "ReleaseByFileNames: '%{public}s' not in caller bundle",
                f.c_str());
            return ERR_INVALID_VALUE;
        }
        hapBases.push_back(f);
    }

    // 3. execute (all validated)
    int32_t done = 0;
    for (const auto& s : soNames) {
        if (MadviseUtil::MadviseSingleLibrary(s.c_str())) {
            done++;
        }
    }
    if (!hapBases.empty()) {
        done += MadviseUtil::MadviseGeneralFiles(hapBases);
    }
    TAG_LOGI(AAFwkTag::ABILITY,
        "ReleaseByFileNames: requested=%{public}zu, applied=%{public}d",
        fileNames.size(), done);
    return ERR_OK;
}

int32_t ReleaseRawFileByModule(const std::string& moduleName)
{
    if (moduleName.empty()) {
        TAG_LOGW(AAFwkTag::ABILITY, "ReleaseRawFileByModule: empty module");
        return ERR_INVALID_VALUE;
    }
    if (moduleName.find('/') != std::string::npos || moduleName.find('\\') != std::string::npos) {
        TAG_LOGW(AAFwkTag::ABILITY, "ReleaseRawFileByModule: illegal module name");
        return ERR_INVALID_VALUE;
    }
    BundleInfo info;
    if (!QueryOwnBundleInfo(info)) {
        return ERR_INTERNAL_ERROR;
    }
    const std::string bundleName = info.bundleName;
    const HapModuleInfo* mod = FindModule(info, moduleName);
    if (mod == nullptr || mod->hapPath.empty()) {
        TAG_LOGW(AAFwkTag::ABILITY, "ReleaseRawFileByModule: module %{public}s not found in %{public}s",
            moduleName.c_str(), bundleName.c_str());
        return ERR_INVALID_VALUE;
    }
    const std::string& hapPath = mod->hapPath;

    // Scheme A: parse hap zip, collect top-level rawfile entry data spans.
    std::vector<ZipEntry> entries;
    if (!ParseZipEntries(hapPath, entries)) {
        TAG_LOGE(AAFwkTag::ABILITY, "ReleaseRawFileByModule: parse hap failed: %{private}s", hapPath.c_str());
        return ERR_INTERNAL_ERROR;
    }
    std::vector<FileOffsetSpan> spans;
    int fd = open(hapPath.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "open hap failed errno=%{public}d", errno);
        return ERR_INTERNAL_ERROR;
    }
    size_t prefixLen = std::strlen(RAWFILE_DIR_PREFIX);
    for (const auto& e : entries) {
        if (e.name.compare(0, prefixLen, RAWFILE_DIR_PREFIX) != 0) {
            continue;
        }
        // top-level only: after the prefix there must be no further '/'
        std::string rest = e.name.substr(prefixLen);
        if (rest.empty() || rest.find('/') != std::string::npos) {
            continue; // directory entry or nested file -> skip
        }
        FileOffsetSpan span {0, 0};
        if (ResolveEntryDataSpan(fd, e, span) && span.end > span.start) {
            spans.push_back(span);
        }
    }
    close(fd);

    if (spans.empty()) {
        TAG_LOGI(AAFwkTag::ABILITY, "ReleaseRawFileByModule: no top-level rawfile in %{public}s/%{public}s",
            bundleName.c_str(), moduleName.c_str());
        return ERR_OK; // nothing to release is not an error
    }
    int32_t released = MadviseUtil::MadviseFilePagesAtOffsets(BaseName(hapPath), spans);
    TAG_LOGI(AAFwkTag::ABILITY,
        "ReleaseRawFileByModule: bundle=%{public}s module=%{public}s spans=%{public}zu released=%{public}d",
        bundleName.c_str(), moduleName.c_str(), spans.size(), released);
    return ERR_OK;
}

} // namespace FilePageReleaser
} // namespace AbilityRuntime
} // namespace OHOS
