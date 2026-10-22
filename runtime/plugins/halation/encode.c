#include "../../plugin.h"
static u32 halation_wanted(volatile u32 *h,const PictureSet *set) {
    u32 v=h[GRKIT_HOST_HALATION/4]; (void)set; return v && v<=52;
}
static void halation_encode(volatile u32 *h,PictureSet *set) {
    diffusion_apply(set,h[GRKIT_HOST_HALATION/4],1);
}
