#pragma once

#include <string_view>

#ifdef _LIBCPP_BEGIN_NAMESPACE_FILESYSTEM
_LIBCPP_BEGIN_NAMESPACE_FILESYSTEM
#else
namespace std::filesystem {
#endif

#if _WIN32
	using path_view = std::wstring_view;
#else
	using path_view = std::string_view;
#endif

#ifdef _LIBCPP_END_NAMESPACE_FILESYSTEM
_LIBCPP_END_NAMESPACE_FILESYSTEM
#else
}
#endif
