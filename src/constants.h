#pragma once
#include "all.h"

/** The boolean true value. */
#ifdef true
    #undef true
#endif
#define true 1

/** The boolean false value. */
#ifdef false
    #undef false
#endif
#define false 0

/** The mathematical constant π. */
#define PI 3.14159265358979323846

/** The maximum length of a file path. */
#define MAX_PATH 128

/** The maximum number of shortcuts that can be registered. */
#define MAX_SHORTCUTS 16

/** The maximum length of a shortcut name. */
#define MAX_SHORTCUT_NAME 32

/** The maximum number of keys that can be used in a shortcut. */
#define MAX_SHORTCUT_KEYS 3
