#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <samplerate.h>

#include "sound_file.h"
#include "output.h"

typedef struct _player_t
{
    sound_file_t *sound_file;
    SRC_STATE *src_state;
    output_t *output;
    bool playing;
    double playback_speed;
} player_t;

player_t *player_new();
void player_delete(player_t **player);
int player_load_song(player_t *player, const char *filename);
int player_unload_song(player_t *player);

void player_play(player_t *player);
void player_stop(player_t *player);

#endif // PLAYER_H