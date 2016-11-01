#ifndef E_H
#define E_H

/*
 * Pseudocode:
 * 
 * When the editor is started it first:
 * - Initializes one or more buffers
 * - It starts the editor which sends commands 
 *   for the buffer to execute.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "utils.h"  /* contains commonly used functions
                        such as logging, etc...    */

#include "list.h"   /* contains the implementation
                        for the doubly linked list */

#include "buffer.h" /* where the most important bit lies,
                        the handling of buffers    */

#include "editor.h" /* where anything related to user input
                       and the user interface lies  */

extern List *bufs;

#endif
