#pragma once
#include "../all.h"

/**
 * An event that gets triggered before the event loop starts.
 * 
 * This event is reserved for initialization tasks that other modules
 * may depend on for their initialization tasks. For regular initialization 
 * logic, use the `Initialize` event instead.
 */
#define Prepare 128
typedef struct {
    int type;
} PrepareEvent;

/**
 * An event that gets triggered before the event loop starts.
 */
#define Initialize 129
typedef struct {
    int type;
} InitializeEvent;

/**
 * An event that gets triggered at regular intervals based on the configured 
 * framerate (60 FPS = every 16.67 ms).
 */
#define Update 130
typedef struct {
    int type;
} UpdateEvent;

/**
 * An event that gets triggered whenever a portal is created.
 */
#define PortalCreated 131
typedef struct {
    int type;
    Portal *portal;
} PortalCreatedEvent;

/**
 * An event that gets triggered whenever a portal is destroyed.
 * 
 * Both the client and frame windows are destroyed at this point, but their 
 * ID's may still be used to perform any necessary cleanup.
 */
#define PortalDestroyed 132
typedef struct {
    int type;
    Window client_window;
    Window frame_window;
} PortalDestroyedEvent;

/**
 * An event that gets triggered whenever a portal is raised.
 */
#define PortalRaised 133
typedef struct {
    int type;
    Portal *portal;
} PortalRaisedEvent;

/**
 * An event that gets triggered when a portal is moved or resized.
 */
#define PortalTransformed 134
typedef struct {
    int type;
    Portal *portal;
} PortalTransformedEvent;

/**
 * An event triggered when a pointer button is pressed on a portal.
 */
#define PortalButtonPress 135
typedef struct {
    int type;
    Portal *portal;
    int x_root; int y_root;
    int x_portal; int y_portal;
    int button;
} PortalButtonPressEvent;

/**
 * An event triggered when a pointer button is released on a portal.
 */
#define PortalButtonRelease 136
typedef struct {
    int type;
    Portal *portal;
    int x_root; int y_root;
    int x_portal; int y_portal;
    int button;
} PortalButtonReleaseEvent;

/**
 * An event triggered when a pointer hovers over a portal.
 */
#define PortalMotionNotify 137
typedef struct {
    int type;
    Portal *portal;
    int x_root; int y_root;
    int x_portal; int y_portal;
} PortalMotionNotifyEvent;

/**
 * An event that gets triggered when a shortcut is pressed.
 */
#define ShortcutPressed 138
typedef struct {
    int type;
    char *name;
} ShortcutPressedEvent;

/**
 * An event triggered before the `ButtonPress` event, provided by the XInput2 
 * extension.
 * 
 * It bypasses X11's event mask ownership system, at the cost of losing out on 
 * most of the data typically provided with a traditional `ButtonPress` event.
 */
#define RawButtonPress 139
typedef struct {
    int type;
    int button;
} RawButtonPressEvent;

/**
 * An event triggered before the `ButtonRelease` event, provided by the XInput2 
 * extension.
 * 
 * It bypasses X11's event mask ownership system, at the cost of losing out on 
 * most of the data typically provided with a traditional `ButtonRelease` event.
 */
#define RawButtonRelease 140
typedef struct {
    int type;
    int button;
} RawButtonReleaseEvent;

/**
 * An event triggered before the `MotionNotify` event, provided by the XInput2 
 * extension.
 * 
 * It bypasses X11's event mask ownership system, at the cost of losing out on 
 * all of the data typically provided with a traditional `MotionNotify` event.
 */
#define RawMotionNotify 141
typedef struct {
    int type;
} RawMotionNotifyEvent;

/**
 * An event triggered before the `KeyPress` event, provided by the XInput2 
 * extension.
 * 
 * It bypasses X11's event mask ownership system, at the cost of losing out on 
 * most of the data typically provided with a traditional `KeyPress` event.
 */
#define RawKeyPress 142
typedef struct {
    int type;
    int key_code;
} RawKeyPressEvent;

/**
 * An event triggered before the `KeyRelease` event, provided by the XInput2
 * extension.
 * 
 * It bypasses X11's event mask ownership system, at the cost of losing out on 
 * most of the data typically provided with a traditional `KeyRelease` event.
 */
#define RawKeyRelease 143
typedef struct {
    int type;
    int key_code;
} RawKeyReleaseEvent;

/**
 * A union of all possible event types that can be handled by the window
 * manager.
 * 
 * This union matches the XEvent structure's memory layout, allowing 
 * safe downcasting from XEvent to Event. Upcasting from Event to XEvent will 
 * result in undefined behavior and should be avoided.
 */
typedef union {
    int type;

    // Generic events.
    PrepareEvent prepare;
    InitializeEvent initialize;
    UpdateEvent update;

    // Portal events.
    PortalCreatedEvent portal_created;
    PortalDestroyedEvent portal_destroyed;
    PortalRaisedEvent portal_raised;
    PortalTransformedEvent portal_transformed;
    PortalButtonPressEvent portal_button_press;
    PortalButtonReleaseEvent portal_button_release;
    PortalMotionNotifyEvent portal_motion_notify;

    // Shortcut events.
    ShortcutPressedEvent shortcut_pressed;

    // XInput2 events.
    RawButtonPressEvent raw_button_press;
    RawButtonReleaseEvent raw_button_release;
    RawMotionNotifyEvent raw_motion_notify;
    RawKeyPressEvent raw_key_press;
    RawKeyReleaseEvent raw_key_release;

    // Xlib events.
    XAnyEvent xany;
	XKeyEvent xkey;
	XButtonEvent xbutton;
	XMotionEvent xmotion;
	XCrossingEvent xcrossing;
	XFocusChangeEvent xfocus;
	XExposeEvent xexpose;
	XGraphicsExposeEvent xgraphicsexpose;
	XNoExposeEvent xnoexpose;
	XVisibilityEvent xvisibility;
	XCreateWindowEvent xcreatewindow;
	XDestroyWindowEvent xdestroywindow;
	XUnmapEvent xunmap;
	XMapEvent xmap;
	XMapRequestEvent xmaprequest;
	XReparentEvent xreparent;
	XConfigureEvent xconfigure;
	XGravityEvent xgravity;
	XResizeRequestEvent xresizerequest;
	XConfigureRequestEvent xconfigurerequest;
	XCirculateEvent xcirculate;
	XCirculateRequestEvent xcirculaterequest;
	XPropertyEvent xproperty;
	XSelectionClearEvent xselectionclear;
	XSelectionRequestEvent xselectionrequest;
	XSelectionEvent xselection;
	XColormapEvent xcolormap;
	XClientMessageEvent xclient;
	XMappingEvent xmapping;
	XErrorEvent xerror;
	XKeymapEvent xkeymap;
	XGenericEvent xgeneric;
	XGenericEventCookie xcookie;
} Event;

/**
 * Initiates an infinite event loop, handling X11/XInput2 events as they come 
 * in, and calling the appropriate registered event handlers.
 */
void initialize_event_loop();
