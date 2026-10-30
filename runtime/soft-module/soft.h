#ifndef SOFT_COMMON_H
#define SOFT_COMMON_H
#include "grkit.h"
#include "grkit_module.h"
typedef short s16;
#define GRKIT_PICTURE_MASKS GRKIT_PLANE_SCRATCH
#define GRKIT_MAYBE_UNUSED __attribute__((unused))
#define PICTURE_MAP(address) ((u8 *)GRKIT_BYTES(address))
#define GRKIT_HOST_HEADER_SIZE 320
#define GRKIT_HOST_VERSION_VALUE 1
#define GRKIT_HOST_COUNT 0
#define GRKIT_HOST_RECORDS 4
#define GRKIT_HOST_ACTIVE 8
#define GRKIT_HOST_LAST_APPLIED 12
#define GRKIT_HOST_MAGIC 16
#define GRKIT_HOST_VERSION 20
#define GRKIT_HOST_PROCESSING_CONTEXT 24
#define GRKIT_HOST_PROCESSING_CALLS 28
#define GRKIT_HOST_MENU_CONTROLLER 32
#define GRKIT_HOST_MENU_LEVEL 36
#define GRKIT_HOST_MENU_CURSOR 40
#define GRKIT_HOST_MENU_SLOT 44
#define GRKIT_HOST_GUIDE_PREVIEW 48
#define GRKIT_HOST_STAMP 52
#define GRKIT_HOST_PLUGINS 56
#define GRKIT_HOST_MENU_REBUILDING 60
#define GRKIT_HOST_MENU_LIST 64
#define GRKIT_HOST_MENU_LIST_WORDS 2
#define GRKIT_HOST_GRAIN 72
#define GRKIT_HOST_MENU_EVENTS 76
#define GRKIT_HOST_MENU_APPLIES 80
#define GRKIT_HOST_OFF_ICON 84
#define GRKIT_HOST_MENU_EDGE 88
#define GRKIT_HOST_BACK_PENDING 92
#define GRKIT_HOST_MENU_TITLE 96
#define GRKIT_HOST_MENU_TITLE_WORDS 2
#define GRKIT_HOST_MENU_DETAIL 104
#define GRKIT_HOST_MENU_DETAIL_WORDS 2
#define GRKIT_HOST_MENU_EVENT_RING 112
#define GRKIT_HOST_MENU_EVENT_RING_WORDS 8
#define GRKIT_HOST_PARAMETERS 144
#define GRKIT_HOST_PARAMETER_CHANGES 148
#define GRKIT_HOST_LIVE_PARAMETER_CHANGES 152
#define GRKIT_HOST_GRAIN_SEQUENCE 156
#define GRKIT_HOST_GRAIN_SEED 160
#define GRKIT_HOST_BLOOM 164
#define GRKIT_HOST_HALATION 168
#define GRKIT_HOST_STAMP_LOOK 172
#define GRKIT_HOST_PLUGINS_ICON 176
#define GRKIT_HOST_TOOLBAR_WIDGET 180
#define GRKIT_HOST_TOOLBAR_OWNER 184
#define GRKIT_HOST_MODULE 188
#define GRKIT_HOST_ICON_CHUNKS 192
#define GRKIT_HOST_FRAME_FILE 196
#define GRKIT_HOST_LIVE_RECORD 200
#define GRKIT_HOST_GUIDE 204
#define GRKIT_HOST_FRAME_RATIO 208
#define GRKIT_HOST_FRAME_PREVIEW 212
#define GRKIT_HOST_FRAME_DRAWS 216
#define GRKIT_HOST_FRAME_PREVIEW_CANVAS 220
#define GRKIT_HOST_TILE_ICON 224
#define GRKIT_HOST_FRAME_OWNER 228
#define GRKIT_HOST_FRAME_CANVAS 232
#define GRKIT_HOST_FRAME_BITMAP 236
#define GRKIT_HOST_FRAME_SKIPPED 240
#define GRKIT_HOST_PREVIEW 244
#define GRKIT_HOST_STILL 248
#define GRKIT_HOST_RETURN 252
#define GRKIT_HOST_BLACK_FLOOR 256
#define GRKIT_HOST_SPLIT_TONE 260
#define GRKIT_HOST_COLOUR_RANGE 264
#define GRKIT_HOST_COLOUR_SATURATION 268
#define GRKIT_HOST_COLOUR_HUE 272
#define GRKIT_HOST_COLOUR_BRIGHTNESS 276
#define GRKIT_HOST_HIGHLIGHT_ROLLOFF 280
#define GRKIT_HOST_COLOUR_MENU 284
#define GRKIT_HOST_SHORTCUT 288
#define GRKIT_HOST_SHORTCUT_STATE 292
#define GRKIT_HOST_SHORTCUT_REQUESTS 296
#define GRKIT_HOST_MENU_SECTION 300
#define GRKIT_HOST_SATURATION_UPPER 304
#define GRKIT_HOST_SPLIT_STRENGTH 308
#define GRKIT_HOST_GRAIN_BOOST 312
#define GRKIT_HOST_GRAIN_PATTERN 316
/* Versioned 5% controls. Legacy codes remain readable until edited. */
#define COLOUR_V2 (1U<<28)
static inline u32 strength_code(u32 row) { return row && row<=20 ? row+32 : 0; }
static inline u32 strength_value(u32 code) {
    static const u8 units[13]={0,4,8,12,1,2,3,5,6,7,9,10,11};
    return code<13 ? units[code] : 0;
}
static inline u32 strength_q(u32 code) {
    return code>=33 && code<=52 ? ((code-32)*3072+10)/20 : strength_value(code)*256;
}
static inline u32 strength_row(u32 code,u32 diffusion) {
    if(code>=33 && code<=52) return code-32;
    u32 v=strength_value(code);if(diffusion) return ((v<=8 ? v : 2*v-8)*20+8)/16;
    return (v*20+6)/12;
}
static inline u32 diffusion_gain(u32 code) {
    if(code>=33 && code<=52) return ((code-32)*1024+10)/20;
    u32 v=strength_value(code);return (v<=8 ? v : 2*v-8)*64;
}
static inline int colour_setting(u32 packed,u32 upper,u32 range,u32 saturation) {
    static const signed char sat[8]={0,-25,-50,-75,25,50,-10,10},other[8]={0,-20,-10,10,20,-5,5,-15};
    if(!(packed&COLOUR_V2)) return saturation ? sat[(packed>>(range*3))&7] : other[(packed>>(range*3))&7];
    u32 code=saturation ? (range<4 ? (packed>>(range*5))&31 : (upper>>((range-4)*5))&31) : (packed>>(range*4))&15;
    if(code>(saturation ? 25U : 8U)) return 0;
    return code<=(saturation ? 15U : 4U) ? -(int)code*5 : (int)(code-(saturation ? 15U : 4U))*5;
}
static inline u32 colour_encode(int value,u32 saturation) { return value<=0 ? (u32)(-value/5) : (saturation ? 15U : 4U)+(u32)(value/5); }
static inline u32 colour_menu_code(u32 row,u32 saturation) {
    int value=(int)row*5-(saturation ? 75 : 20);return colour_encode(value,saturation);
}
static inline u32 colour_menu_row(u32 code,u32 saturation) {
    int value=code<=(saturation ? 15U : 4U) ? -(int)code*5 : (int)(code-(saturation ? 15U : 4U))*5;
    return (u32)((value+(saturation ? 75 : 20))/5);
}
static inline u32 colour_read(volatile u32 *h,u32 word,u32 max) {
    u32 range=h[GRKIT_HOST_COLOUR_RANGE/4];(void)max;
    if(!range || range>7) return 0;
    u32 sat=word==GRKIT_HOST_COLOUR_SATURATION;
    return colour_encode(colour_setting(h[word/4],sat ? h[GRKIT_HOST_SATURATION_UPPER/4] : 0,range-1,sat),sat);
}
static inline void colour_write(volatile u32 *h,u32 word,u32 code,u32 max) {
    u32 range=h[GRKIT_HOST_COLOUR_RANGE/4],sat=word==GRKIT_HOST_COLOUR_SATURATION,lower=COLOUR_V2,upper=0;(void)max;
    if(!range || range>7 || code>(sat ? 25U : 8U)) return;
    for(u32 i=0;i<7;++i) {
        u32 c=i==range-1 ? code : colour_encode(colour_setting(h[word/4],sat ? h[GRKIT_HOST_SATURATION_UPPER/4] : 0,i,sat),sat);
        if(sat && i>=4) upper|=c<<((i-4)*5);else lower|=c<<(i*(sat ? 5 : 4));
    }
    if(sat) h[GRKIT_HOST_SATURATION_UPPER/4]=upper;
    h[word/4]=lower==COLOUR_V2 && !upper ? 0 : lower;
}
static inline u32 split_preset(u32 code) {
    static const u8 preset[17]={0,1,1,2,2,3,4,1,1,2,2,3,4,5,6,7,8};return code<17 ? preset[code] : 0;
}
static inline u32 split_percent(volatile u32 *h) {
    static const u8 percent[17]={25,50,100,50,100,100,100,25,75,25,75,50,50,100,100,100,100};
    u32 value=h[GRKIT_HOST_SPLIT_STRENGTH/4],code=h[GRKIT_HOST_SPLIT_TONE/4];
    return value && value<=21 ? (value-1)*5 : code<17 ? percent[code] : 0;
}

#endif
