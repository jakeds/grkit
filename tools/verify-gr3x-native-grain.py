#!/usr/bin/env python3
"""Offline ARM verification for Soft's GR IIIx 1.60 hooks.

Requires Python unicorn. Inputs: privately unpacked GR IIIx 1.60 RTOS and
build/soft-runtime-final/objects/module-framing.elf. No firmware is distributed.
OS time and local-calendar services are substituted; this is not a device test.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_CODE
from unicorn.arm_const import *

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('rtos', type=Path)
parser.add_argument('runtime', type=Path)
parser.add_argument('--baseline', type=Path, help='Previous runtime ELF; compare default noise byte-for-byte')
args = parser.parse_args()
firmware = args.rtos.read_bytes()
assert hashlib.sha256(firmware).hexdigest() in {'0f11b0d28bc67932d044c5d01bc0314844a756057c89984278c73a23449a24c6', '8a4eafcad2716e0b21e9909264e1242e534f0e39e05cba82a53546ceb4070b67'}
# Pin the input container's already verified RTOS via hook words and exact size.
assert len(firmware) == 17520064
assert firmware[0x68a970:0x68a974].hex() == '4c001be5'
assert firmware[0x67ae28:0x67ae2c].hex() == '020055e3'
HEADER = 0x55030000
def load_runtime(path):
    elf = path.read_bytes()
    shoff, shsize, count, strings_index = struct.unpack_from('<I', elf, 32)[0], *struct.unpack_from('<HHH', elf, 46)
    sections = [struct.unpack_from('<10I', elf, shoff + i * shsize) for i in range(count)]
    def data(section):
        return elf[section[4]:section[4] + section[5]]
    def string(table, offset):
        return table[offset:table.index(b'\0', offset)].decode()
    names = data(sections[strings_index])
    text_index = next(i for i, section in enumerate(sections) if string(names, section[0]) == '.text')
    text = bytearray(data(sections[text_index]))
    base = sections[text_index][3]
    for i in range(0, len(text) - 3, 4):
        if struct.unpack_from('<I', text, i)[0] == 0x58012340:
            struct.pack_into('<I', text, i, HEADER)
    symbols = {}
    for section in sections:
        if section[1] != 2:
            continue
        table = data(sections[section[6]])
        for offset in range(section[4], section[4] + section[5], section[9]):
            name, value, size, info, other, index = struct.unpack_from('<IIIBBH', elf, offset)
            if name and index == text_index:
                symbols[string(table, name)] = value
    return elf, base, text, symbols

elf, base, text, symbols = load_runtime(args.runtime)
baseline = load_runtime(args.baseline) if args.baseline else None

def branch(address, target, link):
    delta = target - address - 8
    assert delta % 4 == 0 and -(1 << 25) <= delta < (1 << 25)
    return struct.pack('<I', (0xeb000000 if link else 0xea000000) | ((delta // 4) & 0xffffff))

def engine(code, boost=0, pattern=0, runtime=None):
    _, runtime_base, runtime_text, runtime_symbols = runtime or (elf, base, text, symbols)
    u = Uc(UC_ARCH_ARM, UC_MODE_ARM)
    u.mem_map(0x53000000, 0x1200000)
    u.mem_write(0x53000000, firmware)
    u.mem_write(runtime_base, bytes(runtime_text))
    u.mem_map(0x55000000, 0x400000)
    u.mem_map(0x57000000, 0x10000)
    u.mem_map(0x58000000, 0x10000)
    u.mem_write(0x550019b0, struct.pack('<I', 0x55010000))
    u.mem_write(0x55020004, struct.pack('<I', 0x55021000))
    u.mem_write(HEADER + 72, struct.pack('<I', code))
    u.mem_write(HEADER + 312, struct.pack('<II', boost, pattern))
    u.mem_write(0x5368a970, branch(0x5368a970, runtime_symbols['grain_seed'], True))
    u.mem_write(0x5367ae28, branch(0x5367ae28, runtime_symbols['grain_amplitude'], False))
    u.reg_write(UC_ARM_REG_C1_C0_2, 0xf << 20)
    u.reg_write(UC_ARM_REG_FPEXC, 0x40000000)
    def hook(uc, address, size, user):
        if address == 0x536afbe4:
            uc.mem_write(uc.reg_read(UC_ARM_REG_R0), struct.pack('<II', 12345678, 2))
            uc.reg_write(UC_ARM_REG_R0, 1)
        elif address == 0x536bd838:
            uc.reg_write(UC_ARM_REG_R0, 0x55020000)
        elif address == 0x536b6510:
            uc.mem_write(uc.reg_read(UC_ARM_REG_R1), struct.pack('<HBBBBB', 2026, 10, 19, 12, 30, 1))
        else:
            return
        uc.reg_write(UC_ARM_REG_PC, uc.reg_read(UC_ARM_REG_LR))
    u.hook_add(UC_HOOK_CODE, hook)
    return u

def noise(u, caller=0x5367ae18):
    u.reg_write(UC_ARM_REG_SP, 0x5700fff0)
    u.reg_write(UC_ARM_REG_LR, caller)
    u.reg_write(UC_ARM_REG_R0, 0x58000000)
    u.reg_write(UC_ARM_REG_R1, 4096)
    u.emu_start(0x5368a92c, caller, count=200000)
    assert u.reg_read(UC_ARM_REG_PC) == caller
    return bytes(u.mem_read(0x58000000, 4096))

def word(u, address):
    return struct.unpack('<I', u.mem_read(address, 4))[0]

first_engine = engine(3)
a = noise(first_engine)
first_seed = word(first_engine, HEADER + 160)
b = noise(first_engine)
second_seed = word(first_engine, HEADER + 160)
assert a != b and first_seed != second_seed
assert a == noise(engine(3)), 'same time and reset sequence must reproduce the source noise'
assert word(first_engine, HEADER + 156) == 2
native = noise(engine(0))
assert native != a
# Other callers of the shared helper retain stock seed behaviour.
assert noise(engine(3), 0x53000100) == noise(engine(0), 0x53000100)
# Defaults must reproduce the released runtime's actual native noise bytes.
if baseline:
    prior=engine(3,runtime=baseline)
    assert noise(prior)==a and noise(prior)==b
fixed=engine(3,pattern=1)
fixed_a=noise(fixed)
assert noise(fixed)==fixed_a==noise(engine(3,pattern=1))
assert word(fixed,HEADER+156)==0 and word(fixed,HEADER+160)==0x534f4654
fixed.mem_write(HEADER+316,struct.pack('<I',0))
assert noise(fixed)==a and noise(fixed)==b, 'restoring Moment restores the original path'
assert noise(engine(3,pattern=999))==a, 'unknown modes fall back to Moment'
assert noise(engine(0,boost=2,pattern=1))==native, 'Off preserves stock grain'
expected = [256,64,128,256,26,51,77,102,154,179,205,230,256]
base_cases=list(enumerate(expected))+[(32+row,(row*256+10)//20) for row in range(1,21)]
cases=[]
for code,amplitude in base_cases:
    for boost in [0,1,2,999]:
        valid=1<=code<12 or 33<=code<=52
        value=(amplitude*3+1)//2 if valid and boost==1 else amplitude*2 if valid and boost==2 else amplitude
        cases.append((code,boost,value))
for code,boost,amplitude in cases:
    u=engine(code,boost=boost)
    u.reg_write(UC_ARM_REG_SP,0x5700fff0)
    u.reg_write(UC_ARM_REG_R5,1)
    for register,value in [(UC_ARM_REG_R0,111),(UC_ARM_REG_R1,222),(UC_ARM_REG_R2,333),(UC_ARM_REG_R3,444),(UC_ARM_REG_R12,555),(UC_ARM_REG_LR,666)]:
        u.reg_write(register,value)
    u.emu_start(0x5367ae28,0x5367ae40,count=10000)
    assert u.reg_read(UC_ARM_REG_R6)==amplitude,(code,boost,amplitude)
    assert u.reg_read(UC_ARM_REG_SP)==0x5700fff0
    for register,value in [(UC_ARM_REG_R0,111),(UC_ARM_REG_R1,222),(UC_ARM_REG_R2,333),(UC_ARM_REG_R3,444),(UC_ARM_REG_R12,555),(UC_ARM_REG_LR,666)]:
        assert u.reg_read(register)==value
print(json.dumps({'status':'passed','amplitude_cases':len(cases),'defaults_match_prior_noise':bool(baseline),'fixed_seed_repeats':True,'return_to_default_matches_prior':True,'off_bypasses_overrides':True,'native_pipeline_pixels_tested':False,'runtime_sha256':hashlib.sha256(elf).hexdigest()},indent=2))
