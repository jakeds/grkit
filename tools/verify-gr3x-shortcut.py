#!/usr/bin/env python3
"""Exercise compiled Soft menu entry/exit plus stock Open ADJ in ARM emulation.
Usage: python verify-gr3x-shortcut.py module-menu.elf rtos.bin
Native UI/OS calls and drawing are stubbed. This is not camera validation.
"""
import hashlib,json,struct,sys
from pathlib import Path
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_CODE
from unicorn.arm_const import UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_PC,UC_ARM_REG_CPSR
elf=Path(sys.argv[1]).read_bytes();firmware=Path(sys.argv[2]).read_bytes()
assert hashlib.sha256(firmware).hexdigest()=='0f11b0d28bc67932d044c5d01bc0314844a756057c89984278c73a23449a24c6'
shoff=struct.unpack_from('<I',elf,32)[0];shsize,count,namesidx=struct.unpack_from('<HHH',elf,46)
sections=[struct.unpack_from('<10I',elf,shoff+i*shsize) for i in range(count)]
def data(s):return elf[s[4]:s[4]+s[5]]
def name(t,o):return t[o:t.index(b'\0',o)].decode()
names=data(sections[namesidx]);textidx=next(i for i,s in enumerate(sections) if name(names,s[0])=='.text')
text=bytearray(data(sections[textidx]));base=sections[textidx][3];symbols={}
for s in sections:
 if s[1]!=2:continue
 strings=data(sections[s[6]])
 for at in range(s[4],s[4]+s[5],s[9]):
  no,address,size,info,other,index=struct.unpack_from('<IIIBBH',elf,at)
  if no and index==textidx:symbols[name(strings,no)]=address
H,LIVE,ADJ,EVENT,ARG,STOP,STACK=0x55030000,0x55080000,0x55090000,0x550a0000,0x550a0100,0x57008000,0x5700fff0
for i in range(0,len(text)-3,4):
 if struct.unpack_from('<I',text,i)[0]==0x58012340:struct.pack_into('<I',text,i,H)
HEADER_BYTES=int(__import__('re').search(r'#define GRKIT_HOST_HEADER_SIZE ([0-9]+)',Path(__file__).parents[1].joinpath('runtime/abi.h').read_text())[1])
class Probe:
 def __init__(self,enabled=1,movie=0,installed=True):
  self.u=Uc(UC_ARCH_ARM,UC_MODE_ARM);self.u.mem_map(0x53000000,0x2000000);self.u.mem_write(0x53000000,firmware);self.u.mem_write(base,bytes(text))
  self.u.mem_map(0x55000000,0x400000);self.u.mem_map(0x57000000,0x10000)
  self.events=[];self.w(H+4,H+HEADER_BYTES);self.w(H+56,1<<14 if installed else 0);self.w(H+288,enabled)
  self.u.mem_write(LIVE+0x2d2,bytes([movie]));self.u.mem_write(ADJ+0x270,bytes([movie]))
  self.hooks={};self.thumb_stubs=set()
  def add(symbol,handler):
   self.hooks[symbols[symbol]&~1]=handler
   if symbols[symbol]&1:self.thumb_stubs.add(symbols[symbol]&~1)
  add('menu_live_event_original',lambda:self.ret(42,'native-live-event'))
  add('menu_live_start_original',lambda:self.ret(43,'native-live-start'))
  add('menu_refresh_original',self.refresh)
  add('menu_exit_original',lambda:self.ret(0,'native-adj-exit'))
  add('toolbar',lambda:self.ret(0,'toolbar'))
  add('render',self.render)
  add('detail_width',lambda:self.ret(0,'detail-width'))
  self.hooks[0x5320ad88]=lambda:self.ret(0x55010000,'root')
  self.hooks[0x5320aefc]=lambda:self.ret(0x55011000,'manager')
  self.hooks[0x53228a90]=lambda:self.ret(LIVE,'current-controller')
  self.hooks[0x53228c4c]=lambda:self.ret(0x8000000,'current-id')
  self.hooks[0x5316c9d8]=self.transition
  self.hooks[0x533947f0]=self.save
  self.hooks[0x53395224]=self.restore
  self.hooks[0x531a4114]=lambda:self.ret(0,'native-list-rebuild')
  self.hooks[0x533ad3c0]=lambda:self.ret(0,'view-visible')
  # Give each stub an actual BX LR instruction in the emulated copy. This
  # avoids redirecting PC mid-translation across mixed ARM/Thumb veneers.
  for address in self.hooks:
   self.u.mem_write(address,b'\x70\x47' if address in self.thumb_stubs else struct.pack('<I',0xe12fff1e))
  self.u.hook_add(UC_HOOK_CODE,self.hook)
 def w(self,a,v):self.u.mem_write(a,struct.pack('<I',v))
 def r(self,a):return struct.unpack('<I',self.u.mem_read(a,4))[0]
 def reg(self,r):return self.u.reg_read(r)
 def ret(self,v,label):
  self.events.append(label);
  self.u.reg_write(UC_ARM_REG_R0,v)
 def hook(self,u,a,size,_):
  if a in self.hooks:self.hooks[a]()
  elif a!=STOP and not(base<=a<base+len(text) or 0x532264ec<=a<=0x53226528):raise AssertionError('Unstubbed firmware call '+hex(a)+' events='+str(self.events))
 def transition(self):
  assert (self.reg(UC_ARM_REG_R0),self.reg(UC_ARM_REG_R1),self.reg(UC_ARM_REG_R2))==(LIVE,0x2000000,0x8000000)
  self.ret(1,'queued-adj')
 def refresh(self):
  assert self.reg(UC_ARM_REG_R0)==ADJ
  self.w(ADJ+0x1f0,0x550c0000);self.ret(0,'native-adj-refresh')
 def save(self):
  assert self.reg(UC_ARM_REG_R0)==0x550c0000
  self.w(self.reg(UC_ARM_REG_R1),3);self.w(self.reg(UC_ARM_REG_R2),9);self.ret(0,'saved-native-list')
 def restore(self):
  assert (self.reg(UC_ARM_REG_R1),self.reg(UC_ARM_REG_R2))==(3,9)
  self.ret(0,'restored-native-list')
 def render(self):
  assert self.reg(UC_ARM_REG_R0)==ADJ and self.r(H+32)==ADJ and self.r(H+36)==1
  assert self.events.index('native-adj-refresh')<self.events.index('saved-native-list')
  self.ret(0,'render-soft-root')
 def call(self,symbol,a,b=0):
  for reg,v in [(UC_ARM_REG_R0,a),(UC_ARM_REG_R1,b),(UC_ARM_REG_SP,STACK),(UC_ARM_REG_LR,STOP)]:self.u.reg_write(reg,v)
  self.u.emu_start(symbols[symbol],STOP,count=100000)
  assert self.reg(UC_ARM_REG_PC)==STOP and self.reg(UC_ARM_REG_SP)==STACK
  return self.reg(UC_ARM_REG_R0)
 def fn(self,symbol='menu_live_event',phase=2,repeats=0):
  self.w(EVENT,0x1b);self.w(EVENT+4,ARG);self.w(ARG,phase);self.w(ARG+4,repeats)
  return self.call(symbol,LIVE if symbol=='menu_live_event' else ADJ,EVENT)
