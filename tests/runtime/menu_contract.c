#include <assert.h>
#include <string.h>
/* The build's label table, which the Rust test writes for this harness. */
#include "menu_text.h"
static unsigned framing_requests;
#define MENU_FRAMING_REQUEST() (++framing_requests)
/* Camera addresses of the menu text map to text_arena, relative to the text
   start of the header, so the count of records can change during the test. */
static const unsigned short *text_at(unsigned int address);
#define MENU_TEXT(address) text_at(address)
static unsigned test_language;
#define MENU_LANGUAGE() test_language
#include "menu.c"
#define ALL_PLUGINS ((1U << PLUGIN_COUNT) - 1)
/* The global icon layout of the plugin tiles, from PLUGIN_ICON. */
#define FRAMING_RATIO_ICON(ratio) (PLUGIN_ICON + IMAGE_CONTROL_TILES)
#define GUIDES_ICON (PLUGIN_ICON + IMAGE_CONTROL_TILES + FRAMING_TILES)
#define GUIDE_STYLE_ICON(style) GUIDES_ICON
#define STAMP_ICON (GUIDES_ICON + GUIDES_TILES)
#define STAMP_STYLE_ICON(style) STAMP_ICON
#define GRAIN_ICON (STAMP_ICON + DATE_STAMP_TILES)
#define GRAIN_LEVEL_ICON(level) GRAIN_ICON
/* A page icon as its global ID. */
#define PAGE_ICON(module, row) module_icon((module), modules[(module)].icon(test_header, (row)))
#define CURRENT_ICON(row) module_icon(current_index(test_header), current_module(test_header)->icon(test_header, (row)))

static volatile u32 test_header[GRKIT_HOST_HEADER_SIZE/4];
static u16 text_arena[GRKIT_TEXT_SIZE / 2];
static const u16 *text_at(u32 address)
{
    u32 offset = address - GRKIT_TEXT(test_header);
    assert(offset < GRKIT_TEXT_SIZE && !(offset & 1));
    return text_arena + offset / 2;
}
/* A menu string equals an ASCII string. */
static int same(const u16 *actual, const char *expected)
{
    u32 i = 0;
    for (; expected[i]; ++i) if (actual[i] != (u8)expected[i]) return 0;
    return actual[i] == 0;
}
static const char *const ratio_names[] = {"Native", "6x7 (7:6)", "5:4", "FP100C", "4:3", "16:9"};
static const char *const guide_names[] = {"Off", "Thirds", "Center", "Golden Ratio", "Diagonals", "Thirds + Center"};
static const char *const stamp_names[] = {"Off", "'YY M D", "'YY M D H:MM", "M D 'YY", "D M 'YY"};
static const char *const grain_names[] = {"Off","5%","10%","15%","20%","25%","30%","35%","40%","45%","50%","55%","60%","65%","70%","75%","80%","85%","90%","95%","100%"};
static const unsigned char expected_grain_codes[]={0,4,5,1,6,7,2,8,9,10,11,3};
static u32 native_changes;
static u32 native_events;
volatile u32 *menu_header(void) { return test_header; }
u32 menu_event_original(u32 controller, const u16 *event)
{ (void)controller; (void)event; ++native_events; return 42; }
#ifdef FW_SOFT_OPEN_ADJ
u32 menu_live_event_original(u32 controller,const u16 *event) { (void)controller; (void)event; ++native_events; return 43; }
u32 menu_live_start_original(u32 controller,u32 identity) { (void)controller; return identity; }
#endif
void menu_exit_original(u32 controller) { (void)controller; }
u32 menu_destroy_original(u32 controller) { return controller; }
void menu_refresh_original(u32 controller) { (void)controller; }
void menu_changed_original(u32 controller) { (void)controller; ++native_changes; }

#ifdef FW_SOFT_OPEN_ADJ
static void test_shortcut_state(void) {
    volatile u32 h[GRKIT_HOST_HEADER_SIZE/4]={0};
    h[GRKIT_HOST_PLUGINS/4]=1U<<PLUGIN_SOFT_SHORTCUT;
    assert(!shortcut_request(h,0,EV_FN,2,0)); /* Off is exact pass-through. */
    h[GRKIT_HOST_SHORTCUT/4]=1;
    assert(!shortcut_request(h,1,EV_FN,2,0)); /* Movie excluded. */
    assert(!shortcut_request(h,0,EV_CONFIRM,2,0));
    assert(!shortcut_request(h,0,EV_FN,1,0));
    assert(!shortcut_request(h,0,EV_FN,2,1));
    h[GRKIT_HOST_PLUGINS/4]=0;
    assert(!shortcut_request(h,0,EV_FN,2,0)); /* Omitted means no requests. */
    h[GRKIT_HOST_PLUGINS/4]=1U<<PLUGIN_SOFT_SHORTCUT;
    assert(shortcut_request(h,0,EV_FN,2,0));
    assert(h[GRKIT_HOST_SHORTCUT_REQUESTS/4]==1 && h[GRKIT_HOST_SHORTCUT_STATE/4]==3);
    assert(shortcut_consume(h,EV_FN,2,0)); /* No second transition while pending. */
    assert(h[GRKIT_HOST_SHORTCUT_REQUESTS/4]==1);
    assert(shortcut_consume(h,EV_FN,2,1));
    assert(!shortcut_consume(h,EV_CONFIRM,2,0));
    assert(shortcut_ready(h,0)); assert(!shortcut_ready(h,0)); /* Exactly one entry. */
    h[GRKIT_HOST_SHORTCUT/4]=0;
    assert(shortcut_consume(h,EV_FN,1,0)); /* Consume release after disable. */
    assert(!shortcut_consume(h,EV_FN,1,0));
    h[GRKIT_HOST_SHORTCUT/4]=1;
    assert(shortcut_request(h,0,EV_FN,2,0));
    assert(shortcut_consume(h,EV_FN,1,0)); /* Early release retains pending entry. */
    assert(shortcut_ready(h,0)); assert(!h[GRKIT_HOST_SHORTCUT_STATE/4]);
    assert(shortcut_request(h,0,EV_FN,2,0));
    assert(!shortcut_ready(h,1)); /* A movie refresh cancels pending. */
    assert(!shortcut_consume(h,EV_FN,2,0)); /* A lost release cannot lock Fn. */
    assert(!h[GRKIT_HOST_SHORTCUT_STATE/4]);
    soft_shortcut_apply(h,1); assert(soft_shortcut_value(h)==1);
    soft_shortcut_apply(h,9); assert(!soft_shortcut_value(h));
}
#endif

