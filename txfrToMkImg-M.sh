#!/bin/bash
#
cp ./arch/arm/dts/imx8mm-beacon-kit.dtb ../imx-mkimage/iMX8M/imx8mm-evk.dtb
cp ./u-boot-nodtb.bin ../imx-mkimage/iMX8M
cp ./u-boot-dtb.bin ../imx-mkimage/iMX8M
cp ./spl/u-boot-spl.bin ../imx-mkimage/iMX8M
cp ./spl/u-boot-spl-nodtb.bin ../imx-mkimage/iMX8M
cd ../imx-mkimage
make SOC=iMX8MM flash_hdmi_spl_uboot
cp iMX8M/flash.bin ../uboot-imx/.
cd ../uboot-imx

