#include "buffer.h"

/*
 * Global buffer, ONLY for temp storage for strings etc..
 * Is free'd by an atexit() callback.
 */

#define G_BUF_DEFAULT_SIZE BUFSIZ

static char *g_buffer = NULL;
static size_t g_bufsize = 0;

static void free_g_buffer(void)
{
    if (g_buffer) {
        free(g_buffer);
    }
}

static void init_g_buffer(void)
{
    if (!g_bufsize || !g_buffer) {
        g_bufsize = G_BUF_DEFAULT_SIZE;
        g_buffer = xmalloc(g_bufsize);

        /* cleanup at exit */
        atexit(free_g_buffer);
    }
}

static void extend_g_buffer(void)
{
    init_g_buffer();

    g_bufsize *= 2;
    g_buffer = xrealloc(g_buffer, g_bufsize);
}

/*
 * Lines / Strings
 */

Line *Line_new(void)
{
    return xcalloc(sizeof(Line), 1);
}

void Line_free(Line *line)
{
    if (!line) {
        return;
    }

    free(line->str);
}

int Line_insertat(Line *line, const char *s, size_t len, size_t cno)
{
    size_t nlen = 0; /* new line length */
    size_t offset = 0;

    if (!line) {
        return 0;
    }

    if (line->len <= cno) {
        /* cno is out of bounds, extend the buffer */
        nlen += cno-line->len + 1;
    }

    /* Calculate needed length */
    nlen += line->len + len;

    /* Extend the buffer if neeeded */
    if (g_bufsize < nlen) {
        extend_g_buffer();
    }

    if (cno >= line->len) {
        /* cno is outside of the current string's length
         * so we extend it using spaces */
        memcpy(&g_buffer[offset], line->str, line->len);
        offset += line->len;

        memset(&g_buffer[offset], ' ', cno - offset);
        offset += cno - offset;

    } else {
        /* cno is within the current string's length
         * so we write up until that point */
        memcpy(&g_buffer[offset], line->str, cno);
        offset += cno;
    }

    /* append/insert the new string */
    memcpy(&g_buffer[offset], s, len);
    offset += len;

    if (cno <= line->len) {
        /* write the rest of the string */
        memcpy(&g_buffer[offset], line->str+cno, line->len - cno);
        offset += line->len - cno;
    }

    free(line->str);

    line->str = xmalloc(offset);
    memcpy(line->str, g_buffer, offset);

    line->size = offset;
    line->len = offset;

    return 1;
}


int Line_removeat(Line *line, size_t len, size_t cno)
{
    size_t i;

    if (cno > line->len - 1) {
        return 1;
    }

#define MIN(x, y) ((x) < (y) ? (x) : (y))
    len = MIN(line->len - cno, len);
#undef MIN

    /* concat substring before cno with substring after cno+len */
    memmove(line->str+cno, line->str+cno+len, line->len - cno - len);

    line->len -= len;

    return 1;
}

/*
 * Lines
 */

Lines *Lines_new(void)
{
    Lines *lines = xmalloc(sizeof (Lines));

    Lines_init(lines);

    return lines;
}

void Lines_free(Lines *lines)
{
    size_t i;

    if (!lines || !lines->linev) {
        return;
    }

    for (i = 0; i < lines->len; i++) {
        Line_free(lines->linev[i]);
        free(lines->linev[i]);
    }

    free(lines->linev);
}

void Lines_init(Lines *lines)
{
    lines->size = LINES_DEFAULT_SIZE;
    lines->len = 0;

    lines->linev = xmalloc(sizeof (Lines *) * lines->size);
    memset(lines->linev, 0, lines->size);
}

void Lines_extend(Lines *lines, size_t factor)
{
    lines->size *= factor;
    lines->linev = xrealloc(lines->linev, sizeof(Lines*) * lines->size);

    memset(&lines->linev[lines->len], 0, lines->size - lines->len);
}

/*
 * THIS FUNCTION IS A HACK RIGHT NOW; AND SHOULD BE RELACED IN THE FUTURE.
 */
int Lines_pushback(Lines *lines, size_t lno, ssize_t n)
{

    if (!lines || !n) {
        return 0;
    }

    /*
     * Pushback the lines n indecies starting at lno
     */

    /* Check if there is space, if not then extend it to fit */
    if (lines->size <= lines->len + n + 1) {
        Lines_extend(lines, DEFAULT_LINES_EXTEND_FACTOR);

        if (!lines->linev) {
            elogf("Unable to extend buffer\n");
            exit(1);
        }
    }


    if (lines->len == 0 || ((ssize_t)lno + n) < 0) {
        return 1;
    }

    if (n > 0) {
        /* Move the memory after lno to the new location */
        memmove(&lines->linev[lno+n], &lines->linev[lno], (lines->len - lno) * sizeof(&lines->linev[lno]));

        /* Initialize memory inbetween to zero */
        memset(&lines->linev[lno], 0, n * sizeof(&lines->linev[lno]));
    } else {
        /* Move the memory after lno to the new location */
        memmove(&lines->linev[lno+n], &lines->linev[lno], (lines->len - lno) * sizeof(&lines->linev[lno]));
    }

    /* Update buffer line length to be the at the end of the lines that was moved */
    lines->len = n + lines->len;

    return 1;
}

