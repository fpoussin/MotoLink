
DRIVERSRC = drivers/drivers.c
DRIVERSRC += drivers/timcap.c
DRIVERSRC += drivers/dac.c
DRIVERSRC += drivers/iwdg.c

DRIVERINC += drivers

STM32DRIVERSRC = $(DRIVERSRC)
STM32DRIVERSRC += drivers/stm32/iwdg_lld.c
STM32DRIVERSRC += drivers/stm32/timcap_lld.c
STM32DRIVERSRC += drivers/stm32/dac_lld.c

STM32DRIVERINC = $(DRIVERINC)
STM32DRIVERINC += drivers/stm32
