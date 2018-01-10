# sx127x
Semtech SX1272 and SX1276 radios driver for Mongoose OS and Linux

## Description
This library contains the driver for radio devices of the Semtech SX127x series (SX1272 and SX1276). Only LoRa long range modem is supported at the moment.

For more information on Semtech SX1272 and SX1276 modules see:
* [SX1272/73 datasheet](http://www.semtech.com/images/datasheet/sx1272.pdf)
* [SX1276/77/78/79 datasheet](http://www.semtech.com/images/datasheet/sx1276_77_78_79.pdf)

Based on RIOT-OS sx127x driver.

See https://github.com/RIOT-OS/RIOT/tree/master/drivers/sx127x

## Linux
For Linux the driver is a library, we test using an Orange Pi Zero. To compile follow the instructions:

	$ sudo apt-get install gcc-4.7-arm-linux-gnueabihf
	$ cd sx127x
	$ ./builder.sh armhf
	$ cd build.armhf
	$ make