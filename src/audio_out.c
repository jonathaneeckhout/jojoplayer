/*
** Copyright (c) 1999-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/libsndfile/libsamplerate/blob/master/COPYING
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_out.h"

#include <math.h>

#include <sndfile.h>

#define BUFFER_LEN (2048)

#define MAKE_MAGIC(a, b, c, d, e, f, g, h) \
    ((a) + ((b) << 1) + ((c) << 2) + ((d) << 3) + ((e) << 4) + ((f) << 5) + ((g) << 6) + ((h) << 7))

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API

#include <alsa/asoundlib.h>
#include <sys/time.h>

#define ALSA_MAGIC MAKE_MAGIC('L', 'n', 'x', '-', 'A', 'L', 'S', 'A')

typedef struct AUDIO_OUT
{
    int magic;
    snd_pcm_t *dev;
    int channels;
} ALSA_AUDIO_OUT;

static int alsa_write_float(snd_pcm_t *alsa_dev, float *data, int frames, int channels);

AUDIO_OUT *audio_open(int channels, unsigned samplerate)
{
    ALSA_AUDIO_OUT *alsa_out;
    const char *device = "default";
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_uframes_t buffer_size;
    snd_pcm_uframes_t alsa_period_size, alsa_buffer_frames;
    snd_pcm_sw_params_t *sw_params;

    int err;

    alsa_period_size = 1024;
    alsa_buffer_frames = 4 * alsa_period_size;

    if ((alsa_out = calloc(1, sizeof(ALSA_AUDIO_OUT))) == NULL)
    {
        perror("alsa_open : malloc ");
        exit(1);
    };

    alsa_out->magic = ALSA_MAGIC;
    alsa_out->channels = channels;

    if ((err = snd_pcm_open(&alsa_out->dev, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        fprintf(stderr, "cannot open audio device \"%s\" (%s)\n", device, snd_strerror(err));
        goto catch_error;
    };

    snd_pcm_nonblock(alsa_out->dev, 0);

    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
    {
        fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
        goto catch_error;
    };

    if ((err = snd_pcm_hw_params_any(alsa_out->dev, hw_params)) < 0)
    {
        fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
        goto catch_error;
    };

    if ((err = snd_pcm_hw_params_set_access(alsa_out->dev, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        fprintf(stderr, "cannot set access type (%s)\n", snd_strerror(err));
        goto catch_error;
    };

    if ((err = snd_pcm_hw_params_set_format(alsa_out->dev, hw_params, SND_PCM_FORMAT_FLOAT)) < 0)
    {
        fprintf(stderr, "cannot set sample format (%s)\n", snd_strerror(err));
        goto catch_error;
    };

    if ((err = snd_pcm_hw_params_set_rate_near(alsa_out->dev, hw_params, &samplerate, 0)) < 0)
    {
        fprintf(stderr, "cannot set sample rate (%s)\n", snd_strerror(err));
        goto catch_error;
    };

    if ((err = snd_pcm_hw_params_set_channels(alsa_out->dev, hw_params, channels)) < 0)
    {
        fprintf(stderr, "cannot set channel count (%s)\n", snd_strerror(err));
        goto catch_error;
    };

    if ((err = snd_pcm_hw_params_set_buffer_size_near(alsa_out->dev, hw_params, &alsa_buffer_frames)) < 0)
    {
        fprintf(stderr, "cannot set buffer size (%s)\n", snd_strerror(err));
        goto catch_error;
    };

    if ((err = snd_pcm_hw_params_set_period_size_near(alsa_out->dev, hw_params, &alsa_period_size, 0)) < 0)
    {
        fprintf(stderr, "cannot set period size (%s)\n", snd_strerror(err));
        goto catch_error;
    };

    if ((err = snd_pcm_hw_params(alsa_out->dev, hw_params)) < 0)
    {
        fprintf(stderr, "cannot set parameters (%s)\n", snd_strerror(err));
        goto catch_error;
    };

    /* extra check: if we have only one period, this code won't work */
    snd_pcm_hw_params_get_period_size(hw_params, &alsa_period_size, 0);
    snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
    if (alsa_period_size == buffer_size)
    {
        fprintf(stderr, "Can't use period equal to buffer size (%lu == %lu)", alsa_period_size, buffer_size);
        goto catch_error;
    };

    snd_pcm_hw_params_free(hw_params);

    if ((err = snd_pcm_sw_params_malloc(&sw_params)) != 0)
    {
        fprintf(stderr, "%s: snd_pcm_sw_params_malloc: %s", __func__, snd_strerror(err));
        goto catch_error;
    };

    if ((err = snd_pcm_sw_params_current(alsa_out->dev, sw_params)) != 0)
    {
        fprintf(stderr, "%s: snd_pcm_sw_params_current: %s", __func__, snd_strerror(err));
        goto catch_error;
    };

    /* note: set start threshold to delay start until the ring buffer is full */
    snd_pcm_sw_params_current(alsa_out->dev, sw_params);

    if ((err = snd_pcm_sw_params_set_start_threshold(alsa_out->dev, sw_params, buffer_size)) < 0)
    {
        fprintf(stderr, "cannot set start threshold (%s)\n", snd_strerror(err));
        goto catch_error;
    };

    if ((err = snd_pcm_sw_params(alsa_out->dev, sw_params)) != 0)
    {
        fprintf(stderr, "%s: snd_pcm_sw_params: %s", __func__, snd_strerror(err));
        goto catch_error;
    };

    snd_pcm_sw_params_free(sw_params);

    snd_pcm_reset(alsa_out->dev);

