#pragma once

#include <yajl/yajl_version.h>

#if YAJL_MAJOR >= 2
#define YAJL_UINT std::size_t
#define YAJL_LONG long long
#else
#define YAJL_UINT unsigned int
#define YAJL_LONG long
#endif
