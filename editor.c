#include "editor.h"

#define PRINT_LINE_NUMBERS

char buf[2] = {0, 0};

Editor *Editor_new(void)
{
    Editor *e = xmalloc(sizeof(Editor));
    Editor_init(e);
    return e;
}

void Editor_destroy(Editor *e)
{
    endwin();
    if (e->cmd) {
        Line_free(e->cmd);
        free(e->cmd);
    }
    free(e);
}

void Editor_init(Editor *e)
{
    e->state.type = EDITOR_NORMAL;

    e->buf = NULL;
    e->win = NULL;

    e->coff = 0;
    e->rtoff = 0;
    e->rboff = 1; /* for the status line */

    e->crow = 0;

    e->cmd = Line_new();

}

#define COLOR_TEXT 2
#define COLOR_BAR 1

int Editor_init_curses(Editor *e)
{
    e->win = initscr();

    start_color();

    init_pair(COLOR_TEXT, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_BAR, COLOR_BLACK, COLOR_WHITE);

    if (!e->win) {
        return 0;
    }

    noecho();
    raw();

    intrflush(e->win, FALSE);

    keypad(e->win, TRUE);

    return 1;
}

int Editor_set_buffers(Editor *e, List *b)
{
    if (!e || !b) {
        return 0;
    }

    e->bufs = b;
    e->buf = e->bufs->data;

    return 1;
}

static void drawtab(Editor *e)
{
    wattron(e->win, COLOR_PAIR(COLOR_BAR));
    waddch(e->win, '>');
    wattroff(e->win, COLOR_PAIR(COLOR_BAR));
}

static void drawchar(Editor *e, int c)
{
    if (c == '\t') {
        drawtab(e);
    } else {
        waddch(e->win, c);
    }
}

int Editor_drawstrline(Editor *e, const char *str, size_t len)
{
    size_t i;

    /* Print line */
    for (i = 0; i < len; i++) {
        drawchar(e, str[i]);
    }

    /* Clear rest of the line by printing spaces */
    for (; i < (size_t)COLS; i++) {
        waddch(e->win, ' ');
    }
    return 1;
}

int Editor_drawline(Editor *e, Line *line)
{
    return Editor_drawstrline(e, line->str, line->len);
}

int Editor_dbuffer(Editor *e)
{
    /* * only draw the visible portion of the buffer */

    size_t i, j, k, loffset;

    if (!e || !e->buf || !e->win || !e->buf->lines || !e->buf->lines->linev) {
        return 0;
    }

    /* Calculate screen line offset by the line displayed at the top of the screen */
    if (E_GET_POS_LINE > 0) {
        loffset = E_GET_POS_LINE - (e->crow - e->rtoff);
    } else {
        loffset = 0;
    }

    i = loffset, j = e->rtoff;


    wattron(e->win, COLOR_PAIR(COLOR_TEXT));

    for (; i < loffset + LINES - e->rboff && j < LINES - e->rboff; i++, j++) {
        wmove(e->win, j, 0);


        if (i < e->buf->lines->len && e->buf->lines->linev[i]) {
            Editor_drawline(e, e->buf->lines->linev[i]);

        } else {
            waddch(e->win, '~');
            for (k = 1; k < (size_t)COLS; k++) {
                waddch(e->win, ' ');
            }
        }
    }

    wattroff(e->win, COLOR_PAIR(COLOR_TEXT));

    wmove(e->win, e->crow, E_GET_POS_COL);


    return 1;
}

void Editor_drawstatusbar(Editor *e, const char *state)
{
    Line line;
    char str[256];
    size_t i;

    line.str = str;

    wmove(e->win, LINES - 1, 0);

    wattron(e->win, COLOR_PAIR(COLOR_BAR));

    /* i = snprintf(line.str, 256, "[%s] l:%lu c:%lu cr:%lu cll:%lu f:%s %s", state, E_GET_POS_LINE, E_GET_POS_COL, e->crow, E_GET_CLINE ? E_GET_CLINE_LEN : '0', e->buf->path, e->cmd->str ? e->cmd->str : ""); */
    i = snprintf(line.str, 256, "-- %s --  l:%5lu c:%5lu f:%s %s", state, E_GET_POS_LINE, E_GET_POS_COL, e->buf->path, e->cmd->str ? e->cmd->str : "");
    line.len = line.size = i;

    Editor_drawline(e, &line);

    wattroff(e->win, COLOR_PAIR(COLOR_BAR));

}

