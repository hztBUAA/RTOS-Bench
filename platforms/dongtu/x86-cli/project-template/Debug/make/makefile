################################################################################
# Automatically-generated file. Do not edit!
################################################################################
-include ../config_components.mk
-include ./sources.mk
-include ./objects.mk
-include ./v_makefile.mk
-include ../bin_to_inte.mk
-include $(CONFIG_PATH)/make/preandpost.mk

#输出所有变量
.EXPORT_ALL_VARIABLES:

################################makefile公共变量(宏)部分################################
RTOS:=RTCore
RTOS_MAKE_PATH:=$(PLATFORM)/template/$(RTOS)/make

-include $(RTOS_MAKE_PATH)/util.make

################################公共工具链部分################################

#Intewell交叉工具链可执行文件统一前缀
#自定义编译参数
USER_OPTION :=

CP_FILE := *.a

PUB_BIN_PATH:=$(PLATFORM)/host/pub
IMGINFOGEN :=$(PUB_BIN_PATH)/ImgInfoGen.exe
CONFIG_IMG_TYPE:= os
PARTITION_OS:= ttos

# 启动代码路径
VM_CRT0_PATH = $(RTOS_PATH)/bsp/board/$(CONFIG_ARCH)/start/partition/$(PARTITION_OS)
VM_BOARD_PATH = $(RTOS_PATH)/bsp/board/$(CONFIG_ARCH)/$(COMPANY)/$(BOARD_SUFFIX)/partition/$(PARTITION_OS)
CRT0 = $(VM_CRT0_PATH)/crt0.o $(VM_BOARD_PATH)/board_crt0.o
TTOS_INIT0 := $(RTOS_PATH)/init/partition/ttos/ttosInit.o

#统一使用base.make定义的$(CFLAGS)
-include $(RTOS_PATH)/base.make
USER_OPTION += $(CFLAGS)

# All of the sources participating in the build are defined here
-include $(SUBDIR:%=%/subdir.mk)

-include $(PROJECT_PATH)/config_os.mk

ALL +=app
CLEAN +=app_clean
CLEAN_OBJ +=app_clean_obj

#default link library
ifeq ($(CONFIG_TTOS), 1)
    LIBS += -lgcc -lcommon -lrtl -lshell -lvbsp -lvbspEx -lshm -lbsp -ldm -lrootfs -lfatfs -lyaffs -llwip -llwip_interface -lvmdevinfo -ljson -ldeadlock -lftp -lvnet -lsmipc -ldfs -lrpc -lnfs -lbsp_pci -lsoem -ligh -lethercat
else
	LIBS += -lgcc -lrtl -lvbsp -lvbspEx -lcommon
endif

ifeq ($(CONFIG_CPLUSPLUS), 1)
	LIBS += -lstdc++
endif
	
ifeq ($(CONFIG_GJB7714), 1)
	ifeq ($(CONFIG_TTOS), 1)
		LIBS += -lttos_GJB7714
	endif
else
	ifeq ($(CONFIG_TTOS), 1)
		LIBS += -lttos
	endif
endif

ifeq ($(CONFIG_ARCH), mips) 
	ifeq ($(CONFIG_TTOS), 1)
		LIBS += -lusb
	endif
endif

# All Target
all:$(ALL)
clean: crt0_clean TTOS_INIT0_clean $(CLEAN)
clean_obj: crt0_clean TTOS_INIT0_clean $(CLEAN_OBJ)

$(CRT0):CRT0_create
$(TTOS_INIT0):TTOS_INIT0_create

CRT0_create:
	-@#生成linkcmds
	$(MAKE) -C $(CONFIG_MAKEFILE_PATH) GenerateAppLinkcmds CONFIG_PROJ=$(CONFIG_PATH) IMAGE_NAME=$(PROJECT_NAME)
	$(MAKE) -C $(VM_CRT0_PATH)
	$(MAKE) -C $(VM_BOARD_PATH)

crt0_clean:
	$(MAKE) -C $(VM_CRT0_PATH) clean		
	$(MAKE) -C $(VM_BOARD_PATH) clean		

