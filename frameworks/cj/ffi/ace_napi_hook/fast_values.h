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

#ifndef ACE_NAPI_HOOK_FAST_VALUES_H
#define ACE_NAPI_HOOK_FAST_VALUES_H

#include <cmath>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <new>

#include "napi_ohos_compat.h"

#if defined(__GNUC__) || defined(__clang__)
#define CJNAPI_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define CJNAPI_ALWAYS_INLINE inline
#endif

namespace ace_napi_hook::fast {

enum class Kind : uint16_t {
    Undefined = 0,
    Null = 1,
    Bool = 2,
    Int32 = 3,
    UInt32 = 4,
    Int64 = 5,
    Double = 6,
};

struct Value {
    uint16_t kind;
    // Byte offset back to ValueChunk enables O(1) Store provenance checks.
    uint16_t pad0;
    // Low 32 bits of the logical owner scope generation. The logical owner
    // can differ from the physical chunk owner after a partial-chunk capture.
    uint32_t pad1;
    union {
        bool b;
        int32_t i32;
        uint32_t u32;
        int64_t i64;
        double f64;
    };
};

namespace detail {

inline constexpr uintptr_t FAST_IMMEDIATE_TAG = 0x8ull << 60;
inline constexpr uintptr_t FAST_POINTER_TAG = 0x9ull << 60;
inline constexpr uintptr_t LAZY_POINTER_TAG = 0xAull << 60;
inline constexpr uintptr_t FAST_TAG_MASK = 0xFull << 60;
inline constexpr uintptr_t FAST_KIND_SHIFT = 56;
inline constexpr uintptr_t FAST_KIND_MASK = 0xFull << FAST_KIND_SHIFT;
inline constexpr uintptr_t FAST_PAYLOAD_MASK = 0x00FFFFFFFFFFFFFFull;
inline constexpr uintptr_t FAST_POINTER_MASK = 0x0FFFFFFFFFFFFFFFull;
inline constexpr size_t CHUNK_SIZE = 4096;
inline constexpr size_t MAX_FREE_CHUNK_BYTES = 2 * 1024 * 1024;
inline constexpr size_t CHUNK_HEADER_SIZE = 32;
inline constexpr size_t VALUES_PER_CHUNK = (CHUNK_SIZE - CHUNK_HEADER_SIZE) / sizeof(Value);
inline constexpr size_t LAZY_CHUNK_DEFAULT_PAYLOAD = 4096;

struct Scope;
struct ScopeList;

struct ValueChunk {
    ValueChunk* next {nullptr};
    Scope* owner {nullptr};
    ScopeList* store {nullptr};
    uint16_t used {0};
    bool sealed {false};
    uint8_t pad0[5] {0, 0, 0, 0, 0};
    Value values[VALUES_PER_CHUNK];
};

static_assert(sizeof(ValueChunk) == CHUNK_SIZE, "fast NAPI value chunk must stay 4KB");

struct LazyChunk {
    LazyChunk* next {nullptr};
    Scope* owner {nullptr};
    size_t capacity {0};
    size_t used {0};
    bool sealed {false};
    alignas(std::max_align_t) unsigned char data[1];
};

struct Scope {
    uintptr_t id {0};
    uint16_t index {0};
    bool escapable {false};
    bool closed {false};
    bool chunks_released {false};
    Scope* prev {nullptr};
    Scope* all_next {nullptr};
    ValueChunk* head {nullptr};
    ValueChunk* current {nullptr};
    struct LazyValue* lazy_head {nullptr};
    LazyChunk* lazy_head_chunk {nullptr};
    LazyChunk* lazy_current_chunk {nullptr};
};

enum class LazyKind : uint16_t {
    Object = 1,
    Array = 2,
    StringUtf8 = 3,
    StringLatin1 = 4,
};

inline constexpr uint16_t LAZY_SEGMENT_INITIAL_CAPACITY = 4;
inline constexpr uint16_t LAZY_SEGMENT_MAX_CAPACITY = 16;

struct LazyNamedPropertyEntry {
    char* name {nullptr};
    size_t name_len {0};
    napi_value value {nullptr};
};

struct LazyNamedPropertySegment {
    LazyNamedPropertySegment* next {nullptr};
    uint16_t used {0};
    uint16_t capacity {0};
    uint32_t pad0 {0};
    LazyNamedPropertyEntry entries[1];
};

struct LazyElementEntry {
    uint32_t index {0};
    uint32_t pad0 {0};
    napi_value value {nullptr};
};

struct LazyElementSegment {
    LazyElementSegment* next {nullptr};
    uint16_t used {0};
    uint16_t capacity {0};
    uint32_t pad0 {0};
    LazyElementEntry entries[1];
};

struct LazyValue {
    uint16_t kind {static_cast<uint16_t>(LazyKind::Object)};
    uint16_t pad0 {0};
    uint32_t array_length {0};
    char* text {nullptr};
    size_t text_len {0};
    LazyNamedPropertySegment* named_head {nullptr};
    LazyNamedPropertySegment* named_tail {nullptr};
    LazyElementSegment* elements_head {nullptr};
    LazyElementSegment* elements_tail {nullptr};
    napi_value materialized {nullptr};
    LazyValue* scope_next {nullptr};
    Scope* scope {nullptr};
    ScopeList* store {nullptr};
};

struct ChunkPool {
    ValueChunk* free_head {nullptr};
    size_t cached_bytes {0};

    ~ChunkPool()
    {
        while (free_head != nullptr) {
            auto* next = free_head->next;
            delete free_head;
            free_head = next;
        }
    }

    ValueChunk* alloc()
    {
        if (free_head != nullptr) {
            auto* chunk = free_head;
            free_head = chunk->next;
            cached_bytes -= CHUNK_SIZE;
            chunk->next = nullptr;
            chunk->owner = nullptr;
            chunk->store = nullptr;
            chunk->used = 0;
            chunk->sealed = false;
            return chunk;
        }
        return new ValueChunk();
    }

    void release(ValueChunk* chunk)
    {
        chunk->owner = nullptr;
        chunk->store = nullptr;
        chunk->used = 0;
        chunk->sealed = false;
        if (cached_bytes + CHUNK_SIZE <= MAX_FREE_CHUNK_BYTES) {
            chunk->next = free_head;
            free_head = chunk;
            cached_bytes += CHUNK_SIZE;
            return;
        }
        delete chunk;
    }
};

struct LazyChunkPool {
    LazyChunk* free_head {nullptr};
    size_t cached_bytes {0};

    ~LazyChunkPool()
    {
        while (free_head != nullptr) {
            auto* next = free_head->next;
            ::operator delete(free_head);
            free_head = next;
        }
    }

    LazyChunk* alloc(size_t payload)
    {
        if (payload <= LAZY_CHUNK_DEFAULT_PAYLOAD && free_head != nullptr) {
            auto* chunk = free_head;
            free_head = chunk->next;
            cached_bytes -= chunk->capacity;
            chunk->next = nullptr;
            chunk->owner = nullptr;
            chunk->used = 0;
            chunk->sealed = false;
            return chunk;
        }
        const size_t capacity = payload <= LAZY_CHUNK_DEFAULT_PAYLOAD ? LAZY_CHUNK_DEFAULT_PAYLOAD : payload;
        auto* mem = ::operator new(sizeof(LazyChunk) - 1 + capacity);
        auto* chunk = static_cast<LazyChunk*>(mem);
        chunk->next = nullptr;
        chunk->owner = nullptr;
        chunk->capacity = capacity;
        chunk->used = 0;
        chunk->sealed = false;
        return chunk;
    }

