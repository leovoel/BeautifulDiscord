#pragma once

#define BD_WINDOWS 0
#define BD_LINUX 0
#define BD_OSX 0

#if _WIN32 || _WIN64
# undef BD_WINDOWS
# define BD_WINDOWS 1
# ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0600
# endif
#elif __APPLE__
# undef BD_OSX
# define BD_OSX 1
#elif __linux__
# undef BD_LINUX
# define BD_LINUX 1
#endif // OS detection

#define BD_POSIX BD_OSX || BD_LINUX
