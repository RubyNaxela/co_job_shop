#ifndef JOB_SHOP_PLATFORM
#define JOB_SHOP_PLATFORM

#ifdef _WIN32
#define WINDOZE
#else
#ifdef __WIN32__
#define WINDOZE
#else
#ifdef __WINDOWS__
#define WINDOZE
#endif
#endif
#endif

#endif //JOB_SHOP_PLATFORM
