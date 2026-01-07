#include "../all.h"

/**
 * This code is responsible for handling the terminal shortcut action,
 * launching the configured terminal application.
 */

void launch_terminal()
{
    // Get the terminal command from config.
    char terminal_command[CFG_MAX_VALUE_LENGTH];
    GET_CONFIG(terminal_command, sizeof(terminal_command), CFG_BUNDLE_TERMINAL_COMMAND);

    LOG_INFO("Launching terminal: %s", terminal_command);

    // Fork and execute the terminal.
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process.
        // Close stdin/stdout/stderr.
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        // Create a new session.
        setsid();

        // Execute the terminal command.
        execlp(terminal_command, terminal_command, NULL);

        // If exec fails, exit.
        _exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        LOG_ERROR("Failed to fork for terminal launch.");
    }
}

HANDLE(ShortcutPressed)
{
    ShortcutPressedEvent *_event = &event->shortcut_pressed;

    // Check if this is the terminal shortcut.
    if (strcmp(_event->name, "terminal") == 0)
    {
        launch_terminal();
    }
}
