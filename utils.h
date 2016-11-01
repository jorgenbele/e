#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <stdarg.h>

void *xmalloc(size_t size);
void *xrealloc(void *p, size_t size);
void *xcalloc(size_t nmemb, size_t size);
int elogf(const char *fmt, ...);
void *mem_clone(const void *ptr, size_t size);

#endif
