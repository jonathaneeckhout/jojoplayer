#ifndef SOUND_FILE_H
#define SOUND_FILE_H

#include <sndfile.h>

#define BUFFER_LEN 4096

typedef struct _sound_file_t
{
    SNDFILE *sound_file;
    SF_INFO sound_file_info;
    float buffer[BUFFER_LEN];
} sound_file_t;

sound_file_t *sound_file_new(const char *filename);
void sound_file_delete(sound_file_t **sound_file);

#endif // SOUND_FILE_H