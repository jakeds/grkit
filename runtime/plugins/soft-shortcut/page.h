#include "../../plugin.h"
static u32 soft_shortcut_value(volatile u32 *h) { return h[GRKIT_HOST_SHORTCUT/4]==1; }
static const u16 *soft_shortcut_title(volatile u32 *h) { return text(h,TEXT_SOFT_SHORTCUT); }
static const u16 *soft_shortcut_summary(volatile u32 *h) { return text(h,TEXT_SHORTCUT_SUMMARY+soft_shortcut_value(h)); }
static u32 soft_shortcut_rows(volatile u32 *h) { (void)h; return 2; }
static const u16 *soft_shortcut_label(volatile u32 *h,u32 row) { return text(h,row ? TEXT_SHORTCUT_FN : TEXT_OFF); }
static u32 soft_shortcut_icon(volatile u32 *h,u32 row) { (void)h; (void)row; return FN_GUIDE_ICON; }
static u32 soft_shortcut_checked(volatile u32 *h,u32 row) { return row==soft_shortcut_value(h); }
static void soft_shortcut_apply(volatile u32 *h,u32 row) {
    h[GRKIT_HOST_SHORTCUT/4]=row<soft_shortcut_rows(h) ? row : 0;
    if(!h[GRKIT_HOST_SHORTCUT/4]) h[GRKIT_HOST_SHORTCUT_STATE/4]&=1U;
}
#define SOFT_SHORTCUT_PAGE {soft_shortcut_title,soft_shortcut_summary,soft_shortcut_rows,soft_shortcut_label,soft_shortcut_icon,soft_shortcut_checked,soft_shortcut_apply,soft_shortcut_value,0,0,SOFT_SHORTCUT_TILES}
