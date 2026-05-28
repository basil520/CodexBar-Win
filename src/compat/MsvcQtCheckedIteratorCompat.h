#pragma once

#if defined(_MSC_VER) && _MSC_VER >= 1951

#include <cstddef>

// Qt 6.5.x maps QT_MAKE_*_ARRAY_ITERATOR to the legacy MSVC stdext helpers.
// MSVC 14.51 removed those non-standard helpers, so provide the portable
// pointer passthrough that newer Qt versions use.
namespace stdext {
template <typename Pointer>
constexpr Pointer make_checked_array_iterator(Pointer pointer, std::size_t) noexcept
{
    return pointer;
}

template <typename Pointer>
constexpr Pointer make_unchecked_array_iterator(Pointer pointer) noexcept
{
    return pointer;
}
}

#endif
