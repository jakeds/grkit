#include "../../runtime/soft-module/soft.h"
#include "../../runtime/soft-module/colour.h"
void soft_split(PictureSet *set,u32 value) {static const u8 code[9]={0,13,14,15,16,5,2,4,6};u32 h[80]={0};h[GRKIT_HOST_SPLIT_TONE/4]=value<9 ? code[value] : 0;h[GRKIT_HOST_SPLIT_STRENGTH/4]=setting_get(SETTING_STRENGTH)+1;split_toning_apply_picture(h,set);}
