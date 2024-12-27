#include "../all.h"

static ConfigEntry config_entries[CFG_MAX_ENTRIES];
static int config_count = 0;

// clang-format off
static const char default_config[] =
    "# ---\n"
    "# Background\n"
    "# --- \n\n"
    "# May either be 'solid' or 'image'.\n"
    CFG_KEY_BACKGROUND_MODE "=" CFG_DEFAULT_BACKGROUND_MODE "\n\n"
    "# A hexadecimal representation of a color.\n"
    "# Used when " CFG_KEY_BACKGROUND_MODE " is set to 'solid'.\n"
    CFG_KEY_BACKGROUND_COLOR "=" CFG_DEFAULT_BACKGROUND_COLOR "\n\n"
    "# A file path to a PNG background image.\n"
    "# Other file formats such as JPG are not supported.\n"
    "# Used when " CFG_KEY_BACKGROUND_MODE " is set to 'image'.\n"
    CFG_KEY_BACKGROUND_IMAGE_PATH "=" CFG_DEFAULT_BACKGROUND_IMAGE_PATH "\n\n";
// clang-format on

static void create_config_directory(const char *path)
{
    size_t path_length = strlen(path);
    if (path_length >= MAX_PATH)
    {
        LOG_ERROR("Configuration directory path too long (%s).", path);
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_PATH];
    for (size_t i = 1; i <= path_length; ++i)
    {
        // Copy the portion of the path up to the current character.
        memcpy(buffer, path, i);
        buffer[i] = '\0';

        // If we encounter a slash or the end of the path, it means we've
        // reached a directory boundary.
        if (path[i] == '/' || i == path_length)
        {
            // Create the directory if it does not exist.
            if (mkdir(buffer, 0755) != 0 && errno != EEXIST)
            {
                LOG_ERROR("Failed to create configuration directory (%s).\n%s", buffer, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }
}

static void create_config_file(const char *path)
{
    FILE *config_file = fopen(path, "w");
    if (!config_file)
    {
        LOG_ERROR("Could not create the default configuration file.");
        return;
    }

    fputs(default_config, config_file);
    fclose(config_file);
}

static void parse_config_file(const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        LOG_ERROR("Could not open the configuration file for parsing.");
        return;
    }

    char line[CFG_MAX_LINE_LENGTH];
    char key[CFG_MAX_KEY_LENGTH];
    char value[CFG_MAX_VALUE_LENGTH];

    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == '#')
        {
            continue; // Ignore comments.
        }

        if (sscanf(line, "%63[^=]=%63s", key, value) == 2)
        {
            strncpy(config_entries[config_count].key, key, CFG_MAX_KEY_LENGTH);
            strncpy(config_entries[config_count].value, value, CFG_MAX_VALUE_LENGTH);
            config_count++;
        }
    }

    fclose(file);
}

void get_config_value_str(char *dest, size_t dest_size, char *key, char *fallback)
{
    if (dest == NULL)
        return;

    if (dest_size > 0)
    {
        for (int i = 0; i < config_count; i++)
        {
            if (strcmp(config_entries[i].key, key) == 0)
            {
                strncpy(dest, config_entries[i].value, dest_size);
                dest[dest_size - 1] = '\0';
                return;
            }
        }
    }

    strncpy(dest, fallback, dest_size);
    dest[dest_size - 1] = '\0';
}

void get_config_value_path(char *dest, size_t dest_size, char *key, char *fallback)
{
    if (dest == NULL)
        return;

    if (dest_size > 0)
    {
        char path[MAX_PATH];
        get_config_value_str(path, sizeof(path), key, fallback);

        char expanded_path[MAX_PATH];
        if (expand_path(path, expanded_path, sizeof(expanded_path)) == 0)
        {
            strncpy(dest, expanded_path, dest_size);
            dest[dest_size - 1] = '\0';
            return;
        }
    }

    strncpy(dest, fallback, dest_size);
    dest[dest_size - 1] = '\0';
}

void get_config_value_hex(unsigned long *dest, size_t dest_size, char *key, char *fallback)
{
    (void)dest_size;
    
    if (dest == NULL)
        return;

    char hex_string[16];
    get_config_value_str(hex_string, sizeof(hex_string), key, fallback);
    *dest = strtoul(hex_string, NULL, 16);
}

HANDLE(Prepare)
{
    // Expand the configuration directory path.
    char config_dir_path[MAX_PATH];
    if (expand_path(CFG_DIRECTORY, config_dir_path, sizeof(config_dir_path)) != 0)
    {
        LOG_WARNING("Failed to expand configuration directory path (%s).", CFG_FILE_PATH);
        return;
    }

    // Expand the configuration file path.
    char config_file_path[MAX_PATH];
    if (expand_path(CFG_FILE_PATH, config_file_path, sizeof(config_file_path)) != 0)
    {
        LOG_WARNING("Failed to expand configuration file path (%s).", CFG_FILE_PATH);
        return;
    }

    // Ensure the configuration file exists. If not, create it.
    struct stat buffer;
    if (stat(config_file_path, &buffer) != 0)
    {
        create_config_directory(config_dir_path);
        create_config_file(config_file_path);
    }

    // Finally, parse the configuration file.
    parse_config_file(config_file_path);
}
