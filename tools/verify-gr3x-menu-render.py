#!/usr/bin/env python3
"""Exercise the compiled ADJ renderer/navigation with native UI services stubbed.
Usage: verify-gr3x-menu-render.py module-menu.elf rtos.bin module_payload.h
Requires Unicorn and the pinned private GR IIIx RTOS. No hardware claim.
"""
from pathlib import Path
import re,sys,json,struct
# Reuse the ELF loader, pinned firmware check and mixed ARM/Thumb call harness.
source=Path(__file__).with_name('verify-gr3x-shortcut.py').read_text().split('checks=[]')[0]
source=source.replace("  add('render',self.render)\n",'')
exec(compile(source,'menu_probe_harness','exec'))
p=Probe(enabled=0,installed=False)
U=p.u;VIEW=0x550c0000;ROW=0x550d0000;RECT=0x533fff00
p.frames=[];p.moves=[];p.cursor=0;p.row_count=0;p.kinds=[1,1,1,1,1]
R0,R1,R2=UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2
def r(reg):return p.reg(reg)
def stub(address,fn):
 p.hooks[address]=fn;U.mem_write(address,struct.pack('<I',0xe12fff1e))
def count():
 p.row_count=r(R1);U.mem_write(VIEW+0x108,bytes([p.row_count]));p.ret(0,'row-count')
def cursor():p.cursor=r(R1);p.ret(0,'cursor')
def move():
 delta=r(R1);p.moves.append(delta);p.cursor=(p.cursor+(1 if delta==1 else -1))%p.row_count;p.ret(0,'move')
def rectangle():p.frames.append(struct.unpack('<4I',U.mem_read(r(R1),16)));p.ret(0,'rectangle')
stub(0x531a2820,lambda:p.ret(p.kinds[r(R1)],'slot-type'))
stub(0x53394e8c,count);stub(0x53394758,lambda:p.ret(ROW+r(R1)*256,'row'))
stub(0x53394820,lambda:p.ret(p.cursor,'get-cursor'));stub(0x533948c4,lambda:p.ret(100,'width'))
stub(0x53395430,cursor);stub(0x53395260,move);stub(RECT,rectangle)
for a in [0x533a1734,0x533a19b4,0x53396f80,0x533ad308,0x5339b7d4,0x533ac568]:stub(a,lambda:p.ret(0,'native-view'))
for n in ['menu_event_original','menu_changed_original']:
 a=symbols[n]&~1;p.hooks[a]=lambda:p.ret(0,'native-event');U.mem_write(a,b'\x70\x47' if symbols[n]&1 else struct.pack('<I',0xe12fff1e))
p.w(ADJ+0x1f0,VIEW);p.w(VIEW,0x550e0000);p.w(0x550e0018,RECT)
p.w(VIEW+0xf4,0x550e1000);p.w(VIEW+0xf8,0x550e1080)
for offset in [0x160,0x164,0x168,0x1ac]:p.w(ADJ+offset,0x550f0000+offset*256)
p.w(H,16);p.w(H+56,(1<<14)-1)
textbase=H+HEADER_BYTES+16*0x740+141448
payload=Path(sys.argv[3]).read_text()
blob=bytes(int(x) for x in re.search(r'static const u8 menu_text\[\][^=]*= \{([^}]+)\}',payload)[1].split(',') if x.strip())
U.mem_write(textbase,blob)
for i in range(16):U.mem_write(H+HEADER_BYTES+i*0x740+0x668,('Look '+str(i)+'\0').encode('utf-16le'))
header=Path(__file__).parents[1].joinpath('runtime/targets/gr3-2.10.h').read_text()
right=int(re.search(r'#define EV_RIGHT (0x[0-9a-f]+)',header)[1],16)
def event(kind):p.w(EVENT,kind);return p.call('menu_event',ADJ,EVENT)
def title():
 a=p.r(H+100);b=bytes(U.mem_read(a,64));return b.decode('utf-16le').split('\0')[0]
