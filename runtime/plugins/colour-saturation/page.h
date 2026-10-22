#include "../../plugin.h"
static u32 colour_saturation_value(volatile u32 *h) { return colour_menu_row(colour_read(h,GRKIT_HOST_COLOUR_SATURATION,7),1); }
static const u16 *colour_saturation_title(volatile u32 *h) { return text(h,TEXT_COLOUR_SATURATION); }
static const u16 *colour_saturation_summary(volatile u32 *h) { return colour_saturation_title(h); }
static u32 colour_saturation_rows(volatile u32 *h) { return h[GRKIT_HOST_COLOUR_RANGE/4]>0 && h[GRKIT_HOST_COLOUR_RANGE/4]<=7 ? 26 : 1; }
static const u16 *colour_saturation_label(volatile u32 *h,u32 row) { return text(h,h[GRKIT_HOST_COLOUR_RANGE/4]>0 && h[GRKIT_HOST_COLOUR_RANGE/4]<=7 ? TEXT_COLOUR_SATURATION_LEVEL+row : TEXT_COLOUR_FIRST); }
static u32 colour_saturation_icon(volatile u32 *h,u32 row) { (void)h; (void)row; return GRKIT_ICON_TILE(0); }
static u32 colour_saturation_checked(volatile u32 *h,u32 row) { return row==colour_saturation_value(h); }
static void colour_saturation_apply(volatile u32 *h,u32 row) { colour_write(h,GRKIT_HOST_COLOUR_SATURATION,colour_menu_code(row,1),7); }
#define COLOUR_SATURATION_PAGE {colour_saturation_title,colour_saturation_summary,colour_saturation_rows,colour_saturation_label,colour_saturation_icon,colour_saturation_checked,colour_saturation_apply,colour_saturation_value,0,0,COLOUR_SATURATION_TILES}
