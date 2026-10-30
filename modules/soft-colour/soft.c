#include "../../runtime/soft-module/soft.h"
#include "../../runtime/soft-module/colour.h"
static int decoded(u32 value,u32 sat) {return !value ? 0 : sat ? (value<=15 ? ((int)value-16)*5 : ((int)value-15)*5) : (value<=4 ? ((int)value-5)*5 : ((int)value-4)*5);}
void soft_colour(PictureSet *set,u32 value) {
 u32 h[80]={0},lower=COLOUR_V2,upper=0,hue=COLOUR_V2,light=COLOUR_V2,any=0;
 for(u32 i=0;i<7;++i) {u32 base=4+i*3;u32 s=colour_encode(decoded(setting_get(GRKIT_SETTING(base)),1),1),a=colour_encode(decoded(setting_get(GRKIT_SETTING(base+1)),0),0),b=colour_encode(decoded(setting_get(GRKIT_SETTING(base+2)),0),0);any|=s|a|b;if(i<4)lower|=s<<(i*5);else upper|=s<<((i-4)*5);hue|=a<<(i*4);light|=b<<(i*4);}
 if(!any)return;
 h[GRKIT_HOST_COLOUR_RANGE/4]=value;h[GRKIT_HOST_COLOUR_SATURATION/4]=lower;h[GRKIT_HOST_SATURATION_UPPER/4]=upper;h[GRKIT_HOST_COLOUR_HUE/4]=hue;h[GRKIT_HOST_COLOUR_BRIGHTNESS/4]=light;colour_range_apply_picture(h,set);
}
void soft_colour_event(u32 event,u32 handle,u32 value) {
 u32 range=setting_get(SETTING_RANGE);if(!range || range>7)return;u32 base=4+(range-1)*3;
 if(event==GRKIT_EVENT_SYSTEM_READY || handle==SETTING_RANGE) {setting_set(SETTING_SAT,setting_get(GRKIT_SETTING(base)));setting_set(SETTING_HUE,setting_get(GRKIT_SETTING(base+1)));setting_set(SETTING_LIGHT,setting_get(GRKIT_SETTING(base+2)));}
 else if(handle==SETTING_SAT)setting_set(GRKIT_SETTING(base),value);else if(handle==SETTING_HUE)setting_set(GRKIT_SETTING(base+1),value);else if(handle==SETTING_LIGHT)setting_set(GRKIT_SETTING(base+2),value);
}
