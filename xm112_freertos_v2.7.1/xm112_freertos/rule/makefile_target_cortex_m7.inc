TOOLS_PREFIX := $(GNU_INSTALL_ROOT)/arm-none-eabi-
TOOLS_AR         := $(TOOLS_PREFIX)ar
TOOLS_AS         := $(TOOLS_PREFIX)as
TOOLS_CC         := $(TOOLS_PREFIX)gcc
TOOLS_OBJDUMP    := $(TOOLS_PREFIX)objdump
TOOLS_OBJCOPY    := $(TOOLS_PREFIX)objcopy
TOOLS_SIZE       := $(TOOLS_PREFIX)size
TOOLS_LD         := $(TOOLS_PREFIX)gcc

TARGET_ARCHITECTURE := cm7
TARGET_ARCHITECTURE_FLAGS := -mcpu=cortex-m7 -mthumb -mfloat-abi=softfp -mfpu=fpv5-sp-d16

ARFLAGS := cr

CFLAGS += \
	$(TARGET_ARCHITECTURE_FLAGS) -DTARGET_ARCH_cm7 \
	-std=c99 -pedantic -Wall -Werror -Wextra \
	-Wdouble-promotion -Wstrict-prototypes -Wcast-qual -Wmissing-prototypes -Winit-self -Wpointer-arith \
	-MMD -MP \
	-O2 -g \
	-fno-math-errno \
	-ffunction-sections -fdata-sections

# Override optimization level
ifneq ($(ACC_CFG_OPTIM_LEVEL),)
	CFLAGS  += $(ACC_CFG_OPTIM_LEVEL)
endif

ASFLAGS += $(TARGET_ARCHITECTURE_FLAGS)

LDFLAGS += \
	$(TARGET_ARCHITECTURE_FLAGS) \
	-L$(OUT_DIR) \
	-Werror \
	-Wl,--gc-sections

LDLIBS += -lc -lm -lgcc