    void release(LazyChunk* chunk)
    {
        chunk->owner = nullptr;
        chunk->used = 0;
        chunk->sealed = false;
        if (chunk->capacity == LAZY_CHUNK_DEFAULT_PAYLOAD &&
            cached_bytes + chunk->capacity <= MAX_FREE_CHUNK_BYTES) {
            chunk->next = free_head;
            free_head = chunk;
            cached_bytes += chunk->capacity;
            return;
        }
        ::operator delete(chunk);
    }
};

struct ScopeList {
    Scope* all_scopes {nullptr};
    Scope* top {nullptr};
    size_t size {0};
    ChunkPool chunks;
    LazyChunkPool lazy_chunks;

    explicit ScopeList(bool = false) {}

    ~ScopeList()
    {
        while (all_scopes != nullptr) {
            auto* scope = all_scopes;
            all_scopes = scope->all_next;
            releaseChunks(scope);
            delete scope;
        }
    }

    Scope* currentTop()
    {
        while (top != nullptr && top->closed) {
            top = top->prev;
        }
        return top;
    }

    bool push(uintptr_t id, bool escapable)
    {
        auto* scope = new Scope();
        scope->id = id;
        scope->escapable = escapable;
        while (top != nullptr && top->closed) {
            top = top->prev;
        }
        scope->prev = top;
        scope->index = top == nullptr ? 1 : static_cast<uint16_t>(top->index + 1);
        scope->all_next = all_scopes;
        all_scopes = scope;
        top = scope;
        ++size;
        return true;
    }

    bool removeTop(uintptr_t id)
    {
        if (top == nullptr || top->id != id) {
            return false;
        }
        auto* scope = top;
        top = scope->prev;
        unlinkAllScope(scope);
        releaseChunks(scope);
        delete scope;
        --size;
        return true;
    }

    bool remove(uintptr_t id)
    {
        Scope* prev = nullptr;
        auto* scope = top;
        while (scope != nullptr) {
            if (scope->closed) {
                if (prev == nullptr) {
                    top = scope->prev;
                    scope = top;
                } else {
                    prev->prev = scope->prev;
                    scope = prev->prev;
                }
                continue;
            }
            if (scope->id == id) {
                if (prev == nullptr) {
                    top = scope->prev;
                } else {
                    prev->prev = scope->prev;
                }
                unlinkAllScope(scope);
                releaseChunks(scope);
                delete scope;
                --size;
                return true;
            }
            prev = scope;
            scope = scope->prev;
        }
        return false;
    }

    bool closeAny(uintptr_t id)
    {
        auto* scope = all_scopes;
        while (scope != nullptr) {
            if (!scope->closed && scope->id == id) {
                scope->closed = true;
                --size;
                return true;
            }
            scope = scope->all_next;
        }
        return false;
    }

    bool contains(uintptr_t id) const
    {
        auto* scope = top;
        while (scope != nullptr) {
            if (scope->id == id) {
                return true;
            }
            scope = scope->prev;
        }
        return false;
    }

    bool containsScope(const Scope* target) const
    {
        auto* scope = all_scopes;
        while (scope != nullptr) {
            if (scope == target && !scope->closed) {
                return true;
            }
            scope = scope->all_next;
        }
        return false;
    }

    Scope* scopeAtIndex(uint16_t index) const
    {
        auto* scope = top;
        while (scope != nullptr) {
            if (!scope->closed && scope->index == index) {
                return scope;
            }
            scope = scope->prev;
        }
        return nullptr;
    }

    void unlinkAllScope(Scope* target)
    {
        Scope* prev = nullptr;
        auto* scope = all_scopes;
        while (scope != nullptr) {
            if (scope == target) {
                if (prev == nullptr) {
                    all_scopes = scope->all_next;
                } else {
                    prev->all_next = scope->all_next;
                }
                return;
            }
            prev = scope;
            scope = scope->all_next;
        }
    }

