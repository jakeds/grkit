#ifndef SOFT_COLOUR_H
#define SOFT_COLOUR_H
#include "soft.h"
/* JPEG colour controls use bounded existing scratch, with no allocation or
   firmware calls. They run before bloom/halation and the date stamp. */
static u8 colour_clip(int v) { return v<0 ? 0 : v>255 ? 255 : (u8)v; }
static int colour_divide(int value,u32 divisor) {
    return value<0 ? -(int)divide((u32)-value,divisor) : (int)divide((u32)value,divisor);
}
static u32 black_floor_value(u32 y,u32 level) {
    /* Fine controls: 100% reaches Y=48 with a smooth taper to Y=128.
       Legacy codes retain their previous response until edited. */
    if(level>=33 && level<=52) {
        if(y>=128) return y;
        return y+divide((level-32)*48*(128-y)*(128-y)+163840,327680);
    }
    level=strength_q(level);
    if(!level || y>=96) return y;
    return y+divide(level*(96-y)*(96-y)+1179648,2359296);
}
static void black_floor_apply_picture(volatile u32 *h,PictureSet *set) {
    u32 level=h[GRKIT_HOST_BLACK_FLOOR/4],i,x,y;
    u8 *table=set->scratch;
    if(!strength_q(level)) return;
    for(i=0;i<256;++i) table[i]=(u8)black_floor_value(i,level);
    for(i=0;i<set->count;++i) {
        const Plane *p=&set->planes[i];
        for(y=p->top;y<p->bottom;++y) {
            u8 *row=PICTURE_MAP(p->y+y*p->pitch);
            for(x=p->left;x<p->right;++x) row[x]=table[row[x]];
        }
    }
}
/* Shadow/highlight endpoints in Cb/Cr. A dead band 112..144 preserves neutral
   midtones. Labels always name shadows first, highlights second. */
static void split_toning_apply_picture(volatile u32 *h,PictureSet *set) {
    static const s16 tint[12][4]={
        {6,-4,-4,6},{12,-8,-8,12},{4,-6,3,6},
        {8,-12,6,12},{-6,6,0,0},{6,-4,0,0},
        {3,-2,-2,3},{9,-6,-6,9},{2,-3,2,3},{6,-9,5,9},{-3,3,0,0},{3,-2,0,0}
    };
    static const s16 full[8][4]={{12,-8,-8,12},{8,-12,6,12},{-6,6,0,0},{6,-4,0,0},{-4,4,-6,2},{-8,6,0,6},{-4,8,-4,2},{-8,8,-6,6}};
    u32 preset=h[GRKIT_HOST_SPLIT_TONE/4],i,x,y,percent=split_percent(h),family=split_preset(preset);
    s16 cb[256],cr[256];
    if(!preset || preset>16) return;
    for(i=0;i<256;++i) {
        u32 sw=i<112 ? divide((112-i)*(112-i)*256,12544) : 0;
        u32 hw=i>144 ? divide((i-144)*(i-144)*256,12321) : 0;
        if(h[GRKIT_HOST_SPLIT_STRENGTH/4] || preset>12) {
            /* Four times the earlier endpoint range. Round signed values
               symmetrically so low warm strengths survive 8-bit quantisation. */
            int cbq=(full[family-1][0]*(int)sw+full[family-1][2]*(int)hw)*(int)percent*4;
            int crq=(full[family-1][1]*(int)sw+full[family-1][3]*(int)hw)*(int)percent*4;
            cb[i]=(s16)(cbq<0 ? -colour_divide(-cbq+12800,25600) : colour_divide(cbq+12800,25600));
            cr[i]=(s16)(crq<0 ? -colour_divide(-crq+12800,25600) : colour_divide(crq+12800,25600));
        } else {
            cb[i]=(s16)((tint[preset-1][0]*(int)sw+tint[preset-1][2]*(int)hw)/256);
            cr[i]=(s16)((tint[preset-1][1]*(int)sw+tint[preset-1][3]*(int)hw)/256);
        }
    }
    for(i=0;i<set->count;++i) {
        const Plane *p=&set->planes[i];
        /* The camera's valid capture areas and widths are pair-aligned. */
        for(y=p->half ? (p->top+1)&~1U : p->top;y<p->bottom;y+=p->half ? 2 : 1) {
            u8 *chroma=PICTURE_MAP(chroma_row(p,y));
            const u8 *row=PICTURE_MAP(p->y+y*p->pitch);
            if(p->half && y+1>=p->bottom) continue;
            for(x=(p->left+1)&~1U;x+1<p->right;x+=2) {
                u32 luma=(row[x]+row[x+1]+1)>>1;
                if(p->half && y+1<p->bottom) {
                    const u8 *next=PICTURE_MAP(p->y+(y+1)*p->pitch);
                    luma=(row[x]+row[x+1]+next[x]+next[x+1]+2)>>2;
                }
                chroma[x]=colour_clip(chroma[x]+cb[luma]);
                chroma[x+1]=colour_clip(chroma[x+1]+cr[luma]);
            }
        }
    }
}
/* Hue centres of the seven colour ranges in the JPEG chroma plane (Q8).
   Soft overlapping angular windows avoid hard selection edges. This is chroma
   rotation rather than a perceptually uniform HSL editor. */
