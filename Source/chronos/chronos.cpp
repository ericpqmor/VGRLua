#ifdef _WIN32
#include <windows.h>
#else
#include <ctime>
#include <sys/time.h>
#endif

#include "chronos/chronos.h"

Chronos::
Chronos(void) {
    reset();
}

void
Chronos::
reset(void) {
    m_reset = time();
}

double
Chronos::
elapsed(void) {
    return time() - m_reset;
}

double
Chronos::time(void) {
#ifdef _WIN32
    LARGE_INTEGER counter, freq;
    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&freq);
    return (1.0*counter.QuadPart)/(1.0*freq.QuadPart);
#else
    struct timeval v;
    gettimeofday(&v, (struct timezone *) NULL);
    return v.tv_sec + v.tv_usec/1.0e6;
#endif
}
