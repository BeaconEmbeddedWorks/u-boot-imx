#
# Copyright 2020 Compass Electronics Group, LLC
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx8mm_beacon.o
obj-y += ../../freescale/common/

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
ifdef CONFIG_IMX8M_4G_LPDDR4
obj-y += lpddr4_timing_4g.o
else
obj-y += lpddr4_timing.o
endif
endif
