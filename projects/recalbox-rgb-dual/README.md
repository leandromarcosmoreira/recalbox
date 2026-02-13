# recalbox-rgb-dual

## EEPROM

Programe a EEPROM do Recalbox RGB Dual

Uma vez programada, a HAT cria arquivos em `/sys/firmware/devicetree/base/hat/` e o dtb é carregado automaticamente da EEPROM.

## Módulo do kernel - baseado no rpi-dpidac do cpasjuste

https://github.com/Cpasjuste/rpi-dpidac/

Compilação (compilação cruzada, para raspbian os)
- make -f Makefile.cross ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- KERNELDIR=/path/to/linux

## Recursos
### DTS
https://www.embedded.com/linux-device-driver-development-the-pin-control-subsystem/
https://gist.github.com/0xff07/d286f45649a7e05c32c4523631bd15e0
https://github.com/raspberrypi/linux/blob/rpi-5.10.y/arch/arm/boot/dts/overlays/gpio-key-overlay.dts
https://www.programmerall.com/article/1048180876/
https://www.kernel.org/doc/html/latest/driver-api/gpio/board.html

### Kernel
https://github.com/raspberrypi/linux/blob/rpi-5.10.y/drivers/power/reset/gpio-poweroff.c
