#pragma once
#define __VERBOSE_TEST_IO__
#include <type_traits>

// clang-format off
#include <gtest/gtest.h>
// clang-format on

// the problem with namspacing <huffman.h> prior to including <gtest/gtest.h> is that all the symbols from headers directly and
// indirecty included in <huffman.h> get scoped inside the namespace, won't be available in the global namespace
// this includes symbols from __STDC__ headers :(
// but the header guards copied into our TUs will prevent these headers from being reincluded in gtest.h, hence we run in to a slew of errors
// so we move the gtest include before the namespacing of <huffman.h> included symbols

static constexpr unsigned long long N_RANDNUMS { 1 << 8 }, N_EXTRANDOMS { 5 << 10 }; // explicit external linkage
static constexpr float              RAND_LLIMIT { -25.0 }, RAND_ULIMIT { 25.0 };

using node_type             = unsigned short; // for testing using fixtures
using node_pointer          = node_type*;
using constant_node_pointer = const node_type*;

// return true when a swap is needed, i.e when the child is heavier than the parent
extern "C" [[nodiscard]] static
    __declspec(noinline) bool __stdcall comp(_In_ const void* const child, _In_ const void* const parent) noexcept {
    return *reinterpret_cast<constant_node_pointer>(child) > *reinterpret_cast<constant_node_pointer>(parent);
}

// argument typedef int (__cdecl* _CoreCrtSecureSearchSortCompareFunction)(void*, void const*, void const*)
extern "C" [[nodiscard]] static __declspec(noinline) int __cdecl ptrcompare( // to be used with qsort_s()
    _In_opt_ [[maybe_unused]] void* const context,                           // we do not need this for our tests
    _In_ constant_node_pointer* const     current,                           // cannot use long double here directly
    _In_ constant_node_pointer* const     next
) noexcept {
    assert(reinterpret_cast<uintptr_t>(*current) & reinterpret_cast<uintptr_t>(*next));
    return (**current == **next) ? 0 : (**current > **next) ? 1 : -1;
}

template<typename _TyNode>
[[nodiscard]] static __declspec(noinline) bool __stdcall nodecomp(_In_ const void* const child, _In_ const void* const parent) noexcept
    requires requires(const _TyNode& _left, const _TyNode& _right) { _left.operator>(_right); }
{ // explicitly calling the .operator>() member instead of using > because we do not want primitive types meeting this type constraint
    return (reinterpret_cast<typename std::add_pointer_t<typename std::add_const_t<_TyNode>>>(child))
        ->operator>(*reinterpret_cast<typename std::add_pointer_t<typename std::add_const_t<_TyNode>>>(parent));
}
