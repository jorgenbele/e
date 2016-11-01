#ifndef EDITOR_FUNCTIONS_H
#define EDITOR_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"
#include "editor.h"

int Editor_handle_prev_line(Editor *e);
int Editor_handle_next_line(Editor *e);

int Editor_handle_scroll_prev_line(Editor *e);
int Editor_handle_scroll_next_line(Editor *e);

int Editor_next_word(Editor *e);
int Editor_prev_word(Editor *e);
int Editor_endof_word(Editor *e);
int Editor_move_down(Editor *e);

int Editor_next_buffer(Editor *e);
int Editor_prev_buffer(Editor *e);

#endif
