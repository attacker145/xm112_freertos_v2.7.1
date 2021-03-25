CFLAGS-$(OUT_OBJ_DIR)/acc_wrap_assert.o += -Wno-missing-prototypes -fno-lto

LDFLAGS += -Wl,--wrap=__assert -Wl,--wrap=__assert_func

$(OUT_LIB_DIR)/libwrapassert.a :  $(OUT_OBJ_DIR)/acc_wrap_assert.o
	@echo "    Creating archive $(notdir $@)"
	$(SUPPRESS)rm -f $@
	$(SUPPRESS)$(TOOLS_AR) $(ARFLAGS) $@ $^

BUILD_LIBS += $(OUT_LIB_DIR)/libwrapassert.a

LDLIBS += -lwrapassert
