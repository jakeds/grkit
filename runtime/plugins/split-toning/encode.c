#include "../../plugin.h"
static u32 split_toning_wanted(volatile u32 *h,const PictureSet *set) { (void)set; return h[GRKIT_HOST_SPLIT_TONE/4]>0 && h[GRKIT_HOST_SPLIT_TONE/4]<=16; }
static void split_toning_encode(volatile u32 *h,PictureSet *set) { split_toning_apply_picture(h,set); }
