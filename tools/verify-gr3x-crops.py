#!/usr/bin/env python3
"""Run compiled crop geometry and A32 adapters; no camera/hardware claim."""
from pathlib import Path
import re,struct,json,sys
source=Path(__file__).with_name('verify-gr3x-colour.py').read_text().split('def run(')[0]
exec(compile(source,'crop_elf_loader','exec'))
from unicorn.arm_const import *
abi=Path(__file__).parents[1].joinpath('runtime/abi.h').read_text()
def host(n):return int(re.search(r'#define GRKIT_HOST_'+n+r' (0x[0-9a-fA-F]+|[0-9]+)',abi)[1],0)
u=Uc(UC_ARCH_ARM,UC_MODE_ARM);u.mem_map(0x53000000,0x2000000);u.mem_write(base,bytes(text));u.mem_map(0x55000000,0x400000);u.mem_map(0x57000000,0x10000)
STACK,STOP,OUT=0x5700f000,0x57008000,0x550a0000
regs=[UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3,UC_ARM_REG_R4,UC_ARM_REG_R5,UC_ARM_REG_R6,UC_ARM_REG_R7,UC_ARM_REG_R8,UC_ARM_REG_R9,UC_ARM_REG_R10,UC_ARM_REG_R11,UC_ARM_REG_R12]
def w(a,v):u.mem_write(a,struct.pack('<I',v))
def r(reg):return u.reg_read(reg)
def call(name,args=(),initial=None,nonlocal_return=False):
 for i,reg in enumerate(regs):u.reg_write(reg,0x100+i)
 for i,value in enumerate(args):u.reg_write(regs[i],value)
 if initial:
  for reg,value in initial.items():u.reg_write(reg,value)
 u.reg_write(UC_ARM_REG_CPSR,0x13);u.reg_write(UC_ARM_REG_SP,STACK);u.reg_write(UC_ARM_REG_LR,STOP)
 if nonlocal_return:u.mem_write(STACK,struct.pack('<7I',11,12,13,14,15,STACK+256,STOP))
 u.emu_start(symbols[name],STOP,count=1000000)
 assert r(UC_ARM_REG_PC)==STOP,name
 assert r(UC_ARM_REG_SP)==STACK+(256 if nonlocal_return else 0),name
 return r(UC_ARM_REG_R0)
expected={4:[(5008,4000),(4000,3200),(2800,2240),(1600,1280)],5:[(4672,4000),(3728,3200),(2608,2240),(1488,1280)],6:[(5200,4000),(4160,3200),(2912,2240),(1664,1280)]}
boxes={4:(60,0,6,0),5:(80,0,10,0),6:(48,0,2,0)}
cases=0
for ratio,dimensions in expected.items():
 for size,(width,height) in enumerate(dimensions):
  w(HEADER+host('FRAME_RATIO'),ratio);w(HEADER+host('FRAME_FILE'),0)
  assert call('framing_geometry',[size])==width|(height<<16)
  stock=[6000,4800,3360,1920][size]
  call('framing_height_'+str(size),[101,102,stock])
  assert (r(UC_ARM_REG_R0),r(UC_ARM_REG_R1),r(UC_ARM_REG_R2),r(UC_ARM_REG_R3))==(101,102,width,height)
  for ww,hh in [(width,height),(height,width)]:
   assert call('framing_classify_size',[ww,hh])==(ratio<<8)|size
   # The real classifier adapter returns through its native caller's frame.
   w(OUT,0xdeadbeef)
   assert call('framing_classify',[ww,hh],{UC_ARM_REG_R6:OUT},True)==1
   assert u.mem_read(OUT,1)==bytes([size])
   assert u.mem_read(HEADER+host('FRAME_FILE'),1)==bytes([ratio+1])
   assert call('framing_image_aspect',[ww,hh])==2
  # Playback geometry follows the saved image, independently of the shooting selection.
  w(HEADER+host('FRAME_RATIO'),2)
  assert call('framing_geometry',[size])==width|(height<<16)
  cases+=1
 w(HEADER+host('FRAME_RATIO'),ratio);w(HEADER+host('FRAME_FILE'),0)
 for thumbnail,name in [(0,'framing_screennail_inset'),(1,'framing_thumbnail_inset')]:
  call(name);assert (r(UC_ARM_REG_R0),r(UC_ARM_REG_R1))==boxes[ratio][thumbnail*2:thumbnail*2+2];cases+=1
# Ordinary native dimensions, including 1:1, must remain native.
for width,height in [(6000,4000),(4000,4000),(4800,3200),(3200,3200)]:
 assert call('framing_classify_size',[width,height])==0xffffffff
 assert u.mem_read(HEADER+host('FRAME_FILE'),1)==b'\1'
 assert call('framing_geometry',[0])==0;cases+=1
# An ordinary id-2 resolution keeps its original width and 16:9 height.
w(HEADER+host('FRAME_RATIO'),2);w(HEADER+host('FRAME_FILE'),0)
for i in range(4):
 call('framing_height_'+str(i),[101,102,[6000,4800,3360,1920][i]])
 assert r(UC_ARM_REG_R2)==[6000,4800,3360,1920][i] and r(UC_ARM_REG_R3)==[3376,2704,1888,1080][i];cases+=1
print(json.dumps({'compiled_ARM_crop_geometry':'pass','cases':cases,'crops':['5:4','6x7 (7:6)','FP100C'],'native_square_preserved':True,'capture_and_playback_adapters':True,'hardware_tested':False}))
