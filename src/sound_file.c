#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "logging.h"
#include "sound_file.h"

sound_file_t *sound_file_new(const char *filename)
{
    int retval = -1;
    sound_file_t *sound_file = (sound_file_t *)malloc(sizeof(sound_file_t));
    when_null_log_error(sound_file, exit, "Error: Could not allocate memory for sound file");

    memset(&sound_file->sound_file_info, 0, sizeof(SF_INFO));

    sound_file->sound_file = sf_open(filename, (int)SFM_READ, &sound_file->sound_file_info);
    when_null_log_error(sound_file->sound_file, exit, "Error: Could not open sound file");

    memset(sound_file->buffer, 0, sizeof(sound_file->buffer));

    retval = 0;

exit:
    if (retval != 0)
    {
        sound_file_delete(&sound_file);
    }

    return sound_file;
}

void sound_file_delete(sound_file_t **sound_file)
{
    when_null(*sound_file, exit);

    if ((*sound_file)->sound_file != NULL)
    {
        sf_close((*sound_file)->sound_file);
    }

    free(*sound_file);
    *sound_file = NULL;

exit:
    return;
}