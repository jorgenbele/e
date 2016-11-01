#include "e.h"

List *bufs = NULL;

static int init_buffers(int argc, const char **files);
static void print_buffers(void);
static void cleanup_buffers(void);

int main(int argc, const char **argv)
{
    Editor *e;

    setlocale(LC_ALL, "");

    if (argc > 1) {
        init_buffers(argc-1, argv+1);
    } else {
        init_buffers(0, NULL);

        /*
        printf("Usage: %s FILES\n", argv[0]);
        exit(0);
        */
    }

    e = Editor_new();

    Editor_set_buffers(e, bufs->next);
    Editor_init_curses(e);
    Editor_loop(e);

    Editor_destroy(e);
    cleanup_buffers();

    return 0;
}

static int init_buffers(int argc, const char **files)
{
    int i;
    Buffer *buffer;
    List *node;

    bufs = List_new();

    if (!bufs) {
        return 0;
    }

    if (argc == 0) {
        buffer = Buffer_new();
        Buffer_init(buffer);

        node = List_new();

        if (!node) {
            return 0;
        }

        node->data = buffer;
        List_append(bufs, node);
    }

    for (i = 0; i < argc; i++) {
        /* Create and load buffer */
        buffer = Buffer_new();
        if (!Buffer_load(buffer, files[i])) {
            if (!Buffer_init(buffer)) {
                elogf("Unable to load buffer: %s\n", files[i]);
                Buffer_free(buffer);
                free(buffer);
                continue;
            }

            buffer->path = strdup(files[i]);
        }

        node = List_new();

        if (!node) {
            return 0;
        }

        node->data = buffer;
        List_append(bufs, node);
    }

    return 1;
}

static void print_buffers(void)
{
    List *node = bufs->next;

    while (node != NULL) {
        Buffer_print(node->data);
        node = node->next;
    }
}

static void cleanup_buffers(void)
{
    List *t;
    List *node = bufs;

    while (node != NULL) {
        Buffer_free(node->data);
        free(node->data);
        t = node->next;
        free(node);
        node = t;
    }
}

