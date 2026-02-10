# RTOS-Bench Top-level SConscript
from building import *
import os

cwd = GetCurrentDir()
objs = []

objs += SConscript(os.path.join('generator', 'SConscript'))
objs += SConscript(os.path.join('workloads', 'SConscript'))

Return('objs')
