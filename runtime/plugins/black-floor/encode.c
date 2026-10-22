#include "../../plugin.h"
static u32 black_floor_wanted(volatile u32 *h,const PictureSet *set) { (void)set; return h[GRKIT_HOST_BLACK_FLOOR/4]>0 && h[GRKIT_HOST_BLACK_FLOOR/4]<=52; }
static void black_floor_encode(volatile u32 *h,PictureSet *set) { black_floor_apply_picture(h,set); }
