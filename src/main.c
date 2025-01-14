#include <stdlib.h>

#include "utils.h"
#include "logging.h"
#include "player.h"

int main(int argc, char *argv[])
{
    const char *file_name = NULL;
    player_t *player = NULL;

    logging_init("jojoplayer", LOG_INFO);

    when_false_log_error(argc == 2, exit, "Usage: %s <file_name>", argv[0]);

    file_name = argv[1];

    player = player_new();

    log_info("Playing file: %s", file_name);

    when_null_log_error(player, exit, "Error: Could not create player");

    when_failed_log_error(player_play(player, file_name), exit, "Error: Could not play file");

exit:

    player_delete(&player);

    logging_cleanup();

    return 0;
}