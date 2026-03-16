// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/imx93_pins.h>
#include <asm/arch/mu.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch-mx7ulp/gpio.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/mach-imx/syscounter.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <ctype.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <linux/delay.h>
#include <asm/arch/clock.h>
#include <asm/arch/ccm_regs.h>
#include <asm/arch/ddr.h>
#include <power/pmic.h>
#include <power/pca9450.h>
#include <power/pf0900.h>
#include <asm/arch/trdc.h>
#include "../common/imx9_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

static struct cargt_eeprom eeprom = {0};

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
#ifdef CONFIG_SPL_BOOTROM_SUPPORT
	return BOOT_DEVICE_BOOTROM;
#else
	switch (boot_dev_spl) {
	case SD1_BOOT:
	case MMC1_BOOT:
		return BOOT_DEVICE_MMC1;
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC2;
	default:
		return BOOT_DEVICE_NONE;
	}
#endif
}

void spl_board_init(void)
{
	struct cargt_eeprom *ep = CARGT_EEPROM_DATA;
	int ret;

	ret = ele_start_rng();
	if (ret)
		printf("Fail to start RNG: %d\n", ret);

	/* Copy EEPROM contents to DRAM */
	memcpy(ep, &eeprom, sizeof(*ep));
}

extern struct dram_timing_info dram_timing_1866mts;
extern struct dram_timing_info cargt_lpddr4x_1gbyte_timing;
extern struct dram_timing_info cargt_lpddr4x_2gbyte_timing;
void spl_dram_init(void)
{
	struct dram_timing_info *ptiming = &dram_timing;
	struct gpio_desc desc[4];
	struct udevice *dev;
	int ret;
	uint8_t board_id = 0;

	struct udevice *i2c_dev;
	int addr, res;

#if IS_ENABLED(CONFIG_CARGT_IMX93_00359_LPDDR4X_TIMING_BY_EEPROM)
	uint16_t dram_size_mbytes = 0;
	printf("Reading Cargt EEPROM over I2C...\n");
	cargt_eeprom_read(&eeprom);
	printf("Cargt EEPROM read complete.\n");
	printf("Cargt EEPROM contents at address: 0x%p\n", &eeprom);
	cargt_eeprom_print_prod_info(&eeprom);	

	/* Determine timing based on size */
	dram_size_mbytes = cargt_eeprom_get_dram_size_mb(&eeprom);
	switch(dram_size_mbytes)
	{
		case 2048:
			ptiming = &cargt_lpddr4x_2gbyte_timing;
			printf("\tLoading timing for Cargt 2Gbyte LPDDR4x\r\n");
			break;
		case 1024:
			ptiming = &cargt_lpddr4x_1gbyte_timing;
			printf("\tLoading timing for Cargt 1Gbyte LPDDR4x\r\n");
			break;
		default:
			printf("Unsupported DRAM size %d MB in EEPROM, defaulting to 1Gbyte timing\r\n", dram_size_mbytes);
			ptiming = &cargt_lpddr4x_1gbyte_timing;
			printf("\tLoading timing for Cargt 1Gbyte LPDDR4x\r\n");
			break;
	}

#endif /* CONFIG_CARGT_IMX93_00359_LPDDR4X_TIMING_BY_EEPROM */
#if IS_ENABLED (CONFIG_CARGT_IMX93_00359_LPDDR4X_TIMING_BY_BOARD_ID)
/* Check HWID pins to determine which dram timing to load */

/*
BANKS ARE WRONG IN SPL becuase u-boot doesn't use proper alias!
*/
// #define BOARD_ID_0 "GPIO3_26"
	#define BOARD_ID_0 "GPIO2_26"
// #define BOARD_ID_1 "GPIO3_27"
	#define BOARD_ID_1 "GPIO2_27"
// #define BOARD_ID_2 "GPIO4_28"
	#define BOARD_ID_2 "GPIO3_28"
// #define BOARD_ID_3 "GPIO4_29"
	#define BOARD_ID_3 "GPIO3_29"


	printf("Setting DRAM timing based on Cargt Board ID \r\n");

	ret = dm_gpio_lookup_name(BOARD_ID_0, &desc[0]);
	ret = dm_gpio_request(&desc[0], BOARD_ID_0);
	dm_gpio_set_dir_flags(&desc[0], GPIOD_IS_IN);
	ret = dm_gpio_get_value(&desc[0]);
	if (ret < 0) {
                printf("%s error in retrieving GPIO value ret = %d\n", __func__, ret);
                return 0;
    }
	board_id += (ret << 0);

	ret = dm_gpio_lookup_name(BOARD_ID_1, &desc[0]);
	ret = dm_gpio_request(&desc[0], BOARD_ID_1);
	dm_gpio_set_dir_flags(&desc[0], GPIOD_IS_IN);
	ret = dm_gpio_get_value(&desc[0]);
	if (ret < 0) {
                printf("%s error in retrieving GPIO value ret = %d\n", __func__, ret);
                return 0;
    }
	board_id += (ret << 1);

	ret = dm_gpio_lookup_name(BOARD_ID_2, &desc[0]);
	ret = dm_gpio_request(&desc[0], BOARD_ID_2);
	dm_gpio_set_dir_flags(&desc[0], GPIOD_IS_IN);
	ret = dm_gpio_get_value(&desc[0]);
	if (ret < 0) {
                printf("%s error in retrieving GPIO value ret = %d\n", __func__, ret);
                return 0;
    }
	board_id += (ret << 2);

	ret = dm_gpio_lookup_name(BOARD_ID_3, &desc[0]);
	ret = dm_gpio_request(&desc[0], BOARD_ID_3);
	dm_gpio_set_dir_flags(&desc[0], GPIOD_IS_IN);
	ret = dm_gpio_get_value(&desc[0]);
	if (ret < 0) {
                printf("%s error in retrieving GPIO value ret = %d\n", __func__, ret);
                return 0;
    }
	board_id += (ret << 3);


	printf("Cargt Board ID = 0x%01x\r\n", board_id);

	switch(board_id)
	{
		case 0x0f:
		ptiming = &cargt_lpddr4x_2gbyte_timing;
		printf("\tLoading timing for Cargt 2Gbyte LPDDR4x\r\n");
		break;

		case 0x0e:
		default:
		ptiming = &cargt_lpddr4x_1gbyte_timing;
		printf("\tLoading timing for Cargt 1Gbyte LPDDR4x\r\n");
		break;
	}
#endif /* CONFIG_CARGT_IMX93_00359_LPDDR4X_TIMING_BY_BOARD_ID */
#if IS_ENABLED(CONFIG_CARGT_IMX93_00359_LPDDR4X_TIMING_BY_NAME)
#ifdef CONFIG_CARGT_IMX93_00359_LPDDR4X_TIMING_FILENAME
static char *cargt_lpddr4t_timing_string = CONFIG_CARGT_IMX93_00359_LPDDR4X_TIMING_FILENAME;
#else
static char *cargt_lpddr4t_timing_string = "default";
#endif
	printf("\r\nSetting LPDDR4 timing (%s) \r\n", cargt_lpddr4t_timing_string);

	if (!strncmp("cargt_lpddr4x_2gbyte_timing", cargt_lpddr4t_timing_string, strlen("cargt_lpddr4x_2gbyte_timing")))
	{
		ptiming = &cargt_lpddr4x_2gbyte_timing;
		printf("\tLoading timing for Cargt 2Gbyte LPDDR4x\r\n");		
	}
	else
	{
		ptiming = &cargt_lpddr4x_1gbyte_timing;
		printf("\tLoading timing for Cargt 1Gbyte LPDDR4x\r\n");
	}
#endif /* CONFIG_CARGT_IMX93_00359_LPDDR4X_TIMING_BY_NAME */

	printf("DDR: %uMTS\n", ptiming->fsp_msg[0].drate);
	ddr_init(ptiming);
}
	
