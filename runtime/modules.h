#ifndef GRKIT_MODULES_H
#define GRKIT_MODULES_H
#include "generated/pages.h"
#include "about.h"
#define ABOUT_MODULE PLUGIN_COUNT
#define GROUP_FRAME (PLUGIN_COUNT+1U)
#define GROUP_TONE (PLUGIN_COUNT+2U)
#define GROUP_TEXTURE (PLUGIN_COUNT+3U)
#define ARRAY_COUNT(a) (sizeof(a)/sizeof((a)[0]))
static u32 current_index(volatile u32 *h);
static u32 menu_back(volatile u32 *h);
static const u16 *group_text(volatile u32 *h) { return text(h,TEXT_MENU_GROUP_SUMMARY+current_index(h)-GROUP_FRAME); }
#define GROUP_PAGE {group_text,group_text,about_none,about_label,about_icon,about_checked,about_apply,about_none,0,0,0}
static const Module modules[]={PLUGIN_PAGES,ABOUT_PAGE,GROUP_PAGE,GROUP_PAGE,GROUP_PAGE};
#define PLUGIN_ICON (GRKIT_ICON_GRKIT-PLUGIN_TILES)
GRKIT_MAYBE_UNUSED static u32 module_icon_count(void) {
    u32 i,n=0;for(i=0;i<ARRAY_COUNT(modules);++i) n+=modules[i].tiles;return n;
}
static u32 module_icon(u32 module,u32 icon) {
    u32 i,first=PLUGIN_ICON;
    if(icon<GRKIT_ICON_TILE(0)) return icon;
    for(i=0;i<module;++i) first+=modules[i].tiles;
    return first+icon-GRKIT_ICON_TILE(0);
}
static void frame_preview(volatile u32 *h,u32 canvas,u32 movie) {
    u32 module=h[GRKIT_HOST_MENU_LEVEL/4]==MENU_MODULE ? h[GRKIT_HOST_MODULE/4] : 0;
    u32 page=module==PLUGIN_FRAMING || module==PLUGIN_GUIDES;
    u32 valid=!page || h[GRKIT_HOST_MENU_CURSOR/4]<modules[module].rows(h);
    if(!valid) page=0;
    u32 ratio=module==PLUGIN_FRAMING && valid ? frame_code(h[GRKIT_HOST_MENU_CURSOR/4]) : frame_ratio(h);
    u32 style=module==PLUGIN_GUIDES && valid ? h[GRKIT_HOST_MENU_CURSOR/4] : guide_style(h);
    if(movie) { h[GRKIT_HOST_FRAME_PREVIEW/4]=0; return; }
    h[GRKIT_HOST_GUIDE_PREVIEW/4]=style;
    h[GRKIT_HOST_FRAME_PREVIEW/4]=ratio || style || page ? ratio+1 : 0;
    h[GRKIT_HOST_FRAME_PREVIEW_CANVAS/4]=canvas;
}
static u32 visible_modules(volatile u32 *h) {
    u32 mask=h[GRKIT_HOST_PLUGINS/4]&((1U<<PLUGIN_COUNT)-1);
    if(!h[GRKIT_HOST_COUNT/4]) mask&=~PLUGIN_NEEDS_EFFECTS;
    return mask;
}
/* The range selector owns its three parameter pages. Packages can omit any
   controls; singleton categories become direct entries to avoid a needless level. */
