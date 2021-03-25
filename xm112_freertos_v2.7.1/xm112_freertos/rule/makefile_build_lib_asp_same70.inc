BUILD_LIBS += \
	$(OUT_LIB_DIR)/libasp_same70.a

vpath %.c atmel_software_package/arch
vpath %.c atmel_software_package/arch/arm
vpath %.c atmel_software_package/target/samv71
vpath %.c atmel_software_package/target/samv71/toolchain
vpath %.c atmel_software_package/target/samv71
vpath %.c atmel_software_package/target/common
vpath %.c atmel_software_package/utils
vpath %.c atmel_software_package/drivers/nvm/i2c
vpath %.c atmel_software_package/drivers/nvm/flash
vpath %.c atmel_software_package/drivers/a111
vpath %.c atmel_software_package/drivers/irq
vpath %.c atmel_software_package/drivers/gpio
vpath %.c atmel_software_package/drivers/mm
vpath %.c atmel_software_package/drivers/spi
vpath %.c atmel_software_package/drivers/dma
vpath %.c atmel_software_package/drivers/peripherals
vpath %.c atmel_software_package/drivers/i2c
vpath %.c atmel_software_package/drivers/serial
vpath %.c atmel_software_package/drivers/led

CFLAGS += -Iatmel_software_package/include
CFLAGS += -Iatmel_software_package/drivers/i2c
CFLAGS += -Iatmel_software_package/drivers/spi
CFLAGS += -Iatmel_software_package/drivers/pio
CFLAGS += -Iatmel_software_package/drivers/serial
CFLAGS += -Iatmel_software_package/drivers/peripherals
CFLAGS += -Iatmel_software_package/drivers/gpio
CFLAGS += -Iatmel_software_package/drivers/mm
CFLAGS += -Iatmel_software_package/target/common
CFLAGS += -Iatmel_software_package/target/samv71
CFLAGS += -Iatmel_software_package/utils
CFLAGS += -Iatmel_software_package/target/samv71/pio
CFLAGS += -Iatmel_software_package/arch
CFLAGS += -Iatmel_software_package/drivers

CFLAGS += -DTRACE_LEVEL=5 -DVARIANT_FLASH -DCONFIG_ARCH_ARMV7M -DCONFIG_ARCH_ARM \
          -DCONFIG_SOC_SAMV71 -DCONFIG_CHIP_SAME70 -DCONFIG_CHIP_SAME70Q21 -DCONFIG_FLASH_2048KB \
          -DCONFIG_PACKAGE_144PIN -DCONFIG_BOARD_SAME70_XM112 -DCONFIG_HAVE_PIO3 \
          -DCONFIG_HAVE_GMAC_QUEUES -DCONFIG_HAVE_PMC_UPLLDIV2 -DCONFIG_HAVE_PWMC_DMA \
          -DCONFIG_HAVE_PWMC_SPREAD_SPECTRUM -DCONFIG_HAVE_PWMC_EXTERNAL_TRIGGER \
          -DCONFIG_HAVE_PWMC_FAULT_PROT_HIZ -DCONFIG_HAVE_PWMC_STEPPER_MOTOR \
          -DCONFIG_HAVE_PWMC_CMP_UNIT -DCONFIG_HAVE_PWMC_SYNC_MODE -DCONFIG_HAVE_PWMC_OOV \
          -DCONFIG_HAVE_PWMC_FMODE -DCONFIG_HAVE_PWMC_WP -DCONFIG_HAVE_PWMC_DTIME \
          -DCONFIG_HAVE_PWMC_ELINE -DCONFIG_HAVE_XDMAC -DCONFIG_HAVE_PWMC -DCONFIG_HAVE_TC_DMA_MODE \
          -DCONFIG_HAVE_SPI -DCONFIG_HAVE_A111 -DCONFIG_HAVE_TWI -DCONFIG_HAVE_I2C_BUS \
          -DCONFIG_HAVE_TWI_AT24 -DCONFIG_DRV_AT24 -DCONFIG_HAVE_LED -DCONFIG_HAVE_MPU \
          -DCONFIG_HAVE_L1CACHE -DCONFIG_HAVE_NVIC -DCONFIG_HAVE_EEFC -DCONFIG_HAVE_SPI_BUS \
          -DCONFIG_HAVE_UART -DCONFIG_HAVE_SERIALD_UART -DCONFIG_HAVE_USART \
          -DCONFIG_HAVE_SERIALD_USART -DSOFTPACK_VERSION="\"2.13\"" -DCONFIG_TIMER_POLLING -DSAME70

