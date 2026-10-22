/* Framing: the menu page. Row 0 is Off, then the ratios of the target
   (XPan 65:24, 16:9, 4:3). The ratio is the capture area that the core's
   capture.aspect hooks and the other plugins read (GRKIT_HOST_FRAME_RATIO). */
#include "../../plugin.h"

static u32 frame_code(u32 row) {
#ifdef GRKIT_TARGET_GR3_210
    static const u8 codes[6]={0,5,4,6,3,2};return row<6 ? codes[row] : 0;
#else
    return row<framing_ratios() ? row : 0;
#endif
}
static u32 frame_ratio(volatile u32 *h) {
    u32 value=h[GRKIT_HOST_FRAME_RATIO/4];
#ifdef GRKIT_TARGET_GR3_210
    if(value==1) return 0; /* XPan is no longer offered. */
#endif
    return value<framing_ratios() ? value : 0;
}
static u32 frame_committed(volatile u32 *h) { for(u32 row=0;row<6;++row) if(frame_code(row)==frame_ratio(h)) return row;return 0; }
static const u16 *frame_title(volatile u32 *h) { return text(h,TEXT_FRAMING); }
static const u16 *frame_summary(volatile u32 *h) {
#ifdef GRKIT_TARGET_GR3_210
    return text(h,TEXT_CROP_SUMMARY+frame_ratio(h));
#else
    return text(h,TEXT_FRAMING_SUMMARY+frame_ratio(h));
#endif
}
static u32 frame_rows(volatile u32 *h) { (void)h; return framing_ratios()==7 ? 6 : framing_ratios(); }
/* 65:24 is named after the XPan panoramic format. */
static const u16 *frame_label(volatile u32 *h,u32 row) {
#ifdef GRKIT_TARGET_GR3_210
    return text(h,TEXT_CROP_NAME+frame_code(row));
#else
    return text(h,row ? TEXT_RATIO+row-1 : TEXT_OFF);
#endif
}
/* The root tile shows the selected ratio, like Image Control shows its effect. */
static u32 frame_icon(volatile u32 *h,u32 row) {
    return h[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT || row ? GRKIT_ICON_TILE(0) : GRKIT_ICON_OFF;
}
static u32 frame_checked(volatile u32 *h,u32 row) { return frame_code(row)==frame_ratio(h); }
static void frame_apply(volatile u32 *h,u32 row) {
    h[GRKIT_HOST_FRAME_RATIO/4]=frame_code(row);
    request_native_aspect();
}

/* The module tile, then XPan, 16:9, and 4:3. */
#define FRAMING_PAGE {frame_title,frame_summary,frame_rows,frame_label,frame_icon, \
    frame_checked,frame_apply,frame_committed,0,0,FRAMING_TILES}
