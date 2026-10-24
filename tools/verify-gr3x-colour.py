#!/usr/bin/env python3
"""Run the compiled Soft colour encoders under ARM emulation (not a camera test).
Usage: python verify-gr3x-colour.py path/to/module-framing.elf
Requires Unicorn. No Ricoh firmware bytes or operating-system stubs are needed.
"""
import json
import struct
import sys
from pathlib import Path
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM
from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC, UC_ARM_REG_C1_C0_2, UC_ARM_REG_FPEXC

elf = Path(sys.argv[1]).read_bytes()
shoff = struct.unpack_from('<I', elf, 32)[0]
shsize, count, names_index = struct.unpack_from('<HHH', elf, 46)
sections = [struct.unpack_from('<10I', elf, shoff + i * shsize) for i in range(count)]
def data(section):
    return elf[section[4]:section[4] + section[5]]
def name(table, offset):
    return table[offset:table.index(b'\0', offset)].decode()
names = data(sections[names_index])
text_index = next(i for i, s in enumerate(sections) if name(names, s[0]) == '.text')
text = bytearray(data(sections[text_index])); base = sections[text_index][3]
HEADER = 0x55030000
for i in range(0, len(text) - 3, 4):
    if struct.unpack_from('<I', text, i)[0] == 0x58012340:
        struct.pack_into('<I', text, i, HEADER)
symbols = {}
for section in sections:
    if section[1] != 2:
        continue
    strings = data(sections[section[6]])
    for at in range(section[4], section[4] + section[5], section[9]):
        no, address, size, info, other, index = struct.unpack_from('<IIIBBH', elf, at)
        if no and index == text_index:
            symbols[name(strings, no)] = address

def run(symbol, y, chroma, settings, half=0):
    u = Uc(UC_ARCH_ARM, UC_MODE_ARM)
    start = base & ~4095
    u.mem_map(start, (base - start + len(text) + 4095) & ~4095)
    u.mem_write(base, bytes(text))
    u.mem_map(0x55000000, 0x400000)
    u.mem_map(0x57000000, 0x10000)
    u.mem_map(0x58000000, 0x10000)
    for offset, value in settings.items():
        u.mem_write(HEADER + offset, struct.pack('<I', value))
    u.mem_write(0x55080000, bytes([y]) * 256)
    u.mem_write(0x55090000, chroma * (64 if half else 128))
    plane = struct.pack('<10I', 0x55080000, 0x55090000, 16, 16, 16, half, 0, 0, 16, 16)
    u.mem_write(0x58000000, plane + bytes(120) + struct.pack('<4I', 1, 0, 0, 0x55040000))
    stop = 0x58008000
    for reg, value in [(UC_ARM_REG_R0, HEADER), (UC_ARM_REG_R1, 0x58000000), (UC_ARM_REG_SP, 0x5700fff0), (UC_ARM_REG_LR, stop)]:
        u.reg_write(reg, value)
    u.reg_write(UC_ARM_REG_C1_C0_2, 0xf << 20); u.reg_write(UC_ARM_REG_FPEXC, 0x40000000)
    u.emu_start(symbols[symbol], stop, count=200000000)
    assert u.reg_read(UC_ARM_REG_PC) == stop, 'Instruction budget exhausted'
    assert u.reg_read(UC_ARM_REG_SP) == 0x5700fff0
    return bytes(u.mem_read(0x55080000, 256)), bytes(u.mem_read(0x55090000, 128 if half else 256))

cases = 0
for level in range(13):
    for y in [0, 48, 96, 200]:
        expected = y if not level or y >= 96 else y + (([0,4,8,12,1,2,3,5,6,7,9,10,11][level]) * (96-y)**2 + 4608)//9216
        out, c = run('black_floor_encode', y, b'\x80\x80', {256: level})
        assert out == bytes([expected])*256 and c == b'\x80\x80'*128
        cases += 1
# Modern percentage curves must reach a visibly matte endpoint without
# changing neutral midtones, on the actual compiled ARM encoder.
for step in range(1,21):
    for y in [0,16,48,96,128,200,255]:
        expected=y if y>=128 else y+(step*48*(128-y)**2+163840)//327680
        out,c=run('black_floor_encode',y,b'\x80\x80',{256:32+step})
        assert out==bytes([expected])*256 and c==b'\x80\x80'*128
        cases+=1
