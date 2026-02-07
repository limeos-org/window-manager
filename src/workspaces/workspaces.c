/**
 * This code is responsible for managing workspaces, including tracking the
 * current workspace, switching between workspaces, and assigning portals to
 * workspaces when they are initialized.
 */

#include "../all.h"

/** The current active workspace index. */
static int current_workspace = 0;

/** Tracks the last focused portal on each workspace for focus restoration. */
static Portal *last_focused_portal[MAX_WORKSPACES] = {NULL};

int get_current_workspace()
{
    return current_workspace;
}

int determine_portal_workspace(Portal *portal)
{
    return (portal->workspace != -1) ? portal->workspace : current_workspace;
}

int count_workspace_portals(int workspace)
{
    int count = 0;
    Portal *portals = get_unsorted_portals();
    for (unsigned int i = 0; i < MAX_PORTALS; i++)
    {
        Portal *portal = &portals[i];
        if (!portal->active) continue;
        if (!portal->initialized) continue;
        if (portal->workspace != workspace) continue;
        if (portal->visibility == PORTAL_HIDDEN) continue;
        if (portal->transient_for != NULL) continue;
        if (portal->override_redirect) continue;
        count++;
    }
    return count;
}

void move_portal_to_workspace(Portal *portal, int workspace)
{
    if (portal == NULL) return;
    if (workspace < 0 || workspace >= MAX_WORKSPACES) return;
    if (portal->workspace == workspace) return;

    // Deny move to a full workspace (transients are exempt).
    if (portal->transient_for == NULL &&
        count_workspace_portals(workspace) >= MAX_WORKSPACE_PORTALS)
    {
        LOG_WARNING(
            "Workspace %d is full (%d portals); move denied.",
            workspace, MAX_WORKSPACE_PORTALS
        );
        return;
    }

    // Store old workspace for the event.
    int old_workspace = portal->workspace;

    // Suspend if currently visible on active workspace.
    if (portal->workspace == current_workspace &&
        portal->visibility == PORTAL_VISIBLE
    ) {
        suspend_portal(portal);
    }

    // Update workspace assignment.
    portal->workspace = workspace;

    // Map if target is the active workspace.
    if (workspace == current_workspace)
    {
        map_portal(portal);
    }

    // Notify listeners of the workspace change.
    call_event_handlers((Event*)&(PortalWorkspaceChangedEvent){
        .type = PortalWorkspaceChanged,
        .portal = portal,
        .old_workspace = old_workspace,
        .new_workspace = workspace
    });
}

void switch_workspace(int workspace)
{
    Display *display = DefaultDisplay;

    // Ensure workspace index is valid.
    if (workspace < 0 || workspace >= MAX_WORKSPACES) return;

    // Ensure we're switching to a different workspace.
    if (workspace == current_workspace) return;

    // Save the currently focused window for the current workspace.
    Window focused_window = None;
    int revert_to = 0;
    XGetInputFocus(display, &focused_window, &revert_to);
    if (focused_window != None && focused_window != PointerRoot)
    {
        Portal *focused_portal = find_portal_by_window(focused_window);
        if (focused_portal != NULL && focused_portal->workspace == current_workspace)
        {
            last_focused_portal[current_workspace] = focused_portal;
        }
    }

    // Update current workspace.
    int old_workspace = current_workspace;
    current_workspace = workspace;

    // Update portal visibility based on workspace assignment.
    Portal *portals = get_unsorted_portals();
    for (unsigned int i = 0; i < MAX_PORTALS; i++)
    {
        Portal *portal = &portals[i];

        // Skip inactive slots, uninitialized portals, override-redirect windows
        // (they manage themselves), and unassigned portals (workspace < 0).
        if (!portal->active) continue;
        if (!portal->initialized) continue;
        if (portal->override_redirect) continue;
        if (portal->workspace < 0) continue;

        if (portal->workspace == old_workspace)
        {
            suspend_portal(portal);
        }
        else if (portal->workspace == workspace)
        {
            reveal_portal(portal);
        }
    }

    // Restore focus on the new workspace.
    Portal *to_focus = last_focused_portal[workspace];

    // Verify the last focused portal is still valid and on this workspace.
    if (to_focus != NULL)
    {
        if (!to_focus->initialized || to_focus->visibility != PORTAL_VISIBLE || to_focus->workspace != workspace)
        {
            to_focus = NULL;
        }
    }

    // Fallback to topmost visible portal if no last focused portal is available.
    if (to_focus == NULL)
    {
        unsigned int sorted_count = 0;
        Portal **sorted = get_sorted_portals(&sorted_count);
        for (int i = (int)sorted_count - 1; i >= 0; i--)
        {
            Portal *portal = sorted[i];
            if (portal == NULL) continue;
            if (!portal->initialized || portal->visibility != PORTAL_VISIBLE) continue;
            if (portal->workspace != workspace) continue;
            to_focus = portal;
            break;
        }
    }

    // Focus the determined portal or clear focus.
    if (to_focus != NULL)
    {
        // Focus the portal.
        x_focus_window(display, to_focus->client_window);
        raise_portal(to_focus);
        call_event_handlers((Event*)&(PortalFocusedEvent){
            .type = PortalFocused,
            .portal = to_focus
        });
    }
    else
    {
        // No portals on this workspace, clear focus to root.
        x_focus_window(display, DefaultRootWindow(display));
    }

    // Notify listeners of the workspace switch.
    call_event_handlers((Event*)&(WorkspaceSwitchedEvent){
        .type = WorkspaceSwitched,
        .old_workspace = old_workspace,
        .new_workspace = workspace
    });
}

HANDLE(PortalInitialized)
{
    Portal *portal = event->portal_initialized.portal;

    // Assign the portal to its target workspace.
    portal->workspace = determine_portal_workspace(portal);
}

HANDLE(PortalDestroyed)
{
    Portal *portal = event->portal_destroyed.portal;

    // Clear portal from last_focused_portal array if it's tracked.
    for (int i = 0; i < MAX_WORKSPACES; i++)
    {
        if (last_focused_portal[i] == portal)
        {
            last_focused_portal[i] = NULL;
        }
    }
}
