/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025 Cargt Inc.
 *
 */

#ifndef _MX9_CARGT_EEPROM_H_
#define _MX9_CARGT_EEPROM_H_

#ifdef CONFIG_ARCH_IMX9
#include <asm/arch-imx9/ddr.h>
#endif

#define CARGT_EEPROM_MAGIC	0x43524754 /* == HEX("CRGT") */

#define CARGT_EEPROM_I2C_ADDR	0x50

/*
 * Example EEPROM data for testing and reference.
 * This array represents a sample EEPROM structure with:
 * - Magic number ("CRGT")
 * - Structure size (48 bytes)
 * - Structure version (1)
 * - Part number ("105-00359-02") (null terminated)
 * - Serial number (0x12345678)
 * - Build date ("20240610") (null terminated)
 * - DRAM size (1024 MB)
 * - DRAM speed (0 MT/s)
 * - Reserved bytes
 * - Example CRC32
 *
 * Usage: For unit tests or as a template for EEPROM programming.
 *
 * static const uint8_t cargt_eeprom_example[48] = {
 *   0x43, 0x52, 0x47, 0x54,             // magic "CRGT"
 *   0x30, 0x00,                         // size (48 bytes)
 *   0x01,                               // struct_ver
 *   '1','0','5','-','0','0','3','5','9','-','0','2',0x00, // cpn
 *   0x12, 0x34, 0x56, 0x78,             // serial
 *   '2','0','2','4','0','6','1','0',0x00, // date
 *   0x00, 0x04,                         // dramsize_mbytes (1024)
 *   0x00, 0x00,                         // dramspeed_mts (0)
 *   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // reserved
 *   0xE5, 0xB7, 0xA2, 0xC3              // crc32 (example)
 * };
 */
struct __packed cargt_eeprom
{
	u32 magic;				/*  0 - magic number       */
	u16 size;				/*  4 - size of structure in bytes */
	u8 struct_ver;      	/*  6 - structure version  */
	u8 cpn[13];				/*  7 - cargt part number (null terminated) xxx-xxxxx-xx */
	u32 serial;				/* 20 - cargt serial number */
	u8 date[9];				/* 24 - build date YYYYMMDD (null terminated) */
	u16 dramsize_mbytes;	/* 33 - DRAM size in MBytes */
	u16 dramspeed_mts;		/* 35 - DRAM speed in MT/s  */
	u8 reserved[7];			/* 37 - reserved */
	u32 crc32;				/* 44 - CRC32 of this structure */
};

#define CARGT_EEPROM_DATA ((struct cargt_eeprom *)CARGT_EEPROM_DRAM_START)

int cargt_eeprom_read(struct cargt_eeprom *e);
int cargt_eeprom_get_serial(struct cargt_eeprom *e, u8 *serial);
int cargt_eeprom_get_dram_size(struct cargt_eeprom *ep, phys_size_t *size);
int cargt_eeprom_get_dram_speed(struct cargt_eeprom *ep, phys_size_t *speed);
void cargt_eeprom_print_prod_info(struct cargt_eeprom *e);
int cargt_eeprom_is_valid(struct cargt_eeprom *ep);
int cargt_eeprom_get_cpn_string(struct cargt_eeprom *ep, char *cpn_buf, size_t buf_size);


#endif /* _MX9_CARGT_EEPROM_H_ */
