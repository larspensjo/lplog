#pragma once

#ifdef DEBUG
extern void LPLog(const char *func, const char *file, int line, const char *fmt, ...);
#define LPLOG(...) LPLog(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LPLOG(...)
#endif