    void releaseChunks(Scope* scope)
    {
        if (scope->chunks_released) {
            return;
        }
        scope->chunks_released = true;
        scope->lazy_head = nullptr;
        auto* lazy_chunk = scope->lazy_head_chunk;
        while (lazy_chunk != nullptr) {
            auto* next = lazy_chunk->next;
            lazy_chunks.release(lazy_chunk);
            lazy_chunk = next;
        }
        scope->lazy_head_chunk = nullptr;
        scope->lazy_current_chunk = nullptr;
        auto* chunk = scope->head;
        while (chunk != nullptr) {
            auto* next = chunk->next;
            chunks.release(chunk);
            chunk = next;
        }
        scope->head = nullptr;
        scope->current = nullptr;
    }
};

}  // namespace detail

using Store = detail::ScopeList;

namespace detail {

CJNAPI_ALWAYS_INLINE bool IsAncestorOrSelf(const Scope* ancestor, const Scope* scope)
{
    auto* current = scope;
    while (current != nullptr) {
        if (current == ancestor) {
            return true;
        }
        current = current->prev;
    }
    return false;
}

CJNAPI_ALWAYS_INLINE ValueChunk* FindValueChunk(Store* store, const Value* value)
{
    if (store == nullptr || value == nullptr) {
        return nullptr;
    }
    constexpr size_t values_offset = offsetof(ValueChunk, values);
    const size_t offset = value->pad0;
    if (offset < values_offset || offset > CHUNK_SIZE - sizeof(Value) ||
        (offset - values_offset) % sizeof(Value) != 0) {
        return nullptr;
    }
    const auto address = reinterpret_cast<uintptr_t>(value);
    auto* chunk = reinterpret_cast<ValueChunk*>(address - offset);
    const size_t index = (offset - values_offset) / sizeof(Value);
    return chunk->store == store && index < chunk->used && &chunk->values[index] == value ? chunk : nullptr;
}

CJNAPI_ALWAYS_INLINE Scope* GetValueOwnerScope(Store* store, const Value* value)
{
    if (FindValueChunk(store, value) == nullptr) {
        return nullptr;
    }
    auto* scope = store->all_scopes;
    while (scope != nullptr) {
        if (!scope->closed && static_cast<uint32_t>(scope->id) == value->pad1) {
            return scope;
        }
        scope = scope->all_next;
    }
    return nullptr;
}

CJNAPI_ALWAYS_INLINE void SetValueOwnerScope(Value* value, const Scope* scope)
{
    value->pad1 = scope == nullptr ? 0 : static_cast<uint32_t>(scope->id);
}

CJNAPI_ALWAYS_INLINE LazyChunk* FindLazyChunk(Store* store, const void* value)
{
    if (store == nullptr || value == nullptr) {
        return nullptr;
    }
    const auto address = reinterpret_cast<uintptr_t>(value);
    auto* scope = store->all_scopes;
    while (scope != nullptr) {
        auto* chunk = scope->lazy_head_chunk;
        while (chunk != nullptr) {
            const auto begin = reinterpret_cast<uintptr_t>(chunk->data);
            const auto end = reinterpret_cast<uintptr_t>(chunk->data + chunk->used);
            if (address >= begin && address < end) {
                return chunk;
            }
            chunk = chunk->next;
        }
        scope = scope->all_next;
    }
    return nullptr;
}

CJNAPI_ALWAYS_INLINE bool IsKnownLazyValue(Store* store, const LazyValue* value)
{
    if (store == nullptr || value == nullptr) {
        return false;
    }
    auto* scope = store->all_scopes;
    while (scope != nullptr) {
        auto* lazy = scope->lazy_head;
        while (lazy != nullptr) {
            if (lazy == value) {
                return true;
            }
            lazy = lazy->scope_next;
        }
        scope = scope->all_next;
    }
    return false;
}

CJNAPI_ALWAYS_INLINE bool UnlinkValueChunk(ValueChunk* chunk)
{
    if (chunk == nullptr || chunk->owner == nullptr) {
        return false;
    }
    auto* owner = chunk->owner;
    ValueChunk* previous = nullptr;
    auto* current = owner->head;
    while (current != nullptr && current != chunk) {
        previous = current;
        current = current->next;
    }
    if (current == nullptr) {
        return false;
    }
    if (previous == nullptr) {
        owner->head = current->next;
    } else {
        previous->next = current->next;
    }
    if (owner->current == current) {
        owner->current = nullptr;
    }
    current->next = nullptr;
    return true;
}

CJNAPI_ALWAYS_INLINE bool UnlinkLazyChunk(LazyChunk* chunk)
{
    if (chunk == nullptr || chunk->owner == nullptr) {
        return false;
    }
    auto* owner = chunk->owner;
    LazyChunk* previous = nullptr;
    auto* current = owner->lazy_head_chunk;
    while (current != nullptr && current != chunk) {
        previous = current;
        current = current->next;
    }
    if (current == nullptr) {
        return false;
    }
    if (previous == nullptr) {
        owner->lazy_head_chunk = current->next;
    } else {
        previous->next = current->next;
    }
    if (owner->lazy_current_chunk == current) {
        owner->lazy_current_chunk = nullptr;
    }
    current->next = nullptr;
    return true;
}

CJNAPI_ALWAYS_INLINE bool CanPromoteOwnerToScope(Store* store, const Scope* owner, const Scope* target)
{
    if (store == nullptr || owner == nullptr || target == nullptr ||
        !store->containsScope(owner) || !store->containsScope(target)) {
        return false;
    }
    return IsAncestorOrSelf(owner, target) || IsAncestorOrSelf(target, owner);
}

CJNAPI_ALWAYS_INLINE bool PromoteValueChunk(Store* store, ValueChunk* chunk, Scope* target)
{
    if (chunk == nullptr || !CanPromoteOwnerToScope(store, chunk->owner, target)) {
        return false;
    }
    if (IsAncestorOrSelf(chunk->owner, target)) {
        return true;
    }
    if (!UnlinkValueChunk(chunk)) {
        return false;
    }
    chunk->owner = target;
    chunk->sealed = true;
    chunk->next = target->head;
    target->head = chunk;
    return true;
}

CJNAPI_ALWAYS_INLINE bool PromoteLazyChunk(Store* store, LazyChunk* chunk, Scope* target)
{
    if (chunk == nullptr || !CanPromoteOwnerToScope(store, chunk->owner, target)) {
        return false;
    }
    if (IsAncestorOrSelf(chunk->owner, target)) {
        return true;
    }
    if (!UnlinkLazyChunk(chunk)) {
        return false;
    }
    chunk->owner = target;
    chunk->sealed = true;
    chunk->next = target->lazy_head_chunk;
    target->lazy_head_chunk = chunk;
    return true;
}

CJNAPI_ALWAYS_INLINE bool PromoteLazyValue(Store* store, LazyValue* value, Scope* target)
{
    if (store == nullptr || value == nullptr || value->store != store || value->scope == nullptr ||
        !IsKnownLazyValue(store, value) || !CanPromoteOwnerToScope(store, value->scope, target)) {
        return false;
    }
    auto* owner = value->scope;
    if (IsAncestorOrSelf(owner, target)) {
        return true;
    }
    LazyValue* previous = nullptr;
    auto* current = owner->lazy_head;
    while (current != nullptr && current != value) {
        previous = current;
        current = current->scope_next;
    }
    if (current == nullptr) {
        return false;
    }
    if (previous == nullptr) {
        owner->lazy_head = current->scope_next;
    } else {
        previous->scope_next = current->scope_next;
    }
    current->scope = target;
    current->scope_next = target->lazy_head;
    target->lazy_head = current;
    return true;
}

CJNAPI_ALWAYS_INLINE ValueChunk* AddChunk(Store& store, Scope& scope)
{
    auto* chunk = store.chunks.alloc();
    chunk->owner = &scope;
    chunk->store = &store;
    chunk->sealed = false;
    chunk->next = scope.head;
    scope.head = chunk;
    scope.current = chunk;
    return chunk;
}

CJNAPI_ALWAYS_INLINE Value* AllocateUnlocked(Store* store, Kind kind)
{
    if (store == nullptr) {
        return nullptr;
    }
    auto* scope = store->currentTop();
    if (scope == nullptr) {
        return nullptr;
    }
    auto* chunk = scope->current;
    if (chunk == nullptr || chunk->used == VALUES_PER_CHUNK) {
        chunk = AddChunk(*store, *scope);
    }
    auto* raw = &chunk->values[chunk->used++];
    raw->kind = static_cast<uint16_t>(kind);
    raw->pad0 = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(raw) -
        reinterpret_cast<uintptr_t>(chunk));
    SetValueOwnerScope(raw, scope);
    return raw;
}

CJNAPI_ALWAYS_INLINE Value* Allocate(Store* store, Kind kind)
{
    return AllocateUnlocked(store, kind);
}

CJNAPI_ALWAYS_INLINE size_t AlignLazySize(size_t size)
{
    constexpr size_t align = alignof(std::max_align_t);
    return (size + align - 1) & ~(align - 1);
}

CJNAPI_ALWAYS_INLINE void* AllocateLazyBytesInScopeRawUnlocked(Store* store, Scope* scope, size_t size)
{
    if (store == nullptr || scope == nullptr || size == 0) {
        return nullptr;
    }
    size = AlignLazySize(size);
    auto* chunk = scope->lazy_current_chunk;
    if (chunk == nullptr || chunk->used + size > chunk->capacity) {
        chunk = store->lazy_chunks.alloc(size);
        chunk->owner = scope;
        chunk->sealed = false;
        chunk->next = scope->lazy_head_chunk;
        scope->lazy_head_chunk = chunk;
        scope->lazy_current_chunk = chunk;
    }
    auto* out = chunk->data + chunk->used;
    chunk->used += size;
    return out;
}

CJNAPI_ALWAYS_INLINE void* AllocateLazyBytesInScopeUnlocked(Store* store, Scope* scope, size_t size)
{
    auto* out = AllocateLazyBytesInScopeRawUnlocked(store, scope, size);
    if (out != nullptr) {
        std::memset(out, 0, AlignLazySize(size));
    }
    return out;
}

CJNAPI_ALWAYS_INLINE void* AllocateLazyBytesRawUnlocked(Store* store, size_t size)
{
    if (store == nullptr) {
        return nullptr;
    }
    return AllocateLazyBytesInScopeRawUnlocked(store, store->currentTop(), size);
}

CJNAPI_ALWAYS_INLINE void* AllocateLazyBytesUnlocked(Store* store, size_t size)
{
    if (store == nullptr) {
        return nullptr;
    }
    return AllocateLazyBytesInScopeUnlocked(store, store->currentTop(), size);
}

CJNAPI_ALWAYS_INLINE void* AllocateLazyBytes(Store* store, size_t size)
{
    return AllocateLazyBytesUnlocked(store, size);
}

template <typename T>
CJNAPI_ALWAYS_INLINE bool InRangeInt(double value)
{
    return value >= static_cast<double>(std::numeric_limits<T>::min()) &&
           value <= static_cast<double>(std::numeric_limits<T>::max()) &&
           std::isfinite(value);
}

CJNAPI_ALWAYS_INLINE uint32_t ToUInt32(double value)
{
    if (!std::isfinite(value) || value == 0.0) {
        return 0;
    }
    auto truncated = std::trunc(value);
    auto wrapped = std::fmod(truncated, 4294967296.0);
    if (wrapped < 0.0) {
        wrapped += 4294967296.0;
    }
    return static_cast<uint32_t>(wrapped);
}

}  // namespace detail

