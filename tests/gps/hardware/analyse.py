from pathlib import Path
from collections import Counter
import hashlib
import json
import re

root = Path(__file__).parent
def rows(name, prefix):
    lines = (root/(name+'.log')).read_text(encoding='utf-8').splitlines()
    return [dict((key, int(value)) for key, value in re.findall(r'([a-z_]+)=(-?\d+)', line))
            for line in lines if line.startswith(prefix+' ')]

raw = (root/'raw-lab4-05.log').read_bytes()
frames = re.findall(rb'\$[A-Z]{5},[^\r\n]*', raw)
types, fixes = Counter(), Counter()
bad = 0
for frame in frames:
    body, checksum = frame[1:].rsplit(b'*', 1)
    check = 0
    for byte in body:
        check ^= byte
    if len(checksum) != 2 or check != int(checksum, 16):
        bad += 1
    fields = body.split(b',')
    kind = fields[0].decode()
    types[kind] += 1
    if kind == 'GPGGA': fixes['GGA '+fields[6].decode()] += 1
    if kind == 'GPRMC': fixes['RMC '+fields[2].decode()] += 1
assert len(frames) >= 100 and bad == 0
assert fixes.get('GGA 0', 0) == types['GPGGA'] and fixes.get('RMC V', 0) == types['GPRMC']

backlog = rows('backlog-8s', 'PAUSE_END')[-1]
overrun = rows('overrun-15s', 'PAUSE_END')[-1]
restart = rows('restart-50', 'RESTART_END')[-1]
legacy = rows('legacy-api', 'LEGACY_END')[-1]
stats = rows('soak-180s', 'STATS')
states = rows('soak-180s', 'STATE')
assert backlog['overrun'] == backlog['rxerr'] == backlog['parseerr'] == 0
assert overrun['overrun'] >= 1 and overrun['rxerr'] == overrun['parseerr'] == 0
assert restart['count'] == 50 and restart['failures'] == 0
assert restart['heap_before'] == restart['heap_after']
assert legacy['gga_ok'] == legacy['rmc_ok'] == legacy['all_ok'] == 0
assert legacy['gga_unchanged'] == legacy['rmc_unchanged'] == legacy['all_unchanged'] == 1
assert len(stats) >= 175 and stats[-1]['calls'] >= 17900, 'Soak is incomplete'
assert all(s['missed'] == 0 for s in stats)
assert len({s['heap'] for s in stats}) == 1
assert all(s['state'] == 1 and s['pos'] == s['vel'] == s['quality'] == 0 for s in states)
assert all(s['overrun'] == s['rxerr'] == s['parseerr'] == 0 for s in states)
assert states[-1]['gga'] - states[0]['gga'] >= 175
assert states[-1]['rmc'] - states[0]['rmc'] >= 175
lab_text = (root/'lab4-06-unchanged.log').read_text()
example_text = (root/'final-example.log').read_text()
lab_counts = {'gga_no_fix': lab_text.count('GPGGA: no fix'), 'rmc_no_fix': lab_text.count('GPRMC: no fix')}
example_count = example_text.count('GPS: no fix')
assert min(lab_counts.values()) >= 5 and example_count >= 5
normal_stats = []
for name in ('initial-harness','backlog-8s','overrun-15s','restart-50','legacy-api','soak-180s'):
    normal_stats.extend(rows(name, 'STATS'))
summary = {
    'raw_bytes': len(raw), 'raw_frames': len(frames), 'raw_bad_checksum': bad,
    'types': dict(types), 'fixes': dict(fixes),
    'backlog_8s': backlog, 'overrun_15s': overrun, 'restart_50': restart, 'legacy': legacy,
    'unchanged_lab4_06': lab_counts, 'latest_example_no_fix_lines': example_count,
    'max_normal_poll_us': max(s['max_gps_us'] for s in normal_stats),
    'soak': {'reports': len(stats), 'calls': stats[-1]['calls'],
             'first_ms': stats[0]['ms'], 'last_ms': stats[-1]['ms'],
             'max_gps_us': max(s['max_gps_us'] for s in stats),
             'mean_gps_us': stats[-1]['mean_gps_us'],
             'max_late_us': max(s['late_us'] for s in stats),
             'missed': stats[-1]['missed'], 'heap': stats[-1]['heap'],
             'first_gga': states[0]['gga'], 'last_gga': states[-1]['gga'],
             'first_rmc': states[0]['rmc'], 'last_rmc': states[-1]['rmc']},
    'log_sha256': {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in root.glob('*.log')}
}
(root/'hardware_results.json').write_text(json.dumps(summary, indent=2), encoding='utf-8')
print(json.dumps(summary, indent=2))
