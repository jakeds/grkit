#include <assert.h>
#include "grkit_module.h"
#define W 64U
#define H 48U
static PictureSet plane(u32 y,u32 cb,u32 cr) {PictureSet s={{{GRKIT_TEST_PLANES,GRKIT_TEST_PLANES+W*H,W,H,W,0,0,0,W,H}},1,0,0,grkit_test_scratch,0};for(u32 i=0;i<W*H;++i){GRKIT_BYTES(s.planes[0].y+i)[0]=y;GRKIT_BYTES(s.planes[0].c+i)[0]=(i&1)?cr:cb;}return s;}
void soft_colour(PictureSet *,u32);void soft_colour_event(u32,u32,u32);
int main(void){grkit_test_settings[SETTING_RANGE]=4;soft_colour_event(GRKIT_EVENT_SETTING_CHANGED,SETTING_RANGE,4);assert(grkit_test_settings[SETTING_SAT]==0);soft_colour_event(GRKIT_EVENT_SETTING_CHANGED,SETTING_SAT,1);assert(grkit_test_settings[GRKIT_SETTING(13)]==1);grkit_test_settings[SETTING_RANGE]=6;soft_colour_event(GRKIT_EVENT_SETTING_CHANGED,SETTING_RANGE,6);assert(grkit_test_settings[SETTING_SAT]==0);grkit_test_settings[SETTING_RANGE]=4;soft_colour_event(GRKIT_EVENT_SETTING_CHANGED,SETTING_RANGE,4);assert(grkit_test_settings[SETTING_SAT]==1);PictureSet s=plane(64,64,200);soft_colour(&s,0);assert(GRKIT_BYTES(s.planes[0].c)[0]==64);soft_colour(&s,4);assert(GRKIT_BYTES(s.planes[0].y)[0]==64);return 0;}
