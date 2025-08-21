#pragma once

#if PLUGIFY_DEBUG
#  include <cassert>
#  define PL_ASSERT assert
#else
#  define PL_ASSERT(...)
#endif

#if PLUGIFY_CORE
#  define PLUGIFY_ACCESS public
#else
#  define PLUGIFY_ACCESS private
#endif
