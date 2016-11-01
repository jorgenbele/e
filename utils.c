#include "utils.h"

#define LOGFILE "e.log"

static FILE *log_file;

void *xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr) {
        perror("malloc");
        exit(1);
    }

    return ptr;
}

void *xrealloc(void *p, size_t size)
{
    void *ptr = realloc(p, size);
    if (!ptr) {
        perror("realloc");
        exit(1);
    }

    return ptr;
}

void *xcalloc(size_t nmemb, size_t size)
{
    void *ptr = calloc(nmemb, size);
    if (!ptr) {
        perror("calloc");
        exit(1);
    }

    return ptr;
}

#if 0
int elogf(const char *fmt, ...)
{
    va_list ap;
    int ret;

    if (!log_file) {
        log_file = fopen(LOGFILE, "a");
    }

    va_start(ap, fmt);
    ret = vfprintf(log_file, fmt, ap);
    va_end(ap);
    return ret;
}
#else
int elogf(const char *fmt, ...)
{
    return 0;
}
#endif

void *mem_clone(const void *ptr, size_t size)
{
    void *p = malloc(size);
    if (!p)
        return NULL;

    memmove(p, ptr, size);

    return p;

}
