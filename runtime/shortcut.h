#ifndef SOFT_SHORTCUT_H
#define SOFT_SHORTCUT_H
#include "plugin.h"
#define SHORTCUT_HELD 1U
#define SHORTCUT_PENDING 2U
/* A swallowed press owns its repeat/release even if the option is switched
   off meanwhile. A fresh initial press also clears a stale lost-release latch. */
static u32 shortcut_consume(volatile u32 *h,u32 kind,u32 phase,u32 repeats) {
    if(kind!=EV_FN) return 0;
    /* A second press before ADJ is ready must not queue another transition. */
    if(phase==2 && !repeats && (h[GRKIT_HOST_SHORTCUT_STATE/4]&SHORTCUT_PENDING)) return 1;
    if(!(h[GRKIT_HOST_SHORTCUT_STATE/4]&SHORTCUT_HELD)) return 0;
    if(phase==1 || (phase==2 && !repeats)) h[GRKIT_HOST_SHORTCUT_STATE/4]&=~SHORTCUT_HELD;
    return phase!=2 || repeats!=0;
}
static u32 shortcut_request(volatile u32 *h,u32 movie,u32 kind,u32 phase,u32 repeats) {
    if(movie || kind!=EV_FN || phase!=2 || repeats || h[GRKIT_HOST_SHORTCUT/4]!=1 ||
       !(h[GRKIT_HOST_PLUGINS/4]>>PLUGIN_SOFT_SHORTCUT&1)) return 0;
    h[GRKIT_HOST_SHORTCUT_STATE/4]=SHORTCUT_HELD|SHORTCUT_PENDING;
    ++h[GRKIT_HOST_SHORTCUT_REQUESTS/4];
    return 1;
}
static u32 shortcut_ready(volatile u32 *h,u32 movie) {
    u32 pending=h[GRKIT_HOST_SHORTCUT_STATE/4]&SHORTCUT_PENDING;
    h[GRKIT_HOST_SHORTCUT_STATE/4]&=~SHORTCUT_PENDING;
    return pending && !movie && h[GRKIT_HOST_SHORTCUT/4]==1 &&
           (h[GRKIT_HOST_PLUGINS/4]>>PLUGIN_SOFT_SHORTCUT&1);
}
#endif
