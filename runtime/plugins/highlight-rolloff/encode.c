#include "../../plugin.h"
static u32 highlight_rolloff_wanted(volatile u32 *h,const PictureSet *set) { (void)set; return h[GRKIT_HOST_HIGHLIGHT_ROLLOFF/4]>0 && h[GRKIT_HOST_HIGHLIGHT_ROLLOFF/4]<=52; }
static void highlight_rolloff_encode(volatile u32 *h,PictureSet *set) { highlight_rolloff_apply_picture(h,set); }