CJNAPI_ALWAYS_INLINE bool IsFastValue(napi_value value)
{
    auto raw = reinterpret_cast<uintptr_t>(value);
    auto tag = raw & detail::FAST_TAG_MASK;
    return tag == detail::FAST_IMMEDIATE_TAG || tag == detail::FAST_POINTER_TAG;
}

CJNAPI_ALWAYS_INLINE bool IsLazyValue(napi_value value)
{
    auto raw = reinterpret_cast<uintptr_t>(value);
    return (raw & detail::FAST_TAG_MASK) == detail::LAZY_POINTER_TAG;
}

CJNAPI_ALWAYS_INLINE bool IsImmediateValue(napi_value value)
{
    auto raw = reinterpret_cast<uintptr_t>(value);
    return (raw & detail::FAST_TAG_MASK) == detail::FAST_IMMEDIATE_TAG;
}

CJNAPI_ALWAYS_INLINE bool IsPointerValue(napi_value value)
{
    auto raw = reinterpret_cast<uintptr_t>(value);
    return (raw & detail::FAST_TAG_MASK) == detail::FAST_POINTER_TAG;
}

CJNAPI_ALWAYS_INLINE Kind DecodeKind(napi_value value)
{
    if (IsPointerValue(value)) {
        auto* raw = reinterpret_cast<Value*>(reinterpret_cast<uintptr_t>(value) & detail::FAST_POINTER_MASK);
        return static_cast<Kind>(raw->kind);
    }
    auto raw = reinterpret_cast<uintptr_t>(value);
    return static_cast<Kind>((raw & detail::FAST_KIND_MASK) >> detail::FAST_KIND_SHIFT);
}

CJNAPI_ALWAYS_INLINE uint64_t DecodePayload(napi_value value)
{
    auto raw = reinterpret_cast<uintptr_t>(value);
    return raw & detail::FAST_PAYLOAD_MASK;
}

CJNAPI_ALWAYS_INLINE Value* DecodePointer(napi_value value)
{
    return reinterpret_cast<Value*>(reinterpret_cast<uintptr_t>(value) & detail::FAST_POINTER_MASK);
}

CJNAPI_ALWAYS_INLINE detail::LazyValue* DecodeLazyPointer(napi_value value)
{
    return reinterpret_cast<detail::LazyValue*>(reinterpret_cast<uintptr_t>(value) & detail::FAST_POINTER_MASK);
}

CJNAPI_ALWAYS_INLINE napi_value GetMaterializedHandle(napi_value value)
{
    if (!IsLazyValue(value)) {
        return nullptr;
    }
    auto* lazy = DecodeLazyPointer(value);
    return lazy == nullptr ? nullptr : lazy->materialized;
}

CJNAPI_ALWAYS_INLINE bool HasMaterializedHandle(napi_value value)
{
    return GetMaterializedHandle(value) != nullptr;
}

CJNAPI_ALWAYS_INLINE bool IsUnmaterializedLazyValue(napi_value value)
{
    return IsLazyValue(value) && !HasMaterializedHandle(value);
}

CJNAPI_ALWAYS_INLINE napi_value EncodeImmediate(Kind kind, uint64_t payload)
{
    auto raw = detail::FAST_IMMEDIATE_TAG | (static_cast<uintptr_t>(kind) << detail::FAST_KIND_SHIFT) |
        (payload & detail::FAST_PAYLOAD_MASK);
    return reinterpret_cast<napi_value>(raw);
}

CJNAPI_ALWAYS_INLINE napi_value EncodePointer(Value* value)
{
    auto raw = detail::FAST_POINTER_TAG | (reinterpret_cast<uintptr_t>(value) & detail::FAST_POINTER_MASK);
    return reinterpret_cast<napi_value>(raw);
}

CJNAPI_ALWAYS_INLINE napi_value EncodeLazyPointer(detail::LazyValue* value)
{
    auto raw = detail::LAZY_POINTER_TAG | (reinterpret_cast<uintptr_t>(value) & detail::FAST_POINTER_MASK);
    return reinterpret_cast<napi_value>(raw);
}

CJNAPI_ALWAYS_INLINE bool OpenScope(Store* store, uintptr_t scope_id, bool escapable)
{
    if (store == nullptr) {
        return false;
    }
    return store->push(scope_id, escapable);
}

CJNAPI_ALWAYS_INLINE bool HasOpenScope(Store* store)
{
    return store != nullptr && store->currentTop() != nullptr;
}

CJNAPI_ALWAYS_INLINE bool CanCloseScope(Store* store, uintptr_t scope_id)
{
    if (store == nullptr) {
        return false;
    }
    return store->contains(scope_id);
}

CJNAPI_ALWAYS_INLINE bool CloseScope(Store* store, uintptr_t scope_id)
{
    if (store == nullptr) {
        return false;
    }
    return store->remove(scope_id) || store->closeAny(scope_id);
}

CJNAPI_ALWAYS_INLINE detail::LazyValue* AllocateLazyUnlocked(Store* store, detail::LazyKind kind)
{
    if (store == nullptr || store->currentTop() == nullptr) {
        return nullptr;
    }
    auto* scope = store->currentTop();
    auto* value = static_cast<detail::LazyValue*>(detail::AllocateLazyBytesRawUnlocked(store, sizeof(detail::LazyValue)));
    if (value == nullptr) {
        return nullptr;
    }
    value->kind = static_cast<uint16_t>(kind);
    value->pad0 = 0;
    value->array_length = 0;
    value->text = nullptr;
    value->text_len = 0;
    value->named_head = nullptr;
    value->named_tail = nullptr;
    value->elements_head = nullptr;
    value->elements_tail = nullptr;
    value->materialized = nullptr;
    value->scope = scope;
    value->store = store;
    value->scope_next = scope->lazy_head;
    scope->lazy_head = value;
    return value;
}

