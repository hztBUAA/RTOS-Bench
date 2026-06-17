################################################################################
# Example config_os.mk for Dongtu/Intewell projects.
#
# Copy this file to the Intewell project root and rename it to config_os.mk.
################################################################################

RTOS_BENCH_ROOT := A:/IntelWell-IDE/eclipse/workspace/rtos_bench/RTOS-Bench

# x86/i386 project:
ARCH := _X86_

# Orange Pi / vm_3588 project:
# ARCH := __ARM64__

include $(RTOS_BENCH_ROOT)/platforms/dongtu/intewell.mk
