#include <assert.h>
#include "grkit_module.h"
#define W 64U
#define H 48U
static PictureSet plane(u32 y,u32 cb,u32 cr) {PictureSet s={{{GRKIT_TEST_PLANES,GRKIT_TEST_PLANES+W*H,W,H,W,0,0,0,W,H}},1,0,0,grkit_test_scratch,0};for(u32 i=0;i<W*H;++i){GRKIT_BYTES(s.planes[0].y+i)[0]=y;GRKIT_BYTES(s.planes[0].c+i)[0]=(i&1)?cr:cb;}return s;}
void soft_bloom(PictureSet *,u32);void soft_halation(PictureSet *,u32);
int main(void){PictureSet s=plane(32,128,128);soft_bloom(&s,20);soft_halation(&s,20);for(u32 i=0;i<W*H;++i)assert(GRKIT_BYTES(s.planes[0].y+i)[0]==32);s=plane(32,128,128);GRKIT_BYTES(s.planes[0].y+24*W+32)[0]=255;soft_bloom(&s,20);assert(GRKIT_BYTES(s.planes[0].y+24*W+31)[0]>32);return 0;}
