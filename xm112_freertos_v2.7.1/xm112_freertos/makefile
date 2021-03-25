# Prevent make message "Nothing to be done for 'all'." by having a top level
# target which always does something (in this case a no-operation)
all_all : all
	@:

all :

BUILD_ALL  :=
BUILD_LIBS :=
BUILD_POST :=

CFLAGS  :=
LDFLAGS :=
LDLIBS  :=

OUT_DIR 	:= out
OUT_OBJ_DIR	:= $(OUT_DIR)/obj
OUT_LIB_DIR	:= $(OUT_DIR)/lib
VPATH		+= $(OUT_LIB_DIR)

ifneq ($(V),)
SUPPRESS :=
else
SUPPRESS := @
endif

include $(sort $(wildcard rule/makefile_target_*.inc))

TARGET := $(TARGET_OS)_$(TARGET_ARCHITECTURE)

AR      := $(TOOLS_AR)
AS      := $(TOOLS_AS)
CC      := $(CCACHE) $(TOOLS_CC)
OBJDUMP := $(TOOLS_OBJDUMP)
OBJCOPY := $(TOOLS_OBJCOPY)
SIZE    := $(TOOLS_SIZE)

VPATH   += include/ lib/ source/ user_include/ user_lib/ user_source/
CFLAGS  += -Iinclude/ -Isource/ -Iuser_include/ -Iuser_source/
LDFLAGS += -L$(OUT_LIB_DIR) -Llib/ -Luser_lib/

include $(sort $(wildcard rule/makefile_define_*.inc))
include $(sort $(wildcard rule/makefile_build_*.inc))
include $(sort $(wildcard user_rule/makefile_define_*.inc))
include $(sort $(wildcard user_rule/makefile_build_*.inc))

SOURCES := $(sort $(wildcard source/*.c)) $(sort $(wildcard user_source/*.c))
DEPENDS := $(addprefix out/, $(notdir $(SOURCES:.c=.d)))

-include $(DEPENDS)

all : $(BUILD_ALL) $(BUILD_LIBS) $(BUILD_POST)

$(DEPENDS) : | $(OUT_OBJ_DIR)

$(BUILD_LIBS) : | $(OUT_LIB_DIR)

$(BUILD_ALL) : | $(BUILD_LIBS) $(OUT_DIR)

$(BUILD_POST) : | $(BUILD_ALL)

$(OUT_DIR):
	$(SUPPRESS)mkdir -p $@

$(OUT_OBJ_DIR):
	$(SUPPRESS)mkdir -p $@

$(OUT_LIB_DIR):
	$(SUPPRESS)mkdir -p $@

$(OUT_OBJ_DIR)/%.o : %.c
	@echo "    Compiling $(notdir $<)"
	$(SUPPRESS)$(COMPILE.c) $(CFLAGS-$@) -o $@ $<

$(OUT_OBJ_DIR)/%.o : %.s
	@echo "    Assembling $(notdir $<)"
	$(SUPPRESS)$(COMPILE.s) -o $@ $<

$(OUT_OBJ_DIR)/%.o : %.S
	@echo "    Assembling $(notdir $<)"
	$(SUPPRESS)$(PREPROCESS.S) $< > $(@:.o=.s)
	$(SUPPRESS)$(COMPILE.s) -o $@ $<

.PHONY : clean
clean :
	$(SUPPRESS)rm -rf out/
	$(SUPPRESS)rm -rf utils/
