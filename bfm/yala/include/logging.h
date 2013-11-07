#if ! defined(__LOGGING__)
#define __LOGGING__

#include <cstdarg>
#include <cstdio>

extern FILE* logFile;

#define LOG(msg, ...) do { fprintf(logFile, msg, __VA_ARGS__); fflush(logFile); } while (0)
#define ASSERT(pred) do { if (!(pred)) { LOG("assert failed: %s\n", #pred); assert(pred); } } while (0)

#endif // __LOGGING__
