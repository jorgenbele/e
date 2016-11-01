#include "list.h"

List *List_new(void)
{
    List *list = malloc(sizeof(List));

    if (!list)
        return NULL;

    list->next = NULL;
    list->prev = NULL;
    list->data = NULL;

    return list;
}

List *List_end(List *head)
{
    if (!head)
        return NULL;

    while (head->next)
        head = head->next;

    return head;
}

List *List_append(List *head, List *l)
{
    List *end;

    if (!l || !head)
        return NULL;

    end = List_end(head);
    end->next = l;
    l->prev = end;

    head->prev = l;

    return head;
}

List *List_prepend(List *head, List *l)
{
    List *end;

    if (!l || !head)
        return NULL;

    end = List_end(l);

    end->next = head;
    head->prev = end;

    return end;
}

#if 0
/* NOT TESTED */

List *List_remove(List *head, List *l)
{
    List *tmp_l;

    for (tmp_l = head; tmp_l != l && tmp_l->next != NULL; tmp_l = tmp_l->next);

    if (tmp_l == l) {

        if (tmp_l->next && tmp_l->prev) {
            tmp_l->prev->next = tmp_l->next;
            tmp_l->next->prev = tmp_l->prev;
        } else if (tmp_l->next) {
            tmp_l->next->prev = NULL;
        } else if (tmp_l->prev) {
            tmp_l->prev->next = NULL;
        }

        if (head == tmp_l) {
            head = tmp_l->next;
        }

        return head;
    }
    return NULL;
}

List *List_remove_by_data(List *head, void *l)
{
    List *tmp_l;

    for (tmp_l = head; &tmp_l->data != l && tmp_l->next != NULL; tmp_l = tmp_l->next);

    if (&tmp_l->data == l) {

        if (tmp_l->next && tmp_l->prev) {
            tmp_l->prev->next = tmp_l->next;
            tmp_l->next->prev = tmp_l->prev;
        } else if (tmp_l->next) {
            tmp_l->next->prev = NULL;
        } else if (tmp_l->prev) {
            tmp_l->prev->next = NULL;
        }

        if (head == tmp_l) {
            head = tmp_l->next;
        }

        return head;
    }
    return NULL;
}
#endif

void List_free(List *head)
{
    List *tmp_l;

    while (head) {
        free(head->data);
        tmp_l = head->next;
        free(head);
        head = tmp_l;
    }
}
