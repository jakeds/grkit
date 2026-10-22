#include "../../plugin.h"
static u32 colour_range_value(volatile u32 *h) { return (h[GRKIT_HOST_COLOUR_RANGE/4]<8 ? h[GRKIT_HOST_COLOUR_RANGE/4] : 0); }
static u32 colour_child(volatile u32 *h,u32 row) {
    const u32 child[3]={PLUGIN_COLOUR_SATURATION,PLUGIN_COLOUR_HUE,PLUGIN_COLOUR_BRIGHTNESS};
    for(u32 i=0;i<3;++i) if(h[GRKIT_HOST_PLUGINS/4]>>child[i]&1) { if(!row--) return child[i]; }
    return PLUGIN_COLOUR_RANGE;
}
static const u16 *colour_range_title(volatile u32 *h) { return text(h,h[GRKIT_HOST_COLOUR_MENU/4] && colour_range_value(h) ? TEXT_COLOUR_RANGE_NAME+colour_range_value(h)-1 : TEXT_COLOUR_RANGE); }
static const u16 *colour_range_summary(volatile u32 *h) { return text(h,TEXT_COLOUR_RANGE_SUMMARY+colour_range_value(h)); }
static u32 colour_range_rows(volatile u32 *h) {
    if(!h[GRKIT_HOST_COLOUR_MENU/4]) return 8;
    u32 n=0; while(n<3 && colour_child(h,n)!=PLUGIN_COLOUR_RANGE) ++n; return n;
}
static const u16 *colour_range_label(volatile u32 *h,u32 row) {
    if(h[GRKIT_HOST_COLOUR_MENU/4]) {
        u32 child=colour_child(h,row);
        return text(h,child==PLUGIN_COLOUR_SATURATION ? TEXT_COLOUR_SATURATION : child==PLUGIN_COLOUR_HUE ? TEXT_COLOUR_HUE : TEXT_COLOUR_BRIGHTNESS);
    }
    return text(h,(row && row<8 ? TEXT_COLOUR_RANGE_NAME+row-1 : TEXT_OFF));
}
static u32 colour_range_icon(volatile u32 *h,u32 row) { (void)h; (void)row; return GRKIT_ICON_TILE(0); }
static u32 colour_range_checked(volatile u32 *h,u32 row) { return !h[GRKIT_HOST_COLOUR_MENU/4] && row==colour_range_value(h); }
static void colour_range_apply(volatile u32 *h,u32 row) {
    if(h[GRKIT_HOST_COLOUR_MENU/4]) {
        u32 child=colour_child(h,row),word=child==PLUGIN_COLOUR_SATURATION ? GRKIT_HOST_COLOUR_SATURATION : child==PLUGIN_COLOUR_HUE ? GRKIT_HOST_COLOUR_HUE : GRKIT_HOST_COLOUR_BRIGHTNESS;
        if(child==PLUGIN_COLOUR_RANGE) return;
        h[GRKIT_HOST_MODULE/4]=child;
        h[GRKIT_HOST_MENU_CURSOR/4]=colour_menu_row(colour_read(h,word,7),child==PLUGIN_COLOUR_SATURATION);
    } else {
        h[GRKIT_HOST_COLOUR_RANGE/4]=row<8 ? row : 0;
        if(row && row<8 && colour_child(h,0)!=PLUGIN_COLOUR_RANGE) {
            h[GRKIT_HOST_COLOUR_MENU/4]=1; h[GRKIT_HOST_MENU_CURSOR/4]=0;
        }
    }
}
#define COLOUR_RANGE_PAGE {colour_range_title,colour_range_summary,colour_range_rows,colour_range_label,colour_range_icon,colour_range_checked,colour_range_apply,colour_range_value,0,0,COLOUR_RANGE_TILES}