#if CONFIG_IS_ENABLED(DM_PMIC_PF0900)
int power_init_board(void)
{
	struct udevice *dev;
	int ret;
	unsigned int sw_val;

	ret = pmic_get("pmic@8", &dev);
	if (ret != 0) {
		puts("ERROR: Get PMIC PF0900 failed!\n");
		return ret;
	}
	puts("PMIC: PF0900\n");
	if (is_voltage_mode(VOLT_LOW_DRIVE)) {
		sw_val = 0x39; /* 0.8v for Low drive mode */
		printf("PMIC: Low Drive Voltage Mode\n");
	} else if (is_voltage_mode(VOLT_NOMINAL_DRIVE)) {
		sw_val = 0x41; /* 0.85v for Nominal drive mode */
		printf("PMIC: Nominal Voltage Mode\n");
	} else {
		sw_val = 0x49; /* 0.9v for Over drive mode */
		printf("PMIC: Over Drive Voltage Mode\n");
	}

	ret = pmic_reg_read(dev, PF0900_REG_SW1_VRUN);
	if (ret < 0)
		return ret;

	sw_val = (sw_val & SW_VRUN_MASK) | (ret & ~SW_VRUN_MASK);
	ret = pmic_reg_write(dev, PF0900_REG_SW1_VRUN, sw_val);
	if (ret != 0)
		return ret;

	ret = pmic_reg_read(dev, PF0900_REG_SW1_VSTBY);
	if (ret < 0)
		return ret;

	/* set standby voltage to 0.65v */
	sw_val = 0x21;
	sw_val = (sw_val & SW_STBY_MASK) | (ret & ~SW_STBY_MASK);
	ret = pmic_reg_write(dev, PF0900_REG_SW1_VSTBY, sw_val);
	if (ret != 0)
		return ret;

	ret = pmic_reg_read(dev, PF0900_REG_GPO_CTRL);
	if (ret < 0)
		return ret;

	/* I2C_LT_EN*/
	sw_val = 0x40;
	sw_val = (sw_val & GPO3_RUN_MASK) | (ret & ~GPO3_RUN_MASK);
	ret = pmic_reg_write(dev, PF0900_REG_GPO_CTRL, sw_val);
	if (ret != 0)
		return ret;

	ret = pmic_reg_read(dev, PF0900_REG_SYS_CFG1);
	if (ret < 0)
		return ret;
	/*disable stby xrst*/
	sw_val = 0x0;
	sw_val = (sw_val & XRST_STBY_EN_MASK) | (ret & ~XRST_STBY_EN_MASK);
	ret = pmic_reg_write(dev, PF0900_REG_SYS_CFG1, sw_val);
	if (ret != 0)
		return ret;
	return 0;
}
#endif

