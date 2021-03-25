# OpenOCD

EXAMPLE_DETECTOR_DISTANCE_RECORDED := example_detector_distance_recorded
OPENOCD                            := openocd

# General make

BUILD_ALL += $(OUT_DIR)/$(EXAMPLE_DETECTOR_DISTANCE_RECORDED)_xm112_a111_r2c.hex

$(OUT_DIR)/$(EXAMPLE_DETECTOR_DISTANCE_RECORDED)_xm112_a111_r2c.hex : \
					$(OUT_OBJ_DIR)/$(EXAMPLE_DETECTOR_DISTANCE_RECORDED).o \
					libacc_detector_distance.a \
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

flash_$(EXAMPLE_DETECTOR_DISTANCE_RECORDED)_xm112_a111_r2c:
	$(OPENOCD) -d2 $(OPENOCD_CONFIG) -c "program $(OUT_DIR)/$(EXAMPLE_DETECTOR_DISTANCE_RECORDED)_xm112_a111_r2c.hex verify reset exit"
