#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

typedef struct List_struct {
  struct List_struct *next;
  struct List_struct *prev;
  void *data;
} List;

/* Function Prototypes */
List *List_new(void);
List *List_end(List *head);
List *List_append(List *head, List *l);
List *List_prepend(List *head, List *l);
List *List_remove(List *head, List *l);         /* not implemented */
List *List_remove_by_data(List *head, void *l); /* not implemented */
void List_free(List *head);

#endif
