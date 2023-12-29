#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <optional>
#include <utility>
#include <algorithm>
#include <span>
#include <unordered_map>
#include <set>
#include <functional>
#include <iterator>
#include <ranges>

#if __has_include(<format>)
#include <format>
#else
#include <fmt/format.h>
#endif

#include <utils/log.h>
#include <utils/json.h>
#include <utils/paths.h>
