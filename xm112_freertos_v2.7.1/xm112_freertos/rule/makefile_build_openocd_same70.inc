
openocd_gpnvm_fix:
	$(OPENOCD) $(OPENOCD_CONFIG) $(OPENOCD_CMDS) -c init -c targets -c "reset halt" \
		-c "atsamv gpnvm set 1" \
		-c "atsamv gpnvm clear 7" \
		-c "atsamv gpnvm clear 8" \
		-c "atsamv gpnvm show all" \
		-c shutdown

openocd_enable_service_mode:
	$(OPENOCD) $(OPENOCD_CONFIG) $(OPENOCD_CMDS) -c init -c targets \
		-c "mww 0x400E18A4 0xACC01CED" -c "mdw 0x400E1890 8" \
		-c shutdown

openocd_disable_service_mode:
	$(OPENOCD) $(OPENOCD_CONFIG) $(OPENOCD_CMDS) -c init -c targets \
		-c "mww 0x400E18A4 0x00000000" -c "mdw 0x400E1890 8" \
		-c shutdown

openocd_read_gpbr:
	$(OPENOCD) $(OPENOCD_CONFIG) $(OPENOCD_CMDS) -c init -c targets -c "reset halt" \
		-c "mdw 0x400E1890 8" \
		-c shutdown

openocd_clear_gpbr:
	$(OPENOCD) $(OPENOCD_CONFIG) $(OPENOCD_CMDS) -c init -c targets \
		-c "mww 0x400E1890 0x00000000" \
		-c "mww 0x400E1894 0x00000000" \
		-c "mww 0x400E1898 0x00000000" \
		-c "mww 0x400E189C 0x00000000" \
		-c "mww 0x400E18A0 0x00000000" \
		-c "mww 0x400E1894 0x00000000" \
		-c "mww 0x400E1898 0x00000000" \
		-c "mww 0x400E189C 0x00000000" \
		-c "mdw 0x400E1890 8" \
		-c shutdown
