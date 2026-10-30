#!/usr/bin/env python3
"""Compare all chroma pairs and patterned JPEG planes against Soft.16.
Host timings compare algorithm cost only; they are not camera timings.
"""
import json
import subprocess
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BASELINE = 'da4d0886d349247077fe186854b9c62a2bdace1f'
SDK = ROOT / 'runtime/sdk'
HARNESS = r'''
#include <stdio.h>
#include <stdint.h>
#include COLOUR_HEADER
#define W 512U
#define H 256U
static uint64_t digest(const volatile u8 *p,u32 n) {
    uint64_t h=14695981039346656037ULL;
    for(u32 i=0;i<n;++i) h=(h^p[i])*1099511628211ULL;
    return h;
}
int main(void) {
    for(u32 half=0;half<2;++half) for(u32 c=0;c<49;++c) {
        u32 h[80]={0},s=COLOUR_V2,a=COLOUR_V2,b=COLOUR_V2,upper=0;
        if(c<42) {
            u32 range=c/6, kind=c%6;
            u32 code=kind==0?15:kind==1?25:kind==2?4:kind==3?8:kind==4?4:8;
            if(kind<2) {if(range<4)s|=code<<(range*5);else upper|=code<<((range-4)*5);}
            else if(kind<4)a|=code<<(range*4);else b|=code<<(range*4);
        } else if(c==43 || c==44) {
            for(u32 k=0;k<7;++k) {
                u32 v=c==43?k+1:25-k;
                if(k<4)s|=v<<(k*5);else upper|=v<<((k-4)*5);
                a|=(k+1)<<(k*4);b|=(8-k)<<(k*4);
            }
        } else if(c==45) {s=0x00123456;a=0x00098765;b=0x00145678;}
        else if(c==46) {s=0xffffffff;a=0xffffffff;b=0xffffffff;upper=0xffffffff;}
        h[GRKIT_HOST_COLOUR_RANGE/4]=c==47?0:4;
        h[GRKIT_HOST_COLOUR_SATURATION/4]=s;
        h[GRKIT_HOST_SATURATION_UPPER/4]=upper;
        h[GRKIT_HOST_COLOUR_HUE/4]=a;h[GRKIT_HOST_COLOUR_BRIGHTNESS/4]=b;
        u32 yn=W*H, cn=half?yn/2:yn;
        PictureSet set={{{GRKIT_TEST_PLANES,GRKIT_TEST_PLANES+yn,W,H,W,half,0,0,W,H}},1,0,c==48,grkit_test_scratch,0};
        volatile u8 *yp=GRKIT_BYTES(GRKIT_TEST_PLANES),*cp=GRKIT_BYTES(GRKIT_TEST_PLANES+yn);
        for(u32 i=0;i<yn;++i)yp[i]=(i*37+i/W*13)&255;
        /* Every Cb/Cr pair occurs in 4:2:2; complementary quadrants in 4:2:0. */
        for(u32 i=0;i<cn;i+=2) {cp[i]=(i/2)>>8;cp[i+1]=(i/2)&255;}
        colour_range_apply_picture(h,&set);
        printf("%u %u %016llx %016llx\n",half,c,(unsigned long long)digest(yp,yn),(unsigned long long)digest(cp,cn));
    }
    return 0;
}
'''

def main():
    with tempfile.TemporaryDirectory(prefix='soft-colour-') as folder:
        tmp = Path(folder)
        # The pure colour helpers do not use generated module setting indices.
        (tmp/'grkit_module.h').write_text('/* No module setting indices required. */\n')
        old = tmp / 'before.h'
        old.write_bytes(subprocess.check_output(['git','show',f'{BASELINE}:runtime/soft-module/colour.h'],cwd=ROOT))
        harness=tmp/'check.c';harness.write_text(HARNESS)
        results={}
        for name, source in [('before',old),('after',ROOT/'runtime/soft-module/colour.h')]:
            exe=tmp/name
            subprocess.run(['cc','-O2','-std=c11','-Wno-unused-function','-include',str(SDK/'grkit_test.h'),'-I',str(SDK),'-I',str(tmp),'-I',str(ROOT/'runtime/soft-module'),f'-DCOLOUR_HEADER="{source}"',str(harness),str(SDK/'grkit_test.c'),'-o',str(exe)],check=True)
            runs=[]
            for _ in range(3):
                start=time.perf_counter();output=subprocess.check_output([str(exe)]);runs.append(time.perf_counter()-start)
            results[name]=(output,sorted(runs)[1])
        assert results['before'][0]==results['after'][0], 'JPEG plane output differs from Soft.16'
        print(json.dumps({'cases':len(results['after'][0].splitlines()),'saved_plane_output_identical':True,'host_median_seconds':{n:round(v[1],4) for n,v in results.items()},'host_speedup':round(results['before'][1]/results['after'][1],2),'camera_timing_measured':False},indent=2))

if __name__=='__main__':main()