CJNAPI_ALWAYS_INLINE detail::LazyValue* AllocateLazy(Store* store, detail::LazyKind kind)
{
    return AllocateLazyUnlocked(store, kind);
}

CJNAPI_ALWAYS_INLINE bool IsLazyObjectLike(const detail::LazyValue* lazy)
{
    if (lazy == nullptr) {
        return false;
    }
    if (lazy->materialized != nullptr) {
        return false;
    }
    return lazy->kind == static_cast<uint16_t>(detail::LazyKind::Object) ||
        lazy->kind == static_cast<uint16_t>(detail::LazyKind::Array);
}

CJNAPI_ALWAYS_INLINE char* CopyBytesUnlocked(detail::LazyValue* lazy, const char* src, size_t len)
{
    if (lazy == nullptr) {
        return nullptr;
    }
    auto* out = static_cast<char*>(detail::AllocateLazyBytesInScopeRawUnlocked(lazy->store, lazy->scope, len + 1));
    if (out == nullptr) {
        return nullptr;
    }
    if (len > 0) {
        std::memcpy(out, src, len);
    }
    out[len] = '\0';
    return out;
}

CJNAPI_ALWAYS_INLINE char* CopyBytes(detail::LazyValue* lazy, const char* src, size_t len)
{
    if (lazy == nullptr || lazy->store == nullptr) {
        return nullptr;
    }
    return CopyBytesUnlocked(lazy, src, len);
}

CJNAPI_ALWAYS_INLINE uint16_t NextLazySegmentCapacity(uint16_t current)
{
    if (current == 0) {
        return detail::LAZY_SEGMENT_INITIAL_CAPACITY;
    }
    const uint32_t next = static_cast<uint32_t>(current) * 2u;
    return static_cast<uint16_t>(
        next > detail::LAZY_SEGMENT_MAX_CAPACITY ? detail::LAZY_SEGMENT_MAX_CAPACITY : next);
}

CJNAPI_ALWAYS_INLINE detail::LazyNamedPropertySegment* AddNamedPropertySegment(detail::LazyValue* lazy)
{
    if (lazy == nullptr || lazy->store == nullptr || lazy->scope == nullptr) {
        return nullptr;
    }
    const uint16_t capacity = NextLazySegmentCapacity(lazy->named_tail == nullptr ? 0 : lazy->named_tail->capacity);
    const size_t bytes = sizeof(detail::LazyNamedPropertySegment) +
        (static_cast<size_t>(capacity) - 1u) * sizeof(detail::LazyNamedPropertyEntry);
    auto* segment = static_cast<detail::LazyNamedPropertySegment*>(
        detail::AllocateLazyBytesInScopeRawUnlocked(lazy->store, lazy->scope, bytes));
    if (segment == nullptr) {
        return nullptr;
    }
    segment->next = nullptr;
    segment->used = 0;
    segment->capacity = capacity;
    segment->pad0 = 0;
    if (lazy->named_tail == nullptr) {
        lazy->named_head = segment;
    } else {
        lazy->named_tail->next = segment;
    }
    lazy->named_tail = segment;
    return segment;
}

CJNAPI_ALWAYS_INLINE detail::LazyElementSegment* AddElementSegment(detail::LazyValue* lazy)
{
    if (lazy == nullptr || lazy->store == nullptr || lazy->scope == nullptr) {
        return nullptr;
    }
    const uint16_t capacity = NextLazySegmentCapacity(lazy->elements_tail == nullptr ? 0 : lazy->elements_tail->capacity);
    const size_t bytes = sizeof(detail::LazyElementSegment) +
        (static_cast<size_t>(capacity) - 1u) * sizeof(detail::LazyElementEntry);
    auto* segment = static_cast<detail::LazyElementSegment*>(
        detail::AllocateLazyBytesInScopeRawUnlocked(lazy->store, lazy->scope, bytes));
    if (segment == nullptr) {
        return nullptr;
    }
    segment->next = nullptr;
    segment->used = 0;
    segment->capacity = capacity;
    segment->pad0 = 0;
    if (lazy->elements_tail == nullptr) {
        lazy->elements_head = segment;
    } else {
        lazy->elements_tail->next = segment;
    }
    lazy->elements_tail = segment;
    return segment;
}

CJNAPI_ALWAYS_INLINE detail::LazyNamedPropertyEntry* FindNamedProperty(detail::LazyValue* lazy, const char* name, size_t name_len)
{
    auto* segment = lazy == nullptr ? nullptr : lazy->named_head;
    while (segment != nullptr) {
        for (uint16_t i = 0; i < segment->used; ++i) {
            auto& prop = segment->entries[i];
            if (prop.name_len == name_len && std::memcmp(prop.name, name, name_len) == 0) {
                return &prop;
            }
        }
        segment = segment->next;
    }
    return nullptr;
}

CJNAPI_ALWAYS_INLINE detail::LazyElementEntry* FindElement(detail::LazyValue* lazy, uint32_t index)
{
    auto* segment = lazy == nullptr ? nullptr : lazy->elements_head;
    while (segment != nullptr) {
        for (uint16_t i = 0; i < segment->used; ++i) {
            auto& element = segment->entries[i];
            if (element.index == index) {
                return &element;
            }
        }
        segment = segment->next;
    }
    return nullptr;
}

struct NamedPropertyReservation {
    detail::LazyValue* lazy {nullptr};
    detail::LazyNamedPropertySegment* segment {nullptr};
    detail::LazyNamedPropertyEntry* entry {nullptr};
    bool existing {false};
};

struct ElementReservation {
    detail::LazyValue* lazy {nullptr};
    detail::LazyElementSegment* segment {nullptr};
    detail::LazyElementEntry* entry {nullptr};
    uint32_t index {0};
    bool existing {false};
};

