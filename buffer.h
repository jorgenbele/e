#ifndef BUFFER_H
#define BUFFER_H

#define _POSIX_C_SOURCE 200809L

/*
#define _WITH_GETLINE
*/

#include <stdio.h>
#include <string.h>
#include <string.h>

#include <unistd.h>

#include "utils.h"
#include "list.h"

typedef struct Buffer_position_struct {
  size_t col, line;
} Buffer_position;

typedef struct Line_struct {
  char *str;          /* Line stored as NON null-terminated string */
  size_t size;        /* The size of the allocated memory for str  */
  size_t len;         /* The length of str                         */
} Line;

#define LINES_DEFAULT_SIZE          BUFSIZ
#define DEFAULT_LINES_EXTEND_FACTOR 4

typedef struct Lines_struct {
  Line **linev;      /* Array of lines that is NULL if empty */
  size_t size;       /* Number of lines allocated to fit     */
  size_t len;        /* Number of lines used in buffer       */
} Lines;
	
typedef struct Buffer_struct {
  Lines *lines;             /* Array of lines            */
  Buffer_position pos;      /* Current line and column   */
  char *path;
} Buffer;

/*
 * Line
 */
Line *Line_new(void);
void Line_free(Line *line);
int Line_insertat(Line *line, const char *s, size_t len, size_t cno);
int Line_removeat(Line *line, size_t len, size_t cno);

/*
 * Lines
 */
Lines *Lines_new(void);
void Lines_free(Lines *lines);
void Lines_init(Lines *lines);
void Lines_extend(Lines *lines, size_t factor);
int Lines_iline(Lines *lines, Line *line, size_t lno);
int Lines_rline(Lines *lines, size_t lno);
int Lines_append(Lines *lines, const char *str, size_t len);
int Lines_insertat(Lines *lines, const char *s, size_t len, size_t lno, size_t cno);
Lines *Lines_load(Lines *lines, const char *fpath);
int Lines_write(Lines *lines, const char *fpath);
int Lines_pushback(Lines *lines, size_t lno, ssize_t n);

/*
 * Buffer 
 */
Buffer *Buffer_new(void);
int Buffer_free(Buffer *buf);
int Buffer_init(Buffer *buf);
int Buffer_load(Buffer *buf, const char *fpath);
int Buffer_write(Buffer *buf, const char *fpath);
void Buffer_print(Buffer *buf);

#endif
