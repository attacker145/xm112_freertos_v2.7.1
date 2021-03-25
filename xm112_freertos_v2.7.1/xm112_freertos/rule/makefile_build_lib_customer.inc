BUILD_LIBS += $(OUT_LIB_DIR)/libcustomer.a

$(OUT_LIB_DIR)/libcustomer.a : $(addprefix $(OUT_OBJ_DIR)/,$(notdir $(patsubst %.c,%.o,$(sort $(wildcard source/acc_driver_*.c))))) \
		    $(addprefix $(OUT_OBJ_DIR)/,$(notdir $(patsubst %.c,%.o,$(sort $(wildcard source/acc_device_*.c))))) \
		    $(addprefix $(OUT_OBJ_DIR)/,$(notdir $(patsubst %.c,%.o,$(sort $(wildcard source/acc_heap_*.c))))) \
		    $(addprefix $(OUT_OBJ_DIR)/,$(notdir $(patsubst %.c,%.o,$(sort $(wildcard source/acc_log*.c))))) \
		    $(addprefix $(OUT_OBJ_DIR)/,$(notdir $(patsubst %.c,%.o,$(sort $(wildcard source/acc_hal_integration_*.c))))) \
		    $(addprefix $(OUT_OBJ_DIR)/,$(notdir $(patsubst %.c,%.o,$(sort $(wildcard source/acc_app_integration_*.c)))))
	@echo "    Creating archive $(notdir $@)"
	$(SUPPRESS)rm -f $@
	$(SUPPRESS)$(TOOLS_AR) $(ARFLAGS) $@ $^
