#pragma once

#if PLUGIFY_CORE
#  define PLUGIFY_ACCESS public
#else
#  define PLUGIFY_ACCESS private
#endif