catch_error:

    if (err < 0 && alsa_out->dev != NULL)
    {
        snd_pcm_close(alsa_out->dev);
        return NULL;
    };

    return (AUDIO_OUT *)alsa_out;
} /* alsa_open */

void audio_play(AUDIO_OUT *audio_out, get_audio_callback_t callback, void *priv)
{
    static float buffer[BUFFER_LEN];
    ALSA_AUDIO_OUT *alsa_out;
    int read_frames;

    if ((alsa_out = (ALSA_AUDIO_OUT *)audio_out) == NULL)
    {
        printf("alsa_close : AUDIO_OUT is NULL.\n");
        return;
    };

    if (alsa_out->magic != ALSA_MAGIC)
    {
        printf("alsa_close : Bad magic number.\n");
        return;
    };

    while ((read_frames = callback(buffer, BUFFER_LEN / alsa_out->channels, priv)))
        alsa_write_float(alsa_out->dev, buffer, read_frames, alsa_out->channels);

    return;
} /* alsa_play */

static int
alsa_write_float(snd_pcm_t *alsa_dev, float *data, int frames, int channels)
{
    static int epipe_count = 0;

    int total = 0;
    int retval;

    if (epipe_count > 0)
        epipe_count--;

    while (total < frames)
    {
        retval = snd_pcm_writei(alsa_dev, data + total * channels, frames - total);

        if (retval >= 0)
        {
            total += retval;
            if (total == frames)
                return total;

            continue;
        };

        switch (retval)
        {
        case -EAGAIN:
            puts("alsa_write_float: EAGAIN");
            continue;
            break;

        case -EPIPE:
            if (epipe_count > 0)
            {
                printf("alsa_write_float: EPIPE %d\n", epipe_count);
                if (epipe_count > 140)
                    return retval;
            };
            epipe_count += 100;

            snd_pcm_prepare(alsa_dev);
            break;

        case -EBADFD:
            fprintf(stderr, "alsa_write_float: Bad PCM state.n");
            return 0;
            break;

        case -ESTRPIPE:
            fprintf(stderr, "alsa_write_float: Suspend event.n");
            return 0;
            break;

        case -EIO:
            puts("alsa_write_float: EIO");
            return 0;

        default:
            fprintf(stderr, "alsa_write_float: retval = %d\n", retval);
            return 0;
            break;
        }; /* switch */
    }; /* while */

    return total;
} /* alsa_write_float */

void audio_close(AUDIO_OUT *audio_out)
{
    ALSA_AUDIO_OUT *alsa_out;

    if ((alsa_out = (ALSA_AUDIO_OUT *)audio_out) == NULL)
    {
        printf("alsa_close : AUDIO_OUT is NULL.\n");
        return;
    };

    if (alsa_out->magic != ALSA_MAGIC)
    {
        printf("alsa_close : Bad magic number.\n");
        return;
    };

    memset(alsa_out, 0, sizeof(ALSA_AUDIO_OUT));

    free(alsa_out);

    return;
} /* alsa_close */