CJNAPI_ALWAYS_INLINE napi_status PrepareNamedProperty(
    napi_value object, const char* name, NamedPropertyReservation* reservation)
{
    if (!IsUnmaterializedLazyValue(object) || name == nullptr || reservation == nullptr) {
        return napi_invalid_arg;
    }
    auto* lazy = DecodeLazyPointer(object);
    if (lazy == nullptr || lazy->store == nullptr || !IsLazyObjectLike(lazy)) {
        return napi_invalid_arg;
    }
    const size_t name_len = std::strlen(name);
    if (auto* prop = FindNamedProperty(lazy, name, name_len); prop != nullptr) {
        *reservation = {lazy, nullptr, prop, true};
        return napi_ok;
    }
    auto* segment = lazy->named_tail;
    if (segment == nullptr || segment->used == segment->capacity) {
        segment = AddNamedPropertySegment(lazy);
    }
    if (segment == nullptr || segment->used >= segment->capacity) {
        return napi_generic_failure;
    }
    auto* prop = &segment->entries[segment->used];
    auto* copied_name = CopyBytesUnlocked(lazy, name, name_len);
    if (copied_name == nullptr) {
        return napi_generic_failure;
    }
    prop->name = copied_name;
    prop->name_len = name_len;
    prop->value = nullptr;
    *reservation = {lazy, segment, prop, false};
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE void CommitNamedProperty(
    const NamedPropertyReservation& reservation, napi_value value)
{
    reservation.entry->value = value;
    if (!reservation.existing) {
        ++reservation.segment->used;
    }
}

CJNAPI_ALWAYS_INLINE napi_status PrepareElement(
    napi_value object, uint32_t index, ElementReservation* reservation)
{
    if (!IsUnmaterializedLazyValue(object) || reservation == nullptr) {
        return napi_invalid_arg;
    }
    auto* lazy = DecodeLazyPointer(object);
    if (lazy == nullptr || lazy->store == nullptr || !IsLazyObjectLike(lazy)) {
        return napi_invalid_arg;
    }
    if (auto* element = FindElement(lazy, index); element != nullptr) {
        *reservation = {lazy, nullptr, element, index, true};
        return napi_ok;
    }
    auto* segment = lazy->elements_tail;
    if (segment == nullptr || segment->used == segment->capacity) {
        segment = AddElementSegment(lazy);
    }
    if (segment == nullptr || segment->used >= segment->capacity) {
        return napi_generic_failure;
    }
    auto* element = &segment->entries[segment->used];
    element->index = index;
    element->pad0 = 0;
    element->value = nullptr;
    *reservation = {lazy, segment, element, index, false};
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE void CommitElement(const ElementReservation& reservation, napi_value value)
{
    reservation.entry->value = value;
    if (!reservation.existing) {
        ++reservation.segment->used;
    }
    if (reservation.lazy->kind == static_cast<uint16_t>(detail::LazyKind::Array) &&
        reservation.index >= reservation.lazy->array_length) {
        reservation.lazy->array_length = reservation.index + 1;
    }
}

CJNAPI_ALWAYS_INLINE napi_status CreateObject(Store* store, napi_value* result)
{
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    auto* value = AllocateLazy(store, detail::LazyKind::Object);
    if (value == nullptr) {
        return napi_handle_scope_mismatch;
    }
    *result = EncodeLazyPointer(value);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status CreateArray(Store* store, uint32_t length, napi_value* result)
{
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    auto* value = AllocateLazy(store, detail::LazyKind::Array);
    if (value == nullptr) {
        return napi_handle_scope_mismatch;
    }
    value->array_length = length;
    *result = EncodeLazyPointer(value);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status CreateStringUnlocked(Store* store, detail::LazyKind kind, const char* str, size_t len, napi_value* result)
{
    if (result == nullptr || str == nullptr) {
        return napi_invalid_arg;
    }
    if (len == NAPI_AUTO_LENGTH) {
        len = std::strlen(str);
    }
    constexpr size_t MAX_FAST_STRING_LENGTH = 134217728;
    if (len > MAX_FAST_STRING_LENGTH) {
        return napi_invalid_arg;
    }
    if (store == nullptr) {
        return napi_handle_scope_mismatch;
    }
    auto* value = AllocateLazyUnlocked(store, kind);
    if (value == nullptr) {
        return napi_handle_scope_mismatch;
    }
    value->text = CopyBytesUnlocked(value, str, len);
    if (value->text == nullptr) {
        return napi_generic_failure;
    }
    value->text_len = len;
    *result = EncodeLazyPointer(value);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status CreateString(Store* store, detail::LazyKind kind, const char* str, size_t len, napi_value* result)
{
    return CreateStringUnlocked(store, kind, str, len, result);
}

CJNAPI_ALWAYS_INLINE bool IsLazyString(napi_value value)
{
    if (!IsUnmaterializedLazyValue(value)) {
        return false;
    }
    auto* lazy = DecodeLazyPointer(value);
    if (lazy == nullptr || lazy->materialized != nullptr) {
        return false;
    }
    return lazy != nullptr &&
        (lazy->kind == static_cast<uint16_t>(detail::LazyKind::StringUtf8) ||
         lazy->kind == static_cast<uint16_t>(detail::LazyKind::StringLatin1));
}

CJNAPI_ALWAYS_INLINE const char* LazyStringData(napi_value value)
{
    if (!IsLazyString(value)) {
        return nullptr;
    }
    return DecodeLazyPointer(value)->text;
}

CJNAPI_ALWAYS_INLINE size_t LazyStringSize(napi_value value)
{
    if (!IsLazyString(value)) {
        return 0;
    }
    return DecodeLazyPointer(value)->text_len;
}

CJNAPI_ALWAYS_INLINE napi_status GetString(napi_value value, char* buf, size_t bufsize, size_t* result)
{
    auto* text = LazyStringData(value);
    if (text == nullptr) {
        return napi_invalid_arg;
    }
    const size_t text_len = LazyStringSize(value);
    if (result != nullptr) {
        *result = text_len;
    }
    if (buf != nullptr && bufsize > 0) {
        const size_t copy_len = text_len < (bufsize - 1) ? text_len : (bufsize - 1);
        if (copy_len > 0) {
            std::memcpy(buf, text, copy_len);
        }
        buf[copy_len] = '\0';
    }
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status IsArray(napi_value value, bool* result)
{
    if (result == nullptr || !IsUnmaterializedLazyValue(value)) {
        return napi_invalid_arg;
    }
    auto* lazy = DecodeLazyPointer(value);
    *result = lazy != nullptr && lazy->kind == static_cast<uint16_t>(detail::LazyKind::Array);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status GetArrayLength(napi_value value, uint32_t* result)
{
    if (result == nullptr || !IsUnmaterializedLazyValue(value)) {
        return napi_invalid_arg;
    }
    auto* lazy = DecodeLazyPointer(value);
    if (lazy == nullptr || lazy->kind != static_cast<uint16_t>(detail::LazyKind::Array)) {
        return napi_invalid_arg;
    }
    *result = lazy->array_length;
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status TypeofLazy(napi_value value, napi_valuetype* result)
{
    if (result == nullptr || !IsLazyValue(value)) {
        return napi_invalid_arg;
    }
    auto* lazy = DecodeLazyPointer(value);
    if (lazy == nullptr || lazy->store == nullptr) {
        return napi_invalid_arg;
    }
    if (lazy->materialized != nullptr) {
        return napi_invalid_arg;
    }
    *result = (lazy->kind == static_cast<uint16_t>(detail::LazyKind::StringUtf8) ||
               lazy->kind == static_cast<uint16_t>(detail::LazyKind::StringLatin1)) ? napi_string : napi_object;
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status SetNamedPropertyUnlocked(napi_value object, const char* name, napi_value value)
{
    NamedPropertyReservation reservation;
    auto status = PrepareNamedProperty(object, name, &reservation);
    if (status != napi_ok) {
        return status;
    }
    CommitNamedProperty(reservation, value);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status SetNamedProperty(napi_value object, const char* name, napi_value value)
{
    return SetNamedPropertyUnlocked(object, name, value);
}

CJNAPI_ALWAYS_INLINE napi_status GetNamedPropertyUnlocked(napi_value object, const char* name, napi_value* result)
{
    if (!IsUnmaterializedLazyValue(object) || name == nullptr || result == nullptr) {
        return napi_invalid_arg;
    }
    auto* lazy = DecodeLazyPointer(object);
    if (lazy == nullptr || lazy->store == nullptr) {
        return napi_invalid_arg;
    }
    if (!IsLazyObjectLike(lazy)) {
        return napi_invalid_arg;
    }
    auto* prop = FindNamedProperty(lazy, name, std::strlen(name));
    if (prop != nullptr) {
        *result = prop->value;
        return napi_ok;
    }
    *result = EncodeImmediate(Kind::Undefined, 0);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status GetNamedProperty(napi_value object, const char* name, napi_value* result)
{
    return GetNamedPropertyUnlocked(object, name, result);
}

CJNAPI_ALWAYS_INLINE napi_status HasNamedPropertyUnlocked(napi_value object, const char* name, bool* result)
{
    if (!IsUnmaterializedLazyValue(object) || name == nullptr || result == nullptr) {
        return napi_invalid_arg;
    }
    auto* lazy = DecodeLazyPointer(object);
    if (lazy == nullptr || lazy->store == nullptr) {
        return napi_invalid_arg;
    }
    if (!IsLazyObjectLike(lazy)) {
        return napi_invalid_arg;
    }
    *result = FindNamedProperty(lazy, name, std::strlen(name)) != nullptr;
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status HasNamedProperty(napi_value object, const char* name, bool* result)
{
    return HasNamedPropertyUnlocked(object, name, result);
}

CJNAPI_ALWAYS_INLINE napi_status DeleteNamedPropertyUnlocked(napi_value object, const char* name, bool* result)
{
    if (!IsUnmaterializedLazyValue(object) || name == nullptr) {
        return napi_invalid_arg;
    }
    auto* lazy = DecodeLazyPointer(object);
    if (lazy == nullptr || lazy->store == nullptr) {
        return napi_invalid_arg;
    }
    if (!IsLazyObjectLike(lazy)) {
        return napi_invalid_arg;
    }
    bool deleted = false;
    const size_t name_len = std::strlen(name);
    auto* segment = lazy->named_head;
    while (segment != nullptr) {
        uint16_t i = 0;
        while (i < segment->used) {
            auto& prop = segment->entries[i];
            if (prop.name_len == name_len && std::memcmp(prop.name, name, name_len) == 0) {
                const uint16_t last = segment->used - 1;
                for (uint16_t j = i; j < last; ++j) {
                    segment->entries[j] = segment->entries[j + 1];
                }
                --segment->used;
                deleted = true;
            } else {
                ++i;
            }
        }
        segment = segment->next;
    }
    if (result != nullptr) {
        *result = deleted;
    }
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status DeleteNamedProperty(napi_value object, const char* name, bool* result)
{
    return DeleteNamedPropertyUnlocked(object, name, result);
}

CJNAPI_ALWAYS_INLINE napi_status SetElementUnlocked(napi_value object, uint32_t index, napi_value value)
{
    ElementReservation reservation;
    auto status = PrepareElement(object, index, &reservation);
    if (status != napi_ok) {
        return status;
    }
    CommitElement(reservation, value);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status SetElement(napi_value object, uint32_t index, napi_value value)
{
    return SetElementUnlocked(object, index, value);
}

CJNAPI_ALWAYS_INLINE napi_status GetElementUnlocked(napi_value object, uint32_t index, napi_value* result)
{
    if (!IsUnmaterializedLazyValue(object) || result == nullptr) {
        return napi_invalid_arg;
    }
    auto* lazy = DecodeLazyPointer(object);
    if (lazy == nullptr || lazy->store == nullptr) {
        return napi_invalid_arg;
    }
    if (!IsLazyObjectLike(lazy)) {
        return napi_invalid_arg;
    }
    if (auto* element = FindElement(lazy, index); element != nullptr) {
        *result = element->value;
        return napi_ok;
    }
    *result = EncodeImmediate(Kind::Undefined, 0);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status GetElement(napi_value object, uint32_t index, napi_value* result)
{
    return GetElementUnlocked(object, index, result);
}

CJNAPI_ALWAYS_INLINE napi_status HasElementUnlocked(napi_value object, uint32_t index, bool* result)
{
    if (!IsUnmaterializedLazyValue(object) || result == nullptr) {
        return napi_invalid_arg;
    }
    auto* lazy = DecodeLazyPointer(object);
    if (lazy == nullptr || lazy->store == nullptr) {
        return napi_invalid_arg;
    }
    if (!IsLazyObjectLike(lazy)) {
        return napi_invalid_arg;
    }
    *result = FindElement(lazy, index) != nullptr;
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status HasElement(napi_value object, uint32_t index, bool* result)
{
    return HasElementUnlocked(object, index, result);
}

CJNAPI_ALWAYS_INLINE napi_status DeleteElementUnlocked(napi_value object, uint32_t index, bool* result)
{
    if (!IsUnmaterializedLazyValue(object)) {
        return napi_invalid_arg;
    }
    auto* lazy = DecodeLazyPointer(object);
    if (lazy == nullptr || lazy->store == nullptr) {
        return napi_invalid_arg;
    }
    if (!IsLazyObjectLike(lazy)) {
        return napi_invalid_arg;
    }
    bool deleted = false;
    auto* segment = lazy->elements_head;
    while (segment != nullptr) {
        uint16_t i = 0;
        while (i < segment->used) {
            auto& element = segment->entries[i];
            if (element.index == index) {
                const uint16_t last = segment->used - 1;
                for (uint16_t j = i; j < last; ++j) {
                    segment->entries[j] = segment->entries[j + 1];
                }
                --segment->used;
                deleted = true;
            } else {
                ++i;
            }
        }
        segment = segment->next;
    }
    if (result != nullptr) {
        *result = deleted;
    }
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status DeleteElement(napi_value object, uint32_t index, bool* result)
{
    return DeleteElementUnlocked(object, index, result);
}

CJNAPI_ALWAYS_INLINE napi_status CreateUndefined(Store* store, napi_value* result)
{
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    if (!HasOpenScope(store)) {
        return napi_handle_scope_mismatch;
    }
    *result = EncodeImmediate(Kind::Undefined, 0);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status CreateNull(Store* store, napi_value* result)
{
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    if (!HasOpenScope(store)) {
        return napi_handle_scope_mismatch;
    }
    *result = EncodeImmediate(Kind::Null, 0);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status CreateBoolean(Store* store, bool value, napi_value* result)
{
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    if (!HasOpenScope(store)) {
        return napi_handle_scope_mismatch;
    }
    *result = EncodeImmediate(Kind::Bool, value ? 1 : 0);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status CreateInt32(Store* store, int32_t value, napi_value* result)
{
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    if (!HasOpenScope(store)) {
        return napi_handle_scope_mismatch;
    }
    *result = EncodeImmediate(Kind::Int32, static_cast<uint32_t>(value));
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status CreateUInt32(Store* store, uint32_t value, napi_value* result)
{
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    if (!HasOpenScope(store)) {
        return napi_handle_scope_mismatch;
    }
    *result = EncodeImmediate(Kind::UInt32, value);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status CreateInt64(Store* store, int64_t value, napi_value* result)
{
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    if (!HasOpenScope(store)) {
        return napi_handle_scope_mismatch;
    }
    auto* fast = detail::Allocate(store, Kind::Int64);
    if (fast == nullptr) {
        return napi_handle_scope_mismatch;
    }
    fast->i64 = value;
    *result = EncodePointer(fast);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status CreateDouble(Store* store, double value, napi_value* result)
{
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    if (!HasOpenScope(store)) {
        return napi_handle_scope_mismatch;
    }
    auto* fast = detail::Allocate(store, Kind::Double);
    if (fast == nullptr) {
        return napi_handle_scope_mismatch;
    }
    fast->f64 = value;
    *result = EncodePointer(fast);
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status GetValueInt32(napi_env, napi_value value, int32_t* result)
{
    if (result == nullptr || !IsFastValue(value)) {
        return napi_invalid_arg;
    }
    auto* ptr = IsPointerValue(value) ? DecodePointer(value) : nullptr;
    auto payload = DecodePayload(value);
    switch (DecodeKind(value)) {
        case Kind::Int32:
            *result = static_cast<int32_t>(static_cast<uint32_t>(payload));
            return napi_ok;
        case Kind::UInt32:
            if (payload > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
                return napi_invalid_arg;
            }
            *result = static_cast<int32_t>(payload);
            return napi_ok;
        case Kind::Int64:
            if (ptr == nullptr) {
                return napi_invalid_arg;
            }
            if (ptr->i64 < std::numeric_limits<int32_t>::min() || ptr->i64 > std::numeric_limits<int32_t>::max()) {
                return napi_invalid_arg;
            }
            *result = static_cast<int32_t>(ptr->i64);
            return napi_ok;
        case Kind::Double:
            if (ptr == nullptr || !detail::InRangeInt<int32_t>(ptr->f64)) {
                return napi_invalid_arg;
            }
            *result = static_cast<int32_t>(ptr->f64);
            return napi_ok;
        default:
            return napi_invalid_arg;
    }
}

CJNAPI_ALWAYS_INLINE napi_status GetValueUInt32(napi_env, napi_value value, uint32_t* result)
{
    if (result == nullptr || !IsFastValue(value)) {
        return napi_invalid_arg;
    }
    auto* ptr = IsPointerValue(value) ? DecodePointer(value) : nullptr;
    auto payload = DecodePayload(value);
    switch (DecodeKind(value)) {
        case Kind::UInt32:
            *result = static_cast<uint32_t>(payload);
            return napi_ok;
        case Kind::Int32:
            *result = static_cast<uint32_t>(static_cast<int32_t>(static_cast<uint32_t>(payload)));
            return napi_ok;
        case Kind::Int64:
            if (ptr == nullptr) {
                return napi_invalid_arg;
            }
            *result = static_cast<uint32_t>(ptr->i64);
            return napi_ok;
        case Kind::Double:
            if (ptr == nullptr) {
                return napi_invalid_arg;
            }
            *result = detail::ToUInt32(ptr->f64);
            return napi_ok;
        default:
            return napi_invalid_arg;
    }
}

CJNAPI_ALWAYS_INLINE napi_status GetValueInt64(napi_env, napi_value value, int64_t* result)
{
    if (result == nullptr || !IsFastValue(value)) {
        return napi_invalid_arg;
    }
    auto* ptr = IsPointerValue(value) ? DecodePointer(value) : nullptr;
    auto payload = DecodePayload(value);
    switch (DecodeKind(value)) {
        case Kind::Int64:
            if (ptr == nullptr) {
                return napi_invalid_arg;
            }
            *result = ptr->i64;
            return napi_ok;
        case Kind::Int32:
            *result = static_cast<int32_t>(static_cast<uint32_t>(payload));
            return napi_ok;
        case Kind::UInt32:
            *result = static_cast<uint32_t>(payload);
            return napi_ok;
        case Kind::Double:
            if (ptr == nullptr || !detail::InRangeInt<int64_t>(ptr->f64)) {
                return napi_invalid_arg;
            }
            *result = static_cast<int64_t>(ptr->f64);
            return napi_ok;
        default:
            return napi_invalid_arg;
    }
}

CJNAPI_ALWAYS_INLINE napi_status GetValueDouble(napi_env, napi_value value, double* result)
{
    if (result == nullptr || !IsFastValue(value)) {
        return napi_invalid_arg;
    }
    auto* ptr = IsPointerValue(value) ? DecodePointer(value) : nullptr;
    auto payload = DecodePayload(value);
    switch (DecodeKind(value)) {
        case Kind::Int32:
            *result = static_cast<double>(static_cast<int32_t>(static_cast<uint32_t>(payload)));
            return napi_ok;
        case Kind::UInt32:
            *result = static_cast<double>(static_cast<uint32_t>(payload));
            return napi_ok;
        case Kind::Int64:
            if (ptr == nullptr) {
                return napi_invalid_arg;
            }
            *result = static_cast<double>(ptr->i64);
            return napi_ok;
        case Kind::Double:
            if (ptr == nullptr) {
                return napi_invalid_arg;
            }
            *result = ptr->f64;
            return napi_ok;
        default:
            return napi_invalid_arg;
    }
}

CJNAPI_ALWAYS_INLINE napi_status GetValueBool(napi_env, napi_value value, bool* result)
{
    if (result == nullptr || !IsFastValue(value)) {
        return napi_invalid_arg;
    }
    if (DecodeKind(value) != Kind::Bool) {
        return napi_invalid_arg;
    }
    *result = DecodePayload(value) != 0;
    return napi_ok;
}

CJNAPI_ALWAYS_INLINE napi_status Typeof(napi_env, napi_value value, napi_valuetype* result)
{
    if (result == nullptr || !IsFastValue(value)) {
        return napi_invalid_arg;
    }
    switch (DecodeKind(value)) {
        case Kind::Undefined:
            *result = napi_undefined;
            return napi_ok;
        case Kind::Null:
            *result = napi_null;
            return napi_ok;
        case Kind::Bool:
            *result = napi_boolean;
            return napi_ok;
        case Kind::Int32:
        case Kind::UInt32:
        case Kind::Int64:
        case Kind::Double:
            *result = napi_number;
            return napi_ok;
        default:
            return napi_invalid_arg;
    }
}

CJNAPI_ALWAYS_INLINE napi_status StrictEquals(napi_env env, napi_value lhs, napi_value rhs, bool* result)
{
    if (result == nullptr || !IsFastValue(lhs) || !IsFastValue(rhs)) {
        return napi_invalid_arg;
    }
    auto lk = DecodeKind(lhs);
    auto rk = DecodeKind(rhs);
    if (lk == Kind::Undefined || lk == Kind::Null || lk == Kind::Bool) {
        if (lk != rk) {
            *result = false;
            return napi_ok;
        }
        *result = (lk == Kind::Bool) ? (DecodePayload(lhs) == DecodePayload(rhs)) : true;
        return napi_ok;
    }
    double ld = 0.0;
    double rd = 0.0;
    if (GetValueDouble(env, lhs, &ld) != napi_ok || GetValueDouble(env, rhs, &rd) != napi_ok) {
        *result = false;
        return napi_ok;
    }
    *result = (ld == rd);
    return napi_ok;
}

}  // namespace ace_napi_hook::fast

#endif
