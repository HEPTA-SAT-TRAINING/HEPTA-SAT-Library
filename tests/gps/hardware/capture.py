import argparse
from pathlib import Path
import re
import time
from collections import Counter
import serial
from serial.tools import list_ports

parser = argparse.ArgumentParser()
parser.add_argument('--seconds', type=float, default=20)
parser.add_argument('--name', default='raw-lab4-05')
args = parser.parse_args()
root = Path(__file__).parent
ports = [p for p in list_ports.comports() if p.vid == 0x2e8a and p.pid == 0x000f]
assert len(ports) == 1, [(p.device, p.vid, p.pid) for p in list_ports.comports()]
port = ports[0].device
print('Capturing', port, args.seconds, 'seconds', flush=True)
data = bytearray()
start = time.monotonic()
with serial.Serial(port, 115200, timeout=0.2) as link:
    while time.monotonic() - start < args.seconds:
        data.extend(link.read(8192))
(root/(args.name+'.log')).write_bytes(data)
lines = re.findall(rb'\$[A-Z]{5},[^\r\n]*', data)
valid = bad = 0
types = Counter()
fixes = Counter()
for line in lines:
    try:
        body, check = line[1:].rsplit(b'*', 1)
        actual = 0
        for byte in body:
            actual ^= byte
        if len(check) != 2 or actual != int(check, 16):
            bad += 1
            continue
    except (ValueError, IndexError):
        bad += 1
        continue
    valid += 1
    fields = body.split(b',')
    kind = fields[0].decode()
    types[kind] += 1
    if kind.endswith('GGA'):
        fixes['GGA quality '+fields[6].decode()] += 1
    if kind.endswith('RMC'):
        fixes['RMC status '+fields[2].decode()] += 1
print('Bytes:', len(data), 'valid:', valid, 'bad:', bad, 'types:', dict(types), 'fixes:', dict(fixes))
print(data.decode('utf-8', errors='replace')[-3000:])