static void browse_to(u32 module) {
    menu_start(test_header);
    if(!(root_modules(test_header)>>module&1)) {
        for(u32 section=1;section<=3;++section) if(category_mask(test_header,section)>>module&1) {
            test_header[GRKIT_HOST_MENU_CURSOR/4]=root_row(test_header,GROUP_FRAME+section-1);menu_confirm(test_header);break;
        }
    }
    test_header[GRKIT_HOST_MENU_CURSOR/4]=root_row(test_header,module);
    assert(current_index(test_header)==module);
}

/* Every possible builder selection must leave every included control
   reachable exactly once, with arrow navigation to its parent and no Back rows. */
static void test_menu_packages(void) {
    u32 saved[GRKIT_HOST_HEADER_SIZE/4];memcpy(saved,(const void *)test_header,sizeof(saved));
    for(u32 effects=0;effects<2;++effects) for(u32 mask=0;mask<=ALL_PLUGINS;++mask) {
        memset((void *)test_header,0,sizeof(test_header));
        test_header[GRKIT_HOST_PLUGINS/4]=mask;test_header[GRKIT_HOST_COUNT/4]=effects ? 16 : 0;
        test_header[GRKIT_HOST_RECORDS/4]=0x59005000;menu_start(test_header);
        u32 expected=visible_modules(test_header)&ALL_PLUGINS,seen=0;
        if(expected>>PLUGIN_COLOUR_RANGE&1) expected&=~((1U<<PLUGIN_COLOUR_SATURATION)|(1U<<PLUGIN_COLOUR_HUE)|(1U<<PLUGIN_COLOUR_BRIGHTNESS));
        u32 roots=root_count(test_header);assert(roots && roots<=7);
        assert(root_module(test_header,roots-1)!=ABOUT_MODULE || !expected);
        for(u32 row=0;row<roots;++row) {
            test_header[GRKIT_HOST_MENU_CURSOR/4]=row;
            u32 item=current_index(test_header),group=item>=GROUP_FRAME && item<=GROUP_TEXTURE;
            if(item!=ABOUT_MODULE) assert(menu_row_icon(test_header,row)!=GRKIT_ICON_GRKIT);
            if(item==ABOUT_MODULE) continue;
            if(group) {
                assert(menu_right(test_header));
                assert(test_header[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT && root_count(test_header)>=2);
                assert(root_module(test_header,0)<PLUGIN_COUNT);
            }
            u32 first=group ? 0 : row,last=group ? root_count(test_header) : row+1;
            for(u32 child=first;child<last;++child) {
                test_header[GRKIT_HOST_MENU_CURSOR/4]=child;
                u32 module=current_index(test_header);assert(module<PLUGIN_COUNT && !(seen>>module&1));seen|=1U<<module;
                assert(menu_right(test_header));assert(test_header[GRKIT_HOST_MENU_LEVEL/4]==MENU_MODULE);
                if(module==PLUGIN_COLOUR_RANGE) {
                    test_header[GRKIT_HOST_MENU_CURSOR/4]=1;assert(menu_right(test_header));
                    if(test_header[GRKIT_HOST_COLOUR_MENU/4]) {
                        u32 controls=count(test_header),visited=0;
                        assert(controls>=1 && controls<=3);
                        for(u32 option=0;option<controls;++option) {
                            u32 parameter=colour_child(test_header,option);
                            test_header[GRKIT_HOST_MENU_CURSOR/4]=option;assert(menu_right(test_header));
                            assert(current_index(test_header)==parameter && !(visited>>parameter&1));visited|=1U<<parameter;
                            assert(same(menu_heading(test_header),parameter==PLUGIN_COLOUR_SATURATION ? "Reds / Saturation" : parameter==PLUGIN_COLOUR_HUE ? "Reds / Hue" : "Reds / Brightness"));
                            assert(menu_back(test_header));
                            assert(current_index(test_header)==PLUGIN_COLOUR_RANGE && test_header[GRKIT_HOST_MENU_CURSOR/4]==option);
                        }
                        u32 wanted=mask&((1U<<PLUGIN_COLOUR_SATURATION)|(1U<<PLUGIN_COLOUR_HUE)|(1U<<PLUGIN_COLOUR_BRIGHTNESS));
                        assert(visited==wanted);
                        assert(menu_back(test_header));
                        assert(!test_header[GRKIT_HOST_COLOUR_MENU/4] && test_header[GRKIT_HOST_MENU_CURSOR/4]==1);
                    }
                }
                if(module==PLUGIN_SPLIT_TONING) {
                    assert(menu_right(test_header) && split_stage(test_header)==3);
                    for(u32 preset=0;preset<9;++preset) {test_header[GRKIT_HOST_MENU_CURSOR/4]=preset;menu_confirm(test_header);assert(split_menu_row(test_header)==preset);}
                    assert(menu_back(test_header));test_header[GRKIT_HOST_MENU_CURSOR/4]=1;
                    assert(menu_right(test_header) && split_stage(test_header)==4);
                    for(u32 strength=0;strength<21;++strength) {test_header[GRKIT_HOST_MENU_CURSOR/4]=strength;menu_confirm(test_header);assert(split_percent(test_header)==strength*5);}
                    assert(menu_back(test_header));
                }
                if(module==PLUGIN_DATE_STAMP) {
                    assert(menu_row_icon(test_header,0)!=menu_row_icon(test_header,1));
                    assert(menu_right(test_header) && stamp_stage(test_header)==1);
                    u32 before=test_header[GRKIT_HOST_STAMP/4];assert(!menu_right(test_header) && test_header[GRKIT_HOST_STAMP/4]==before);
                    assert(menu_back(test_header) && stamp_stage(test_header)==0);
                    test_header[GRKIT_HOST_MENU_CURSOR/4]=1;assert(menu_right(test_header) && stamp_stage(test_header)==2);
                    assert(!menu_right(test_header));assert(menu_back(test_header));
                }
                u32 rows=modules[module].rows(test_header);assert(count(test_header)==rows);
                for(u32 value=0;value<rows;++value) {
                    test_header[GRKIT_HOST_MENU_CURSOR/4]=value;
                    if(module!=PLUGIN_IMAGE_CONTROL || !value) assert(!same(menu_detail(test_header),"Back"));
                }
                assert(menu_back(test_header));
                assert(test_header[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT && test_header[GRKIT_HOST_MENU_CURSOR/4]==child);
            }
            if(group) {
                assert(menu_back(test_header));
                assert(!test_header[GRKIT_HOST_MENU_SECTION/4] && test_header[GRKIT_HOST_MENU_CURSOR/4]==row);
            }
        }
        assert(seen==expected);
    }
    memcpy((void *)test_header,saved,sizeof(saved));
}

static void test_adj_slots(void) {
    for(u32 mask=0;mask<32;++mask) for(u32 slot=0;slot<5;++slot) {
        u32 kinds[5],i;for(i=0;i<5;++i) kinds[i]=mask>>i&1 ? 1 : 0;kinds[slot]=ADJ_IC_SLOT_TYPE;
        SoftAdjSlots p=adj_slots(kinds,ADJ_IC_SLOT_TYPE,0);
        assert(p.soft==slot && (p.mask>>slot&1));
        u32 rank=0;for(i=0;i<5;++i) {assert(adj_slot_rank(p,i)==rank);if(kinds[i]) ++rank;}
        u32 previous=adj_slot_previous(p);
        if(p.mask==(1U<<slot)) assert(previous==5);else assert(previous<5 && previous!=slot && (p.mask>>previous&1));
        assert(adj_slots(kinds,ADJ_IC_SLOT_TYPE,1).soft==5);
    }
    {u32 empty[5]={0};assert(adj_slots(empty,ADJ_IC_SLOT_TYPE,0).soft==5);}
    {u32 duplicate[5]={12,1,12,0,0};assert(adj_slots(duplicate,12,0).soft==0);}
}

int main(void)
{
    volatile u32 slot[2];
#ifdef FW_SOFT_OPEN_ADJ
    test_shortcut_state();
#endif
    assert(sizeof(menu_text_table) <= GRKIT_TEXT_SUMMARY);
    memcpy(text_arena, menu_text_table, sizeof(menu_text_table));
    descriptor(slot, plugins);
    assert(slot[0] == 0x105);
    test_header[GRKIT_HOST_COUNT / 4] = 5;
    /* Every plugin is installed (PLUGINS.TXT is missing). */
    test_header[GRKIT_HOST_PLUGINS / 4] = ALL_PLUGINS;
    test_header[GRKIT_HOST_OFF_ICON / 4] = 0x59001000;
    test_header[GRKIT_HOST_PLUGINS_ICON / 4] = 0x59003000;
    test_header[GRKIT_HOST_RECORDS / 4] = 0x59005000;
    test_menu_packages();
    test_adj_slots();
    assert(selected_icon(test_header) == 0x7fff);
    descriptor(slot, modules[0].summary(test_header));
    assert(same(modules[0].summary(test_header), "Image Control: Off") && slot[0] == 0x113);
    /* Each language reads its own strings, and an unknown language reads English. */
    for (u32 language = 0; language < GRKIT_TEXT_LANGUAGES + 2; ++language) {
        u32 table = language < GRKIT_TEXT_LANGUAGES ? language : 0;
        test_language = language;
        for (u32 id = 0; id < GRKIT_TEXT_STRINGS; ++id)
            assert(text(test_header, id) == text_arena + text_arena[table * GRKIT_TEXT_STRINGS + id] / 2);
    }
    test_language = 2;
    assert(!same(text(test_header, TEXT_FRAMING), "Framing"));
    /* The text hooks select the font of the text language. */
    assert(menu_table_index() == 18);
    test_language = 1;
    assert(menu_table_index() == 19);
    test_language = 3;
    assert(menu_table_index() == 0);
    test_language = 7;
    assert(menu_table_index() == 1);
    test_language = 0;
    assert(menu_table_index() == 1);
    assert(menu_viewfinder_icon_select(0x166, 0) == 0x166);
    assert(menu_viewfinder_icon_select(0, 0) == 0);
    test_header[GRKIT_HOST_ACTIVE / 4] = 0x59005000 + 3 * GRKIT_RECORD_SIZE;
    assert(selected_icon(test_header) == 0x8003);
    /* The summary of record 3 is its slot in the menu language. */
    for (u32 language = 0; language < GRKIT_TEXT_LANGUAGES; ++language) {
        test_language = language;
        assert(modules[0].summary(test_header) ==
               text_arena + (GRKIT_TEXT_SUMMARY + (language * GRKIT_TEXT_RECORDS + 3) * GRKIT_SUMMARY_UNITS * 2) / 2);
    }
    test_language = 0;
    /* x * 2260 >> 16 equals x / 29 for each record offset / 64. */
    for (u32 index = 0; index < GRKIT_TEXT_RECORDS; ++index)
        assert(((index * GRKIT_RECORD_SIZE) >> 6) * 2260U >> 16 == index);
    assert(menu_viewfinder_icon_select(0x166, 0) == 0x8003);
    assert(menu_viewfinder_icon_select(0x166, 1) == 0x166);
    test_header[GRKIT_HOST_ACTIVE / 4] += 4;
    assert(menu_viewfinder_icon_select(0x167, 0) == 0x167);
    test_header[GRKIT_HOST_ACTIVE / 4] = 0;
    /* Fn and its hint apply to effect rows of Image Control only. */
    test_header[GRKIT_HOST_MENU_LEVEL / 4] = MENU_MODULE;
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 0;
    assert(!parameters_row(test_header));
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 2;
    assert(parameters_row(test_header));
    test_header[GRKIT_HOST_MODULE / 4] = 1;
    assert(!parameters_row(test_header));
    test_header[GRKIT_HOST_MODULE / 4] = 0;
    test_header[GRKIT_HOST_MENU_LEVEL / 4] = MENU_ROOT;
    assert(!parameters_row(test_header));
    test_header[GRKIT_HOST_MENU_LEVEL / 4] = 0;
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 0;
    /* Fn works on the Looks root for its committed record, never on Off. */
    menu_start(test_header);test_header[GRKIT_HOST_ACTIVE/4]=test_header[GRKIT_HOST_RECORDS/4];
    assert(parameters_row(test_header));test_header[GRKIT_HOST_ACTIVE/4]=0;assert(!parameters_row(test_header));
    assert(exit_return(RETURN_OPENING|RETURN_ROOT,1)==(RETURN_PARAMETERS|RETURN_ROOT));
    menu_start(test_header);
    assert(menu_press(2, 0));
    assert(!menu_press(1, 0));
    assert(!menu_press(2, 1));
    assert(!menu_press(2, 100));
    assert(back_event(test_header, 5, 1) == 1);
    assert(back_event(test_header, 5, 1) == 2);
    assert(back_event(test_header, 0x19d, 1) == 0);
    assert(back_event(test_header, 0xd8, 0) == 2);
    assert(back_event(test_header, 0xd8, 0) == 0);
    assert(back_event(test_header, 5, 0) == 0);
    assert(back_event(test_header, 5, 1) == 1);
    assert(back_event(test_header, 8, 1) == 0);
    assert(back_event(test_header, 0xd8, 1) == 0);
    /* List restoration changes level to native before rebuilding the rows. */
    test_header[GRKIT_HOST_ACTIVE / 4] = 0x59005000;
    test_header[GRKIT_HOST_MENU_CONTROLLER / 4] = 0x1000;
    test_header[GRKIT_HOST_MENU_REBUILDING / 4] = 1;
    menu_changed(0x1000);
    assert(native_changes == 0);
    assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0x59005000);
    test_header[GRKIT_HOST_MENU_LEVEL / 4] = 1;
    menu_changed(0x1000);
    assert(native_changes == 0);
    test_header[GRKIT_HOST_MENU_LEVEL / 4] = 0;
    test_header[GRKIT_HOST_MENU_REBUILDING / 4] = 0;
    menu_changed(0x1000);
    assert(native_changes == 1);
    {
        const u16 confirm[] = {0xd8};
        test_header[GRKIT_HOST_MENU_LEVEL / 4] = 0;
        test_header[GRKIT_HOST_BACK_PENDING / 4] = 1;
        assert(menu_event(0x1000, confirm) == 0);
        assert(native_events == 0);
        assert(test_header[GRKIT_HOST_BACK_PENDING / 4] == 0);
        assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0x59005000);
    }
    /* The event ring distinguishes MENU's confirm from OK and ADJ press confirms. */
    test_header[GRKIT_HOST_MENU_EVENTS / 4] = 18;
    test_header[GRKIT_HOST_MENU_EVENT_RING / 4] = 5 | (2 << 16);
    test_header[GRKIT_HOST_MENU_EVENT_RING / 4 + 1] = 0xd8;
    assert(after_menu_switch(test_header));
    test_header[GRKIT_HOST_MENU_EVENT_RING / 4] = 8;
    assert(!after_menu_switch(test_header));
    test_header[GRKIT_HOST_MENU_EVENT_RING / 4] = 0x1e;
    assert(!after_menu_switch(test_header));
    test_header[GRKIT_HOST_MENU_EVENTS / 4] = 0;
    test_header[GRKIT_HOST_MENU_EVENT_RING / 4] = test_header[GRKIT_HOST_MENU_EVENT_RING / 4 + 1] = 0;
    /* Playback matches only extracted non-native tags with the private code bit. */
    {
        u32 count = test_header[GRKIT_HOST_COUNT / 4];
        assert(playback_icon(test_header, 0) == 0);
        assert(playback_icon(test_header, 5) == 0);
        assert(playback_icon(test_header, 0x10000 | 0x7eff) == 0);
        assert(playback_icon(test_header, 0x20000 | 0x7f00) == 0);
        test_header[GRKIT_HOST_COUNT / 4] = 0;
        assert(playback_icon(test_header, 0x10000 | 0x7f00) == 0);
        test_header[GRKIT_HOST_COUNT / 4] = count;
    }
    assert(!native_action_release(10, 0));
    assert(!native_action_release(0, 1));
    assert(native_action_release(10, 1));
    assert(menu_icon(0x7ffe) == 0x59003000);
    assert(toolbar_x(0) == 415 && toolbar_x(5) == 665);
    assert(menu_icon(0x7fff) == 0x59001000);
    assert(menu_icon(0x166) == 0);
    assert(menu_icon(0x8005) == 0);
    assert(menu_icon(0xffff) == 0);
    assert(wrapped_edge(0xcf, 3, 0, 0) == 2);
    assert(wrapped_edge(0xce, 1, 4, 0) == 1);
    assert(wrapped_edge(0xcf, 1, 2, 0) == 0);
    assert(wrapped_edge(0xce, 3, 2, 0) == 0);
    assert(wrapped_edge(0xcf, 4, 0, 1) == 0);
    assert(wrapped_edge(0x35, 4, 0, 0) == 0);
    assert(advance_from_private(0xcf, 1) == 1);
    assert(advance_from_private(0xce, 1) == 0);
    assert(advance_from_private(0xcf, 2) == 0);
    assert(advance_from_private(0xce, 2) == 1);
    /* The root lists the installed plugins, then About. */
    test_header[GRKIT_HOST_MENU_LEVEL / 4] = 1;
    assert(count(test_header) == 6);
    test_header[GRKIT_HOST_MENU_LEVEL / 4] = 2;
    assert(count(test_header) == 6);
    test_header[GRKIT_HOST_COUNT / 4] = 0;
    assert(count(test_header) == 1);
    /* Registry routing leaves the existing effect record untouched. */
    test_header[GRKIT_HOST_ACTIVE / 4] = 0x59005000;
    test_header[GRKIT_HOST_COUNT / 4] = 5;
    test_header[GRKIT_HOST_TILE_ICON / 4] = 0x59007000;
    /* Framing tiles are contiguous after the module tile. */
    assert(menu_icon(PLUGIN_ICON) == 0x59007000);
    assert(menu_icon(FRAMING_RATIO_ICON(3)) == 0x59007000 + 4 * GRKIT_ICON_STRIDE);
    /* Guides tiles follow the Framing tiles. */
    assert(menu_icon(GUIDES_ICON) == 0x59007000 + 5 * GRKIT_ICON_STRIDE);
    assert(menu_icon(GUIDE_STYLE_ICON(4)) == 0x59007000 + 5 * GRKIT_ICON_STRIDE);
    /* Module tiles fill 0x7fec..0x7ffd; the GRKit tile (0x7ffe) follows them. */
    assert(menu_icon(PLUGIN_ICON - 1) == 0);
    assert(PLUGIN_ICON + PLUGIN_TILES <= 0x7ffe);
    assert(module_icon_count() == PLUGIN_TILES);
    /* Date Stamp tiles follow the Guides tiles. */
    assert(menu_icon(STAMP_ICON) == 0x59007000 + 6 * GRKIT_ICON_STRIDE);
    assert(menu_icon(STAMP_STYLE_ICON(4)) == 0x59007000 + 6 * GRKIT_ICON_STRIDE);
    /* Grain tiles follow the Date Stamp tiles and end below the GRKit tile. */
    assert(menu_icon(GRAIN_ICON) == 0x59007000 + 8 * GRKIT_ICON_STRIDE);
    assert(menu_icon(GRAIN_LEVEL_ICON(3)) == 0x59007000 + 8 * GRKIT_ICON_STRIDE);
    assert(GRAIN_LEVEL_ICON(3) < GRKIT_ICON_GRKIT);
    browse_to(PLUGIN_FRAMING);
    assert(current_module(test_header) == &modules[1]);
    assert(same(current_module(test_header)->summary(test_header), "Crop: Native"));
    assert(PAGE_ICON(1, 1) == FRAMING_RATIO_ICON(0));
    test_header[GRKIT_HOST_FRAME_RATIO / 4] = 2;
    assert(PAGE_ICON(1, 1) == FRAMING_RATIO_ICON(2));
    /* Framing lists the ratios directly and opens at the selected one. */
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_MODULE && test_header[GRKIT_HOST_MENU_CURSOR / 4] == 5);
    assert(count(test_header) == 6);
    assert(same(current_module(test_header)->title(test_header), "Framing"));
    for (u32 row = 0; row < 6; ++row) {
        test_header[GRKIT_HOST_MENU_CURSOR / 4] = row;
        menu_confirm(test_header);
        assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_MODULE);
        assert(test_header[GRKIT_HOST_FRAME_RATIO / 4] == frame_code(row));
        assert(framing_requests == row + 1);
        assert(current_module(test_header)->checked(test_header, row));
        assert(!current_module(test_header)->checked(test_header, (row + 1) % 6));
        assert(same(current_module(test_header)->label(test_header, row), ratio_names[row]));
        assert(CURRENT_ICON(row) == (row ? FRAMING_RATIO_ICON(row) : 0x7fff));
        assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0x59005000);
    }
    /* The Framing page shows its highlighted row on the ADJ canvas. */
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 5;
    frame_preview(test_header, 0x5a000000, 0);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW / 4] == 3);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW_CANVAS / 4] == 0x5a000000);
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 0;
    frame_preview(test_header, 0x5a000000, 0);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW / 4] == 1);
    frame_preview(test_header, 0x5a000000, 1);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW / 4] == 0);
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 4;
    test_header[GRKIT_HOST_FRAME_RATIO / 4] = 3;
    assert(menu_back(test_header));
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_ROOT && test_header[GRKIT_HOST_MENU_CURSOR / 4] == 0);
    /* Other ADJ pages show the committed ratio, here 4:3. */
    frame_preview(test_header, 0x5b000000, 0);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW / 4] == 4);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW_CANVAS / 4] == 0x5b000000);
    /* A committed Off ends the mask but keeps the canvas for one erasing draw. */
    test_header[GRKIT_HOST_FRAME_RATIO / 4] = 0;
    frame_preview(test_header, 0x5b000000, 0);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW / 4] == 0);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW_CANVAS / 4] == 0x5b000000);
    test_header[GRKIT_HOST_FRAME_RATIO / 4] = 3;
    test_header[GRKIT_HOST_FRAME_PREVIEW_CANVAS / 4] = 0;
    assert(same(current_module(test_header)->summary(test_header), "Crop: 4:3"));
    assert(menu_back(test_header));
    assert(!menu_back(test_header));
    /* Guides opens at the committed style and commits only its own setting. */
    browse_to(PLUGIN_GUIDES);
    assert(same(current_module(test_header)->summary(test_header), "Guides: Off"));
    assert(PAGE_ICON(2, 2) == GUIDES_ICON);
    test_header[GRKIT_HOST_GUIDE / 4] = 3;
    assert(PAGE_ICON(2, 2) == GUIDE_STYLE_ICON(3));
    {
        u32 requests = framing_requests;
        menu_confirm(test_header);
        assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_MODULE && test_header[GRKIT_HOST_MODULE / 4] == PLUGIN_GUIDES);
        assert(test_header[GRKIT_HOST_MENU_CURSOR / 4] == 3);
        assert(count(test_header) == 6);
        assert(same(current_module(test_header)->title(test_header), "Guides"));
        for (u32 row = 0; row < 6; ++row) {
            test_header[GRKIT_HOST_MENU_CURSOR / 4] = row;
            menu_confirm(test_header);
            assert(test_header[GRKIT_HOST_GUIDE / 4] == row);
            assert(current_module(test_header)->checked(test_header, row));
            assert(!current_module(test_header)->checked(test_header, (row + 1) % 6));
            assert(same(current_module(test_header)->label(test_header, row), guide_names[row]));
            assert(CURRENT_ICON(row) == (row ? GUIDE_STYLE_ICON(row) : 0x7fff));
            assert(test_header[GRKIT_HOST_FRAME_RATIO / 4] == 3);
            assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0x59005000);
        }
        assert(framing_requests == requests);
    }
    /* The Guides page previews its highlighted style inside the committed ratio. */
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 2;
    frame_preview(test_header, 0x5a000000, 0);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW / 4] == 4);
    assert(test_header[GRKIT_HOST_GUIDE_PREVIEW / 4] == 2);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW_CANVAS / 4] == 0x5a000000);
    /* A highlighted Off with no ratio keeps the canvas for the next row. */
    test_header[GRKIT_HOST_FRAME_RATIO / 4] = 0;
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 0;
    frame_preview(test_header, 0x5a000000, 0);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW / 4] == 1);
    assert(test_header[GRKIT_HOST_GUIDE_PREVIEW / 4] == 0);
    assert(menu_back(test_header));
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_ROOT && test_header[GRKIT_HOST_MENU_CURSOR / 4] == 1);
    /* Other ADJ pages show the committed style, even without a ratio. */
    test_header[GRKIT_HOST_GUIDE / 4] = 4;
    frame_preview(test_header, 0x5b000000, 0);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW / 4] == 1);
    assert(test_header[GRKIT_HOST_GUIDE_PREVIEW / 4] == 4);
    /* The Framing page keeps the committed style with its highlighted ratio. */
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 0;
    menu_confirm(test_header);
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 5;
    frame_preview(test_header, 0x5b000000, 0);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW / 4] == 3);
    assert(test_header[GRKIT_HOST_GUIDE_PREVIEW / 4] == 4);
    /* Committed Off for both ends the drawing. */
    assert(menu_back(test_header));
    test_header[GRKIT_HOST_GUIDE / 4] = 0;
    frame_preview(test_header, 0x5b000000, 0);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW / 4] == 0);
    test_header[GRKIT_HOST_GUIDE / 4] = 999;
    assert(guide_style(test_header) == 0);
    test_header[GRKIT_HOST_GUIDE / 4] = 0;
    test_header[GRKIT_HOST_FRAME_RATIO / 4] = 3;
    test_header[GRKIT_HOST_FRAME_PREVIEW_CANVAS / 4] = 0;
    /* Date Stamp lists its styles, opens at the committed one, and changes nothing else. */
    browse_to(PLUGIN_DATE_STAMP);
    assert(same(current_module(test_header)->summary(test_header), "Date Stamp: Off"));
    assert(PAGE_ICON(3, 3) == STAMP_ICON);
    test_header[GRKIT_HOST_STAMP / 4] = 2;
    assert(PAGE_ICON(3, 3) == STAMP_STYLE_ICON(2));
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_MODULE && test_header[GRKIT_HOST_MODULE / 4] == PLUGIN_DATE_STAMP);
    assert(test_header[GRKIT_HOST_MENU_CURSOR / 4] == 0 && count(test_header) == 2);
    menu_confirm(test_header);
    assert(stamp_stage(test_header)==1 && test_header[GRKIT_HOST_MENU_CURSOR/4]==2 && count(test_header)==STAMP_STYLE_COUNT);
    assert(same(current_module(test_header)->title(test_header), "Format"));
    for (u32 row = 0; row < 5; ++row) {
        test_header[GRKIT_HOST_MENU_CURSOR / 4] = row;
        menu_confirm(test_header);
        assert(test_header[GRKIT_HOST_STAMP / 4] == row);
        assert(current_module(test_header)->checked(test_header, row));
        assert(same(current_module(test_header)->label(test_header, row), stamp_names[row]));
        assert(CURRENT_ICON(row) == (row ? STAMP_STYLE_ICON(row) : 0x7fff));
        assert(test_header[GRKIT_HOST_FRAME_RATIO / 4] == 3 && test_header[GRKIT_HOST_GUIDE / 4] == 0);
        assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0x59005000);
    }
    current_module(test_header)->apply(test_header,5);
    assert(stamp_style(test_header)==7 && same(stamp_label(test_header,5),"D M 'YY H:MM"));
    current_module(test_header)->apply(test_header,6);
    assert(stamp_style(test_header)==8 && same(stamp_label(test_header,6),"YYYY.MM.DD"));
    assert(menu_back(test_header) && stamp_stage(test_header)==0);
    test_header[GRKIT_HOST_MENU_CURSOR/4]=1;menu_confirm(test_header);
    assert(stamp_stage(test_header)==2 && count(test_header)==5);
    for(u32 look=0;look<5;++look) {test_header[GRKIT_HOST_MENU_CURSOR/4]=look;menu_confirm(test_header);assert(stamp_look(test_header)==look && stamp_style(test_header)==8);}
    assert(menu_back(test_header) && stamp_stage(test_header)==0 && test_header[GRKIT_HOST_MENU_CURSOR/4]==1);
    test_header[GRKIT_HOST_MENU_CURSOR/4]=0;menu_confirm(test_header);
    current_module(test_header)->apply(test_header,4);
    assert(stamp_style(test_header)==4 && stamp_look(test_header)==4);
    /* Older Small Mono selections preserve their date and ink after migration. */
    for(u32 legacy=5;legacy<=6;++legacy) {
        test_header[GRKIT_HOST_STAMP/4]=legacy;
        assert(stamp_format(test_header)==6 && stamp_look(test_header)==legacy-2);
        stamp_migrate(test_header);
        assert(stamp_style(test_header)==8 && stamp_format(test_header)==6 && stamp_look(test_header)==legacy-2);
    }
    test_header[GRKIT_HOST_STAMP_LOOK/4]=0;
    current_module(test_header)->apply(test_header,4);
    /* The Date Stamp page keeps the committed mask and guides on the ADJ canvas. */
    frame_preview(test_header, 0x5c000000, 0);
    assert(test_header[GRKIT_HOST_FRAME_PREVIEW / 4] == 4 && test_header[GRKIT_HOST_GUIDE_PREVIEW / 4] == 0);
    assert(menu_back(test_header) && stamp_stage(test_header)==0 && test_header[GRKIT_HOST_MENU_CURSOR/4]==0);
    assert(menu_back(test_header) && test_header[GRKIT_HOST_MENU_CURSOR/4]==4);
    assert(same(current_module(test_header)->summary(test_header), "Date Stamp: D M 'YY"));
    test_header[GRKIT_HOST_STAMP / 4] = 999;
    assert(stamp_style(test_header) == 0);
    test_header[GRKIT_HOST_STAMP / 4] = 0;
    test_header[GRKIT_HOST_FRAME_PREVIEW_CANVAS / 4] = 0;
    /* Grain lists its levels, opens at the committed one, and changes nothing else. */
    browse_to(PLUGIN_GRAIN);
    assert(same(current_module(test_header)->summary(test_header), "Grain: Off"));
    assert(PAGE_ICON(PLUGIN_GRAIN, 4) == GRAIN_ICON);
    test_header[GRKIT_HOST_GRAIN / 4] = 2;
    assert(PAGE_ICON(PLUGIN_GRAIN, 4) == GRAIN_LEVEL_ICON(2));
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_MODULE && test_header[GRKIT_HOST_MODULE / 4] == PLUGIN_GRAIN);
    assert(test_header[GRKIT_HOST_MENU_CURSOR / 4] == 10 && count(test_header) == 21);
    assert(same(current_module(test_header)->title(test_header), "Grain"));
    for (u32 row = 0; row < 21; ++row) {
        test_header[GRKIT_HOST_MENU_CURSOR / 4] = row;
        menu_confirm(test_header);
        assert(test_header[GRKIT_HOST_GRAIN / 4] == strength_code(row));
        assert(current_module(test_header)->checked(test_header, row));
        assert(same(current_module(test_header)->label(test_header, row), grain_names[row]));
        assert(CURRENT_ICON(row) == (row ? GRAIN_LEVEL_ICON(row<=3 ? 1 : row<=6 ? 2 : 3) : 0x7fff));
        assert(test_header[GRKIT_HOST_FRAME_RATIO / 4] == 3 && test_header[GRKIT_HOST_STAMP / 4] == 0);
        assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0x59005000);
    }
    assert(menu_back(test_header) && test_header[GRKIT_HOST_MENU_CURSOR / 4] == 0);
    assert(same(current_module(test_header)->summary(test_header), "Grain: 100%"));
    test_header[GRKIT_HOST_GRAIN / 4] = 999;
    assert(grain_level(test_header) == 0);
    test_header[GRKIT_HOST_GRAIN / 4] = 0;
    test_header[GRKIT_HOST_FRAME_PREVIEW_CANVAS / 4] = 0;
    menu_start(test_header);
    /* Invalid public navigation cursors cannot form an effect pointer. */
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 999;
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_MODULE && test_header[GRKIT_HOST_MODULE / 4] == 0);
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 999;
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0);
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 5;
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0x59005000 + 4 * GRKIT_RECORD_SIZE);
    assert(test_header[GRKIT_HOST_FRAME_RATIO / 4] == 3);
    assert(menu_back(test_header));
    assert(test_header[GRKIT_HOST_MENU_CURSOR / 4] == 0);
    test_header[GRKIT_HOST_MODULE / 4] = 999;
    test_header[GRKIT_HOST_MENU_LEVEL / 4] = MENU_MODULE + 1;
    menu_normalize(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_ROOT && test_header[GRKIT_HOST_MODULE / 4] == 0);
    test_header[GRKIT_HOST_FRAME_RATIO / 4] = 999;
    assert(frame_ratio(test_header) == 0);
    /* Pages start at multiples of PAGE for every supported catalog row. */
    for (u32 row = 0; row <= 146; ++row) assert(page_start(row) == row / PAGE * PAGE);

    /* Image Control opens at the committed row. */
    test_header[GRKIT_HOST_PREVIEW / 4] = 0;
    test_header[GRKIT_HOST_ACTIVE / 4] = 0x59005000 + 3 * GRKIT_RECORD_SIZE;
    test_header[GRKIT_HOST_MENU_LEVEL / 4] = MENU_ROOT;
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 0;
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_MODULE && test_header[GRKIT_HOST_MODULE / 4] == 0);
    assert(test_header[GRKIT_HOST_MENU_CURSOR / 4] == 4);
    assert(test_header[GRKIT_HOST_PREVIEW / 4] == 0);
    /* Highlighting previews in the live view without committing. Off previews native. */
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 2;
    highlight(test_header);
    assert(test_header[GRKIT_HOST_PREVIEW / 4] == 0x59005000 + GRKIT_RECORD_SIZE);
    assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0x59005000 + 3 * GRKIT_RECORD_SIZE);
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 0;
    highlight(test_header);
    assert(test_header[GRKIT_HOST_PREVIEW / 4] == 1);
    assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0x59005000 + 3 * GRKIT_RECORD_SIZE);
    /* MENU restores the committed effect. */
    assert(menu_back(test_header));
    assert(test_header[GRKIT_HOST_PREVIEW / 4] == 0);
    assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0x59005000 + 3 * GRKIT_RECORD_SIZE);
    /* OK commits the highlighted row and ends the preview. */
    menu_confirm(test_header);
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 1;
    highlight(test_header);
    assert(test_header[GRKIT_HOST_PREVIEW / 4] == 0x59005000);
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0x59005000);
    assert(test_header[GRKIT_HOST_PREVIEW / 4] == 0);
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 0;
    highlight(test_header);
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_ACTIVE / 4] == 0);
    assert(test_header[GRKIT_HOST_PREVIEW / 4] == 0);
    /* Off opens Image Control at the Off row. */
    assert(menu_back(test_header));
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MENU_CURSOR / 4] == 0);
    /* Other levels and modules do not preview effects. */
    assert(menu_back(test_header));
    highlight(test_header);
    assert(test_header[GRKIT_HOST_PREVIEW / 4] == 0);
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 1;
    menu_confirm(test_header);
    highlight(test_header);
    assert(test_header[GRKIT_HOST_PREVIEW / 4] == 0);

    for(u32 row=0;row<21;++row) assert(strength_row(strength_code(row),0)==row);
    assert(strength_value(1)==4 && strength_value(2)==8 && strength_value(3)==12);
    for(u32 kind=0;kind<2;++kind) for(u32 row=0;row<(kind ? 26U : 9U);++row) assert(colour_menu_row(colour_menu_code(row,kind),kind)==row);
    /* Five-percent colour values survive edits to every other range and migration. */
    for(u32 word=GRKIT_HOST_COLOUR_SATURATION;word<=GRKIT_HOST_COLOUR_BRIGHTNESS;word+=4) {
        test_header[word/4]=0;test_header[GRKIT_HOST_SATURATION_UPPER/4]=0;
        u32 sat=word==GRKIT_HOST_COLOUR_SATURATION,limit=sat ? 26 : 9;
        for(u32 range=1;range<=7;++range) {
            test_header[GRKIT_HOST_COLOUR_RANGE/4]=range;
            colour_write(test_header,word,colour_menu_code((range*3)%limit,sat),limit-1);
        }
        for(u32 range=1;range<=7;++range) {test_header[GRKIT_HOST_COLOUR_RANGE/4]=range;assert(colour_menu_row(colour_read(test_header,word,limit-1),sat)==(range*3)%limit);}
        for(u32 range=1;range<=7;++range) {test_header[GRKIT_HOST_COLOUR_RANGE/4]=range;colour_write(test_header,word,0,limit-1);}
        assert(!test_header[word/4]);
    }
    test_header[GRKIT_HOST_COLOUR_RANGE/4]=0;
    /* Independent diffusion settings are Off initially and use the common
       Off/Weak/Medium/Strong controls, without consuming artwork slots. */
    test_header[GRKIT_HOST_PLUGINS / 4] = ALL_PLUGINS;
    assert(test_header[GRKIT_HOST_BLOOM/4]==0 && test_header[GRKIT_HOST_HALATION/4]==0);
    for(u32 module=PLUGIN_BLOOM;module<=PLUGIN_HALATION;++module) {
        menu_open(test_header,module);
        assert(count(test_header)==21 && current_module(test_header)->checked(test_header,0));
        assert(same(current_module(test_header)->label(test_header,2),"10%"));
        test_header[GRKIT_HOST_MENU_CURSOR/4]=2; menu_confirm(test_header);
        assert(current_module(test_header)->checked(test_header,2));
        current_module(test_header)->apply(test_header,0);
        assert(menu_back(test_header));
    }

    /* Colour editing is explicit while Off, and every range retains its own
       saturation and hue when the editor moves to another range. */
    menu_open(test_header,PLUGIN_COLOUR_SATURATION);
    assert(count(test_header)==1 && same(current_module(test_header)->label(test_header,0),"Choose Colour Range first"));
    menu_open(test_header,PLUGIN_BLACK_FLOOR);
    current_module(test_header)->apply(test_header,3);
    assert(current_module(test_header)->checked(test_header,3));
    menu_open(test_header,PLUGIN_SPLIT_TONING);
    current_module(test_header)->apply(test_header,0);
    current_module(test_header)->apply(test_header,1);
    assert(split_preset(test_header[GRKIT_HOST_SPLIT_TONE/4])==5);
    assert(menu_back(test_header));
    current_module(test_header)->apply(test_header,1);
    current_module(test_header)->apply(test_header,10);
    assert(split_percent(test_header)==50);
    assert(menu_back(test_header));
    for(u32 range=1;range<=4;range+=3) {
        menu_open(test_header,PLUGIN_COLOUR_RANGE);
        current_module(test_header)->apply(test_header,range);
        menu_open(test_header,PLUGIN_COLOUR_SATURATION);
        assert(count(test_header)==26);
        current_module(test_header)->apply(test_header,range==1 ? 2 : 3);
        menu_open(test_header,PLUGIN_COLOUR_HUE);
        assert(count(test_header)==9);
        current_module(test_header)->apply(test_header,range==1 ? 3 : 1);
    }
    menu_open(test_header,PLUGIN_COLOUR_RANGE);current_module(test_header)->apply(test_header,1);
    menu_open(test_header,PLUGIN_COLOUR_SATURATION);assert(current_module(test_header)->checked(test_header,2));
    menu_open(test_header,PLUGIN_COLOUR_HUE);assert(current_module(test_header)->checked(test_header,3));
    test_header[GRKIT_HOST_BLACK_FLOOR/4]=test_header[GRKIT_HOST_SPLIT_TONE/4]=0;
    test_header[GRKIT_HOST_COLOUR_RANGE/4]=test_header[GRKIT_HOST_COLOUR_SATURATION/4]=test_header[GRKIT_HOST_COLOUR_HUE/4]=0;

    browse_to(PLUGIN_COLOUR_RANGE);
    menu_confirm(test_header); assert(count(test_header)==8);
    test_header[GRKIT_HOST_MENU_CURSOR/4]=6; menu_confirm(test_header);
    assert(count(test_header)==3 && same(menu_heading(test_header),"Colour / Blues"));
    test_header[GRKIT_HOST_MENU_CURSOR/4]=2; menu_confirm(test_header);
    assert(current_index(test_header)==PLUGIN_COLOUR_BRIGHTNESS && count(test_header)==9);
    test_header[GRKIT_HOST_MENU_CURSOR/4]=3; menu_confirm(test_header);
    assert(colour_read(test_header,GRKIT_HOST_COLOUR_BRIGHTNESS,8)==1);
    assert(menu_back(test_header) && current_index(test_header)==PLUGIN_COLOUR_RANGE && count(test_header)==3);
    assert(menu_back(test_header) && count(test_header)==8 && test_header[GRKIT_HOST_MENU_CURSOR/4]==6);
    assert(menu_back(test_header) && test_header[GRKIT_HOST_MENU_LEVEL/4]==MENU_ROOT);
    test_header[GRKIT_HOST_COLOUR_BRIGHTNESS/4]=test_header[GRKIT_HOST_COLOUR_RANGE/4]=0;

    /* No About/version row takes up a root position. */
    test_header[GRKIT_HOST_PLUGINS/4]=ALL_PLUGINS;
    test_header[GRKIT_HOST_COUNT/4]=5;
    menu_start(test_header);assert(count(test_header)==6);
    for(u32 row=0;row<count(test_header);++row) assert(root_module(test_header,row)!=ABOUT_MODULE);
    /* Category and control icons are unique within each menu. */
    for(u32 section=0;section<=3;++section) {
        test_header[GRKIT_HOST_MENU_SECTION/4]=section;
        for(u32 row=0;row<count(test_header);++row) for(u32 other=0;other<row;++other)
            assert(menu_row_icon(test_header,row)!=menu_row_icon(test_header,other));
    }
    menu_start(test_header);
    /* A plugin that is not installed has no root row. The others keep their order. */
    test_header[GRKIT_HOST_PREVIEW / 4] = 0;
    test_header[GRKIT_HOST_FRAME_RATIO / 4] = 0;
    test_header[GRKIT_HOST_PLUGINS / 4] = ALL_PLUGINS & ~(1U << PLUGIN_GUIDES);
    menu_start(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_ROOT && test_header[GRKIT_HOST_MENU_CURSOR / 4] == 0 && count(test_header) == 6);
    assert(current_module(test_header) == &modules[0]);
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = root_row(test_header,PLUGIN_DATE_STAMP);
    assert(current_module(test_header) == &modules[PLUGIN_DATE_STAMP]);
    assert(same(current_module(test_header)->summary(test_header), "Date Stamp: Off"));
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_MODULE && test_header[GRKIT_HOST_MODULE / 4] == PLUGIN_DATE_STAMP);
    assert(menu_back(test_header));
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_ROOT && test_header[GRKIT_HOST_MENU_CURSOR / 4] == 4);
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 1;
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MODULE / 4] == PLUGIN_FRAMING);
    assert(menu_back(test_header) && test_header[GRKIT_HOST_MENU_CURSOR / 4] == 1);
    /* The page of a plugin that is not installed does not stay open. */
    test_header[GRKIT_HOST_MENU_LEVEL / 4] = MENU_MODULE;
    test_header[GRKIT_HOST_MODULE / 4] = PLUGIN_GUIDES;
    menu_normalize(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_ROOT && test_header[GRKIT_HOST_MENU_CURSOR / 4] == 0);
    /* Image Control that is not installed does not show, even with effects. */
    test_header[GRKIT_HOST_PLUGINS / 4] = ALL_PLUGINS & ~(1U | 1U << PLUGIN_FRAMING);
    assert(visible_modules(test_header) == (ALL_PLUGINS & ~(1U | 1U << PLUGIN_FRAMING)));
    /* Without effects, Image Control does not show. */
    test_header[GRKIT_HOST_PLUGINS / 4] = ALL_PLUGINS;
    test_header[GRKIT_HOST_COUNT / 4] = 0;
    menu_start(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_ROOT && count(test_header) == 5);
    assert(current_module(test_header) == &modules[GROUP_FRAME]);
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = root_row(test_header,PLUGIN_DATE_STAMP);
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MODULE / 4] == PLUGIN_DATE_STAMP);
    assert(menu_back(test_header) && test_header[GRKIT_HOST_MENU_CURSOR / 4] == 3);
    /* One installed plugin keeps the root level, because About follows it. */
    test_header[GRKIT_HOST_PLUGINS / 4] = 1U | 1U << PLUGIN_DATE_STAMP;
    test_header[GRKIT_HOST_STAMP / 4] = 3;
    menu_start(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_ROOT && count(test_header) == 1);
    assert(root_module(test_header, 0) == PLUGIN_DATE_STAMP);
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_MODULE && test_header[GRKIT_HOST_MODULE / 4] == PLUGIN_DATE_STAMP);
    assert(test_header[GRKIT_HOST_MENU_CURSOR/4]==0 && count(test_header)==2);
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MENU_CURSOR/4]==3 && count(test_header)==STAMP_STYLE_COUNT);
    assert(menu_back(test_header));
    assert(menu_back(test_header) && test_header[GRKIT_HOST_MENU_CURSOR / 4] == 0);
    assert(!menu_back(test_header));
    test_header[GRKIT_HOST_STAMP / 4] = 0;
    /* With no plugin visible, the root shows About alone. The installer installs
       no menu hook then, so only an inconsistent header gets here. */
    test_header[GRKIT_HOST_PLUGINS / 4] = 1U;
    menu_start(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_ROOT && count(test_header) == 1);
    assert(current_module(test_header) == &modules[ABOUT_MODULE]);
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_ROOT && !menu_back(test_header));
    /* Image Control alone keeps the root level. Its page opens at the committed
       effect, where Fn applies. */
    test_header[GRKIT_HOST_COUNT / 4] = 5;
    test_header[GRKIT_HOST_ACTIVE / 4] = 0x59005000 + 2 * GRKIT_RECORD_SIZE;
    menu_start(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_ROOT && count(test_header) == 1);
    menu_confirm(test_header);
    assert(test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_MODULE && test_header[GRKIT_HOST_MODULE / 4] == 0);
    assert(test_header[GRKIT_HOST_MENU_CURSOR / 4] == 3 && parameters_row(test_header));
    test_header[GRKIT_HOST_MENU_CURSOR / 4] = 1;
    highlight(test_header);
    assert(test_header[GRKIT_HOST_PREVIEW / 4] == 0x59005000);
    assert(menu_back(test_header));
    assert(test_header[GRKIT_HOST_PREVIEW / 4] == 0 && test_header[GRKIT_HOST_MENU_LEVEL / 4] == MENU_ROOT);
    return 0;
}
