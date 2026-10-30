#include <assert.h>
#include "grkit_module.h"
#define W 64U
#define H 48U
static PictureSet plane(u32 y,u32 cb,u32 cr) {PictureSet s={{{GRKIT_TEST_PLANES,GRKIT_TEST_PLANES+W*H,W,H,W,0,0,0,W,H}},1,0,0,grkit_test_scratch,0};for(u32 i=0;i<W*H;++i){GRKIT_BYTES(s.planes[0].y+i)[0]=y;GRKIT_BYTES(s.planes[0].c+i)[0]=(i&1)?cr:cb;}return s;}
void soft_split(PictureSet *,u32);
int main(void){PictureSet s=plane(0,128,128);soft_split(&s,1);assert(GRKIT_BYTES(s.planes[0].c)[0]==128);grkit_test_settings[SETTING_STRENGTH]=20;soft_split(&s,1);assert(GRKIT_BYTES(s.planes[0].c)[0]==112&&GRKIT_BYTES(s.planes[0].c+1)[0]==144);s=plane(128,128,128);soft_split(&s,1);assert(GRKIT_BYTES(s.planes[0].c)[0]==128);return 0;}
