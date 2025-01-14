#ifndef PLAYER_H
#define PLAYER_H

#include <samplerate.h>

#include "sound_file.h"
#include "audio_out.h"

typedef struct _player_t
{
    sound_file_t *sound_file;
    SRC_STATE *src_state;
    AUDIO_OUT *audio_out;
    int freq_point;
} player_t;

player_t *player_new();
void player_delete(player_t **player);
int player_play(player_t *player, const char *filename);

#endif // PLAYER_H