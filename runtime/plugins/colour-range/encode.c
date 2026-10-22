#include "../../plugin.h"
static u32 colour_range_wanted(volatile u32 *h,const PictureSet *set) { (void)set; return h[GRKIT_HOST_COLOUR_RANGE/4]>0 && h[GRKIT_HOST_COLOUR_RANGE/4]<=7 && (h[GRKIT_HOST_COLOUR_SATURATION/4] || h[GRKIT_HOST_COLOUR_HUE/4] || h[GRKIT_HOST_COLOUR_BRIGHTNESS/4]); }
static void colour_range_encode(volatile u32 *h,PictureSet *set) { colour_range_apply_picture(h,set); }
