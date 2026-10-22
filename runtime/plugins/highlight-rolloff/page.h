#include "../../plugin.h"
static u32 highlight_rolloff_value(volatile u32 *h) { return strength_row(h[GRKIT_HOST_HIGHLIGHT_ROLLOFF/4],0); }
static const u16 *highlight_rolloff_title(volatile u32 *h) { return text(h,TEXT_HIGHLIGHT_ROLLOFF); }
static const u16 *highlight_rolloff_summary(volatile u32 *h) { return highlight_rolloff_title(h); }
static u32 highlight_rolloff_rows(volatile u32 *h) { (void)h; return 21; }
static const u16 *highlight_rolloff_label(volatile u32 *h,u32 row) { return text(h,row && row<21 ? TEXT_STRENGTH_LEVEL+row-1 : TEXT_OFF); }
static u32 highlight_rolloff_icon(volatile u32 *h,u32 row) { (void)h; (void)row; return GRKIT_ICON_TILE(0); }
static u32 highlight_rolloff_checked(volatile u32 *h,u32 row) { return row==highlight_rolloff_value(h); }
static void highlight_rolloff_apply(volatile u32 *h,u32 row) { h[GRKIT_HOST_HIGHLIGHT_ROLLOFF/4]=strength_code(row); }
#define HIGHLIGHT_ROLLOFF_PAGE {highlight_rolloff_title,highlight_rolloff_summary,highlight_rolloff_rows,highlight_rolloff_label,highlight_rolloff_icon,highlight_rolloff_checked,highlight_rolloff_apply,highlight_rolloff_value,0,0,HIGHLIGHT_ROLLOFF_TILES}
