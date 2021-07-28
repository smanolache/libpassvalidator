#pragma once

#if __GNUC__ >= 4
#	define DSO_PUBLIC __attribute__ ((visibility("default")))
#	define DSO_LOCAL  __attribute__ ((visibility("hidden")))
#else
#	define DSO_PUBLIC
#	define DSO_LOCAL
#endif
