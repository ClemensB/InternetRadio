#ifndef INETR_SSIZE_T_H
#define INETR_SSIZE_T_H

#ifndef _SSIZE_T_DEFINED
#ifdef  _WIN64
typedef signed __int64    ssize_t;
#else  // _WIN64
typedef _W64 signed int   ssize_t;
#endif  // _WIN64
#define _SSIZE_T_DEFINED
#endif  // !_SSIZE_T_DEFINED

#endif  // !INETR_SSIZE_T_H
