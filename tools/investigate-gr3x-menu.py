#!/usr/bin/env python3
"""Read-only GR IIIx 1.60 controller-routing probe. No camera connection.
Usage: python investigate-gr3x-menu.py /path/to/rtos.bin
Requires Unicorn. Firmware constructors, allocations and UI services are
stubbed: this proves dispatch decisions only, not a working screen.
"""
import hashlib
import json
import struct
import sys
from pathlib import Path
from unicorn import Uc, UC_ARCH_ARM, UC_HOOK_CODE, UC_MODE_ARM
from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC

firmware = Path(sys.argv[1]).read_bytes()
assert hashlib.sha256(firmware).hexdigest() == '0f11b0d28bc67932d044c5d01bc0314844a756057c89984278c73a23449a24c6'
BASE, OBJECT, ALLOCATION, STOP, STACK = 0x53000000, 0x55080000, 0x550c0000, 0x57008000, 0x5700fff0

def machine():
    u = Uc(UC_ARCH_ARM, UC_MODE_ARM)
    u.mem_map(BASE, (len(firmware)+4095)&~4095)
    u.mem_write(BASE, firmware)
    u.mem_map(0x55000000, 0x200000)
    u.mem_map(0x57000000, 0x10000)
    return u

def registers(u, identity):
    for reg, value in [(UC_ARM_REG_R0, OBJECT), (UC_ARM_REG_R1, identity), (UC_ARM_REG_SP, STACK), (UC_ARM_REG_LR, STOP)]:
        u.reg_write(reg, value)

def word(address):
    return struct.unpack_from('<I', firmware, address-BASE)[0]

def class_name(vtable):
    name = word(word(vtable-4)+4)-BASE
    end = firmware.index(b'\0', name)
    return firmware[name:end].decode()

# For each constructor the address below is the PC-relative vtable literal.
expected = [
    (0x2000000, 0x531a1fe4, 0x531a20e0, '17AdjModeController'),
    (0x2000012, 0x531abd40, 0x531abe08, '22ImageControlController'),
    (0x2000014, 0x531afdb8, 0x531afe44, '28ImageControlDetailController'),
    (0x200004a, 0x5319aa68, None, '17TopMenuController'),
]
controllers = []
for identity, constructor, literal, name in expected:
    u = machine()
    calls = []
    def factory_hook(emulator, address, size, data):
        if address == STOP or 0x5316f3b8 <= address < 0x53170b50:
            return
        assert address in (0x536af2a8, constructor), hex(address)
        calls.append(address)
        if address == 0x536af2a8:
            emulator.reg_write(UC_ARM_REG_R0, ALLOCATION)
        emulator.reg_write(UC_ARM_REG_PC, emulator.reg_read(UC_ARM_REG_LR))
    u.hook_add(UC_HOOK_CODE, factory_hook)
    registers(u, identity)
    u.emu_start(0x5316f3b8, STOP, count=1500)
    assert u.reg_read(UC_ARM_REG_PC) == STOP and u.reg_read(UC_ARM_REG_SP) == STACK
    assert calls == [0x536af2a8, constructor]
    if literal:
        assert class_name(word(literal)) == name
    else:
        # TopMenu's constructor uses the same PC-relative LDR pattern.
        matches = []
        for address in range(constructor, constructor+100, 4):
            instruction = word(address)
            if instruction & 0xffff0000 == 0xe59f0000:
                value = word(address+8+(instruction&4095))
                try:
                    if class_name(value) == name:
                        matches.append(value)
                except (struct.error, ValueError, UnicodeDecodeError):
                    pass
        assert matches, 'TopMenu constructor vtable not established'
    controllers.append({'controller_id':hex(identity),'constructor':hex(constructor),'class':name,'factory_dispatch':'pass'})

# The existing firmware maps physical Fn event 0x1b to action 0x16 only on
# initial phase 2 / repeat 0. Release phase 1 maps to a distinct action 0x17.
buttons = []
for phase, repeats, expected_action in [(0,0,None),(1,0,0x17),(2,0,0x16),(2,1,None),(2,10,None)]:
    u = machine()
    event, argument = 0x55090000, 0x55090100
    u.mem_write(event, struct.pack('<II',0x1b,argument))
    u.mem_write(argument, struct.pack('<II',phase,repeats))
    actions = []
    def action_hook(emulator, address, size, data):
        if address == 0x53221a1c:
            actions.append(emulator.reg_read(UC_ARM_REG_R1))
            emulator.reg_write(UC_ARM_REG_PC, emulator.reg_read(UC_ARM_REG_LR))
        elif address != STOP and not 0x532231e4 <= address < 0x53223a38:
            raise AssertionError(hex(address))
    u.hook_add(UC_HOOK_CODE, action_hook)
    registers(u,event)
    u.emu_start(0x532231e4, STOP, count=500)
    assert u.reg_read(UC_ARM_REG_PC)==STOP and u.reg_read(UC_ARM_REG_SP)==STACK
    assert actions == ([] if expected_action is None else [expected_action])
    buttons.append({'physical_event':'0x1b','phase':phase,'repeats':repeats,'actions':[hex(a) for a in actions]})
print(json.dumps({'firmware':'GR IIIx 1.60','controller_routing':controllers,'Fn_dispatch':buttons,'constructors_and_services':'stubbed','screen_rendering_tested':False,'hardware_tested':False},indent=2))
