#include "../../plugin.h"
static u32 colour_brightness_value(volatile u32 *h) { return colour_menu_row(colour_read(h,GRKIT_HOST_COLOUR_BRIGHTNESS,7),0); }
static const u16 *colour_brightness_title(volatile u32 *h) { return text(h,TEXT_COLOUR_BRIGHTNESS); }
static const u16 *colour_brightness_summary(volatile u32 *h) { return colour_brightness_title(h); }
static u32 colour_brightness_rows(volatile u32 *h) { return h[GRKIT_HOST_COLOUR_RANGE/4]>0 && h[GRKIT_HOST_COLOUR_RANGE/4]<=7 ? 9 : 1; }
static const u16 *colour_brightness_label(volatile u32 *h,u32 row) { return text(h,h[GRKIT_HOST_COLOUR_RANGE/4]>0 && h[GRKIT_HOST_COLOUR_RANGE/4]<=7 ? TEXT_COLOUR_BRIGHTNESS_LEVEL+row : TEXT_COLOUR_FIRST); }
static u32 colour_brightness_icon(volatile u32 *h,u32 row) { (void)h; (void)row; return GRKIT_ICON_TILE(0); }
static u32 colour_brightness_checked(volatile u32 *h,u32 row) { return row==colour_brightness_value(h); }
static void colour_brightness_apply(volatile u32 *h,u32 row) { colour_write(h,GRKIT_HOST_COLOUR_BRIGHTNESS,colour_menu_code(row,0),7); }
#define COLOUR_BRIGHTNESS_PAGE {colour_brightness_title,colour_brightness_summary,colour_brightness_rows,colour_brightness_label,colour_brightness_icon,colour_brightness_checked,colour_brightness_apply,colour_brightness_value,0,0,COLOUR_BRIGHTNESS_TILES}