#if CONFIG_IS_ENABLED(DM_PMIC_PCA9450)
int power_init_board(void)
{
	struct udevice *dev;
	int ret;
	unsigned int val = 0, buck_val;

	ret = pmic_get("pmic@25", &dev);
	if (ret != 0) {
		puts("ERROR: Get PMIC PCA9451A failed!\n");
		return ret;
	}
	puts("PMIC: PCA9451A\n");
	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/* enable DVS control through PMIC_STBY_REQ */
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	ret = pmic_reg_read(dev, PCA9450_PWR_CTRL);
	if (ret < 0)
		return ret;
	else
		val = ret;

	if (is_voltage_mode(VOLT_LOW_DRIVE)) {
		buck_val = 0x0c; /* 0.8v for Low drive mode */
		printf("PMIC: Low Drive Voltage Mode\n");
	} else if (is_voltage_mode(VOLT_NOMINAL_DRIVE)) {
		buck_val = 0x10; /* 0.85v for Nominal drive mode */
		printf("PMIC: Nominal Voltage Mode\n");
	} else {
		buck_val = 0x14; /* 0.9v for Over drive mode */
		printf("PMIC: Over Drive Voltage Mode\n");
	}

	if (val & PCA9450_REG_PWRCTRL_TOFF_DEB) {
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, buck_val);
		pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, buck_val);
	} else {
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, buck_val + 0x4);
		pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, buck_val + 0x4);
	}

	if (IS_ENABLED(CONFIG_IMX93_EVK_LPDDR4)) {
		/* Set VDDQ to 1.1V from buck2 */
		pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x28);
	}

	/* set standby voltage to 0.65v */
	if (val & PCA9450_REG_PWRCTRL_TOFF_DEB)
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x0);
	else
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x4);

	/* I2C_LT_EN*/
	pmic_reg_write(dev, 0xa, 0x3);
	return 0;
}
#endif

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	timer_init();

	arch_cpu_init();

	board_early_init_f();

	spl_early_init();

	preloader_console_init();

	ret = imx9_probe_mu();
	if (ret) {
		printf("Fail to init ELE API\n");
	} else {
		printf("SOC: 0x%x\n", gd->arch.soc_rev);
		printf("LC: 0x%x\n", gd->arch.lifecycle);
	}

	clock_init_late();

	power_init_board();

	if (!is_voltage_mode(VOLT_LOW_DRIVE))
		set_arm_core_max_clk();

	/* Init power of mix */
	soc_power_init();

	/* Setup TRDC for DDR access */
	trdc_init();

	/* DDR initialization */
	spl_dram_init();

	/* Put M33 into CPUWAIT for following kick */
	ret = m33_prepare();
	if (!ret)
		printf("M33 prepare ok\n");

	board_init_r(NULL, 0);
}

#ifdef CONFIG_ANDROID_SUPPORT
int board_get_emmc_id(void) {
	return 0;
}
#endif
