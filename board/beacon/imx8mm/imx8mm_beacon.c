// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Logic PD, Inc. dba Beacon EmbeddedWorks
 */

#include <asm/global_data.h>
#include <env.h>
#include <imx_thermal.h>
#include <asm/mach-imx/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	int minc, maxc;

	if (get_cpu_temp_grade(&minc, &maxc) == TEMP_INDUSTRIAL) {
		env_set("trip0", "0x17318");
		env_set("trip1", "0x19a28");
	}

	return 0;
}
