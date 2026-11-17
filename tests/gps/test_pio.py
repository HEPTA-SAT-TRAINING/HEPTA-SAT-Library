"""Instruction-level check of the committed PIO header (not a silicon test).

FIFO is assumed to drain immediately. This checks sampling, byte placement,
back-to-back frames, baud mismatch and bad-stop/BREAK recovery only.
"""
from pathlib import Path
import re

header = Path(__file__).resolve().parents[2] / 'drv/gps_uart_rx.pio.h'
program = [int(v, 16) for v in re.findall(r'^\s*(0x[0-9a-fA-F]+),', header.read_text(), re.M)]
wrap = int(re.search(r'#define hepta_gps_uart_rx_wrap (\d+)', header.read_text())[1])

def run(bits, period=8.0):
    def pin(t):
        bit = int(t / period)
        return bits[bit] if bit < len(bits) else 1

    pc = x = isr = t = 0
    result = []
    while t < len(bits) * period + 32:
        insn = program[pc]
        op, delay = insn >> 13, (insn >> 8) & 31
        next_pc = 0 if pc == wrap else pc + 1
        if op == 1:  # WAIT pin polarity
            assert ((insn >> 5) & 3) == 1
            if pin(t) != ((insn >> 7) & 1):
                t += 1
                continue
        elif op == 7:  # SET x
            assert ((insn >> 5) & 7) == 1
            x = insn & 31
        elif op == 2:  # IN pins, 1; right shift
            assert (insn & 255) == 1
            isr = (isr >> 1) | (pin(t) << 31)
        elif op == 0:  # JMP, JMP x--, JMP pin
            condition = (insn >> 5) & 7
            if condition == 0:
                take = True
            elif condition == 2:
                take = x != 0
                x = (x - 1) & 0xffffffff
            elif condition == 6:
                take = bool(pin(t))
            else:
                raise AssertionError('unexpected JMP condition')
            if take:
                next_pc = insn & 31
        elif op == 5:  # MOV isr, null
            assert (insn & 255) == 0xc3
            isr = 0
        elif op == 4:  # PUSH block
            assert (insn & 255) == 0x20
            result.append(isr >> 24)
            isr = 0
        else:
            raise AssertionError('unexpected PIO instruction')
        t += 1 + delay
        pc = next_pc
    return result

def frame(byte, stop=1):
    return [0] + [(byte >> i) & 1 for i in range(8)] + [stop]

values = list(range(256))
bits = [1] * 2 + sum((frame(v) for v in values), [])
for period in (8.0, 7.8, 8.2):
    assert run(bits, period) == values, period

assert run([1]*2 + frame(ord('A'), 0) + [0]*30 + [1]*4 + frame(ord('$'))) == [0, ord('$')]
assert run([0]*40 + [1]*4 + frame(ord('$'))) == [0, ord('$')]
print('PASS: PIO model: 256 back-to-back bytes at 3 baud ratios, bad stop and BREAK recovery.')
