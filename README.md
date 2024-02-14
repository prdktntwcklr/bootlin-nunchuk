# Nunchuk Driver

A sample Linux I2C driver for the [Nintendo Wii Nunchuk](https://www.nintendo.co.jp/support/manual/wiiu/accessories/pdf/MAA-RVL-A-F-JPN.pdf) (RVL-004). The driver supports the following inputs using the input subsystem:

- Z Button
- C Button
- Joystick (X and Y)

This project is based on Bootlin's embedded Linux trainings (see `References` section below).

Tested on [BeagleBone Black](https://beagleboard.org/black) hardware running mainline Linux version 5.10.179.

## Hardware

- BeagleBone Black
- Nintendo Wii Nunchuk
- [UEXT connector for Wii Nunchuck](https://www.olimex.com/Products/Modules/Sensors/MOD-WII/MOD-WII-UEXT/open-source-hardware)

## Building the kernel module

To cross-compile, run the following command (using a cross-compiling toolchain provided by Ubuntu as an example):

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
```

## Device Tree

To enable Nunchuk functionality on your board, add the following entry to the device tree:

```dts
&i2c1 {
	pinctrl-names = "default";
	pinctrl-0 = <&i2c1_pins>;
	
	status = "okay";
	clock-frequency = <100000>;
	
	joystick@52 {
		compatible = "nintendo,nunchuk";
		reg = <0x52>;
	};
};
```

## Nunchuk

Connect the Nunchuk to the I2C1 bus of the BeagleBone Black board (pins 17 and 18 of P9, see [pinout](https://docs.beagleboard.org/latest/boards/beaglebone/black/ch07.html#connector-p9)). The Nunchuk should show up at address 0x52:

```bash
# i2cdetect -r 1
i2cdetect: WARNING! This program can confuse your I2C bus
Continue? [y/N] y
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00: -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- 52 -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

Load the kernel module to verify the driver finds the device:

```bash
# modprobe nunchuk
```

Finally, run the `evtest` application to test the inputs:

```bash
# evtest
```

## References

- Bootlin: [Embedded Linux kernel and driver development training](https://bootlin.com/training/kernel/).
