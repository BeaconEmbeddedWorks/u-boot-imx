#
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2022 Logic PD, Inc dba Beacon EmbeddedWorks
#

obj-y += imx8mp_beacon.o
obj-y += ../../freescale/common/

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
ifdef CONFIG_IMX8MP_BEACON_4GB_LPDDR
obj-y += lpddr4_4g_timing.o
else
    ifdef CONFIG_IMX8MP_BEACON_2GB_LPDDR
    obj-y += lpddr4_2g_timing.o
    else
    obj-y += lpddr4_timing.o
endif
endif
endif
