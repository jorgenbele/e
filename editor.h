#ifndef EDITOR_H
#define EDITOR_H

#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <ctype.h>

#include "buffer.h"

#define TAB_WIDTH 4

#define E_GET_LINE_AT(line) (e->buf->lines->len + 1 > (line) ? e->buf->lines->linev[(line)] : NULL)

#define E_GET_POS_LINE (e->buf->pos.line)
#define E_GET_POS_COL (e->buf->pos.col)

#define E_GET_CLINE (E_GET_LINE_AT(E_GET_POS_LINE))
#define E_GET_CLINE_LEN ((E_GET_CLINE) ? (E_GET_CLINE)->len : 0)

#define E_EXCEEDED_LINE ((E_GET_POS_COL + 1) > E_GET_CLINE_LEN)
#define E_EXCEEDED_BUFFER ((E_GET_POS_LINE) + 1 > e->buf->lines->len)

#define E_END_OF_LINE (E_GET_CLINE_LEN ? E_GET_CLINE_LEN - 1 : 0)

enum Editor_state_enum { 
    EDITOR_NORMAL=0, EDITOR_INSERT, EDITOR_COMMAND
    , EDITOR_VISUAL, EDITOR_VISUAL_BLOCK
};

typedef struct Editor_state_struct {
    enum Editor_state_enum type;
} Editor_state;

typedef struct Editor_struct {
    size_t crow;       /* current on screen row */
    size_t ccol;       /* current on screen col */

    size_t coff;       /* column offset */

    size_t rtoff;      /* editor height - row offset from top */
    size_t rboff;      /* editor height - row offset from bottom */

    Buffer *buf;       /* current buffer */
    List *bufs;        /* buffers */
    WINDOW  *win;      /* curses window handle */

    Editor_state state;
    Line *cmd;
} Editor;

#include "editor_functions.h"

Editor *Editor_new(void);
void Editor_init(Editor *editor);
int Editor_init_curses(Editor *editor);
void Editor_destroy(Editor *editor);
int Editor_set_buffers(Editor *editor, List *buffers);

int Editor_dbuffer(Editor *editor);
int Editor_draw(Editor *editor);
int Editor_loop(Editor *editor);
int Editor_handle_inp(Editor *editor, int in_char);

#endif