# Warm Cream must produce a tint at the first step, preserve middle neutrals,
# and produce strong endpoints at maximum, in both chroma layouts.
for half in [0,1]:
    for step in range(1,21):
        for y in [0,56,128,208,255]:
            sw=(112-y)**2*256//12544 if y<112 else 0
            hw=(y-144)**2*256//12321 if y>144 else 0
            def nearest(value):
                return (1 if value>=0 else -1)*((abs(value)+12800)//25600)
            expected=bytes([128+nearest((-4*sw-6*hw)*step*5*4),128+nearest((4*sw+2*hw)*step*5*4)])
            out,c=run('split_toning_encode',y,b'\x80\x80',{260:13,308:step+1},half)
            assert out==bytes([y])*256 and c==expected*(64 if half else 128)
            cases+=1
for half in [0, 1]:
    for y, expected in [(0, (134,124)), (128, (128,128)), (255, (124,134))]:
        out, c = run('split_toning_encode', y, b'\x80\x80', {260:1}, half)
        assert out == bytes([y])*256 and c == bytes(expected)*(64 if half else 128)
        cases += 1
# Independent reference for one range: red centre, -50% saturation.
def red_half(cb, cr):
    u, v = cb-128, cr-128
    along = -82*u + 243*v; cross = abs(243*u + 82*v)
    if along <= 0 or cross*8 >= along*5:
        return bytes([cb, cr])
    weight = 256 - cross*2048//(along*5)
    # The C implementation truncates signed divisions towards zero.
    nu, nv = int(u/2), int(v/2)
    return bytes([cb + int((nu-u)*weight/256), cr + int((nv-v)*weight/256)])
for half in [0, 1]:
    for cb, cr in [(85,255), (43,21), (128,128)]:
        out, c = run('colour_range_encode', 76, bytes([cb,cr]), {264:1,268:2,272:0}, half)
        assert out == bytes([76])*256 and c == red_half(cb,cr)*(64 if half else 128)
        cases += 1
# New shoulder and independently weighted brightness, both chroma formats.
for half in [0, 1]:
    for level in range(13):
        for y in [0, 176, 200, 255]:
            expected = y if not level or y <= 176 else y - (([0,4,8,12,1,2,3,5,6,7,9,10,11][level])*2*(y-176)**2+3120)//6241
            out, c = run('highlight_rolloff_encode', y, b'\x80\x80', {280:level}, half)
            assert out == bytes([expected])*256 and c == b'\x80\x80'*(64 if half else 128)
            cases += 1
    for code, step in [(0,0),(1,-20),(2,-10),(3,10),(4,20),(5,-5),(6,5),(7,-15)]:
        for cb, cr in [(85,255),(43,21),(128,128)]:
            u,v=cb-128,cr-128
            along=-82*u+243*v; cross=abs(243*u+82*v)
            weight=0 if along<=0 or cross*8>=along*5 else 256-cross*2048//(along*5)
            expected=100+int(step*weight/256)
            out,c=run('colour_range_encode',100,bytes([cb,cr]),{264:1,276:code},half)
            assert out==bytes([expected])*256 and c==bytes([cb,cr])*(64 if half else 128)
            cases+=1
# New versioned colour values, including the extra saturation word for Magentas.
directions=[(-82,243),(-193,168),(-253,42),(-159,-200),(82,-243),(253,-42),(159,200)]
for range_index in [0,6]:
 for percent,angle in [(-75,-20),(-5,-5),(5,5),(50,20)]:
  cb,cr=(85,255) if not range_index else (207,228)
  u0,v0=cb-128,cr-128;dx,dy=directions[range_index]
  along=u0*dx+v0*dy;cross=abs(u0*dy-v0*dx)
  weight=0 if along<=0 or cross*8>=along*5 else 256-cross*2048//(along*5)
  scale=256+((-1 if percent<0 else 1)*((abs(percent)*256+50)//100))
  t=angle//5+4;cs=[241,247,252,255,256,255,252,247,241][t];sn=[-88,-66,-44,-22,0,22,44,66,88][t]
  nu=int((u0*cs-v0*sn)*scale/65536);nv=int((u0*sn+v0*cs)*scale/65536)
  expected=bytes([max(0,min(255,cb+int((nu-u0)*weight/256))),max(0,min(255,cr+int((nv-v0)*weight/256)))])
  sc=-percent//5 if percent<0 else 15+percent//5;hc=-angle//5 if angle<0 else 4+angle//5
  low=(1<<28)|(sc<<(range_index*5) if range_index<4 else 0)
  upper=sc<<((range_index-4)*5) if range_index>=4 else 0
  settings={264:range_index+1,268:low,272:(1<<28)|(hc<<(range_index*4)),304:upper}
  for half in [0,1]:
   out,c=run('colour_range_encode',76,bytes([cb,cr]),settings,half)
   assert out==bytes([76])*256 and c==expected*(64 if half else 128)
   cases+=1
print(json.dumps({'compiled_ARM_colour_encoders':'pass','reference_comparisons':cases,'hardware_test':False}))
