#include "utils.h"
#include "logging.h"
#include "output.h"

#define BUFFER_LEN 2048
#define PERIOD_SIZE 1024
#define BUFFER_FRAMES 4 * PERIOD_SIZE

static int xrun_recovery(snd_pcm_t *handle, int err)
{
    int retval = 0;

    if (err == -EPIPE)
    {
        err = snd_pcm_prepare(handle);
        when_true_log_error(err < 0, exit, "Can't recovery from underrun, prepare failed: %s", snd_strerror(err));
    }
    else if (err == -ESTRPIPE)
    {
        while ((err = snd_pcm_resume(handle)) == -EAGAIN)
        {
            sleep(1);
        }

        if (err < 0)
        {
            err = snd_pcm_prepare(handle);
            when_true_log_error(err < 0, exit, "Can't recovery from suspend, prepare failed: %s", snd_strerror(err));
        }

        goto exit;
    }

    retval = err;

exit:
    return retval;
}

static int pcm_write_float(snd_pcm_t *handle, float *data, int frames, int channels)
{
    int out = 0;
    int total = 0;
    int retval = 0;

    while (total < frames)
    {
        retval = snd_pcm_writei(handle, data + total * channels, frames - total);

        if (retval >= 0)
        {
            total += retval;
            if (total == frames)
                return total;

            continue;
        }

        if (retval == -EAGAIN)
            continue;
        if (retval < 0)
        {
            when_true_log_error(xrun_recovery(handle, retval) < 0, exit, "Write error: %s", snd_strerror(retval));

            break;
        }
    }

    out = total;

exit:

    return out;
}

void output_play(output_t *output, get_audio_callback_t callback, void *priv)
{
    int retval = -1;
    int read_frames = 0;
    static float buffer[BUFFER_LEN];

    when_null(output, exit);

    output->playing = true;

    memset(buffer, 0, sizeof(buffer));

    while (output->playing)
    {
        read_frames = callback(buffer, BUFFER_LEN / output->channels, priv);
        if (read_frames == 0)
        {
            output->playing = false;
            break;
        }

        pcm_write_float(output->handle, buffer, read_frames, output->channels);
    }

    retval = 0;

exit:
    if (retval != 0)
    {
        output_stop(output);
    }
}

void output_stop(output_t *output)
{
    when_null(output, exit);

    output->playing = false;

exit:
    return;
}

output_t *output_open(const char *device, int channels, unsigned samplerate)
{
    int retval = -1;
    output_t *output = NULL;
    snd_pcm_hw_params_t *hw_params = NULL;
    snd_pcm_uframes_t buffer_size = 0;
    snd_pcm_uframes_t period_size = PERIOD_SIZE;
    snd_pcm_uframes_t buffer_frames = BUFFER_FRAMES;
    snd_pcm_sw_params_t *sw_params = NULL;

    when_str_empty(device, exit);

    output = calloc(1, sizeof(output_t));
    when_null(output, exit);

    output->handle = NULL;
    output->playing = false;
    output->channels = channels;
    output->samplerate = samplerate;

    when_true_log_error(snd_pcm_open(&output->handle, device, SND_PCM_STREAM_PLAYBACK, 0) < 0, exit, "Could not open audio device.");

    when_failed_log_error(snd_pcm_nonblock(output->handle, 0), exit, "Could not set non-blocking mode.");

    when_failed_log_error(snd_pcm_hw_params_malloc(&hw_params), exit, "Could not allocate hardware parameters.");

    when_true_log_error(snd_pcm_hw_params_any(output->handle, hw_params) < 0, exit, "Could not initialize hardware parameters.");

    when_failed_log_error(snd_pcm_hw_params_set_access(output->handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED), exit, "Could not set access type.");

    when_failed_log_error(snd_pcm_hw_params_set_format(output->handle, hw_params, SND_PCM_FORMAT_FLOAT), exit, "Could not set sample format.");

    when_failed_log_error(snd_pcm_hw_params_set_rate_near(output->handle, hw_params, &samplerate, 0), exit, "Could not set sample rate.");

    when_failed_log_error(snd_pcm_hw_params_set_channels(output->handle, hw_params, channels), exit, "Could not set channels.");

    when_failed_log_error(snd_pcm_hw_params_set_buffer_size_near(output->handle, hw_params, &buffer_frames), exit, "Could not set buffer size.");

    when_failed_log_error(snd_pcm_hw_params_set_period_size_near(output->handle, hw_params, &period_size, 0), exit, "Could not set period size.");

    when_failed_log_error(snd_pcm_hw_params(output->handle, hw_params), exit, "Could not set hardware parameters.");

    snd_pcm_hw_params_get_period_size(hw_params, &period_size, 0);

    snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);

    when_true_log_error(period_size == buffer_size, exit, "Can't use period equal to buffer size (%lu == %lu)", period_size, buffer_size);

    when_failed_log_error(snd_pcm_sw_params_malloc(&sw_params), exit, "Could not allocate software parameters.");

    when_failed_log_error(snd_pcm_sw_params_current(output->handle, sw_params), exit, "Could not initialize software parameters.");

    when_failed_log_error(snd_pcm_sw_params_set_start_threshold(output->handle, sw_params, buffer_size), exit, "Could not set start threshold.");

    when_failed_log_error(snd_pcm_sw_params(output->handle, sw_params), exit, "Could not set software parameters.");

    snd_pcm_reset(output->handle);

    retval = 0;

exit:
    if (retval != 0)
    {
        output_close(&output);
    }

    if (hw_params != NULL)
    {
        snd_pcm_hw_params_free(hw_params);
    }

    if (sw_params != NULL)
    {
        snd_pcm_sw_params_free(sw_params);
    }

    return output;
}

void output_close(output_t **output)
{
    when_null(output, exit);
    when_null(*output, exit);

    snd_pcm_drain((*output)->handle);

    snd_pcm_close((*output)->handle);

    free(*output);
    *output = NULL;

exit:
    return;
}