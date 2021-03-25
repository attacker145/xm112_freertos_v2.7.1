BUILD_LIBS += $(OUT_LIB_DIR)/libfreertos.a

vpath %.h freertos/Source/include
vpath %.c freertos/Source
vpath %.c freertos/Source/portable/MemMang

CFLAGS += -Ifreertos/Source/include

# CM3 port is used in case of CM3
# Warning: Not tested on target HW
ifeq ($(TARGET_ARCHITECTURE),cm3)
vpath %.h freertos/Source/portable/GCC/ARM_CM3
vpath %.c freertos/Source/portable/GCC/ARM_CM3

CFLAGS += -Ifreertos/Source/portable/GCC/ARM_CM3
endif

# CM3 port is used in case of CM4 without FPU
ifeq ($(TARGET_ARCHITECTURE),cm4)
vpath %.h freertos/Source/portable/GCC/ARM_CM3
vpath %.c freertos/Source/portable/GCC/ARM_CM3

CFLAGS += -Ifreertos/Source/portable/GCC/ARM_CM3
endif

# CM4F port is used in case of CM4 with FPU
# Warning: Not tested on target HW and may only be
# activated with a specific target architecture or
# by replacing the ARM_CM3 port above for the cm4 target
#ifeq ($(TARGET_ARCHITECTURE),cm4)
#vpath %.h freertos/Source/portable/GCC/ARM_CM4F
#vpath %.c freertos/Source/portable/GCC/ARM_CM4F

#CFLAGS += -Ifreertos/Source/portable/GCC/ARM_CM4F
#endif

# CM7 r0p1 port is used in case of CM7, as recommended by FreeRTOS developers
ifeq ($(TARGET_ARCHITECTURE),cm7)
vpath %.h freertos/Source/portable/GCC/ARM_CM7/r0p1
vpath %.c freertos/Source/portable/GCC/ARM_CM7/r0p1

CFLAGS += -Ifreertos/Source/portable/GCC/ARM_CM7/r0p1
endif

$(OUT_LIB_DIR)/libfreertos.a : $(OUT_OBJ_DIR)/list.o $(OUT_OBJ_DIR)/queue.o $(OUT_OBJ_DIR)/tasks.o $(OUT_OBJ_DIR)/port.o $(OUT_OBJ_DIR)/timers.o $(OUT_OBJ_DIR)/heap_5.o $(OUT_OBJ_DIR)/acc_heap.o
	@echo "    Creating archive $(notdir $@)"
	$(SUPPRESS)rm -f $@
	$(SUPPRESS)$(TOOLS_AR) $(ARFLAGS) $@ $^
