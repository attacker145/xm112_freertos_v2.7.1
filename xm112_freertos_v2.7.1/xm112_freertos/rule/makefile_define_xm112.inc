LDFLAGS += -Latmel_software_package/target/samv71/toolchain/gnu/ -Tflash.ld -nostdlib
LDLIBS  += -lasp_same70

# Remove/change some compiler options to make Atmel Software Package build
CFLAGS-$(OUT_OBJ_DIR)/acc_board_a1r2_xm112.o += -Wno-missing-prototypes
CFLAGS-$(OUT_OBJ_DIR)/acc_driver_pm_same70.o += -Wno-missing-prototypes
CFLAGS-$(OUT_OBJ_DIR)/start_freertos.o += -Wno-missing-prototypes

CFLAGS := $(filter-out -pedantic,$(CFLAGS))
CFLAGS := $(filter-out -Wextra,$(CFLAGS))
CFLAGS := $(filter-out -Wcast-qual,$(CFLAGS))
CFLAGS := $(filter-out -std=c99,$(CFLAGS))
CFLAGS += -std=gnu99
CFLAGS += -DSAME70 -DUSE_ACCONEER_TICKLESS_IDLE

ifeq ($(OPENOCD_INTERFACE),)
    OPENOCD_INTERFACE   := interface/jlink-swd.cfg
endif

OPENOCD_TARGET      := target/atsamv.cfg
OPENOCD_CONFIG      += -f $(OPENOCD_INTERFACE) -f $(OPENOCD_TARGET)
