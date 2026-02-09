/**
 * This code is responsible for managing workspaces, including tracking the
 * current workspace, switching between workspaces, assigning portals to
 * workspaces, and coordinating layout mode transitions between floating and
 * tiling modes. The tiling layout engine itself lives in tiling.c.
 */

#include "../all.h"

/** The current active workspace index. */
static int current_workspace = 0;

/** Tracks the last focused portal on each workspace for focus restoration. */
static Portal *last_focused_portal[MAX_WORKSPACES] = {NULL};

/** The layout mode of each workspace. */
static WorkspaceLayoutMode workspace_layout_mode[MAX_WORKSPACES] = {WORKSPACE_LAYOUT_FLOATING};

/**
 * Moves a single portal to the given workspace without any group or limit
 * logic. Caller is responsible for validation and workspace-limit checks.
 */
static void move_single_portal_to_workspace(Portal *portal, int workspace)
{
    if (portal->workspace == workspace) return;

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

    // Find the root of this portal's transient group.
    Portal *root = find_portal_transient_root(portal);

    // Nothing to do if the root is already on the target workspace (the
    // entire group is assumed to be co-located).
    if (root->workspace == workspace) return;

    // Deny move to a full workspace (only non-transient root counts toward
    // the limit, skip the check if the root is already on that workspace).
    if (root->transient_for == NULL &&
        count_workspace_portals(workspace) >= MAX_WORKSPACE_PORTALS)
    {
        LOG_WARNING(
            "Workspace %d is full (%d portals); move denied.",
            workspace, MAX_WORKSPACE_PORTALS
        );
        return;
    }

    // Move the root portal first.
    move_single_portal_to_workspace(root, workspace);

    // Move all transient children belonging to the same group.
    Portal *portals = get_unsorted_portals();
    for (unsigned int i = 0; i < MAX_PORTALS; i++)
    {
        Portal *candidate = &portals[i];
        if (!candidate->active) continue;
        if (!candidate->initialized) continue;
        if (candidate->transient_for == NULL) continue;
        if (find_portal_transient_root(candidate) == root)
        {
            move_single_portal_to_workspace(candidate, workspace);
        }
    }

    // Record the moved portal as the last focused on the target workspace
    // so it receives focus when the user switches there.
    last_focused_portal[workspace] = portal;
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

WorkspaceLayoutMode get_workspace_layout_mode(int workspace)
{
    if (workspace < 0 || workspace >= MAX_WORKSPACES)
    {
        return WORKSPACE_LAYOUT_FLOATING;
    }
    return workspace_layout_mode[workspace];
}

bool is_portal_tiled(Portal *portal)
{
    if (portal->transient_for != NULL) return false;
    if (portal->workspace < 0 || portal->workspace >= MAX_WORKSPACES) return false;
    return workspace_layout_mode[portal->workspace] == WORKSPACE_LAYOUT_TILING;
}

void toggle_workspace_layout_mode()
{
    int workspace = current_workspace;

    if (workspace_layout_mode[workspace] == WORKSPACE_LAYOUT_FLOATING)
    {
        // Switch to Tiling: build tile order from eligible portals.
        workspace_layout_mode[workspace] = WORKSPACE_LAYOUT_TILING;
        clear_tile_order(workspace);

        // Cancel active drag/resize.
        if (is_portal_dragging()) stop_dragging_portal();
        if (is_portal_resizing()) stop_resizing_portal();

        // Populate tile order from stacking order (bottom to top).
        unsigned int sorted_count = 0;
        Portal **sorted = get_sorted_portals(&sorted_count);
        for (unsigned int i = 0; i < sorted_count; i++)
        {
            Portal *portal = sorted[i];
            if (portal == NULL) continue;
            if (portal->workspace != workspace) continue;
            if (!is_tiling_eligible(portal)) continue;
            append_to_tile_order(portal, workspace);
        }

        // Apply the tiling layout.
        apply_tiling_layout(workspace);
    }
    else
    {
        // Switch to Floating: cascade portals to their floating positions.
        cascade_tiled_portals(workspace);

        // Clear tile order.
        workspace_layout_mode[workspace] = WORKSPACE_LAYOUT_FLOATING;
        clear_tile_order(workspace);
    }
}

void arrange_workspace_portals(int workspace)
{
    if (workspace < 0 || workspace >= MAX_WORKSPACES) return;
    if (workspace_layout_mode[workspace] != WORKSPACE_LAYOUT_TILING) return;
    apply_tiling_layout(workspace);
}

HANDLE(PortalInitialized)
{
    Portal *portal = event->portal_initialized.portal;

    // Assign the portal to its target workspace.
    portal->workspace = determine_portal_workspace(portal);
}

HANDLE(PortalDestroyed)
{
    PortalDestroyedEvent *_event = &event->portal_destroyed;
    Portal *portal = _event->portal;

    // Clear portal from last_focused_portal array if it's tracked.
    for (int i = 0; i < MAX_WORKSPACES; i++)
    {
        if (last_focused_portal[i] == portal)
        {
            last_focused_portal[i] = NULL;
        }
    }

    // Remove the portal from its workspace's tile order.
    int workspace = portal->workspace;
    if (workspace >= 0 && workspace < MAX_WORKSPACES)
    {
        remove_from_tile_order(portal, workspace);

        // Reset to Floating if no portals remain in tile order.
        if (get_tile_order_count(workspace) == 0)
        {
            workspace_layout_mode[workspace] = WORKSPACE_LAYOUT_FLOATING;
        }
        else if (workspace_layout_mode[workspace] == WORKSPACE_LAYOUT_TILING)
        {
            // Recompute layout for the remaining portals.
            apply_tiling_layout(workspace);
        }
    }
}

HANDLE(PortalTransformed)
{
    PortalTransformedEvent *_event = &event->portal_transformed;
    Portal *portal = _event->portal;

    // Update `geometry_floating_backup` only in floating mode and outside
    // tiling-driven repositioning.
    if (is_applying_tiling_layout()) return;
    int workspace = portal->workspace;
    if (workspace < 0 || workspace >= MAX_WORKSPACES) return;
    if (workspace_layout_mode[workspace] != WORKSPACE_LAYOUT_FLOATING) return;
    if (portal->transient_for != NULL) return;
    if (portal->override_redirect) return;

    portal->geometry_floating_backup = portal->geometry;
}

HANDLE(PortalMapped)
{
    PortalMappedEvent *_event = &event->portal_mapped;
    Portal *portal = _event->portal;
    int workspace = portal->workspace;
    if (workspace < 0 || workspace >= MAX_WORKSPACES) return;

    // Skip transient and override-redirect portals.
    if (!is_tiling_eligible(portal)) return;

    // Cascade from same WM_CLASS sibling on first map in Floating mode.
    if (_event->first_map &&
        workspace_layout_mode[workspace] == WORKSPACE_LAYOUT_FLOATING)
    {
        Display *display = DefaultDisplay;
        char new_class[256] = {0};
        if (x_get_window_class(display, portal->client_window,
            new_class, sizeof(new_class)) == 0 &&
            new_class[0] != '\0')
        {
            // Find topmost sibling with same WM_CLASS.
            unsigned int sorted_count = 0;
            Portal **sorted_portals = get_sorted_portals(&sorted_count);
            for (int i = (int)sorted_count - 1; i >= 0; i--)
            {
                Portal *sibling = sorted_portals[i];
                if (sibling == NULL) continue;
                if (sibling == portal) continue;
                if (sibling->workspace != workspace) continue;
                if (!sibling->active) continue;
                if (!sibling->initialized) continue;
                if (sibling->visibility != PORTAL_VISIBLE) continue;
                char sibling_class[256] = {0};
                if (x_get_window_class(display,
                    sibling->client_window,
                    sibling_class,
                    sizeof(sibling_class)) == 0 &&
                    strcmp(new_class, sibling_class) == 0)
                {
                    // Offset diagonally from sibling.
                    move_portal(portal,
                        sibling->geometry.x_root + WORKSPACE_CASCADE_OFFSET_PX,
                        sibling->geometry.y_root + WORKSPACE_CASCADE_OFFSET_PX);
                    break;
                }
            }
        }
    }

    // Check for auto-tiling: if in Floating mode and portal exceeds the
    // viewport threshold, switch to Tiling.
    if (workspace_layout_mode[workspace] == WORKSPACE_LAYOUT_FLOATING)
    {
        Display *display = DefaultDisplay;
        int screen = DefaultScreen(display);
        int screen_width = DisplayWidth(display, screen);
        int screen_height = DisplayHeight(display, screen);
        int threshold_width = screen_width * WORKSPACE_VIEWPORT_THRESHOLD_PERCENT / 100;
        int threshold_height = screen_height * WORKSPACE_VIEWPORT_THRESHOLD_PERCENT / 100;

        if ((int)portal->geometry.width > threshold_width ||
            (int)portal->geometry.height > threshold_height)
        {
            // Transition to Tiling via toggle.
            toggle_workspace_layout_mode();
            return;
        }
    }

    // If workspace is in Tiling mode, append to tile order and relayout.
    if (workspace_layout_mode[workspace] == WORKSPACE_LAYOUT_TILING)
    {
        append_to_tile_order(portal, workspace);
        apply_tiling_layout(workspace);
    }
}

HANDLE(PortalWorkspaceChanged)
{
    PortalWorkspaceChangedEvent *_event = &event->portal_workspace_changed;
    Portal *portal = _event->portal;
    int old_workspace = _event->old_workspace;
    int new_workspace = _event->new_workspace;

    // Skip transient portals.
    if (portal->transient_for != NULL) return;
    if (portal->override_redirect) return;

    // Remove from source workspace tile order.
    if (old_workspace >= 0 && old_workspace < MAX_WORKSPACES)
    {
        remove_from_tile_order(portal, old_workspace);

        // Reset source to Floating if empty.
        if (get_tile_order_count(old_workspace) == 0)
        {
            workspace_layout_mode[old_workspace] = WORKSPACE_LAYOUT_FLOATING;
        }
        else if (workspace_layout_mode[old_workspace] == WORKSPACE_LAYOUT_TILING)
        {
            apply_tiling_layout(old_workspace);
        }
    }

    // If target workspace is empty, inherit source workspace layout mode.
    if (new_workspace >= 0 && new_workspace < MAX_WORKSPACES)
    {
        if (count_workspace_portals(new_workspace) <= 1 &&
            old_workspace >= 0 && old_workspace < MAX_WORKSPACES)
        {
            workspace_layout_mode[new_workspace] = workspace_layout_mode[old_workspace];
        }

        // Add to target workspace tile order if it is in Tiling mode.
        if (workspace_layout_mode[new_workspace] == WORKSPACE_LAYOUT_TILING)
        {
            append_to_tile_order(portal, new_workspace);
            apply_tiling_layout(new_workspace);
        }
    }
}

HANDLE(ShortcutPressed)
{
    ShortcutPressedEvent *_event = &event->shortcut_pressed;

    // Check if the shortcut matches the arrange shortcut.
    if (strcmp(_event->name, CFG_KEY_ARRANGE_SHORTCUT) != 0) return;

    // Toggle the layout mode of the current workspace.
    toggle_workspace_layout_mode();
}

