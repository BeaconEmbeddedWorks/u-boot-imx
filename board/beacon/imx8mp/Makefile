#
# Copyright 2022 Logic PD, Inc dba Beacon EmbeddedWorks
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx8mp_beacon.o
obj-y += ../../freescale/common/

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o
obj-$(CONFIG_IMX8M_DDR4) += ddr4_timing.o
endif