p.call('plugin_touch',ADJ)
assert p.row_count==5 and p.frames[-1]==(620,120,100,300) and title()=='Soft'
p.w(H+40,4);event(right)
assert p.row_count==2 and title().endswith('Date Stamp') and p.frames[-1][1]==300
p.w(H+40,0);event(right)
assert p.row_count==7 and title()=='Date Stamp / Format' and p.frames[-1][1:]==(60,100,360)
event(int(re.search(r'#define EV_LEFT (0x[0-9a-f]+)',header)[1],16))
assert p.row_count==2 and p.r(H+284)==0
p.w(H+40,1);event(right)
assert p.row_count==5 and title()=='Date Stamp / Appearance'
p.w(H+40,1);event(right);assert p.row_count==5 and p.r(H+172)==0 and p.r(H+36)==2
event(0xd8)
assert p.r(H+172)==1 and p.r(H+36)==2 and p.r(H+40)==1 and p.row_count==5
# Repeated OK commits in place without forwarding a native closing confirm.
before=p.events.count('native-event')
p.w(H+40,4);event(0xd8);assert p.r(H+172)==4 and p.r(H+36)==2 and p.r(H+40)==4
event(0xd8);assert p.r(H+172)==4 and p.r(H+36)==2
assert p.events.count('native-event')==before
left=int(re.search(r'#define EV_LEFT (0x[0-9a-f]+)',header)[1],16)
event(left);assert p.r(H+36)==2 and p.row_count==2 and p.r(H+40)==1
event(left)
assert p.row_count==5 and p.r(H+40)==4
# All expanded strength menus commit the highest row in place.
abi=Path(__file__).parents[1].joinpath('runtime/abi.h').read_text()
def host(name):return int(re.search(r'#define GRKIT_HOST_'+name+r' (0x[0-9a-fA-F]+|[0-9]+)',abi)[1],0)
for module,name,code in [(5,'BLOOM',52),(6,'HALATION',52),(7,'BLACK_FLOOR',52),(13,'HIGHLIGHT_ROLLOFF',52)]:
    p.w(H+36,2);p.w(H+188,module);p.w(H+40,20);p.w(H+284,0)
    before=p.events.count('native-event');event(0xd8)
    assert p.r(H+host(name))==code and p.r(H+36)==2 and p.r(H+40)==20 and p.row_count==21
    assert p.events.count('native-event')==before
