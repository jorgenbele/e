#include "editor_functions.h"

int Editor_handle_prev_line(Editor *e)
{
    if (!e->buf || !e->buf->lines || !e->buf->lines->linev) {
        return 0;
    }

    if (e->crow < e->rtoff + 1) {
        /* try to scroll up */
        if (e->buf->pos.line > 0) {
            e->crow = e->rtoff;
            /*
               e->trow--;
               */
            e->buf->pos.line--;
            return 1;
        }
    } else if (e->buf->pos.line > 0) {
        e->crow--;
        e->buf->pos.line--;
        return 1;
    }
    return 0;
}

int Editor_handle_next_line(Editor *e)
{
    if (!e->buf || !e->buf->lines || !e->buf->lines->linev) {
        return 0;
    }

    if (e->crow >= LINES - e->rboff - 1) {
        /* try to scroll down */
        if (e->buf->pos.line + 1 < e->buf->lines->len) {
            e->buf->pos.line++;
            return 1;
        }
    } else if (e->buf->pos.line + 1 < e->buf->lines->len) {
        e->buf->pos.line++;
        e->crow++;
        return 1;
    }
    return 0;
}

int Editor_handle_scroll_next_line(Editor *e)
{
    if (!e->buf || !e->buf->lines || !e->buf->lines->linev) {
        return 0;
    }

    if (e->buf->pos.line + 1 < e->buf->lines->len) {
        if (e->crow <= e->rtoff) {
            e->buf->pos.line++;
        } else {
            /* e->buf->pos.line++; */
            e->crow--;
        }

        return 1;
    }

    return 0;
}

int Editor_handle_scroll_prev_line(Editor *e)
{
    if (!e->buf || !e->buf->lines || !e->buf->lines->linev) {
        return 0;
    }

    if (e->buf->pos.line > 0) {
        if (e->crow >= (size_t)LINES - e->rboff - 1 && e->crow < E_GET_POS_LINE) {
            e->buf->pos.line--;
        } else if (e->crow < E_GET_POS_LINE) {
            e->crow++;
        } else {
            Editor_handle_prev_line(e);
        }

        return 1;
    }

    return 0;
}

int Editor_next_word(Editor *e)
{
    size_t i = e->buf->pos.col;

    if (!e->buf->lines->linev[e->buf->pos.line]) {
        return 0;
    }

    if (e->buf->pos.col < e->buf->lines->linev[e->buf->pos.line]->len) {
        /* skip to beginning of next word */
        for (i = e->buf->pos.col; i < e->buf->lines->linev[e->buf->pos.line]->len
                && isalnum(e->buf->lines->linev[e->buf->pos.line]->str[i]); i++);

        for (; i < e->buf->lines->linev[e->buf->pos.line]->len
                && !isalnum(e->buf->lines->linev[e->buf->pos.line]->str[i]); i++);

        if (i < e->buf->lines->linev[e->buf->pos.line]->len && isalnum(e->buf->lines->linev[e->buf->pos.line]->str[i])) {
            e->buf->pos.col = i;
            return 1;
        }

    }

    if (e->buf->pos.line + 1 < e->buf->lines->len) {
        /* skip to beginning of next word at next line */
        e->buf->pos.col = 0;

        if (!Editor_handle_next_line(e))
            return 1;

        /* skip to beginning of next word */
        for (i = e->buf->pos.col; i < e->buf->lines->linev[e->buf->pos.line]->len
                && !isalnum(e->buf->lines->linev[e->buf->pos.line]->str[i]); i++);
        e->buf->pos.col = i;
    }

    e->buf->pos.col = i;

    return 1;
}

int Editor_prev_word(Editor *e)
{
    ssize_t i = E_GET_POS_COL;

    if (!E_GET_CLINE) {
        return 0;
    }

    if (i > 0) {
        /*
        * skip to beginning of previous  word
        */
        for (i = E_GET_POS_COL; i >= 0 && i < (ssize_t)E_GET_CLINE_LEN && isalnum(E_GET_CLINE->str[i]); i--);
        for (; i >= 0 && i < (ssize_t)E_GET_CLINE_LEN && !isalnum(E_GET_CLINE->str[i]); i--);
        for (; i >= 0 && i < (ssize_t)E_GET_CLINE_LEN && isalnum(E_GET_CLINE->str[i]); i--);
        i++;

        if (i < (ssize_t)E_GET_CLINE_LEN && isalnum(E_GET_CLINE->str[i])) {
            E_GET_POS_COL = i > 0 ? i : 0;
            return 1;
        }
    }

    if (E_GET_POS_LINE > 0) {
        Editor_handle_prev_line(e);
        E_GET_POS_COL = E_GET_CLINE_LEN - 1;

        /* RECURSIVE CALL */
        return Editor_prev_word(e);
    }
    return 1;
}

int Editor_endof_word(Editor *e)
{
    size_t i = E_GET_POS_COL;

    if (i < E_GET_CLINE_LEN - 1) {
        /*
          if (isalnum(E_GET_CLINE->str[E_GET_POS_COL + 1])) {
              for (i = E_GET_POS_COL; i < E_GET_CLINE_LEN && isalnum(E_GET_CLINE->str[i]); i++);
              for (; i < E_GET_CLINE_LEN && !isalnum(E_GET_CLINE->str[i]); i++);
          }
        */

        for (; i < E_GET_CLINE_LEN && isalnum(E_GET_CLINE->str[i]); i++);
        i--;

        if (i < E_GET_CLINE_LEN && isalnum(E_GET_CLINE->str[i])) {
            E_GET_POS_COL = i;
            return 1;
        }

    }

    return 1;
}

int Editor_move_down(Editor *e)
{
    Line *newline;
    size_t splitindex = e->buf->pos.col;
    int ret;

    newline = Line_new();

    if (!newline) {
        return 0;
    }

    if (splitindex >= e->buf->lines->linev[e->buf->pos.line]->len) {
        /* Nothing to split, just create a new empty line */
        ret = Line_insertat(newline, "", 0, 0);
        Lines_iline(e->buf->lines, newline, e->buf->pos.line + 1);
    } else {
        ret = Line_insertat(newline, e->buf->lines->linev[e->buf->pos.line]->str + splitindex, e->buf->lines->linev[e->buf->pos.line]->len - splitindex, 0);

        Lines_iline(e->buf->lines, newline, e->buf->pos.line + 1);

        /* update line length for the length we just split  (it has to be greater than 0) */
        e->buf->lines->linev[e->buf->pos.line]->len = splitindex;
    }

    return ret;
}

int Editor_next_buffer(Editor *e)
{
    List *temp;

    temp = e->bufs;
    while (temp && temp->data != e->buf) {
        temp = temp->next;
    }

    if (temp && temp->next && temp->next->data && temp->data == e->buf) {
        e->buf = temp->next->data;
        return 1;
    }

    return 0;
}

int Editor_prev_buffer(Editor *e)
{
    List *temp;

    temp = e->bufs;

    while (temp && temp->data != e->buf) {
        temp = temp->prev;
    }

    if (temp && temp->prev && temp->prev->data && temp->data == e->buf) {
        e->buf = temp->prev->data;
    }

    return 1;
}
