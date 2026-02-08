# RTOS-Bench SConscript for OneOS
# This file integrates RTOS-Bench into OneOS build system

Import('OS_ROOT')
Import('osconfig')
from build_tools import *

pwd = PresentDir()

# Source directories
generator_dir = pwd + '/generator'
workloads_dir = pwd + '/workloads'
platform_dir = generator_dir + '/platform/oneos'

# Include paths
path = [
    generator_dir,
    workloads_dir,
]

# Platform definition
CPPDEFINES = ['ONEOS_PLATFORM']

# Core source files
src = [
    generator_dir + '/workload_registry.c',
    generator_dir + '/workload_stub.c',
    generator_dir + '/workload_busywait.c',
    generator_dir + '/oneos_entry.c',
]

# OneOS platform layer
src += [
    platform_dir + '/timer.c',
    platform_dir + '/sync.c',
    platform_dir + '/timestamp.c',
    platform_dir + '/scheduler.c',
    platform_dir + '/signal.c',
]

# C workloads (no C++ dependency)
src += [
    workloads_dir + '/CUSUM/cusum_bench.c',
    workloads_dir + '/EWMA/ewma_bench.c',
]

group = AddCodeGroup('rtos-bench', src, depend = [''], CPPPATH = path, CPPDEFINES = CPPDEFINES)

Return('group')
