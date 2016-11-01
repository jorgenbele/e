#include "getdelim.h"

ssize_t getdelimf(char **p, size_t *size, int (*compar)(int), FILE *f)
{
    size_t bread = 0;
    int c;

    if (*p == NULL || *size == 0) {
        /* allocate memory for line */
        *size = GETDELIM_DEFAULT_MALLOC_SIZE;
        *p = malloc(*size);
        if (*p == NULL) {
            return 0;
        }
    }

    while ((c = fgetc(f)) != EOF && !compar(c)) {
        if (*size <= bread + 1) {
            /* reallocate memory for line */
            *size += GETDELIM_DEFAULT_MALLOC_INCR_SIZE;
            *p = realloc(*p, *size);
            if (*p == NULL) {
                return 0;
            }
        }
        (*p)[bread] = c;
        bread++;
    }

    if (bread == 0 && c == EOF)
        return EOF;

    (*p)[bread] = '\0';

    return bread;
}

/* getdelim: returns string read until i is encountered  */
ssize_t getdelim(char **p, size_t *size, int i, FILE *f)
{
    size_t bread = 0;
    int c;

    if (*p == NULL || *size == 0) {
        /* allocate memory for line */
        *size = GETDELIM_DEFAULT_MALLOC_SIZE;
        *p = malloc(*size);
        if (*p == NULL) {
            return 0;
        }
    }

    while ((c = fgetc(f)) != EOF && c != i) {
        if (*size <= bread + 1) {
            /* reallocate memory for line */
            *size += GETDELIM_DEFAULT_MALLOC_INCR_SIZE;
            *p = realloc(*p, *size);
            if (*p == NULL) {
                return 0;
            }
        }
        (*p)[bread] = c;
        bread++;
    }

    if (bread == 0 && c == EOF)
        return EOF;

    (*p)[bread] = '\0';

    return bread;
}

ssize_t getline(char **p, size_t *size, FILE *f)
{
    return getdelim(p, size, '\n', f);
}