checks=[]
for settings in [dict(enabled=0),dict(movie=1),dict(installed=False),dict(enabled=9)]:
 p=Probe(**settings);assert p.fn()==42 and p.events==['native-live-event'];assert p.r(H+292)==0 and p.r(H+296)==0
 checks.append({'pass_through':settings})
p=Probe();assert p.fn()==1;assert 'queued-adj' in p.events and 'native-live-event' not in p.events;assert p.r(H+292)==3 and p.r(H+296)==1
assert p.fn(phase=2,repeats=0)==1 and p.r(H+296)==1
assert p.events.count('queued-adj')==1
assert p.fn(phase=2,repeats=1)==1 and p.r(H+296)==1
p.call('menu_refresh',ADJ);assert p.events.count('render-soft-root')==1 and p.r(H+292)==1
assert p.fn('menu_event',phase=2,repeats=1)==0
assert p.fn('menu_event',phase=1)==0 and p.r(H+292)==0
p.call('menu_exit',ADJ);assert p.r(H+36)==0 and p.r(H+32)==0 and p.r(H+292)==0;assert 'restored-native-list' in p.events
checks.append({'open_repeat_release_exit':'pass','order':p.events})
p=Probe();p.fn();p.fn(phase=1);assert p.r(H+292)==2;p.call('menu_refresh',ADJ);assert p.r(H+292)==0;checks.append({'early_release_then_open':'pass'})
p=Probe();p.fn();assert p.call('menu_live_start',LIVE,0x8000000)==43 and p.r(H+292)==1;p.fn(phase=1);assert p.r(H+292)==0;checks.append({'cancel_pending_on_return_to_live':'pass'})
p=Probe();p.fn();p.w(H+288,0);assert p.fn(phase=1)==1 and p.r(H+292)==2;p.call('menu_refresh',ADJ);assert 'render-soft-root' not in p.events;checks.append({'disable_before_ready':'pass'})
print(json.dumps({'compiled_ARM_shortcut':'pass','cases':checks,'native_open_ADJ_executed':True,'native_UI_services_and_rendering':'stubbed','hardware_test':False},indent=2))