static const s16 colour_direction[7][2]={
        {-82,243},{-193,168},{-253,42},{-159,-200},
        {82,-243},{253,-42},{159,200}
    };
static void colour_table_extended(u8 *table,u32 saturation,u32 hue,u32 upper) {
    
    static const s16 cosine[9]={241,247,252,255,256,255,252,247,241};
    static const s16 sine[9]={-88,-66,-44,-22,0,22,44,66,88};
    struct {int scale,cosine,sine;u32 range;} active[7];
    u32 cb,cr,range,count=0;
    /* Decode once per table, rather than 458752 times per photograph. */
    for(range=0;range<7;++range) {
        int s=colour_setting(saturation,upper,range,1),angle=colour_setting(hue,0,range,0);
        if(!s && !angle) continue;
        u32 t=(u32)(angle/5+4);
        active[count].scale=256+(s<0 ? -((-s*256+50)/100) : (s*256+50)/100);
        active[count].cosine=cosine[t];active[count].sine=sine[t];active[count++].range=range;
    }
    for(cb=0;cb<256;++cb) for(cr=0;cr<256;++cr) {
        int u=(int)cb-128,v=(int)cr-128,du=0,dv=0;
        u32 weights=0,index=(cb*256+cr)*2;
        for(u32 k=0;k<count;++k) {
            u32 weight;int scale=active[k].scale,cs=active[k].cosine,sn=active[k].sine;
            int along,cross,nu,nv;range=active[k].range;
            along=u*colour_direction[range][0]+v*colour_direction[range][1];
            cross=u*colour_direction[range][1]-v*colour_direction[range][0];
            if(cross<0) cross=-cross;
            if(along<=0 || cross*8>=along*5) continue;
            weight=256-divide((u32)cross*2048,(u32)along*5);
            nu=((u*cs-v*sn)*scale)/65536;
            nv=((u*sn+v*cs)*scale)/65536;
            du+=(nu-u)*(int)weight; dv+=(nv-v)*(int)weight; weights+=weight;
        }
        if(weights<256) weights=256;
        table[index]=colour_clip((int)cb+colour_divide(du,weights));
        table[index+1]=colour_clip((int)cr+colour_divide(dv,weights));
    }
}
GRKIT_MAYBE_UNUSED static void colour_table(u8 *table,u32 saturation,u32 hue) { colour_table_extended(table,saturation,hue,0); }
/* Brightness is a signed luma offset selected using the original chroma.
   Build and consume its 64 KiB lookup before reusing scratch for chroma. */
