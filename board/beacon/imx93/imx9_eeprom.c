// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Beacon Embedded Works
 *
 */
#include <command.h>
#include <dm.h>
#include <i2c.h>
#include <asm/io.h>
#include <cpu_func.h>
#include <u-boot/crc.h>
#include <asm/arch-imx9/ddr.h>

#include "imx9_eeprom.h"

static int beacon_eeprom_get_dev(struct udevice **devp)
{
	int ret;
	struct udevice *bus;

	ret = uclass_get_device_by_name(UCLASS_I2C, BEACON_EEPROM_I2C_NAME, &bus);
	if (ret) {
		printf("%s: No EEPROM I2C bus '%s'\n", __func__,
		       BEACON_EEPROM_I2C_NAME);
		return ret;
	}

	ret = dm_i2c_probe(bus, BEACON_EEPROM_I2C_ADDR, 0, devp);
	if (ret) {
		printf("%s: I2C EEPROM probe failed\n", __func__);
		return ret;
	}

	i2c_set_chip_offset_len(*devp, 1);
	i2c_set_chip_addr_offset_mask(*devp, 1);

	return 0;
}

int beacon_eeprom_read(struct beacon_eeprom *e)
{
	int ret;
	struct udevice *dev;

	ret = beacon_eeprom_get_dev(&dev);
	if (ret) {
		printf("%s: Failed to detect I2C EEPROM\n", __func__);
		return ret;
	}

	/* Read EEPROM contents to memory */
	ret = dm_i2c_read(dev, 0, (void *)e, sizeof(*e));
	if (ret) {
		printf("%s: EEPROM read failed, ret=%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

int beacon_eeprom_get_serial(struct beacon_eeprom *ep, u8 *serial)
{
	flush_dcache_all();
	if (!beacon_eeprom_is_valid(ep))
		return -1;

	memcpy(&serial, &ep->serial, sizeof(ep->serial));

	return 0;
}

uint16_t beacon_eeprom_get_dram_size_mb(struct beacon_eeprom *ep)
{
	/* No data in EEPROM - return default DRAM size */
	if (!beacon_eeprom_is_valid(ep)) {
		return DEFAULT_SDRAM_SIZE_MB;
	}

	return be16_to_cpu(ep->dramsize_mbytes);
}


uint16_t beacon_eeprom_get_dram_speed_mts(struct beacon_eeprom *ep)
{
	/* No data in EEPROM - return default DRAM size */
	if (!beacon_eeprom_is_valid(ep)) {
		return DEFAULT_SDRAM_SPEED_MTS;
	}

	return __be16_to_cpu(ep->dramspeed_mts);
}

int beacon_eeprom_get_cpn_string(struct beacon_eeprom *ep, char *cpn_buf, size_t buf_size)
{
	flush_dcache_all();
	if (!beacon_eeprom_is_valid(ep))
		return -1;

	memset(cpn_buf, 0, buf_size);
	strncpy(cpn_buf, (const char *)ep->cpn,
		min(sizeof(ep->cpn), buf_size - 1));

	return 0;
}

void beacon_eeprom_print_prod_info(struct beacon_eeprom *ep)
{
	flush_dcache_all();

	if (!beacon_eeprom_is_valid(ep))
		return;

	printf("EEPROM Production Information:\n");
	printf("\tPart number: %s\n", ep->cpn);
	printf("\tProduction date: %s\n", ep->date);
	printf("\tSerial number: 0x%08x\n", be32_to_cpu(ep->serial));
	printf("\tEEPROM struct version: 0x%x\n", ep->struct_ver);
	printf("\tDRAM size: %d MB\n", __be16_to_cpu(ep->dramsize_mbytes));
	printf("\tDRAM speed: %d MT/s\n", __be16_to_cpu(ep->dramspeed_mts));
}

int beacon_eeprom_is_valid(struct beacon_eeprom *ep)
{
	u32 crc, crc_offset = offsetof(struct beacon_eeprom, crc32);

	if (be32_to_cpu(ep->magic) != BEACON_EEPROM_MAGIC) {
		 printf("Invalid EEPROM magic 0x%08x, expected 0x%08x\n",
			 be32_to_cpu(ep->magic), BEACON_EEPROM_MAGIC);
		 return 0;
	}

	if (ep->struct_ver < 1) {
		printf("Invalid EEPROM struct version 0x%x\n", ep->struct_ver);
		return 0;
	}

	crc = crc32(0, (void *)ep, crc_offset);

	if (crc != be32_to_cpu(ep->crc32)) {
		printf("EEPROM CRC mismatch (%08x != %08x)\n",
			   crc, be32_to_cpu(ep->crc32));
		printf("EEPROM hex dump:\n");
		for (size_t i = 0; i < sizeof(struct beacon_eeprom); i++) {
			printf("%02x ", ((u8 *)ep)[i]);
			if ((i + 1) % 16 == 0)
				printf("\n");
		}
		printf("\n");
		return 0;
	}

	return 1;
}
