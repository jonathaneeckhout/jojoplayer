#include <stdlib.h>
#include <signal.h>

#include "utils.h"
#include "logging.h"
#include "player.h"

static player_t *player = NULL;

static void handle_sigint(int sig)
{
    if (sig == SIGINT)
    {
        log_info("SIGINT received, stopping player...");
        if (player)
        {
            player_stop(player);
        }
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    const char *file_name = NULL;

    logging_init("jojoplayer", LOG_INFO);

    when_false_log_error(argc == 2, exit, "Usage: %s <file_name>", argv[0]);

    file_name = argv[1];

    player = player_new();

    log_info("Playing file: %s", file_name);

    when_null_log_error(player, exit, "Error: Could not create player");

    when_failed_log_error(player_load_song(player, file_name), exit, "Error: Could not load file");

    signal(SIGINT, handle_sigint);

    player_play(player);

exit:

    player_delete(&player);
    logging_cleanup();

    return 0;
}