static void colour_brightness_apply(PictureSet *set,u32 packed) {
    signed char *table=(signed char *)set->scratch;
    u32 cb,cr,range,i,x,y,count=0;
    int steps[7];u32 ranges[7];
    for(range=0;range<7;++range) {
        int step=colour_setting(packed,0,range,0);
        if(step) {steps[count]=step;ranges[count++]=range;}
    }
    for(cb=0;cb<256;++cb) for(cr=0;cr<256;++cr) {
        int u=(int)cb-128,v=(int)cr-128,delta=0;
        u32 weights=0;
        for(u32 k=0;k<count;++k) {
            int step=steps[k];u32 weight;range=ranges[k];
            int along,cross;
            along=u*colour_direction[range][0]+v*colour_direction[range][1];
            cross=u*colour_direction[range][1]-v*colour_direction[range][0];
            if(cross<0) cross=-cross;
            if(along<=0 || cross*8>=along*5) continue;
            weight=256-divide((u32)cross*2048,(u32)along*5);
            delta+=step*(int)weight; weights+=weight;
        }
        table[cb*256+cr]=(signed char)colour_divide(delta,weights<256 ? 256 : weights);
    }
    for(i=0;i<set->count;++i) {
        const Plane *p=&set->planes[i];
        for(y=p->top;y<p->bottom;++y) {
            const u8 *c=PICTURE_MAP(chroma_row(p,y));
            u8 *row=PICTURE_MAP(p->y+y*p->pitch);
            for(x=p->left;x<p->right;++x) {
                u32 pair=x&~1U;
                if(pair+1>=p->width) continue;
                row[x]=colour_clip(row[x]+table[(u32)c[pair]*256+c[pair+1]]);
            }
        }
    }
}
/* Monotonic shoulder above Y=176; no claim of recovering clipped detail. */
static u32 highlight_rolloff_sample(u32 y,u32 level) {
    level=strength_q(level);
    if(!level || y<=176) return y;
    return y-divide(level*2*(y-176)*(y-176)+798848,1597696);
}
static void highlight_rolloff_apply_picture(volatile u32 *h,PictureSet *set) {
    u32 level=h[GRKIT_HOST_HIGHLIGHT_ROLLOFF/4],i,x,y;
    u8 *table=set->scratch;
    if(!strength_q(level)) return;
    for(i=0;i<256;++i) table[i]=(u8)highlight_rolloff_sample(i,level);
    for(i=0;i<set->count;++i) {
        const Plane *p=&set->planes[i];
        for(y=p->top;y<p->bottom;++y) {
            u8 *row=PICTURE_MAP(p->y+y*p->pitch);
            for(x=p->left;x<p->right;++x) row[x]=table[row[x]];
        }
    }
}
static void colour_range_apply_picture(volatile u32 *h,PictureSet *set) {
    u32 saturation=h[GRKIT_HOST_COLOUR_SATURATION/4],hue=h[GRKIT_HOST_COLOUR_HUE/4];
    u32 brightness=h[GRKIT_HOST_COLOUR_BRIGHTNESS/4];
    u32 range=h[GRKIT_HOST_COLOUR_RANGE/4],i,x,y;
    u8 *table=set->scratch;
    _Static_assert(256*256*2<=GRKIT_PICTURE_MASKS,"Colour table scratch overflow");
    u32 chroma=0,light=0;
    if(!range || range>7 || set->mono) return;
    /* COLOUR_V2 is a format marker, not an adjustment. A neutral marker
       previously ran a full brightness pass on saturation/hue-only edits,
       and built the chroma table on brightness-only edits. Decode also
       handles old values and rejects invalid codes exactly as before. */
    for(u32 k=0;k<7;++k) {
        chroma|=colour_setting(saturation,h[GRKIT_HOST_SATURATION_UPPER/4],k,1)!=0 || colour_setting(hue,0,k,0)!=0;
        light|=colour_setting(brightness,0,k,0)!=0;
    }
    if(light) colour_brightness_apply(set,brightness);
    if(!chroma) return;
    colour_table_extended(table,saturation,hue,h[GRKIT_HOST_SATURATION_UPPER/4]);
    for(i=0;i<set->count;++i) {
        const Plane *p=&set->planes[i];
        for(y=p->half ? (p->top+1)&~1U : p->top;y<p->bottom;y+=p->half ? 2 : 1) {
            u8 *row=PICTURE_MAP(chroma_row(p,y));
            if(p->half && y+1>=p->bottom) continue;
            for(x=(p->left+1)&~1U;x+1<p->right;x+=2) {
                u32 at=((u32)row[x]*256+row[x+1])*2;
                row[x]=table[at]; row[x+1]=table[at+1];
            }
        }
    }
}
#endif
