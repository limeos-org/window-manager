/**
 * This code is responsible for the tiling layout engine, including computing
 * tile geometries, managing the tile order, and applying tiling and cascade
 * layouts to portals on a workspace.
 */

#include "../all.h"

/** The tile order list for each workspace. */
static Portal *tile_order[MAX_WORKSPACES][MAX_WORKSPACE_PORTALS] = {{NULL}};

/** The number of portals in the tile order for each workspace. */
static int tile_order_count[MAX_WORKSPACES] = {0};

/** The gap between tiled portals in pixels. */
static int tile_gap = 0;

/** Reentrancy guard for `apply_tiling_layout()`. */
static bool applying_layout = false;

bool is_tiling_eligible(Portal *portal)
{
    if (!portal->active) return false;
    if (!portal->initialized) return false;
    if (portal->transient_for != NULL) return false;
    if (portal->override_redirect) return false;
    if (portal->visibility == PORTAL_HIDDEN) return false;
    return true;
}

void remove_from_tile_order(Portal *portal, int workspace)
{
    if (workspace < 0 || workspace >= MAX_WORKSPACES) return;

    // Find the portal in the tile order.
    int index = -1;
    for (int i = 0; i < tile_order_count[workspace]; i++)
    {
        if (tile_order[workspace][i] == portal)
        {
            index = i;
            break;
        }
    }
    if (index == -1) return;

    // Shift subsequent entries forward to close the gap.
    for (int i = index; i < tile_order_count[workspace] - 1; i++)
    {
        tile_order[workspace][i] = tile_order[workspace][i + 1];
    }
    tile_order_count[workspace]--;
    tile_order[workspace][tile_order_count[workspace]] = NULL;
}

void append_to_tile_order(Portal *portal, int workspace)
{
    if (workspace < 0 || workspace >= MAX_WORKSPACES) return;
    if (tile_order_count[workspace] >= MAX_WORKSPACE_PORTALS) return;

    // Ensure portal is not already in tile order.
    for (int i = 0; i < tile_order_count[workspace]; i++)
    {
        if (tile_order[workspace][i] == portal) return;
    }

    // Append the portal.
    tile_order[workspace][tile_order_count[workspace]] = portal;
    tile_order_count[workspace]++;
}

int get_tile_order_count(int workspace)
{
    if (workspace < 0 || workspace >= MAX_WORKSPACES) return 0;
    return tile_order_count[workspace];
}

void clear_tile_order(int workspace)
{
    if (workspace < 0 || workspace >= MAX_WORKSPACES) return;
    for (int i = 0; i < tile_order_count[workspace]; i++)
    {
        tile_order[workspace][i] = NULL;
    }
    tile_order_count[workspace] = 0;
}

/**
 * Computes tile geometry for a portal at the given index in a layout
 * of `count` portals. Pure function of count, index, viewport, and gap.
 */