int Editor_draw(Editor *e)
{
    char *state = "unknown";

    switch (e->state.type) {
    case EDITOR_NORMAL:
        state = "normal";
        break;
    case EDITOR_INSERT:
        state = "insert";
        break;
    case EDITOR_COMMAND:
        state = "command";
        break;
    case EDITOR_VISUAL:
        state = "visual";
        break;
    case EDITOR_VISUAL_BLOCK:
        state = "visual block";
        break;
    default:
        state = "unknown";
        break;
    }

    if (e->state.type == EDITOR_NORMAL) {
        Editor_drawstatusbar(e, state);
        /*
           mvwprintw(e->win, LINES-1, 0, "%s l:%lu c:%lu", state, e->buf->pos.line, e->buf->pos.col);
           */
    } else if (e->state.type == EDITOR_COMMAND) {
        wmove(e->win, LINES - 1, 0);
        waddch(e->win, ':');
        Editor_drawline(e, e->cmd);
    } else if (e->state.type == EDITOR_INSERT) {
        Editor_drawstatusbar(e, state);
    }
    /*        mvwprintw(e->win, LINES-1, 0, ":%*s", e->cmd->len, e->cmd->str); */
    Editor_dbuffer(e);

    refresh();

    return 1;
}

int Editor_loop(Editor *e)
{
    int c;

    /* select the first buffer */
    e->buf = ((Buffer*)e->bufs->data);

    do {
        /* Move cursor to a viable position (inside a line's length) */
        if (E_EXCEEDED_BUFFER || !E_GET_CLINE) {
            E_GET_POS_LINE = 0;
        }

        if (E_GET_CLINE && E_EXCEEDED_LINE && e->state.type != EDITOR_INSERT) {
            E_GET_POS_COL = E_END_OF_LINE;
        }

        Editor_draw(e);

        c = wgetch(e->win);

        if (!Editor_handle_inp(e, c)) {
            break;
        }


    } while (true);

    return 1;
}



/*
 * This function is messy, and will be redone.
 */
int Editor_handle_command(Editor *e)
{
    size_t i, l, k;
    int tempc;
    char *eptr;
    List *list;
    Buffer *buffer;

    if (!e || !e->cmd || !e->cmd->str) {
        return 0;
    }

    for (i = 0; i < e->cmd->len; i++) {
        switch(e->cmd->str[i]) {
        case 'w':
            /* check if path is provided as the next argument */
            for (l = i; l < e->cmd->len && e->cmd->str[l] != ' '; l++);
            for (; l < e->cmd->len && e->cmd->str[l] == ' '; l++);
            for (k = l; k < e->cmd->len && e->cmd->str[k] != ' '; k++);

            /* update buffer's filepath if path is provided */
            if (l < e->cmd->len - 1) {
                free(e->buf->path);
                e->buf->path = strndup(&e->cmd->str[l], k - l);
            }

            /* save buffer */
            Buffer_write(e->buf, e->buf->path);

            /* clear the used arguments */
            e->cmd->str[i] = ' ';
            memset(&e->cmd->str[l], ' ', k - l);
            break;

        case 'e':
            /* check if path is provided as the next argument */
            for (l = i; l < e->cmd->len && e->cmd->str[l] != ' '; l++);
            for (; l < e->cmd->len && e->cmd->str[l] == ' '; l++);
            for (k = l; k < e->cmd->len && e->cmd->str[k] != ' '; k++);

            /* open new buffer if path is provided */
            if (l < e->cmd->len - 1) {
                buffer = Buffer_new();
                if (!buffer) {
                    return -1;
                };

                Buffer_init(buffer);
                Buffer_load(buffer, &e->cmd->str[l]);

                list = List_new();
                if (!list) {
                    return -1;
                };

                list->data = buffer;

                e->bufs = List_append(e->bufs, list);

                e->buf = buffer;

            }

            return 1;
            break;

        case 'q':
            /* quit */
            return -1;
            break;

        case 'n':
            Editor_next_buffer(e);
            break;

        case 'p':
            Editor_prev_buffer(e);
            break;

        default:
            /* TODO: THIS IS A HACK, FIX IT  - SEPARATE INTO FUNCTION */
            if (isdigit(e->cmd->str[i])) {
                tempc = e->cmd->str[e->cmd->len - 1];
                e->cmd->str[e->cmd->len - 1] = 0;

                l = strtol(&e->cmd->str[i], &eptr, 10);
                i = eptr - e->cmd->str;

                e->cmd->str[e->cmd->len - 1] = tempc;

                if (isdigit(tempc)) {
                    l *= 10;
                    l += tempc - '0';
                }

                if (l > e->buf->lines->len - 1) {
                    l = e->buf->lines->len - 1;
                }

                E_GET_POS_LINE = l;
                e->crow = 0;

            }
            break;
        }
    }

    e->cmd->len = 0;
    e->cmd->str[0] = '\0';

    return 1;
}

