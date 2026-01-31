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
