#include "player.h"

#include <stdlib.h>
#include <unistd.h>

#include <glib.h>

extern player_t *player_new(int socket)
{
    player_t *player = g_slice_new(player_t);

    player->socket = socket;
    player->id = 0;

    return player;
}

extern void player_free(player_t *player)
{
    close(player->socket);

    g_slice_free(player_t, player);
}

