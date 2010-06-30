#ifndef SERVER_H
#define SERVER_H

#include <glib.h>

typedef struct {
    int socket;
    int epollfd;

    GThreadPool *worker;

    GSList *players;
} server_t;

extern server_t *server_new(unsigned port);
extern void server_free(server_t *server);

#endif

