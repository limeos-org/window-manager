#pragma once

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/shape.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <execinfo.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <signal.h>
#include <time.h>
#include <math.h>

#include "limits.h"

#include "config/config.h"
#include "backgrounds/backgrounds.h"
// #include "markers/markers.h"

// #include "compositor/compositor.h"
// #include "compositor/graphics.h"

// #include "shortcuts/shortcuts.h"
// #include "shortcuts/input.h"
// #include "shortcuts/terminal.h"
// #include "shortcuts/exit.h"

#include "utils/utils.h"
#include "utils/xlib.h"
#include "utils/xinput.h"
#include "utils/log.h"

#include "ewmh/ewmh.h"
#include "ewmh/client_list.h"
// #include "ewmh/active_window.h"

#include "portals/portals.h"
#include "portals/frames.h"
#include "portals/clients.h"
#include "portals/input.h"
#include "portals/focus.h"
#include "portals/triggers.h"
// #include "portals/dragging.h"
// #include "portals/resizing.h"
// #include "portals/title.h"

#include "events/events.h"
#include "events/handlers.h"
#include "events/xinput.h"
