#include "../../plugin.h"
static u32 halation_level(volatile u32 *h) { return strength_row(h[GRKIT_HOST_HALATION/4],1); }
static const u16 *halation_title(volatile u32 *h) { return text(h,TEXT_HALATION); }
static u32 halation_rows(volatile u32 *h) { (void)h; return 21; }
static const u16 *halation_label(volatile u32 *h,u32 row) { return text(h,row && row<21 ? TEXT_STRENGTH_LEVEL+row-1 : TEXT_OFF); }
static u32 halation_icon(volatile u32 *h,u32 row) { (void)h; (void)row; return GRKIT_ICON_TILE(0); }
static u32 halation_checked(volatile u32 *h,u32 row) { return row==halation_level(h); }
static void halation_apply(volatile u32 *h,u32 row) { h[GRKIT_HOST_HALATION/4]=strength_code(row); }
#define HALATION_PAGE {halation_title,halation_title,halation_rows,halation_label,halation_icon,halation_checked,halation_apply,halation_level,0,0,HALATION_TILES}
