
BUILD_ALL += $(OUT_DIR)/ref_app_smart_presence_embedded_xm112_a111_r2c.hex

$(OUT_DIR)/ref_app_smart_presence_embedded_xm112_a111_r2c.hex : \
					$(OUT_OBJ_DIR)/ref_app_smart_presence.o \
					libacc_detector_presence.a \
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

flash_ref_app_smart_presence_embedded_xm112_a111_r2c:
	openocd -d2 $(OPENOCD_CONFIG) -c "program $(OUT_DIR)/ref_app_smart_presence_embedded_xm112_a111_r2c.hex verify reset exit"