#define KEY_ESCAPE_OR_ALT 27

int Editor_handle_mode_normal(Editor *e, int c)
{
    int tc;

    switch (c) {
    /* Movement */
    case 'j':
    /* FALLTHROUGH */
    case KEY_DOWN:
        Editor_handle_next_line(e);
        break;

    case 'J':
        Editor_handle_scroll_next_line(e);
        break;

    case 'k':
    /* FALLTHROUGH */
    case KEY_UP:
        Editor_handle_prev_line(e);
        break;

    case 'K':
        Editor_handle_scroll_prev_line(e);
        break;


    case 'h':
    /* FALLTHROUGH */
    case KEY_LEFT:
        if (E_GET_POS_COL > 0) {
            (E_GET_POS_COL)--;
        }
        break;

    case 'l':
    /* FALLTHROUGH */
    case KEY_RIGHT:
        if (E_GET_POS_COL < E_END_OF_LINE) {
            (E_GET_POS_COL)++;
        }
        break;

    case 'w':
        Editor_next_word(e);
        break;

    case 'b':
        Editor_prev_word(e);
        break;

    case 'e':
        Editor_endof_word(e);
        break;

    case 'x':
        Line_removeat(E_GET_CLINE, 1, E_GET_POS_COL);
        if (E_EXCEEDED_LINE) {
            E_GET_POS_COL = E_END_OF_LINE;;
        }
        break;

    case 'd':
        c = wgetch(e->win);

        if (c == 'd') {
            Lines_rline(e->buf->lines, E_GET_POS_LINE);

            if (E_EXCEEDED_BUFFER) {
                /* Move editor to the previous line so we don't exceed the buffer */
                Editor_handle_prev_line(e);
            }
            E_GET_POS_COL = E_END_OF_LINE;
        } else {
            ungetch(c);
        }

        break;

    case 'i':
        e->state.type = EDITOR_INSERT;
        break;

    case 'a':
        e->buf->pos.col++;
        e->state.type = EDITOR_INSERT;
        break;

    case 'o':
        Lines_iline(e->buf->lines, NULL, E_GET_POS_LINE + 1);
        (E_GET_POS_LINE)++;
        E_GET_POS_COL = 0;
        e->crow++;

        e->state.type = EDITOR_INSERT;
        break;

    case 'O':
        Lines_iline(e->buf->lines, NULL, e->buf->pos.line);
        E_GET_POS_COL = 0;

        e->state.type = EDITOR_INSERT;
        break;

    case 'G':
        /* Move cursor to the last line */
        if (e->buf->lines->len > (size_t)LINES - 1 - e->rboff) {
            e->crow = LINES - 1 - e->rboff;
        } else {
            e->crow = e->buf->lines->len - 1;
        }

        E_GET_POS_LINE = e->buf->lines->len - 1;
        E_GET_POS_COL = 0;
        break;

    case 'g':
        tc = wgetch(e->win);

        if (tc == 't') {
            Editor_next_buffer(e);
        } else if (tc == 'T') {
            Editor_prev_buffer(e);
        } else if (tc == 'g') {
            /* nothing to do here */
            /* WARN: falls through */

        } else {
            ungetch(tc);
            break;
        }
         
        e->crow = 0;
        E_GET_POS_COL = 0;
        E_GET_POS_LINE = 0;

        break;

    case '$':
        E_GET_POS_COL = E_END_OF_LINE;
        break;

    case '0':
        E_GET_POS_COL = 0;
        break;

    case KEY_ENTER:
    case '\n':
        Editor_handle_next_line(e);
        break;

    case KEY_BACKSPACE:
    case KEY_DC:
    case KEY_DL:
    case KEY_BREAK:
    case '\b':
        if (E_GET_POS_COL > 0) {
            (E_GET_POS_COL)--;
        }
        break;

    case ' ':
        if (E_GET_POS_COL < E_GET_CLINE_LEN - 1) {
            (E_GET_POS_COL)++;
        }
        break;

    /* Mode Change */
    case ':':
        e->state.type = EDITOR_COMMAND;
        e->rboff++;
        e->cmd->len = 0;
        break;

    }
    return 1;
}

