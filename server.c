#include "server.h"

#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "player.h"
#include "log.h"

static void server_new_player(server_t *server);
static void server_free_player(server_t *server, player_t *player);
static void server_handle_player(player_t *player, server_t *server); 

extern server_t *server_new(unsigned port)
{
    server_t *server = g_slice_new(server_t);

    server->socket = -1;
    server->epollfd = -1;
    server->players = NULL;

    server->socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server->socket == -1) {
        perror("socket()");
        server_free(server);
        return NULL;
    }

    struct sockaddr_in addr = {
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port),
        .sin_family = AF_INET
    };

    if (bind(server->socket, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind()");
        server_free(server);
        return NULL;
    }

    if (listen(server->socket, 3) == -1) {
        perror("listen()");
        server_free(server);
        return NULL;
    }

    server->epollfd = epoll_create(10);
    if (server->epollfd < 0) {
        perror("epoll_create()");
        server_free(server);
        return NULL;
    }
    
    struct epoll_event event = {
        .events = EPOLLIN | EPOLLPRI,
        .data.ptr = server
    };

    if (epoll_ctl(server->epollfd, EPOLL_CTL_ADD, server->socket, &event)
        == -1)
    {
        perror("epoll_ctl()");
        server_free(server);
        return NULL;
    }

    g_thread_init(NULL);

    GError *error = NULL;
    server->worker = g_thread_pool_new(server_handle_player, server, 3, TRUE,
        &error);

    if (error != NULL) {
        printf("g_thread_pool_new(): %s\n", error->message);
        g_error_free(error);
        server_free(server);
        return NULL;
    }

    g_thread_pool_stop_unused_threads();

    return server;
}

extern int server_run(server_t *server)
{
    int nfds;
    struct epoll_event events[5];

    message("Entering main loop");

    while (1) {
        nfds = epoll_wait(server->epollfd, events, 5, -1);

        if (nfds < 0)
            perror("epoll_wait()");

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.ptr == server) { 
                server_new_player(server);
            } else {
                if (events[i].events & EPOLLHUP)
                    server_free_player(server, events[i].data.ptr);
                else {
                    GError *error = NULL;
                    printf("pushed!\n");
                    g_thread_pool_push(server->worker, events[i].data.ptr, &error);
                    if (error != NULL) {
                        printf("g_thread_pool_push(): %s\n", error->message);
                        g_error_free(error);
                    }
                }
            }
        }
    }

    return 0;
}

extern void server_free(server_t *server)
{
    if (server->epollfd != -1)
        close(server->socket);

    if (server->socket != -1)
        close(server->socket);

    g_slice_free(server_t, server);
}

static void server_new_player(server_t *server)
{
    int client = accept(server->socket, NULL, 0);

    if (client == -1) {
        perror("accept()");
        return;
    }

    player_t *player = player_new(client);

    struct epoll_event event = {
        .events = EPOLLIN | EPOLLPRI,
        .data.ptr = player
    };

    if (epoll_ctl(server->epollfd, EPOLL_CTL_ADD, client, &event) == -1)
        perror("epoll_ctl()");

    server->players = g_slist_prepend(server->players, player);

    printf("New player connected. (Socket: %i)\n", client);
}

static void server_free_player(server_t *server, player_t *player)
{
    printf("Player disconnected. (Socket: %i)\n", player->socket);

    server->players = g_slist_remove(server->players, player);

    player_free(player);
}

static void server_handle_player(player_t *player, server_t *server) 
{
    printf("Handle player message. (Socket: %i)\n", player->socket);

    char buffer[1024];
    int r = recv(player->socket, buffer, 1024, 0);

    if (r <= 0)
        return;

    printf("<< %.*s", r, buffer);

    GRegex *login = g_regex_new("^login ([a-zA-Z0-9]+) ([a-zA-Z0-9]+)$",
        G_REGEX_CASELESS, 0, NULL);

    GMatchInfo *match;

    if (g_regex_match_full(login, buffer, r, 0, 0, &match, NULL) == TRUE) {
        gchar *user = g_match_info_fetch(match, 1);
        gchar *reply = g_strdup_printf("Hello %s\n", user);
        send(player->socket, reply, strlen(reply), 0);
    } else {
        send(player->socket, "Unknown command\n", 16, 0);
    }
}

int main()
{
    server_t *server = server_new(31337);

    if (server != NULL)
        return server_run(server);
    
    return 1;
}

