#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "logging.h"
#include "player.h"

#define BLOCK_LEN 64

static int get_audio_callback(float *samples, int out_frames, void *priv)
{
    player_t *player = (player_t *)priv;
    float *output = NULL;
    int rc = 0;
    int out_frame_count = 0;

    when_null(player, exit);

    for (out_frame_count = 0; out_frame_count < out_frames; out_frame_count += BLOCK_LEN)
    {
        output = samples + out_frame_count * player->sound_file->sound_file_info.channels;

        rc = src_callback_read(player->src_state, player->playback_speed, BLOCK_LEN, output);
        when_true(rc < BLOCK_LEN, exit);
    };

exit:
    return out_frames;
}

static long src_input_callback(void *cb_data, float **audio)
{
    sound_file_t *data = (sound_file_t *)cb_data;
    const int input_frames = ARRAY_LEN(data->buffer) / data->sound_file_info.channels;
    int read_frames = 0;

    for (read_frames = 0; read_frames < input_frames;)
    {
        sf_count_t position;

        read_frames += (int)sf_readf_float(data->sound_file, data->buffer + read_frames * data->sound_file_info.channels, input_frames - read_frames);

        position = sf_seek(data->sound_file, 0, SEEK_CUR);

        if (position < 0 || position == data->sound_file_info.frames)
            sf_seek(data->sound_file, 0, SEEK_SET);
    };

    *audio = &(data->buffer[0]);

    return input_frames;
}

int player_load_song(player_t *player, const char *filename)
{
    int retval = -1;
    int error = 0;

    when_null(player, exit);
    when_str_empty(filename, exit);

    player->sound_file = sound_file_new(filename);
    when_null(player->sound_file, exit);

    player->src_state = src_callback_new(src_input_callback, SRC_SINC_FASTEST, player->sound_file->sound_file_info.channels, &error, player->sound_file);
    when_null_log_error(player->src_state, exit, "Could not create SRC state: %s", src_strerror(error));

    player->output = output_open("default", player->sound_file->sound_file_info.channels, player->sound_file->sound_file_info.samplerate);
    when_null_log_error(player->output, exit, "Could not open audio device.");

    retval = 0;

exit:
    if (retval != 0)
    {
        player_unload_song(player);
    }

    return retval;
}

int player_unload_song(player_t *player)
{
    int retval = -1;

    when_null(player, exit);

    output_close(&player->output);

    src_delete(player->src_state);
    player->src_state = NULL;

    sound_file_delete(&player->sound_file);

    retval = 0;

exit:
    return retval;
}

void player_play(player_t *player)
{
    when_null(player, exit);

    player->playing = true;

    output_play(player->output, (get_audio_callback_t)get_audio_callback, player);
exit:
    return;
}

void player_stop(player_t *player)
{
    when_null(player, exit);

    player->playing = false;

    output_stop(player->output);
exit:
    return;
}

void player_delete(player_t **player)
{
    when_null(*player, exit);

    player_unload_song(*player);

    free(*player);
    *player = NULL;

exit:
    return;
}

player_t *player_new()
{
    int retval = -1;
    player_t *player = (player_t *)malloc(sizeof(player_t));
    when_null(player, exit);

    player->sound_file = NULL;
    player->src_state = NULL;
    player->output = NULL;
    player->playing = false;
    player->playback_speed = 1.0;

    retval = 0;

exit:
    if (retval != 0)
    {
        player_delete(&player);
    }

    return player;
}
