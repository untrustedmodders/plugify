#pragma once

#include <string_view>

_LIBCPP_BEGIN_NAMESPACE_FILESYSTEM

#if _WIN32
using path_view = std::wstring_view;
#else
using path_view = std::string_view;
#endif

_LIBCPP_END_NAMESPACE_FILESYSTEM