TTOS_INIT0_create:
	$(MAKE) -C $(RTOS_PATH)/init/partition/ttos
    
TTOS_INIT0_clean:
	$(MAKE) -C $(RTOS_PATH)/init/partition/ttos clean	

app: 
	-@#因为crt0和init目录为公共代码，所以每次构建应用项目时需清理否则编译不同体系的应用时会报无法识别指令错误
	-@#crt0_clean、 TTOS_INIT0_clean和$(EXECUTABLES).$(TARGET_EXT)不能并行否则会导致链接时报文件crt0.o和ttosInit.o文件已删除错误
	$(MAKE) crt0_clean
	$(MAKE) TTOS_INIT0_clean
	$(MAKE) $(EXECUTABLES).$(TARGET_EXT)

app_clean:
	-$(RM) $(OBJS) $(ARCHIVES) $(DEPS) $(EXECUTABLES).$(TARGET_EXT)
	-$(RM) $(CONFIG_PATH)/_linkcmds $(CONFIG_PATH)/catlinkcmds $(CONFIG_PATH)/config_*.h $(CONFIG_PATH)/config_*.mk $(CONFIG_PATH)/imgHeader.h $(CONFIG_PATH)/prelinkcmds
	-@$(BIN_PATH)/echo ' '

app_clean_obj:
	-$(RM) $(OBJS) $(ARCHIVES) $(DEPS) $(EXECUTABLES).$(TARGET_EXT)
	-@$(BIN_PATH)/echo ' '

# Tool invocations
$(EXECUTABLES).elf : $(CRT0) $(TTOS_INIT0) $(ARCHIVES)
	@$(BIN_PATH)/echo 'Building target: $@'
	@$(BIN_PATH)/echo 'Invoking: GCC C Linker'
	$(LD) $(LDFLAGS) $(LIBS_PATH) $(PREFLAGS) $(USER_OBJS) --start-group $^ $(LIBS) --end-group -o $@
	$(SIZE) -A -x $@ 1>presize.txt
	$(HOSTPUB_PATH)/parse
	$(RM) $@ presize.txt
	$(BIN_PATH)/catlinkcmds $(LINKCMDS_PATH) $(BIN_PATH)
	$(LD) $(LDFLAGS) $(LIBS_PATH) $(FLAGS) $(USER_OBJS) --start-group $^ $(LIBS) --end-group -o $@
	$(SIZE) -A -x $@ 1>presize.txt
	@$(OBJCOPY) --add-section .imgattr=ImgInfoGen $(EXECUTABLES).elf $(EXECUTABLES)_tmp.elf
	@rm $(EXECUTABLES).elf 
	@$(OBJCOPY) $(EXECUTABLES)_tmp.elf $(EXECUTABLES).elf
	@rm $(EXECUTABLES)_tmp.elf
	$(OBJCOPY) -O binary --strip-unneeded -R .note -R .comment -R .stab -R .stabstr $(EXECUTABLES).elf $(EXECUTABLES).bin
ifeq ($(COPY_TO_INTE_BIN),Y)
	@$(BIN_PATH)/bash -c "[ ! -d ${INTE_BIN_PATH} ] || ${BIN_PATH}/mkdir ${INTE_BIN_PATH}"
	@$(BIN_PATH)/cp -f $@ $(EXECUTABLES).elf $(INTE_BIN_PATH)/
	$(RM) $(RECORD_BIN_FILE_PATH)
endif
	#$(RM) $(CONFIG_PATH)/make/symTbl.*
	#$(BIN_PATH)/tclsh $(HOSTPUB_PATH)/hutils/tcl/makeSymTbl.tcl $(CONFIG_ARCH) $(EXECUTABLES).elf symTbl.c $(NM)
	#$(COMPILE_COMMAND) $(CONFIG_PATH)/make/symTbl.o $(CONFIG_PATH)/make/symTbl.c
	@$(BIN_PATH)/echo ' ' 
	@$(BIN_PATH)/echo 'Finished building target: $@'
	@$(BIN_PATH)/echo ' '

