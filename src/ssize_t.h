#ifndef _SSIZE_T_DEFINED
#ifdef  _WIN64
typedef signed __int64    ssize_t;
#else
typedef _W64 signed int   ssize_t;
#endif
#define _SSIZE_T_DEFINED
#endif