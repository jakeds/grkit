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
/* Shared navigation scratch is reset when Grain opens. Stage 5..7 are
   Strength / Boost / Pattern; zero is the Grain directory. */
static u32 grain_stage(volatile u32 *h) { u32 v=h[GRKIT_HOST_COLOUR_MENU/4];return v>=5 && v<=7 ? v : 0; }
static u32 grain_options(void) {
#ifdef GRKIT_TARGET_GR3_210
    return 3;
#else
    return 1; /* No unverified gain or seed hooks on GR IV. */
#endif
}
static u32 grain_boost(volatile u32 *h) { u32 v=h[GRKIT_HOST_GRAIN_BOOST/4];return v<=2 ? v : 0; }
static u32 grain_pattern(volatile u32 *h) { return h[GRKIT_HOST_GRAIN_PATTERN/4]==1; }
static u32 grain_value(volatile u32 *h) { u32 s=grain_stage(h);return s==5 ? grain_level(h) : s==6 ? grain_boost(h) : s==7 ? grain_pattern(h) : 0; }
static const u16 *grain_title(volatile u32 *h) { u32 s=grain_stage(h);return text(h,s ? TEXT_GRAIN_MENU+s-5 : TEXT_GRAIN); }
static const u16 *grain_summary(volatile u32 *h) { return text(h,GRAIN_SUMMARY_TEXT+grain_level(h)); }
static u32 grain_rows(volatile u32 *h) { u32 s=grain_stage(h);return s==5 ? GRAIN_LEVEL_COUNT : s==6 ? 3 : s==7 ? 2 : grain_options(); }
static const u16 *grain_label(volatile u32 *h,u32 row) { u32 s=grain_stage(h);return text(h,s==5 ? (row ? GRAIN_LABEL_TEXT+row-1 : TEXT_OFF) : s==6 ? TEXT_GRAIN_BOOST+row : s==7 ? TEXT_GRAIN_PATTERN+row : TEXT_GRAIN_MENU+row); }
static u32 grain_icon(volatile u32 *h,u32 row) { u32 s=grain_stage(h);return s==5 && !row ? GRKIT_ICON_OFF : GRKIT_ICON_TILE(0); }
static u32 grain_checked(volatile u32 *h,u32 row) { return grain_stage(h) && row==grain_value(h); }
static void grain_apply(volatile u32 *h,u32 row) {
    u32 s=grain_stage(h);
    if(!s) {if(row<grain_options()) {h[GRKIT_HOST_COLOUR_MENU/4]=row+5;h[GRKIT_HOST_MENU_CURSOR/4]=grain_value(h);}return;}
    if(s==5 && row<GRAIN_LEVEL_COUNT) h[GRKIT_HOST_GRAIN/4]=grain_code(row);
#ifdef GRKIT_TARGET_GR3_210
    else if(s==6 && row<3) h[GRKIT_HOST_GRAIN_BOOST/4]=row;
    else if(s==7 && row<2) h[GRKIT_HOST_GRAIN_PATTERN/4]=row;
#endif
}
#define GRAIN_PAGE {grain_title,grain_summary,grain_rows,grain_label,grain_icon, \
    grain_checked,grain_apply,grain_value,0,0,GRAIN_TILES}