static u32 category_mask(volatile u32 *h,u32 category) {
    u32 mask=visible_modules(h),wanted;
    if(mask>>PLUGIN_COLOUR_RANGE&1) mask&=~((1U<<PLUGIN_COLOUR_SATURATION)|(1U<<PLUGIN_COLOUR_HUE)|(1U<<PLUGIN_COLOUR_BRIGHTNESS));
    if(category==1) wanted=(1U<<PLUGIN_FRAMING)|(1U<<PLUGIN_GUIDES);
    else if(category==2) wanted=(1U<<PLUGIN_BLACK_FLOOR)|(1U<<PLUGIN_HIGHLIGHT_ROLLOFF)|(1U<<PLUGIN_SPLIT_TONING)|(1U<<PLUGIN_COLOUR_RANGE)|(1U<<PLUGIN_COLOUR_SATURATION)|(1U<<PLUGIN_COLOUR_HUE)|(1U<<PLUGIN_COLOUR_BRIGHTNESS);
    else wanted=(1U<<PLUGIN_GRAIN)|(1U<<PLUGIN_BLOOM)|(1U<<PLUGIN_HALATION);
    return mask&wanted;
}
static u32 root_modules(volatile u32 *h) {
    u32 category=h[GRKIT_HOST_MENU_SECTION/4],mask;
    if(category) return category_mask(h,category);
    mask=visible_modules(h)&((1U<<PLUGIN_IMAGE_CONTROL)|(1U<<PLUGIN_DATE_STAMP)|(1U<<PLUGIN_SOFT_SHORTCUT)|(1U<<ABOUT_MODULE));
    for(category=1;category<=3;++category) {
        u32 children=category_mask(h,category);
        mask|=children && (children&(children-1)) ? 1U<<(GROUP_FRAME+category-1) : children;
    }
    return mask ? mask : 1U<<ABOUT_MODULE;
}
static const u8 browse_order[]={PLUGIN_IMAGE_CONTROL,GROUP_FRAME,PLUGIN_FRAMING,PLUGIN_GUIDES,GROUP_TONE,PLUGIN_BLACK_FLOOR,PLUGIN_HIGHLIGHT_ROLLOFF,PLUGIN_SPLIT_TONING,PLUGIN_COLOUR_RANGE,PLUGIN_COLOUR_SATURATION,PLUGIN_COLOUR_HUE,PLUGIN_COLOUR_BRIGHTNESS,GROUP_TEXTURE,PLUGIN_GRAIN,PLUGIN_BLOOM,PLUGIN_HALATION,PLUGIN_DATE_STAMP,PLUGIN_SOFT_SHORTCUT,ABOUT_MODULE};
static u32 root_module(volatile u32 *h,u32 row) {
    u32 mask=root_modules(h),i,last=ABOUT_MODULE;
    for(i=0;i<ARRAY_COUNT(browse_order);++i) {
        u32 module=browse_order[i];if(!(mask>>module&1)) continue;
        if(!row--) return module;last=module;
    }
    return last;
}
static u32 root_row(volatile u32 *h,u32 module) {
    u32 mask=root_modules(h),row=0,i;
    for(i=0;i<ARRAY_COUNT(browse_order);++i) {
        u32 item=browse_order[i];if(!(mask>>item&1)) continue;
        if(item==module) return row;++row;
    }
    return row;
}
static u32 root_count(volatile u32 *h) { return root_row(h,ARRAY_COUNT(modules)); }
static u32 current_index(volatile u32 *h) {
    return h[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT ? root_module(h,h[GRKIT_HOST_MENU_CURSOR/4]) : h[GRKIT_HOST_MODULE/4];
}
static const Module *current_module(volatile u32 *h) { return &modules[current_index(h)]; }
static u32 count(volatile u32 *h) {
    if(h[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT) return root_count(h);
    return current_module(h)->rows(h);
}
static void highlight(volatile u32 *h) {
    const Module *module;
    if(h[GRKIT_HOST_MENU_LEVEL/4]!=MENU_MODULE || h[GRKIT_HOST_MODULE/4]>=PLUGIN_COUNT) return;
    module=&modules[h[GRKIT_HOST_MODULE/4]];
    if(h[GRKIT_HOST_MENU_CURSOR/4]>=module->rows(h)) {h[GRKIT_HOST_PREVIEW/4]=0;return;}
    if(module->highlight) module->highlight(h);
}
static u32 parameters_row(volatile u32 *h) {
    const Module *module;
    if(h[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT) return current_index(h)==PLUGIN_IMAGE_CONTROL && image_active_row(h)!=0;
    if(h[GRKIT_HOST_MENU_LEVEL/4]!=MENU_MODULE || h[GRKIT_HOST_MODULE/4]>=PLUGIN_COUNT) return 0;
    module=&modules[h[GRKIT_HOST_MODULE/4]];return h[GRKIT_HOST_MENU_CURSOR/4]<module->rows(h) && module->parameters && module->parameters(h);
}
static void menu_open(volatile u32 *h,u32 module) {
    if(module==PLUGIN_COLOUR_RANGE || module==PLUGIN_DATE_STAMP || module==PLUGIN_SPLIT_TONING) h[GRKIT_HOST_COLOUR_MENU/4]=0;
    h[GRKIT_HOST_MODULE/4]=module;h[GRKIT_HOST_MENU_LEVEL/4]=MENU_MODULE;
    h[GRKIT_HOST_MENU_CURSOR/4]=modules[module].committed(h);
}
static void menu_start(volatile u32 *h) {
    h[GRKIT_HOST_MENU_SECTION/4]=0;h[GRKIT_HOST_COLOUR_MENU/4]=0;
    h[GRKIT_HOST_MENU_LEVEL/4]=MENU_ROOT;h[GRKIT_HOST_MENU_CURSOR/4]=0;h[GRKIT_HOST_MODULE/4]=0;
}
/* Reopen the same section on the previously used control. */
static void menu_normalize(volatile u32 *h);
static void menu_reopen(volatile u32 *h) {
    u32 module=h[GRKIT_HOST_MODULE/4];
    if((module==PLUGIN_COLOUR_SATURATION || module==PLUGIN_COLOUR_HUE || module==PLUGIN_COLOUR_BRIGHTNESS) && (h[GRKIT_HOST_PLUGINS/4]>>PLUGIN_COLOUR_RANGE&1U)) module=PLUGIN_COLOUR_RANGE;
    h[GRKIT_HOST_MODULE/4]=module;
    h[GRKIT_HOST_COLOUR_MENU/4]=0;
    h[GRKIT_HOST_MENU_LEVEL/4]=MENU_ROOT;
    h[GRKIT_HOST_MENU_CURSOR/4]=root_row(h,module);
    menu_normalize(h);
}
static void menu_normalize(volatile u32 *h) {
    u32 module=h[GRKIT_HOST_MODULE/4];
    if(h[GRKIT_HOST_MENU_SECTION/4]>3 || h[GRKIT_HOST_MENU_LEVEL/4]<MENU_ROOT || h[GRKIT_HOST_MENU_LEVEL/4]>MENU_MODULE || module>=ARRAY_COUNT(modules) ||
       (h[GRKIT_HOST_MENU_LEVEL/4]==MENU_MODULE && (module>=PLUGIN_COUNT || !(visible_modules(h)>>module&1) || !modules[module].rows(h)))) menu_start(h);
    if(h[GRKIT_HOST_MENU_CURSOR/4]>=count(h)) h[GRKIT_HOST_MENU_CURSOR/4]=0;
}
static void menu_confirm(volatile u32 *h) {
    menu_normalize(h);
    if(h[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT) {
        u32 module=current_index(h);
        if(module>=GROUP_FRAME && module<=GROUP_TEXTURE) {
            h[GRKIT_HOST_MENU_SECTION/4]=module-GROUP_FRAME+1;h[GRKIT_HOST_MENU_CURSOR/4]=0;return;
        }
        if(modules[module].rows(h)) menu_open(h,module);
    } else current_module(h)->apply(h,h[GRKIT_HOST_MENU_CURSOR/4]);
}
/* Right opens a child page. Values require OK; Left returns to the parent. */
static u32 menu_right(volatile u32 *h) {
    u32 module,row;
    menu_normalize(h);module=current_index(h);row=h[GRKIT_HOST_MENU_CURSOR/4];
    if(h[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT) {
        if((!modules[module].rows(h) && !(module>=GROUP_FRAME && module<=GROUP_TEXTURE))) return 0;
    } else if(module==PLUGIN_DATE_STAMP && !stamp_stage(h) && row<2) { /* Format or Appearance. */ }
    else if(module==PLUGIN_SPLIT_TONING && !split_stage(h) && row<2) { }
    else if(module==PLUGIN_COLOUR_RANGE && row<modules[module].rows(h)) { /* Range or parameter. */ }
    else return 0;
    menu_confirm(h);return 1;
}
static u32 menu_back(volatile u32 *h) {
    u32 module;
    h[GRKIT_HOST_PREVIEW/4]=0;menu_normalize(h);
    if(h[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT) {
        u32 section=h[GRKIT_HOST_MENU_SECTION/4];if(!section) return 0;
        h[GRKIT_HOST_MENU_SECTION/4]=0;h[GRKIT_HOST_MENU_CURSOR/4]=root_row(h,GROUP_FRAME+section-1);return 1;
    }
    module=h[GRKIT_HOST_MODULE/4];
    if(module==PLUGIN_SPLIT_TONING && split_stage(h)) {h[GRKIT_HOST_MENU_CURSOR/4]=split_stage(h)-3;h[GRKIT_HOST_COLOUR_MENU/4]=0;return 1;}
    if(module==PLUGIN_DATE_STAMP && stamp_stage(h)) {h[GRKIT_HOST_MENU_CURSOR/4]=stamp_stage(h)-1;h[GRKIT_HOST_COLOUR_MENU/4]=0;return 1;}
    if(h[GRKIT_HOST_COLOUR_MENU/4]) {
        if(module==PLUGIN_COLOUR_SATURATION || module==PLUGIN_COLOUR_HUE || module==PLUGIN_COLOUR_BRIGHTNESS) {
            u32 row=0;while(row<3 && colour_child(h,row)!=module) ++row;
            h[GRKIT_HOST_MODULE/4]=PLUGIN_COLOUR_RANGE;h[GRKIT_HOST_MENU_CURSOR/4]=row;return 1;
        }
        if(module==PLUGIN_COLOUR_RANGE) { h[GRKIT_HOST_COLOUR_MENU/4]=0;h[GRKIT_HOST_MENU_CURSOR/4]=colour_range_value(h);return 1; }
    }
    h[GRKIT_HOST_MENU_LEVEL/4]=MENU_ROOT;h[GRKIT_HOST_MENU_CURSOR/4]=root_row(h,module);return 1;
}
/* Titles show the parent, and the colour being edited survives down to the
   value page. Two bounded buffers belong to the existing text allocation. */
static const u16 *control_name(volatile u32 *h,u32 module) {
    if(module==PLUGIN_IMAGE_CONTROL) return text(h,TEXT_MENU_LOOKS);
    if(module==PLUGIN_FRAMING) return text(h,TEXT_MENU_CROP);
    if(module==PLUGIN_COLOUR_RANGE) return text(h,TEXT_MENU_COLOUR);
    return modules[module].title(h);
}
static u32 append_text(u16 *out,u32 at,const u16 *in) {
    while(*in && at<GRKIT_SUMMARY_UNITS-1) out[at++]=*in++;
    out[at]=0;return at;
}
static u16 *menu_buffer(volatile u32 *h,u32 index) { return (u16 *)MENU_TEXT(GRKIT_TEXT(h)+GRKIT_TEXT_MENU_PATH+index*GRKIT_SUMMARY_UNITS*2); }
static u32 path_separator(u16 *out,u32 at) {
    static const u16 slash[]={' ','/',' ',0};return append_text(out,at,slash);
}
static const u16 *menu_heading(volatile u32 *h) {
    u32 module=current_index(h),section=h[GRKIT_HOST_MENU_SECTION/4],at=0;
    u16 *out=menu_buffer(h,0);
    if(h[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT) {
        if(!section) return plugins;
        at=append_text(out,0,plugins);at=path_separator(out,at);
        append_text(out,at,text(h,TEXT_MENU_GROUP+section-1));return out;
    }
    if(module==PLUGIN_SPLIT_TONING && split_stage(h)) {at=append_text(out,0,text(h,TEXT_SPLIT_TONING));at=path_separator(out,at);append_text(out,at,split_toning_title(h));return out;}
    if(module==PLUGIN_DATE_STAMP && stamp_stage(h)) {at=append_text(out,0,text(h,TEXT_DATE_STAMP));at=path_separator(out,at);append_text(out,at,stamp_title(h));return out;}
    if(h[GRKIT_HOST_COLOUR_MENU/4] && (module==PLUGIN_COLOUR_RANGE || module==PLUGIN_COLOUR_SATURATION || module==PLUGIN_COLOUR_HUE || module==PLUGIN_COLOUR_BRIGHTNESS)) {
        u32 range=colour_range_value(h);
        if(module==PLUGIN_COLOUR_RANGE) at=append_text(out,0,text(h,TEXT_MENU_COLOUR));
        else if(range) at=append_text(out,0,text(h,TEXT_COLOUR_RANGE_NAME+range-1));
        at=path_separator(out,at);
        append_text(out,at,module==PLUGIN_COLOUR_RANGE && range ? text(h,TEXT_COLOUR_RANGE_NAME+range-1) : control_name(h,module));return out;
    }
    if(section) {at=append_text(out,0,text(h,TEXT_MENU_PARENT+section-1));at=path_separator(out,at);}
    append_text(out,at,control_name(h,module));return out;
}
static const u16 *control_summary(volatile u32 *h,u32 module) {
    if(module==PLUGIN_SPLIT_TONING) {
        u32 preset=split_preset(h[GRKIT_HOST_SPLIT_TONE/4]);u16 *out=menu_buffer(h,1);
        u32 at=append_text(out,0,text(h,TEXT_SPLIT_PRESET+split_menu_row(h)));
        if(preset) {at=path_separator(out,at);append_text(out,at,text(h,split_percent(h) ? TEXT_STRENGTH_LEVEL+split_percent(h)/5-1 : TEXT_OFF));}
        return out;
    }
    if(module==PLUGIN_DATE_STAMP) {
        u16 *out=menu_buffer(h,1);u32 at=append_text(out,0,text(h,TEXT_STAMP_FORMAT+stamp_format(h)));
        if(stamp_format(h)) {at=path_separator(out,at);append_text(out,at,text(h,TEXT_STAMP_LOOK+stamp_look(h)));}
        return out;
    }
    static const u16 colon[]={':',' ',0};
    u16 *out=menu_buffer(h,1);u32 at;
    at=append_text(out,0,control_name(h,module));at=append_text(out,at,colon);
    append_text(out,at,modules[module].label(h,modules[module].committed(h)));return out;
}
static const u16 *menu_detail(volatile u32 *h) {
    u32 module=current_index(h),cursor=h[GRKIT_HOST_MENU_CURSOR/4];
    if(h[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT) return module<PLUGIN_COUNT ? control_summary(h,module) : modules[module].summary(h);
    if(module==PLUGIN_COLOUR_RANGE && h[GRKIT_HOST_COLOUR_MENU/4] ) return control_summary(h,colour_child(h,cursor));
    return modules[module].label(h,cursor);
}
static u32 menu_row_icon(volatile u32 *h,u32 row) {
    u32 owner=h[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT ? root_module(h,row) : h[GRKIT_HOST_MODULE/4];
    if(owner==ABOUT_MODULE) return GRKIT_ICON_GRKIT;
    if(owner==PLUGIN_SOFT_SHORTCUT) return FN_GUIDE_ICON;
    if(owner==PLUGIN_SPLIT_TONING && h[GRKIT_HOST_MENU_LEVEL/4]==MENU_MODULE && (split_stage(h)==4 || (!split_stage(h) && row==1))) return GRKIT_ICON_GRKIT;
    /* Category symbols are separate from every child control. */
    if(owner>=GROUP_FRAME && owner<=GROUP_TEXTURE) return module_icon(PLUGIN_IMAGE_CONTROL,GRKIT_ICON_TILE(owner==GROUP_FRAME ? 1 : owner==GROUP_TONE ? 2 : 3));
    if(owner==PLUGIN_COLOUR_RANGE && h[GRKIT_HOST_COLOUR_MENU/4]) owner=colour_child(h,row);
    if(h[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT || (h[GRKIT_HOST_MODULE/4]==PLUGIN_COLOUR_RANGE && h[GRKIT_HOST_COLOUR_MENU/4])) return module_icon(owner,GRKIT_ICON_TILE(0));
    return module_icon(owner,modules[owner].icon(h,row));
}
#endif
