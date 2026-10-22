#include "../../plugin.h"
static u32 black_floor_value(volatile u32 *h) { return strength_row(h[GRKIT_HOST_BLACK_FLOOR/4],0); }
static const u16 *black_floor_title(volatile u32 *h) { return text(h,TEXT_BLACK_FLOOR); }
static const u16 *black_floor_summary(volatile u32 *h) { return black_floor_title(h); }
static u32 black_floor_rows(volatile u32 *h) { (void)h; return 21; }
static const u16 *black_floor_label(volatile u32 *h,u32 row) { return text(h,row && row<21 ? TEXT_STRENGTH_LEVEL+row-1 : TEXT_OFF); }
static u32 black_floor_icon(volatile u32 *h,u32 row) { (void)h; (void)row; return GRKIT_ICON_TILE(0); }
static u32 black_floor_checked(volatile u32 *h,u32 row) { return row==black_floor_value(h); }
static void black_floor_apply(volatile u32 *h,u32 row) { h[GRKIT_HOST_BLACK_FLOOR/4]=strength_code(row); }
#define BLACK_FLOOR_PAGE {black_floor_title,black_floor_summary,black_floor_rows,black_floor_label,black_floor_icon,black_floor_checked,black_floor_apply,black_floor_value,0,0,BLACK_FLOOR_TILES}
