#include "../../plugin.h"
static u32 bloom_level(volatile u32 *h) { return strength_row(h[GRKIT_HOST_BLOOM/4],1); }
static const u16 *bloom_title(volatile u32 *h) { return text(h,TEXT_BLOOM); }
static u32 bloom_rows(volatile u32 *h) { (void)h; return 21; }
static const u16 *bloom_label(volatile u32 *h,u32 row) { return text(h,row && row<21 ? TEXT_STRENGTH_LEVEL+row-1 : TEXT_OFF); }
static u32 bloom_icon(volatile u32 *h,u32 row) { (void)h; (void)row; return GRKIT_ICON_TILE(0); }
static u32 bloom_checked(volatile u32 *h,u32 row) { return row==bloom_level(h); }
static void bloom_apply(volatile u32 *h,u32 row) { h[GRKIT_HOST_BLOOM/4]=strength_code(row); }
#define BLOOM_PAGE {bloom_title,bloom_title,bloom_rows,bloom_label,bloom_icon,bloom_checked,bloom_apply,bloom_level,0,0,BLOOM_TILES}