static PortalGeometry calc_tile_geometry(
    int count, int index,
    int viewport_width, int viewport_height, int gap
)
{
    PortalGeometry geometry = {0, 0, 0, 0};
    int usable_width = viewport_width;
    int usable_height = viewport_height;

    switch (count)
    {
        case 1:
        {
            // Single portal fills the full viewport minus gap.
            geometry.x_root = gap;
            geometry.y_root = gap;
            geometry.width = usable_width - 2 * gap;
            geometry.height = usable_height - 2 * gap;
            break;
        }
        case 2:
        {
            // Two equal columns, each full height.
            int col_width = (usable_width - 3 * gap) / 2;
            geometry.x_root = gap + index * (col_width + gap);
            geometry.y_root = gap;
            geometry.width = col_width;
            geometry.height = usable_height - 2 * gap;
            break;
        }
        case 3:
        {
            // Left column full height, right column split into two rows.
            int col_width = (usable_width - 3 * gap) / 2;
            if (index == 0)
            {
                geometry.x_root = gap;
                geometry.y_root = gap;
                geometry.width = col_width;
                geometry.height = usable_height - 2 * gap;
            }
            else
            {
                int row_height = (usable_height - 3 * gap) / 2;
                int row = index - 1;
                geometry.x_root = gap + col_width + gap;
                geometry.y_root = gap + row * (row_height + gap);
                geometry.width = col_width;
                geometry.height = row_height;
            }
            break;
        }
        case 4:
        {
            // 2x2 grid of equal quadrants.
            int col_width = (usable_width - 3 * gap) / 2;
            int row_height = (usable_height - 3 * gap) / 2;
            int row = index / 2;
            int col = index % 2;
            geometry.x_root = gap + col * (col_width + gap);
            geometry.y_root = gap + row * (row_height + gap);
            geometry.width = col_width;
            geometry.height = row_height;
            break;
        }
        case 5:
        {
            // Top row: 2 columns. Bottom row: 3 columns.
            int row_height = (usable_height - 3 * gap) / 2;
            if (index < 2)
            {
                int col_width = (usable_width - 3 * gap) / 2;
                geometry.x_root = gap + index * (col_width + gap);
                geometry.y_root = gap;
                geometry.width = col_width;
                geometry.height = row_height;
            }
            else
            {
                int col_width = (usable_width - 4 * gap) / 3;
                int col = index - 2;
                geometry.x_root = gap + col * (col_width + gap);
                geometry.y_root = gap + row_height + gap;
                geometry.width = col_width;
                geometry.height = row_height;
            }
            break;
        }
        case 6:
        {
            // 3x2 grid. Two rows, three columns.
            int col_width = (usable_width - 4 * gap) / 3;
            int row_height = (usable_height - 3 * gap) / 2;
            int row = index / 3;
            int col = index % 3;
            geometry.x_root = gap + col * (col_width + gap);
            geometry.y_root = gap + row * (row_height + gap);
            geometry.width = col_width;
            geometry.height = row_height;
            break;
        }
        case 7:
        {
            // Top row: 3 columns. Bottom row: 4 columns.
            int row_height = (usable_height - 3 * gap) / 2;
            if (index < 3)
            {
                int col_width = (usable_width - 4 * gap) / 3;
                geometry.x_root = gap + index * (col_width + gap);
                geometry.y_root = gap;
                geometry.width = col_width;
                geometry.height = row_height;
            }
            else
            {
                int col_width = (usable_width - 5 * gap) / 4;
                int col = index - 3;
                geometry.x_root = gap + col * (col_width + gap);
                geometry.y_root = gap + row_height + gap;
                geometry.width = col_width;
                geometry.height = row_height;
            }
            break;
        }
        case 8:
        {
            // 4x2 grid. Two rows, four columns.
            int col_width = (usable_width - 5 * gap) / 4;
            int row_height = (usable_height - 3 * gap) / 2;
            int row = index / 4;
            int col = index % 4;
            geometry.x_root = gap + col * (col_width + gap);
            geometry.y_root = gap + row * (row_height + gap);
            geometry.width = col_width;
            geometry.height = row_height;
            break;
        }
        default:
            break;
    }

    return geometry;
}

void apply_tiling_layout(int workspace)
{
    // Prevent re-entrancy.
    if (applying_layout) return;
    applying_layout = true;

    Display *display = DefaultDisplay;
    int screen = DefaultScreen(display);
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);

    // Compute and apply tile geometries.
    int count = tile_order_count[workspace];
    for (int i = 0; i < count; i++)
    {
        Portal *portal = tile_order[workspace][i];
        if (portal == NULL) continue;
        if (portal->fullscreen) continue;

        PortalGeometry geometry = calc_tile_geometry(
            count, i, screen_width, screen_height, tile_gap
        );

        // Apply geometry through move_portal()/resize_portal().
        move_portal(portal, geometry.x_root, geometry.y_root);
        resize_portal(portal, geometry.width, geometry.height);
    }

    applying_layout = false;
}

