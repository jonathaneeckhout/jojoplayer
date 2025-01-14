/*
** Copyright (c) 1999-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/libsndfile/libsamplerate/blob/master/COPYING
*/

typedef struct AUDIO_OUT AUDIO_OUT;

typedef int (*get_audio_callback_t)(float *samples, int frames, void *priv);

AUDIO_OUT *audio_open(int channels, unsigned samplerate);

void audio_play(AUDIO_OUT *audio_out, get_audio_callback_t callback, void *priv);

void audio_close(AUDIO_OUT *audio_data);
