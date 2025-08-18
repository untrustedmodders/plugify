#pragma once

#if PLUGIFY_DEBUG
#  include <cassert>
#  define PL_ASSERT assert
#else
#  define PL_ASSERT(...)
#endif