bool is_applying_tiling_layout()
{
    return applying_layout;
}

void cascade_tiled_portals(int workspace)
{
    Display *display = DefaultDisplay;
    int screen = DefaultScreen(display);
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);

    // Collect eligible portals in stacking order (bottom to top).
    Portal *eligible[MAX_WORKSPACE_PORTALS];
    int eligible_count = 0;
    unsigned int sorted_count = 0;
    Portal **sorted = get_sorted_portals(&sorted_count);
    for (unsigned int i = 0; i < sorted_count; i++)
    {
        Portal *portal = sorted[i];
        if (portal == NULL) continue;
        if (portal->workspace != workspace) continue;
        if (!is_tiling_eligible(portal)) continue;
        if (eligible_count >= MAX_WORKSPACE_PORTALS) break;
        eligible[eligible_count++] = portal;
    }
    if (eligible_count == 0) return;

    // Compute median of last floating widths and heights.
    unsigned int widths[MAX_WORKSPACE_PORTALS];
    unsigned int heights[MAX_WORKSPACE_PORTALS];
    for (int i = 0; i < eligible_count; i++)
    {
        widths[i] = eligible[i]->geometry_floating_backup.width;
        heights[i] = eligible[i]->geometry_floating_backup.height;
        if (widths[i] == 0) widths[i] = eligible[i]->geometry.width;
        if (heights[i] == 0) heights[i] = eligible[i]->geometry.height;
    }

    // Sort widths and heights for median calculation.
    for (int i = 0; i < eligible_count - 1; i++)
    {
        for (int j = i + 1; j < eligible_count; j++)
        {
            if (widths[j] < widths[i])
            {
                unsigned int tmp = widths[i];
                widths[i] = widths[j];
                widths[j] = tmp;
            }
            if (heights[j] < heights[i])
            {
                unsigned int tmp = heights[i];
                heights[i] = heights[j];
                heights[j] = tmp;
            }
        }
    }
    unsigned int median_width = widths[eligible_count / 2];
    unsigned int median_height = heights[eligible_count / 2];

    // Clamp to minimum portal dimensions.
    if (median_width < MINIMUM_PORTAL_WIDTH) median_width = MINIMUM_PORTAL_WIDTH;
    if (median_height < MINIMUM_PORTAL_HEIGHT) median_height = MINIMUM_PORTAL_HEIGHT;

    // Cap to viewport threshold percentage.
    unsigned int max_width = (unsigned int)(screen_width * WORKSPACE_VIEWPORT_THRESHOLD_PERCENT / 100);
    unsigned int max_height = (unsigned int)(screen_height * WORKSPACE_VIEWPORT_THRESHOLD_PERCENT / 100);
    if (median_width > max_width) median_width = max_width;
    if (median_height > max_height) median_height = max_height;

    // Calculate cascade group dimensions and center on screen.
    int cascade_offset = WORKSPACE_CASCADE_OFFSET_PX;
    int group_width = (int)median_width + (eligible_count - 1) * cascade_offset;
    int group_height = (int)median_height + (eligible_count - 1) * cascade_offset;
    int start_x = (screen_width - group_width) / 2;
    int start_y = (screen_height - group_height) / 2;

    // Position portals with diagonal offset in stacking order.
    for (int i = 0; i < eligible_count; i++)
    {
        Portal *portal = eligible[i];
        int x = start_x + i * cascade_offset;
        int y = start_y + i * cascade_offset;
        resize_portal(portal, median_width, median_height);
        move_portal(portal, x, y);
    }
}

HANDLE(Initialize)
{
    // Read the tile gap from config.
    char gap_value[CONFIG_MAX_VALUE_LENGTH];
    common.get_config_str(
        gap_value, sizeof(gap_value),
        CFG_KEY_TILE_GAP, CFG_DEFAULT_TILE_GAP
    );
    tile_gap = atoi(gap_value);
    if (tile_gap < 0) tile_gap = 0;
}
