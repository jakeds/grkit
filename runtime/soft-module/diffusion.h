#ifndef SOFT_DIFFUSION_H
#define SOFT_DIFFUSION_H
#include "soft.h"
/* Experimental JPEG diffusion. Three bounded, low-resolution maps reuse the
   picture scratch before the date stamp. No allocation, firmware calls or RNG.
   A 208/255 highlight threshold and frame-relative radius are intentional first
   test settings. This is a software approximation, not a physical HDF model. */
#define DIFFUSION_SIDE 192U
#define DIFFUSION_MAP (DIFFUSION_SIDE*DIFFUSION_SIDE)
_Static_assert(3*DIFFUSION_MAP<=GRKIT_PICTURE_MASKS,"Diffusion scratch overflow");
static u8 diffusion_clip(int x) { return x<0 ? 0 : x>255 ? 255 : (u8)x; }
static void diffusion_blur(const u8 *in,u8 *out,u32 w,u32 h) {
    u32 x,y;
    for(y=0;y<h;++y) for(x=0;x<w;++x) {
        u32 l=x ? x-1 : x,r=x+1<w ? x+1 : x;
        u32 t=y ? y-1 : y,b=y+1<h ? y+1 : y;
        out[y*w+x]=(u8)((in[t*w+l]+2*in[t*w+x]+in[t*w+r]+
            2*in[y*w+l]+4*in[y*w+x]+2*in[y*w+r]+
            in[b*w+l]+2*in[b*w+x]+in[b*w+r]+8)>>4);
    }
}
/* Bilinear sampling avoids visible blocks; cell centres match the source. */
static u32 diffusion_sample(const u8 *map,u32 w,u32 h,u32 x,u32 y,u32 shift) {
    u32 step=1U<<shift,mask=step-1,ix,iy,rx,ry,jx,jy,a,b;
    x=x>=step/2 ? x-step/2 : 0;
    y=y>=step/2 ? y-step/2 : 0;
    ix=x>>shift; iy=y>>shift; rx=x&mask; ry=y&mask;
    if(ix>=w) ix=w-1;
    if(iy>=h) iy=h-1;
    jx=ix+1<w ? ix+1 : ix; jy=iy+1<h ? iy+1 : iy;
    a=map[iy*w+ix]*(step-rx)+map[iy*w+jx]*rx;
    b=map[jy*w+ix]*(step-rx)+map[jy*w+jx]*rx;
    return (a*(step-ry)+b*ry)>>(2*shift);
}
static void diffusion_apply(const PictureSet *set,u32 level,u32 warm) {
    u32 gain=diffusion_gain(level);
    u32 n;
    if(!gain) return;
    for(n=0;n<set->count;++n) {
        const Plane *p=&set->planes[n];
        u32 width=p->right-p->left,height=p->bottom-p->top;
        u32 step=1,shift=0,w,h,x,y,pass,energy=0;
        u8 *source=set->scratch,*a=source+DIFFUSION_MAP,*b=a+DIFFUSION_MAP,*swap;
        if(!width || !height || width>8192 || height>8192) continue;
        while(((width+step-1)>>shift)>DIFFUSION_SIDE ||
              ((height+step-1)>>shift)>DIFFUSION_SIDE) { step<<=1; ++shift; }
        w=(width+step-1)>>shift; h=(height+step-1)>>shift;
        /* Average highlight energy in each cell, including partial edges. */
        for(y=0;y<h;++y) for(x=0;x<w;++x) {
            u32 xx,yy,sum=0,count=0,xend=(x+1)*step,yend=(y+1)*step;
            if(xend>width) xend=width;
            if(yend>height) yend=height;
            for(yy=y*step;yy<yend;++yy) {
                const u8 *row=PICTURE_MAP(p->y+(p->top+yy)*p->pitch+p->left);
                for(xx=x*step;xx<xend;++xx) { u32 v=row[xx]; if(v>208) sum+=(v-208)*5; ++count; }
            }
            source[y*w+x]=(u8)divide(sum,count);
            energy|=source[y*w+x];
        }
        if(!energy) continue;
        for(x=0;x<w*h;++x) a[x]=source[x];
        for(pass=0;pass<(warm ? 3U : 2U);++pass) {
            diffusion_blur(a,b,w,h); swap=a; a=b; b=swap;
        }
        for(y=0;y<height;++y) {
            u8 *row=PICTURE_MAP(p->y+(p->top+y)*p->pitch+p->left);
            u8 *chroma=PICTURE_MAP(chroma_row(p,p->top+y));
            for(x=0;x<width;++x) {
                u32 glow=diffusion_sample(a,w,h,x,y,shift),original;
                if(warm) {
                    original=diffusion_sample(source,w,h,x,y,shift);
                    glow=glow>original ? glow-original : 0;
                }
                glow*=gain;
                if(!glow) continue;
                row[x]=diffusion_clip(row[x]+(int)(glow>>(warm ? 13 : 12)));
                /* One chroma write per 4:2:2/4:2:0 pair. Owned monochrome
                   records keep their neutral chroma; bloom is always neutral. */
                if(warm && !set->mono && !((p->left+x)&1) &&
                   (!p->half || !((p->top+y)&1)) && p->left+x+1<p->right) {
                    u32 at=p->left+x;
                    chroma[at]=diffusion_clip(chroma[at]-(int)(glow>>13));
                    chroma[at+1]=diffusion_clip(chroma[at+1]+(int)(glow>>12));
                }
            }
        }
    }
}
#endif
