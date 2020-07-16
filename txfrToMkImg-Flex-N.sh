#!/bin/bash
#
cp ./arch/arm/dts/beacon-imx8mn-kit.dtb ../imx-mkimage/iMX8M/fsl-imx8mn-evk.dtb
cp ./u-boot-nodtb.bin ../imx-mkimage/iMX8M
cp ./u-boot-dtb.bin ../imx-mkimage/iMX8M
cp ./spl/u-boot-spl.bin ../imx-mkimage/iMX8M
cp ./spl/u-boot-spl-nodtb.bin ../imx-mkimage/iMX8M
cd ../imx-mkimage
make SOC=iMX8MN flash_evk_flexspi
cp iMX8M/flash.bin ../uboot-imx/flashspi.bin
cd ../uboot-imx

