#ifndef COMMAND_H
#define COMMAND_H

#include "server.h"
#include "player.h"

typedef struct {
    GRegex *regex;
    void (*callback) (server_t *server, player_t *player, GMatchInfo *match);
} command_t;

#ifndef