# Warm-first split-toning presets and an independent five-percent strength.
p.w(H+36,2);p.w(H+188,8);p.w(H+300,2);p.w(H+40,0);p.w(H+284,0);p.w(H+260,0);p.w(H+308,0)
event(right);assert p.row_count==9 and title()=='Split Toning / Preset'
p.w(H+40,1);event(0xd8);assert p.r(H+260)==13 and p.r(H+308)==6 and p.r(H+36)==2
event(left);assert p.row_count==2
p.w(H+40,1);event(right);assert p.row_count==21 and title()=='Split Toning / Strength' and p.r(H+40)==5
p.w(H+40,3);event(0xd8);assert p.r(H+260)==13 and p.r(H+308)==4 and p.r(H+36)==2
# A physical ADJ press still closes the menu deliberately.
event(0x1e);event(0xd8);assert p.r(H+36)==0 and p.events.count('native-event')==before+1
p.call('plugin_touch',ADJ)
p.w(H+300,0);p.w(H+36,1);p.w(H+188,0);p.w(H+40,0)
# Looks uses all 17 rows (Off and sixteen looks), then scrolls one row.
p.w(H+40,0);event(right);assert p.row_count==17
header=Path(__file__).parents[1].joinpath('runtime/targets/gr3-2.10.h').read_text()
down=int(re.search(r'#define EV_DOWN (0x[0-9a-f]+)',header)[1],16)
event(down);assert p.moves[-1]==1 and p.r(H+40)==1
# Left inside a category returns to the main root, rather than leaving ADJ.
p.w(H+36,1);p.w(H+300,0);p.w(H+40,2);p.w(H+188,0)
event(right);assert p.r(H+300)==2
left=int(re.search(r'#define EV_LEFT (0x[0-9a-f]+)',header)[1],16)
event(left);assert p.r(H+300)==0 and p.r(H+36)==1 and p.r(H+40)==2
# Fn from the Looks root retains its record while opening native parameters,
# and returns to the root afterwards (no automatic Fn shooting shortcut).
p.call('plugin_touch',ADJ) if p.r(H+36)==0 else None
p.w(H+36,1);p.w(H+300,0);p.w(H+40,0);p.w(H+188,0);p.w(H+8,H+HEADER_BYTES)
stub(0x531a0150,lambda:p.ret(0,'snapshot'))
stub(0x5316c9d8,lambda:p.ret(0,'parameter-controller'))
p.fn('menu_event',phase=2,repeats=0)
assert p.r(H+252)==17 and p.r(H+144)==(H+HEADER_BYTES)|1
p.call('menu_exit',ADJ);assert p.r(H+252)==18 and p.r(H+144)==(H+HEADER_BYTES)|1
p.w(H+144,H+HEADER_BYTES);p.call('menu_refresh',ADJ)
assert p.r(H+36)==1 and p.r(H+40)==0 and p.r(H+300)==0
# Execute the actual toolbar code for the assigned-slot prototype.
if '#define SOFT_NATIVE_ADJ_SLOT 1' in header:
    toolbar=symbols['toolbar']&~1
    del p.hooks[toolbar]
    length=2 if symbols['toolbar']&1 else 4
    U.mem_write(toolbar,bytes(text[toolbar-base:toolbar-base+length]));U.ctl_remove_cache(toolbar,toolbar+length)
    visible={};positions={};icons={}
    def set_visible():visible[r(R0)]=r(R1);p.ret(0,'visibility')
    def set_x():positions[r(R0)]=r(R1);p.ret(0,'position')
    def set_icon():icons[r(R0)]=r(R1);p.ret(0,'icon')
    stub(0x533ad3c0,set_visible);stub(0x533ac568,set_x);stub(0x5339a9a0,set_icon)
    def select_slot():U.mem_write(ADJ+0x1f4,bytes([r(R1)]));p.ret(0,'select-native-slot')
    stub(0x531a4864,select_slot)
    for i in range(5):
        p.w(ADJ+0x16c+i*4,0x55110000+i*512);p.w(ADJ+0x184+i*4,0x55120000+i*512)
    p.w(ADJ+0x1a8,0x55130000)
    slot_cases=0
    for slot in range(5):
        for only in [False,True]:
            p.kinds=[0 if only else 1]*5;p.kinds[slot]=12
            p.w(H+36,0);p.w(H+32,0);p.w(H+252,0);p.w(H+60,0);p.w(H+8,H+HEADER_BYTES)
            U.mem_write(ADJ+0x1f4,bytes([slot]));U.mem_write(ADJ+0x270,b'\0')
            visible.clear();positions.clear();icons.clear()
            p.call('menu_refresh',ADJ)
            assert p.r(H+36)==1 and p.r(H+88)==4 and p.r(H+8)==H+HEADER_BYTES
            assert icons[0x55120000+slot*512]==0x7ffe
            for i in range(5):assert visible[0x55110000+i*512]==int(not only or i==slot)
            if only:assert positions[0x55120000+slot*512]==430
            # Fn and its return also retain the borrowed icon/layout.
            p.w(H+40,0);p.fn('menu_event',phase=2,repeats=0)
            assert p.r(H+252)==17 and p.r(H+144)==(H+HEADER_BYTES)|1
            p.call('menu_exit',ADJ);assert p.r(H+252)==18
            p.w(H+144,H+HEADER_BYTES);p.call('menu_refresh',ADJ)
            assert p.r(H+36)==1 and icons[0x55120000+slot*512]==0x7ffe
            event(left)
            assert p.r(H+36)==0 and p.r(H+8)==H+HEADER_BYTES
            if only:assert p.r(H+252)==4
            else:assert bytes(U.mem_read(ADJ+0x1f4,1))[0]==(slot+4)%5
            # Native touch on the borrowed slot reopens Soft even if select_slot
            # does not invoke a refresh in this stubbed service environment.
            p.w(H+252,0);p.call('menu_native_touch',ADJ,slot);assert p.r(H+36)==1
            # Restoration must not recursively reopen the borrowed slot.
            p.w(H+36,0);p.w(H+32,0);p.w(H+252,0);p.w(H+60,1)
            p.call('menu_refresh',ADJ);assert p.r(H+36)==0
            p.w(H+60,0)
            # Movie ADJ stays native despite Image Control being assigned.
            U.mem_write(ADJ+0x270,b'\1');p.call('menu_refresh',ADJ);assert p.r(H+36)==0
            for i in range(5):assert visible[0x55110000+i*512]==1
            U.mem_write(ADJ+0x270,b'\0');slot_cases+=1
    # Native MENU Image Control still clears the Soft look when a native choice
    # is confirmed, via its unchanged assembly adapter and original continuation.
    match=re.search(r'#define HOOK_MENU_NATIVE_PAGE_EVENT_RESUME (0x[0-9a-f]+)',Path(sys.argv[3]).with_name('runtime').joinpath('hooks.h').read_text())
    stub(int(match[1],16),lambda:p.ret(0,'native-MENU-continuation'))
    p.w(H+8,H+HEADER_BYTES);p.w(EVENT,0xd8);p.call('menu_native_page_event',ADJ,EVENT)
    assert p.r(H+8)==0
    print(json.dumps({'borrowed_ADJ_slot_cases':slot_cases,'single_visible_Soft_cell':True,'other_assignments_preserved':True,'movie_ADJ_native':True,'native_MENU_Image_Control_preserved':True,'restore_reentry_guard':True,'OK_applies_without_closing':True,'ADJ_press_closes':True,'strength_steps':20,'warm_split_preset_and_strength':True,'hardware_tested':False}))
print(json.dumps({'compiled_ARM_menu_renderer':'pass','native_UI_services':'stubbed','root_rows':5,'date_format_rows':7,'date_appearance_rows':5,'looks_rows':17,'back_rows':0,'native_single_row_scroll':True,'date_back_and_commit':True,'reopen_selected_control':True,'root_Fn_parameter_return':True,'left_returns_from_category':True,'right_opens_without_committing_values':True,'OK_applies_without_closing':True,'ADJ_press_closes':True,'strength_steps':20,'warm_split_preset_and_strength':True,'hardware_tested':False}))
