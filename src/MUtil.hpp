#ifndef INETR_MUTIL_HPP
#define INETR_MUTIL_HPP

#define RWIDTH(rect) (rect.right - rect.left)
#define RHEIGHT(rect) (rect.bottom - rect.top)

#ifdef _WIN64
#define INETR_ARCH string("x64")
#else // _WIN64
#define INETR_ARCH string("Win32")
#endif // _WIN64


#endif // !INETR_MUTIL_HPP