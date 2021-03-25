
REF_APP_RF_CERTIFICATION_TEST := ref_app_rf_certification_test
OPENOCD := openocd

# General make

BUILD_ALL += $(OUT_DIR)/$(REF_APP_RF_CERTIFICATION_TEST)_xm112_a111_r2c.hex

# General make

BUILD_ALL += $(OUT_DIR)/$(REF_APP_RF_CERTIFICATION_TEST)_xm112_a111_r2c.hex

$(OUT_DIR)/$(REF_APP_RF_CERTIFICATION_TEST)_xm112_a111_r2c.hex : \
					$(OUT_OBJ_DIR)/ref_app_rf_certification_test.o \
					libacc_rf_certification_test.a \
					libacconeer.a \
					libcustomer.a \
					$(OUT_OBJ_DIR)/acc_board_a1r2_xm112.o \
					$(OUT_OBJ_DIR)/start_$(TARGET_OS).o

	@echo "    Linking $(notdir $@)"
	$(SUPPRESS)$(LINK.o) -Wl,--start-group $^ $(LDLIBS) -Wl,--end-group -Wl,-Map=$(basename $@).map,--cref $(LDLIBS) -o $(basename $@).elf
	$(SUPPRESS)$(OBJCOPY) -O ihex $(basename $@).elf $@
	$(SUPPRESS)$(OBJCOPY) -O binary $(basename $@).elf $(basename $@).bin
	$(SUPPRESS)$(OBJDUMP) -h -S $(basename $@).elf > $(basename $@).lss
	$(SUPPRESS)$(SIZE) -t $(basename $@).elf > $(basename $@)_size.txt

# Programming

flash_ref_app_rf_certification_test_xm112:
	$(OPENOCD) $(OPENOCD_CONFIG) -c "program $(OUT_DIR)/$(REF_APP_RF_CERTIFICATION_TEST)_xm112_a111_r2c.hex verify exit"