int Editor_handle_mode_insert(Editor *e, int c)
{
    switch (c) {
    case KEY_BACKSPACE: /* backspace */
        if (E_GET_POS_COL > 0) {
            Line_removeat(E_GET_CLINE, 1, --(E_GET_POS_COL));
        }
        break;

    case KEY_DC:        /* delete*/
        Line_removeat(E_GET_CLINE, 1, E_GET_POS_COL);
        break;

    case KEY_ENTER:
    case '\n':
        Editor_move_down(e);
        e->crow++;
        (E_GET_POS_LINE)++;
        E_GET_POS_COL = 0;
        break;

    case '\t':         /* tab */
        Line_insertat(E_GET_CLINE, "    ", TAB_WIDTH, E_GET_POS_COL);
        E_GET_POS_COL += 4;
        break;

    case KEY_ESCAPE_OR_ALT:
        e->state.type = EDITOR_NORMAL;

        if (E_GET_CLINE_LEN - 1 < E_GET_POS_COL) {
            E_GET_POS_COL = E_GET_CLINE_LEN - 1;
        }
        break;

    default:
        if (e->buf->lines->len - 1 < E_GET_POS_LINE) {
            Lines_iline(e->buf->lines, NULL, E_GET_POS_LINE);
        }

        Line_insertat(E_GET_CLINE, (char *)&c, 1, E_GET_POS_COL);
        e->buf->pos.col++;

        break;
    }
    return 1;
}

int Editor_handle_mode_command(Editor *e, int c)
{
    switch (c) {
    case KEY_ENTER:
    /* FALLTHROUGH */
    case '\n':
        e->state.type = EDITOR_NORMAL;
        e->rboff--;

        if (Editor_handle_command(e) == -1) {
            e->cmd->len = 0;
            return 0; /* quit */
        }
        e->cmd->len = 0;
        break;

    case KEY_ESCAPE_OR_ALT:
        e->state.type = EDITOR_NORMAL;
        e->rboff--;
        e->cmd->len = 0;
        *e->cmd->str = '\0';
        break;

    default:
        buf[0] = c;
        Line_insertat(e->cmd, (char *)buf, 1, e->cmd->len);
        break;
    }
    return 1;
}

int Editor_handle_inp(Editor *e, int c)
{
    switch (e->state.type) {
    case EDITOR_NORMAL:
        return Editor_handle_mode_normal(e, c);
        break;

    case EDITOR_INSERT:
        return Editor_handle_mode_insert(e, c);
        break;

    case EDITOR_COMMAND:
        return Editor_handle_mode_command(e, c);
        break;

    case EDITOR_VISUAL:
    /* FALLTHROUGH */

    case EDITOR_VISUAL_BLOCK:
        /* FALLTHROUGH */
        break;

    default:
        e->state.type = EDITOR_NORMAL;
        break;
    }
    return 1;
}
