#include "../../plugin.h"
static u32 bloom_wanted(volatile u32 *h,const PictureSet *set) {
    u32 v=h[GRKIT_HOST_BLOOM/4]; (void)set; return v && v<=52;
}
static void bloom_encode(volatile u32 *h,PictureSet *set) {
    diffusion_apply(set,h[GRKIT_HOST_BLOOM/4],0);
}