CONFIG_CHIP_SAME70Q21 = y
CONFIG_HAVE_PWMC_DMA = y
CONFIG_HAVE_USART = y
CONFIG_HAVE_PWMC_SPREAD_SPECTRUM = y
CONFIG_HAVE_TWI_AT24 = y
CONFIG_HAVE_PWMC_FAULT_PROT_HIZ = y
CONFIG_HAVE_A111 = y
CONFIG_HAVE_TWI = y
CONFIG_HAVE_XDMAC = y
CONFIG_BOARD_SAME70_XM112 = y
CONFIG_HAVE_MPU = y
CONFIG_HAVE_PWMC_CMP_UNIT = y
CONFIG_ARCH_ARM = y
CONFIG_SERIALD_USART = y
CONFIG_HAVE_PWMC_DTIME = y
CONFIG_HAVE_PWMC_WP = y
CONFIG_HAVE_UART = y
CONFIG_HAVE_PIO3 = y
CONFIG_DRV_A111 = y
CONFIG_A111 = y
CONFIG_LIB_ACCONEER = y
CONFIG_PACKAGE_144PIN = y
CONFIG_HAVE_MPDDRC_SDRAM = y
CONFIG_HAVE_MPDDRC_SDRAM_ONLY = y
CONFIG_HAVE_SPI = y
CONFIG_HAVE_PMC_UPLLDIV2 = y
CONFIG_HAVE_PWMC_STEPPER_MOTOR = y
CONFIG_DRV_AT24 = y
CONFIG_TWI = y
CONFIG_CHIP_SAME70 = y
CONFIG_HAVE_PWMC_SYNC_MODE = y
CONFIG_LIB_FREERTOS = y
CONFIG_HAVE_SPI_BUS = y
CONFIG_HAVE_EEFC = y
CONFIG_SERIALD_UART = y
CONFIG_HAVE_PWMC_FMODE = y
CONFIG_HAVE_NVIC = y
CONFIG_HAVE_PWMC_EXTERNAL_TRIGGER = y
CONFIG_HAVE_SUPC = y
CONFIG_HAVE_PWMC_ELINE = y
CONFIG_HAVE_PWMC = y
CONFIG_HAVE_TC_DMA_MODE = y
CONFIG_TWI_AT24 = y
CONFIG_HAVE_PWMC_OOV = y
CONFIG_FLASH_2048KB = y
CONFIG_HAVE_L1CACHE = y
CONFIG_SPI = y
CONFIG_ARCH_ARMV7M = y
CONFIG_HAVE_GMAC_QUEUES = y
CONFIG_SOC_SAMV71 = y
CONFIG_HAVE_I2C_BUS = y
CONFIG_HAVE_LED = y
CONFIG_TIMER_POLLING = y

TOP=atmel_software_package/

INC_FILES = $(shell if [ -d "atmel_software_package" ]; then find atmel_software_package/ -type f -name 'Makefile.inc'; fi)
include $(INC_FILES)

OBJ_FILES = $(notdir $(drivers-y))
OBJ_FILES += $(notdir $(utils-y))
OBJ_FILES += $(notdir $(target-y))
OBJ_FILES += $(notdir $(gnu-cstartup-y))
OBJ_FILES += $(notdir $(ARCH_OBJS))

OBJ_FILES := $(addprefix $(OUT_OBJ_DIR)/,$(OBJ_FILES))

$(OUT_LIB_DIR)/libasp_same70.a : $(OBJ_FILES)
	@echo "    Creating archive $(notdir $@)"
	$(SUPPRESS)rm -f $@
	$(SUPPRESS)$(TOOLS_AR) $(ARFLAGS) $@ $^