int Lines_iline(Lines *lines, Line *line, size_t lno)
{
    if (!lines) {
        return 0;
    }

    /* Extend the buffer if needed */
    if (lines->len + 1 >= lines->size) {
        elogf("Extends lines\n");
        fflush(stderr);

        Lines_extend(lines, DEFAULT_LINES_EXTEND_FACTOR);

        if (!lines->linev) {
            elogf("Unable to extend buffer\n");
            exit(1);
        }
    }

    Lines_pushback(lines, lno, 1); /* IGNORING ERRORS */

    /* Try to create empty line if none is provided */
    if (!line) {
        line = Line_new();
        if (!line) {
            return 0;
        }
    }

    /* Insert line at lno */
    lines->linev[lno] = line;

    /* Update length in the case that the line we just inserted
     * is beyond the current lines length */
    if (lno >= lines->len) {
        lines->len = lno + 1;
    }

    return 1;
}

int Lines_rline(Lines *lines, size_t lno)
{
    if (!lines || !lines->linev || lines->len - 1 < lno) {
        return 0;
    }

    if (lno < lines->len) {
        if (lines->linev && lines->linev[lno]) {
            Line_free(lines->linev[lno]);
            lines->linev[lno] = NULL;
        }

        if (lines->len > 0) {
            /* Move back the rest of the lines (an expensive operation) */
            Lines_pushback(lines, lno+1, -1);
            return 1;
        }
    }

    return 0;
}

int Lines_append(Lines *lines, const char *str, size_t len)
{
    Line *line;

    if (!str) {
        return 0;
    }

    line = Line_new();

    if (!line) {
        elogf("Unable to append line: %s\n", str);
        return 0;
    }

    line->str = mem_clone(str, len);
    line->len = line->size = len;

    if (!line->str) {
        line->len = 0;
        elogf("Unable to duplicate string\n");
        return 0;
    }

    Lines_iline(lines, line, lines->len); /* IGNORING ERROR */

    return 1;
}

int Lines_insertat(Lines *lines, const char *s, size_t len, size_t lno, size_t cno)
{
    Line *line;

    if (lines->len <= lno) {
        /* Line at lno does not exist ... */
        return 0;
    }

    line = lines->linev[lno];

    if (!g_bufsize || !g_buffer) {
        init_g_buffer();
    }

    return Line_insertat(line, s, len, cno);
}

Lines *Lines_load(Lines *lines, const char *fpath)
{
    ssize_t read;
    FILE *fp;

    if (!g_bufsize || !g_buffer) {
        init_g_buffer();
    }

    if (!lines || !fpath) {
        return NULL;
    }

    fp = fopen(fpath, "r");

    if (!fp) {
        return NULL;
    }

    while ((read = getline(&g_buffer, &g_bufsize, fp)) > 0) {
        /* remove newline */
        if (g_buffer[read-1] == '\n') {
            g_buffer[--read] = '\0';
            Lines_append(lines, g_buffer, read);
        }
    }

    fclose(fp);
    return lines;
}

int Lines_write(Lines *lines, const char *fpath)
{
    FILE *fp;
    size_t i, j;

    if (!g_bufsize || !g_buffer) {
        init_g_buffer();
    }

    if (!lines || !fpath || !lines->linev) {
        return 0;
    }

    fp = fopen(fpath, "w");

    if (!fp) {
        return 0;
    }

    /* write lines to file */
    for (i = 0; i < lines->len && i < lines->size; i++) {
        if (lines->linev[i]) {
            for (j = 0; j < lines->linev[i]->len; j++) {
                fputc(lines->linev[i]->str[j], fp);
            }
        }
        fputc('\n', fp);
    }

    fclose(fp);
    return 1;
}

/*
 * Buffer
 */

Buffer *Buffer_new(void)
{
    return xcalloc(sizeof(Buffer), 1);
}

int Buffer_free(Buffer *buf)
{
    if (!buf) {
        return 0;
    }

    Lines_free(buf->lines);
    free(buf->lines);

    buf->lines = NULL;

    free(buf->path);

    return 1;
}

int Buffer_init(Buffer *buf)
{
    Lines *lines;

    if (!buf) {
        return 0;
    }

    lines = Lines_new();

    if (!lines) {
        return 0;
    }

    buf->lines = lines;

    Lines_append(buf->lines, "", 0);

    return 1;
}

int Buffer_load(Buffer *buf, const char *fpath)
{
    Lines *lines;

    if (!buf) {
        return 0;
    }

    /* load file into memory */
    lines = Lines_new();

    if (!lines) {
        return 0;
    }

    if (Lines_load(lines, fpath) == NULL) {
        Lines_free(lines);
        free(lines);
        return 0;
    }

    buf->lines = lines;

    elogf("Read file: %s with length of %lu and size of %lu\n",
          fpath, buf->lines->len, buf->lines->size);

    buf->path = strdup(fpath);

    return 1;
}

int Buffer_write(Buffer *buf, const char *fpath)
{
    if (!buf || !buf->lines) {
        return 0;
    }

    return Lines_write(buf->lines, fpath);
}

void Buffer_print(Buffer *buf)
{
    size_t i, j;

    if (!buf->lines->linev) {
        elogf("linev is not initialized!\n");
        exit(1);
    }

    puts("=== Start of Buffer output ===");

    for (i = 0; i < buf->lines->len && i < buf->lines->size; i++) {
        if (buf->lines->linev[i]) {
            for (j = 0; j < buf->lines->linev[i]->len; j++) {
                putchar(buf->lines->linev[i]->str[j]);
            }
        }
        putchar('\n');
    }
    puts("=== End of Buffer output ===");
}