$(EXECUTABLES).bin : $(EXECUTABLES).elf
	@$(BIN_PATH)/echo 'Building target: $@'
	$(OBJCOPY) -O binary --strip-unneeded -R .note -R .comment -R .stab -R .stabstr $<  $@
ifeq ($(COPY_TO_INTE_BIN),Y)
	@$(BIN_PATH)/bash -c "[ ! -d ${INTE_BIN_PATH} ] || ${BIN_PATH}/mkdir ${INTE_BIN_PATH}"
	@$(BIN_PATH)/cp -f $@ $(INTE_BIN_PATH)/
	$(RM) $(RECORD_BIN_FILE_PATH)
endif
	@$(BIN_PATH)/echo 'Finished building target: $@'
	@$(BIN_PATH)/echo ' '
  
$(EXECUTABLES).hex : $(EXECUTABLES).elf
	@$(BIN_PATH)/echo 'Building target: $@'
	$(OBJCOPY) -O ihex --strip-unneeded -R .note -R .comment -R .stab -R .stabstr $<  $@
ifeq ($(COPY_TO_INTE_BIN),Y)
	@$(BIN_PATH)/bash -c "[ ! -d ${INTE_BIN_PATH} ] || ${BIN_PATH}/mkdir ${INTE_BIN_PATH}"
	@$(BIN_PATH)/cp -f $@ $(INTE_BIN_PATH)/
	$(RM) $(RECORD_BIN_FILE_PATH)
endif	
	@$(BIN_PATH)/echo 'Finished building target: $@'
	@$(BIN_PATH)/echo ' '

$(EXECUTABLES).exe : $(EXECUTABLES).elf
	$(BIN_PATH)/echo 'Building target: $@'
	$(OBJCOPY) -O a.out-i386 --strip-debug -R .note -R .comment -R .stab -R .stabstr -R .jcr -R .rodata.str1.4 -R .rodata.str1.1 $<  $@
ifeq ($(COPY_TO_INTE_BIN),Y)
	@$(BIN_PATH)/bash -c "[ ! -d ${INTE_BIN_PATH} ] || ${BIN_PATH}/mkdir ${INTE_BIN_PATH}"
	@$(BIN_PATH)/cp -f $@ $(INTE_BIN_PATH)/
	$(RM) $(RECORD_BIN_FILE_PATH)
endif	
	@$(BIN_PATH)/echo 'Finished building target: $@'
	@$(BIN_PATH)/echo ' '
  
$(EXECUTABLES).srec : $(EXECUTABLES).elf
	@$(BIN_PATH)/echo 'Building target: $@'
	$(OBJCOPY) -O srec --strip-unneeded -R .note -R .comment -R .stab -R .stabstr $<  $@
ifeq ($(COPY_TO_INTE_BIN),Y)
	@$(BIN_PATH)/bash -c "[ ! -d ${INTE_BIN_PATH} ] || ${BIN_PATH}/mkdir ${INTE_BIN_PATH}"
	@$(BIN_PATH)/cp -f $@ $(INTE_BIN_PATH)/
	$(RM) $(RECORD_BIN_FILE_PATH)
endif
	@$(BIN_PATH)/echo 'Finished building target: $@'
	@$(BIN_PATH)/echo ' '

$(ARCHIVES): $(OBJS)
	@$(BIN_PATH)/echo 'Invoking: GCC Archiver'
	$(AR) $(AR_FLAGS) $(ARCHIVES) @prjObjs.lst
	$(BIN_PATH)/echo 'Finished building: $@'
	@$(BIN_PATH)/echo ' '

compile_source :$(OBJS)

clean_source:

unexport  C_UPPER_SRCS  
unexport  C_SRCS  
unexport  S_UPPER_SRCS  
unexport  CPP_SRCS  
unexport  O_SRCS  
unexport  ASM_SRCS  
unexport  S_SRCS  
unexport  CXX_SRCS  
unexport  CC_SRCS  
unexport  OBJ_SRCS  
unexport  OBJS  
unexport  ARCHIVES  
unexport  DEPS  
unexport  EXECUTABLES  
unexport  MAKEFILE_LIST

.PHONY: all clean clean_obj dependents
.SECONDARY:
