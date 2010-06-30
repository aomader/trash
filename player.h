#ifndef PLAYER_H
#define PLAYER_H

typedef struct {
    unsigned id;
    int socket;
} player_t;

extern player_t *player_new(int socket);
extern void player_write(player_t *player, const char *message,
    unsigned length);
extern void player_free(player_t *player);

#endif

