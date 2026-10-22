/* Date format and appearance are independent. Legacy mono IDs 5/6 remain readable. */
#include "../../plugin.h"
#define STAMP_STYLE_COUNT 7U
static u32 stamp_stage(volatile u32 *h) { return h[GRKIT_HOST_COLOUR_MENU/4]<=2 ? h[GRKIT_HOST_COLOUR_MENU/4] : 0; }
static u32 stamp_style(volatile u32 *h) { u32 v=h[GRKIT_HOST_STAMP/4];return v<=DATE_STAMP_STYLE_MAX ? v : 0; }
static u32 stamp_format(volatile u32 *h) { u32 v=stamp_style(h);return v==5 || v==6 || v==8 ? 6 : v==7 ? 5 : v; }
static u32 stamp_look(volatile u32 *h) { u32 v=stamp_style(h),look=h[GRKIT_HOST_STAMP_LOOK/4];return v==5 ? 3 : v==6 ? 4 : look<=DATE_STAMP_LOOK_MAX ? look : 0; }
static u32 stamp_format_id(u32 row) { return row==6 ? 8 : row==5 ? 7 : row<5 ? row : 0; }
static void stamp_migrate(volatile u32 *h) { u32 v=stamp_style(h);if(v==5 || v==6) {h[GRKIT_HOST_STAMP_LOOK/4]=v==5 ? 3 : 4;h[GRKIT_HOST_STAMP/4]=8;} }
static const u16 *stamp_title(volatile u32 *h) { u32 stage=stamp_stage(h);return text(h,stage ? TEXT_STAMP_MENU+stage-1 : TEXT_DATE_STAMP); }
static const u16 *stamp_summary(volatile u32 *h) { return text(h,TEXT_STAMP_SUMMARY+stamp_style(h)); }
static u32 stamp_rows(volatile u32 *h) { u32 stage=stamp_stage(h);return stage==1 ? STAMP_STYLE_COUNT : stage==2 ? 5 : 2; }
static const u16 *stamp_label(volatile u32 *h,u32 row) {
    u32 stage=stamp_stage(h);
    return text(h,stage==1 ? TEXT_STAMP_FORMAT+row : stage==2 ? TEXT_STAMP_LOOK+row : TEXT_STAMP_MENU+row);
}
static u32 stamp_icon(volatile u32 *h,u32 row) { return stamp_stage(h)==1 && !row ? GRKIT_ICON_OFF : GRKIT_ICON_TILE(stamp_stage(h)==2 || (!stamp_stage(h) && row==1) ? 1 : 0); }
static u32 stamp_checked(volatile u32 *h,u32 row) { u32 stage=stamp_stage(h);return stage && row==(stage==1 ? stamp_format(h) : stamp_look(h)); }
static u32 stamp_committed(volatile u32 *h) { u32 stage=stamp_stage(h);return stage==1 ? stamp_format(h) : stage==2 ? stamp_look(h) : 0; }
static void stamp_apply(volatile u32 *h,u32 row) {
    u32 stage=stamp_stage(h);
    if(!stage) { if(row<2) {h[GRKIT_HOST_COLOUR_MENU/4]=row+1;h[GRKIT_HOST_MENU_CURSOR/4]=stamp_committed(h);} }
    else if(stage==1) {stamp_migrate(h);h[GRKIT_HOST_STAMP/4]=stamp_format_id(row);}
    else {stamp_migrate(h);h[GRKIT_HOST_STAMP_LOOK/4]=row<=DATE_STAMP_LOOK_MAX ? row : 0;}
}
#define DATE_STAMP_PAGE {stamp_title,stamp_summary,stamp_rows,stamp_label,stamp_icon, \
    stamp_checked,stamp_apply,stamp_committed,0,0,DATE_STAMP_TILES}
