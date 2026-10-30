#include <assert.h>
#include "grkit_module.h"
#define W 64U
#define H 48U
static PictureSet plane(u32 y,u32 cb,u32 cr) {PictureSet s={{{GRKIT_TEST_PLANES,GRKIT_TEST_PLANES+W*H,W,H,W,0,0,0,W,H}},1,0,0,grkit_test_scratch,0};for(u32 i=0;i<W*H;++i){GRKIT_BYTES(s.planes[0].y+i)[0]=y;GRKIT_BYTES(s.planes[0].c+i)[0]=(i&1)?cr:cb;}return s;}
void soft_black(PictureSet *,u32);void soft_rolloff(PictureSet *,u32);
int main(void){PictureSet s=plane(0,128,128);soft_black(&s,0);assert(GRKIT_BYTES(s.planes[0].y)[0]==0);soft_black(&s,20);assert(GRKIT_BYTES(s.planes[0].y)[0]==48);s=plane(128,128,128);soft_black(&s,20);assert(GRKIT_BYTES(s.planes[0].y)[0]==128);s=plane(255,128,128);soft_rolloff(&s,20);assert(GRKIT_BYTES(s.planes[0].y)[0]<255);return 0;}
