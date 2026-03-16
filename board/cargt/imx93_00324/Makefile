#
# Copyright 2022 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx93_00324.o
obj-$(CONFIG_TARGET_IMX93_00324) += ../common/imx9_eeprom.o

ifdef CONFIG_SPL_BUILD
obj-y += spl-cargt.o
obj-y += cargt_lpddr4x_1gbyte_timing.o cargt_lpddr4x_2gbyte_timing.o
endif

 