"""Run host checks using CXX (g++/clang++) or ZIG (path to zig).

No board required. One binary links the production driver/parser to fake RX
and time; another links the production DMA transport to an SDK register model.
"""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

repo = Path(__file__).resolve().parents[2]
zig = os.environ.get('ZIG')
command = [zig, 'c++'] if zig else shlex.split(os.environ.get('CXX', 'c++'))
with tempfile.TemporaryDirectory(prefix='hepta-gps-test-') as temp:
    cases = [
        ('parser-driver', 'HEPTA_GPS_HOST_TEST', 'stubs',
         ['common/gps_nmea.cpp', 'drv/gps_gp1818mk.cpp',
          'tests/gps/fake_rx.cpp', 'tests/gps/test_gps.cpp']),
        ('dma-transport', 'HEPTA_GPS_DMA_TEST', 'dma_stubs',
         ['drv/gps_dma_rx.cpp', 'drv/gps_gp1818mk.cpp', 'common/gps_nmea.cpp',
          'tests/gps/dma_stubs/fake_sdk.cpp',
          'tests/gps/test_dma.cpp']),
    ]
    for name, define, stubs, sources in cases:
        exe = Path(temp) / (name + ('.exe' if os.name == 'nt' else ''))
        args = command + ['-std=c++17', '-Wall', '-Wextra', '-Werror', '-O1', '-g', '-UNDEBUG',
                          '-D' + define, '-I', str(repo / 'tests/gps' / stubs)]
        args += [str(repo / source) for source in sources] + ['-o', str(exe)]
        subprocess.run(args, check=True)
        subprocess.run([str(exe)], check=True)
