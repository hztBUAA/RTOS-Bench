################################################################################
# Example vm_3588/config_os.mk for Dongtu/Intewell Orange Pi.
#
# Copy these lines into the vm_3588 project-local config_os.mk. The Intewell
# generated Debug/make/makefile includes it with:
#
#   -include $(PROJECT_PATH)/config_os.mk
#
# Do not commit your project-local config_os.mk into RTOS-Bench. Each developer
# should set RTOS_BENCH_ROOT to their own checkout path.
################################################################################

RTOS_BENCH_ROOT ?= C:/Users/hzt/yihui-workspace/rtos-bench/RTOS-Bench

include $(RTOS_BENCH_ROOT)/platforms/dongtu/intewell_vm3588.mk
