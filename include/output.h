#ifndef OUPUT_H
#define OUPUT_H

#include <stdbool.h>
#include <alsa/asoundlib.h>

typedef struct _output_t
{
    snd_pcm_t *handle;
    bool playing;
    int channels;
    unsigned samplerate;

} output_t;

typedef int (*get_audio_callback_t)(float *samples, int frames, void *priv);

output_t *output_open(const char *device, int channels, unsigned samplerate);
void output_close(output_t **output);

void output_play(output_t *output, get_audio_callback_t callback, void *priv);
void output_stop(output_t *output);

#endif // OUTPUT_H