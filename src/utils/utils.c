#include "../all.h"

bool is_library_available(const char *name)
{
    void *handle = dlopen(name, RTLD_NOW);
    if (handle == NULL) {
        return False;
    }
    dlclose(handle);
    return True;
}

char** split_string(const char* string, const char* delimiter, int* out_count)
{
    // Make a copy of the provided string.
    char* string_copy = strdup(string);
    if (string_copy == NULL) return NULL;

    // Count the number of tokens, using strtok().
    int token_count = 0;
    char* token = strtok(string_copy, delimiter);
    while (token != NULL)
    {
        token_count++;
        token = strtok(NULL, delimiter);
    }

    // Reset the string copy, so we can use strtok() again later.
    strcpy(string_copy, string);

    // Allocate memory for the array of tokens.
    char** tokens = malloc((token_count + 1) * sizeof(char*));
    if (tokens == NULL)
    {
        free(string_copy);
        return NULL;
    }

    // Split the string and store the tokens in the array.
    int index = 0;
    token = strtok(string_copy, delimiter);
    while (token != NULL)
    {
        tokens[index] = strdup(token);
        token = strtok(NULL, delimiter);
        index++;
    }
    tokens[index] = NULL; // Null-terminate the array.

    // Assign the token count to the `out_count` parameter.
    if (out_count != NULL) {
        *out_count = token_count;
    }

    // Free the string copy from memory.
    free(string_copy);

    return tokens;
}

int framerate_to_throttle_ms(int framerate)
{
    if (framerate <= 0) return 1;
    return 1000 / framerate;
}

unsigned long rgb_to_hex(double r, double g, double b)
{
    unsigned long hex = 0;
    hex |= ((int)(r * 255) & 0xFF) << 16;
    hex |= ((int)(g * 255) & 0xFF) << 8;
    hex |= ((int)(b * 255) & 0xFF);
    return hex;
}

unsigned long rgba_to_hex(double r, double g, double b, double a)
{
    unsigned long hex = 0;
    hex |= ((int)(r * 255) & 0xFF) << 24;
    hex |= ((int)(g * 255) & 0xFF) << 16;
    hex |= ((int)(b * 255) & 0xFF) << 8;
    hex |= ((int)(a * 255) & 0xFF);
    return hex;
}

void hex_to_rgb(unsigned long hex, double *r, double *g, double *b)
{
    *r = ((hex >> 16) & 0xFF) / 255.0;
    *g = ((hex >> 8) & 0xFF) / 255.0;
    *b = (hex & 0xFF) / 255.0;
}

void hex_to_rgba(unsigned long hex, double *r, double *g, double *b, double *a)
{
    *r = ((hex >> 24) & 0xFF) / 255.0;
    *g = ((hex >> 16) & 0xFF) / 255.0;
    *b = ((hex >> 8) & 0xFF) / 255.0;
    *a = (hex & 0xFF) / 255.0;
}

int expand_path(const char *path, char *expanded_path, size_t size)
{
    if (path == NULL || expanded_path == NULL || size == 0)
        return -1;

    if (path[0] == '~')
    {
        const char *home = getenv("HOME");
        if (!home)
            return -1; // Home environment variable is not available.

        int char_written_count = snprintf(expanded_path, size, "%s%s", home, path + 1);
        if (char_written_count < 0 || (size_t)char_written_count >= size)
            return -1; // snprintf error or buffer too small.
    }
    else
    {
        int char_written_count = snprintf(expanded_path, size, "%s", path);
        if (char_written_count < 0 || (size_t)char_written_count >= size)
            return -1; // snprintf error or buffer too small.
    }

    return 0;
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}
