#include "../all.h"

static void handle_terminal_shortcut()
{
    pid_t pid = fork();

    // The code below will only execute in the parent process.
    if (pid > 0)
    {
        // Ignore the child process termination signal. By doing so, we ensure
        // that the child process will be reaped by the system when it exits.
        signal(SIGCHLD, SIG_IGN);
    }

    // The code below will only execute in the forked child process.
    if (pid == 0)
    {
        // Replace the current process with the terminal.
        const char *command = get_terminal_command();
        char **args = common.split_string(command, " ", NULL);
        if (args == NULL) _exit(EXIT_FAILURE);
        execvp(args[0], args);

        // If the process was not replaced, exit the current process.
        _exit(EXIT_FAILURE);
    }
}

HANDLE(ShortcutPressed)
{
    ShortcutPressedEvent *_event = &event->shortcut_pressed;

    // Ensure we're handling the terminal shortcut.
    if (strcmp(_event->name, CFG_KEY_TERMINAL_SHORTCUT) != 0) return;

    handle_terminal_shortcut();
}
