#include "../../runtime/soft-module/soft.h"
#include "../../runtime/soft-module/colour.h"
void soft_black(PictureSet *set,u32 value) {u32 h[80]={0};h[GRKIT_HOST_BLACK_FLOOR/4]=strength_code(value);black_floor_apply_picture(h,set);}
void soft_rolloff(PictureSet *set,u32 value) {u32 h[80]={0};h[GRKIT_HOST_HIGHLIGHT_ROLLOFF/4]=strength_code(value);highlight_rolloff_apply_picture(h,set);}
