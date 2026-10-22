#include "../../plugin.h"
static u32 colour_hue_value(volatile u32 *h) { return colour_menu_row(colour_read(h,GRKIT_HOST_COLOUR_HUE,7),0); }
static const u16 *colour_hue_title(volatile u32 *h) { return text(h,TEXT_COLOUR_HUE); }
static const u16 *colour_hue_summary(volatile u32 *h) { return colour_hue_title(h); }
static u32 colour_hue_rows(volatile u32 *h) { return h[GRKIT_HOST_COLOUR_RANGE/4]>0 && h[GRKIT_HOST_COLOUR_RANGE/4]<=7 ? 9 : 1; }
static const u16 *colour_hue_label(volatile u32 *h,u32 row) { return text(h,h[GRKIT_HOST_COLOUR_RANGE/4]>0 && h[GRKIT_HOST_COLOUR_RANGE/4]<=7 ? TEXT_COLOUR_HUE_LEVEL+row : TEXT_COLOUR_FIRST); }
static u32 colour_hue_icon(volatile u32 *h,u32 row) { (void)h; (void)row; return GRKIT_ICON_TILE(0); }
static u32 colour_hue_checked(volatile u32 *h,u32 row) { return row==colour_hue_value(h); }
static void colour_hue_apply(volatile u32 *h,u32 row) { colour_write(h,GRKIT_HOST_COLOUR_HUE,colour_menu_code(row,0),7); }
#define COLOUR_HUE_PAGE {colour_hue_title,colour_hue_summary,colour_hue_rows,colour_hue_label,colour_hue_icon,colour_hue_checked,colour_hue_apply,colour_hue_value,0,0,COLOUR_HUE_TILES}
