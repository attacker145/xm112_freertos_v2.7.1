# OpenOCD

OPENOCD                           := openocd

openocd:
	$(OPENOCD) $(OPENOCD_CONFIG) $(OPENOCD_CMDS) -c init -c targets -c "reset halt"

openocd_run:
	$(OPENOCD) $(OPENOCD_CONFIG) $(OPENOCD_CMDS) -c init -c targets -c "reset run" -c shutdown

openocd_halt:
	$(OPENOCD) $(OPENOCD_CONFIG) $(OPENOCD_CMDS) -c init -c targets -c "reset halt" -c shutdown

openocd_load_image:
	$(OPENOCD) $(OPENOCD_CONFIG) $(OPENOCD_CMDS) -c init -c targets \
		-c "load_image $(IMAGE) $(ADDRESS)" \
		-c shutdown
