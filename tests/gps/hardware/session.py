import argparse
from pathlib import Path
import re
import time
import serial
from serial.tools import list_ports

parser = argparse.ArgumentParser()
parser.add_argument('--seconds', type=float, default=20)
parser.add_argument('--command', action='append', default=[])
parser.add_argument('--name', default='initial-harness')
args = parser.parse_args()
root = Path(__file__).parent
ports = [p for p in list_ports.comports() if p.vid == 0x2e8a and p.pid == 0x000f]
assert len(ports) == 1, [(p.device, p.vid, p.pid) for p in list_ports.comports()]
print('Opening', ports[0].device, flush=True)
data = bytearray()
pending = bytearray()
with serial.Serial(ports[0].device, 115200, timeout=0.2) as link:
    for command in args.command:
        link.write((command+'\n').encode())
    start = time.monotonic()
    with (root/(args.name+'.log')).open('wb') as log:
        while time.monotonic() - start < args.seconds:
            chunk = link.read(4096)
            if not chunk:
                continue
            data.extend(chunk)
            log.write(chunk)
            log.flush()
            pending.extend(chunk)
            while b'\n' in pending:
                line, _, pending = pending.partition(b'\n')
                print(line.decode('utf-8', errors='replace').rstrip(), flush=True)
print('Saved', len(data), 'bytes to', args.name+'.log', flush=True)
