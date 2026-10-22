#include "../../plugin.h"
static u32 split_stage(volatile u32 *h) { u32 v=h[GRKIT_HOST_COLOUR_MENU/4];return v==3 || v==4 ? v : 0; }
static u32 split_code(u32 row) {static const u8 codes[9]={0,13,14,15,16,5,2,4,6};return row<9 ? codes[row] : 0;}
static u32 split_menu_row(volatile u32 *h) {u32 family=split_preset(h[GRKIT_HOST_SPLIT_TONE/4]);for(u32 row=0;row<9;++row) if(split_preset(split_code(row))==family) return row;return 0;}
static u32 split_toning_value(volatile u32 *h) { return split_stage(h)==3 ? split_menu_row(h) : split_stage(h)==4 ? split_percent(h)/5 : 0; }
static const u16 *split_toning_title(volatile u32 *h) { return text(h,split_stage(h) ? TEXT_SPLIT_MENU+split_stage(h)-3 : TEXT_SPLIT_TONING); }
static const u16 *split_toning_summary(volatile u32 *h) { return text(h,TEXT_SPLIT_TONING); }
static u32 split_toning_rows(volatile u32 *h) { return split_stage(h)==3 ? 9 : split_stage(h)==4 ? 21 : 2; }
static const u16 *split_toning_label(volatile u32 *h,u32 row) { return text(h,split_stage(h)==3 ? TEXT_SPLIT_PRESET+row : split_stage(h)==4 ? (row ? TEXT_STRENGTH_LEVEL+row-1 : TEXT_OFF) : TEXT_SPLIT_MENU+row); }
static u32 split_toning_icon(volatile u32 *h,u32 row) { (void)h;(void)row;return GRKIT_ICON_TILE(0); }
static u32 split_toning_checked(volatile u32 *h,u32 row) { return split_stage(h) && row==split_toning_value(h); }
static void split_toning_apply(volatile u32 *h,u32 row) {
    u32 stage=split_stage(h);
    if(!stage) {if(row<2) {h[GRKIT_HOST_COLOUR_MENU/4]=row+3;h[GRKIT_HOST_MENU_CURSOR/4]=split_toning_value(h);}return;}
    if(stage==4) {h[GRKIT_HOST_SPLIT_STRENGTH/4]=row<21 ? row+1 : 1;return;}
    if(row>=9) return;
    if(!h[GRKIT_HOST_SPLIT_STRENGTH/4]) h[GRKIT_HOST_SPLIT_STRENGTH/4]=h[GRKIT_HOST_SPLIT_TONE/4] ? split_percent(h)/5+1 : 6;
    h[GRKIT_HOST_SPLIT_TONE/4]=split_code(row);
}
#define SPLIT_TONING_PAGE {split_toning_title,split_toning_summary,split_toning_rows,split_toning_label,split_toning_icon,split_toning_checked,split_toning_apply,split_toning_value,0,0,SPLIT_TONING_TILES}
