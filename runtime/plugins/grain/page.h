/* Grain: native strengths on GR IV; native-amplitude percentages on GR III/IIIx.
   Saved codes 1/2/3 retain their original meaning across upgrades. */
#include "../../plugin.h"
#ifdef GRKIT_TARGET_GR3_210
#define GRAIN_LEVEL_COUNT 21U
static const u8 grain_percent[12]={0,25,50,100,10,20,30,40,60,70,80,90};
static u32 grain_code(u32 row) { return strength_code(row); }
static u32 grain_tile(u32 row) { return !row ? 0 : row<=5 ? 1 : row<=10 ? 2 : 3; }
#define GRAIN_LABEL_TEXT TEXT_STRENGTH_LEVEL
#define GRAIN_SUMMARY_TEXT TEXT_GRAIN_PERCENT_SUMMARY
#else
#define GRAIN_LEVEL_COUNT 4U
static u32 grain_code(u32 row) { return row<GRAIN_LEVEL_COUNT ? row : 0; }
static u32 grain_tile(u32 row) { return row<GRAIN_LEVEL_COUNT ? row : 0; }
#define GRAIN_LABEL_TEXT TEXT_GRAIN_LEVEL
#define GRAIN_SUMMARY_TEXT TEXT_GRAIN_SUMMARY
#endif
static u32 grain_level(volatile u32 *h) {
#ifdef GRKIT_TARGET_GR3_210
    u32 code=h[GRKIT_HOST_GRAIN/4];if(code<12) return grain_percent[code]/5;
#endif
    u32 row; for(row=0;row<GRAIN_LEVEL_COUNT;++row) if(grain_code(row)==h[GRKIT_HOST_GRAIN/4]) return row;
    return 0;
}
static const u16 *grain_title(volatile u32 *h) { return text(h,TEXT_GRAIN); }
static const u16 *grain_summary(volatile u32 *h) { return text(h,GRAIN_SUMMARY_TEXT+grain_level(h)); }
static u32 grain_rows(volatile u32 *h) { (void)h; return GRAIN_LEVEL_COUNT; }
static const u16 *grain_label(volatile u32 *h,u32 row) { return text(h,row ? GRAIN_LABEL_TEXT+row-1 : TEXT_OFF); }
static u32 grain_icon(volatile u32 *h,u32 row) {
    return h[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT || row ? GRKIT_ICON_TILE(0) : GRKIT_ICON_OFF;
}
static u32 grain_checked(volatile u32 *h,u32 row) { return row==grain_level(h); }
static void grain_apply(volatile u32 *h,u32 row) { h[GRKIT_HOST_GRAIN/4]=grain_code(row); }
#define GRAIN_PAGE {grain_title,grain_summary,grain_rows,grain_label,grain_icon, \
    grain_checked,grain_apply,grain_level,0,0,GRAIN_TILES}
