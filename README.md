# lmic-rpi-lora-gps-hat

Hardware Abstraction Layer (HAL) for IBM's LMIC 1.6 communication stack 
targeted to RPi and Dragino LoRA/GPS HAT.

The goal was to keep the LMIC 1.6 sourcecode untouched, and just provide a
Hardware Abstraction Layer (HAL) for Raspberry Pi and Dragino LoRa/GPS HAT.

## Installation

### WiringPi
To control the RPi's GPI ports, the WiringPi GPIO interface library has to
be installed. On some operating systems WiringPi is already installed per
default. For instructions on manual installation please refer to the 
following site: 
http://wiringpi.com/download-and-install/

### Check hardware revision of Dragino LoRa/GPS HAT
Look for a hardware revision number at the LoRa/GPS HAT PCB.
It should read "Lora/GPS HAT for RPI v1.3". If the version is below 1.3, you
need to add some traces. Locate the header pins DIO0, DIO1 and DIO2 on the PCB.
* DIO0 is already connected to physical pin #7 aka WiringPi pin 7 aka BCM pin 4.
* DIO1 *has to be manually connected* to physical pin #16 aka WiringPi pin 4 aka BCM pin 23.
* DIO2 *has to be manually connected* to physical pin #18 aka WiringPi pin 5 aka BCM pin 24.

The pin numbering may be confusing, but WiringPi can be used to print out all these
pin numbers.

    pi@rpi3:~/lmic-rpi-lora-gps-hat $ gpio readall

    +-----+-----+---------+------+---+---Pi 3---+---+------+---------+-----+-----+
    | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
    +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
    |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
    |   2 |   8 |   SDA.1 |   IN | 1 |  3 || 4  |   |      | 5V      |     |     |
    |   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
    |   4 |   7 | GPIO. 7 |   IN | 0 |  7 || 8  | 0 | IN   | TxD     | 15  | 14  |
    |     |     |      0v |      |   |  9 || 10 | 1 | IN   | RxD     | 16  | 15  |
    |  17 |   0 | GPIO. 0 |   IN | 1 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
    |  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
    |  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
    |     |     |    3.3v |      |   | 17 || 18 | 0 | IN   | GPIO. 5 | 5   | 24  |
    |  10 |  12 |    MOSI | ALT0 | 0 | 19 || 20 |   |      | 0v      |     |     |


### Enable SPI interface
Per default, the SPI ports on the Raspberry Pi are disabled. You need to
manually enable them using raspi-config.
Follow the instructions given here: 
https://www.raspberrypi.org/documentation/hardware/raspberrypi/spi/README.md

## Note on LMIC 1.6 license
Text copied from https://www.research.ibm.com/labs/zurich/ics/lrsc/lmic.html

IBM "LoRa WAN in C" is the LoRa WAN implementation of choice, and a perfect
match to the IBM LRSC on the end device. It is provided as open source under 
the BSD License.

## Compile Code
Directory: /src

Modifications necessary: 

File /src/sendConsumption/main.c:

* Adapt "NWKSKEY" 
* Adapt "APPSKEY" 
* Adapt "DEVADDR" 

File /src/sendConsumption/rest_client.c

* Adapt "http_uuid" 

Build:

    cd /src/sendConsumption/sendConsumption
    make clean
    make
    sudo ./build/sendConsumption.out

Possible output:

    000000000 HAL: Initializing ...
    000000004 HAL: Detected SX1276 radio module.
    000000004 HAL: Set radio RST pin to 0x00
    000000005 HAL: Wait until 000000006 ms
    000000006 HAL: Set radio RST pin to 0x02
    000000006 HAL: Wait until 000000011 ms
    000000013 HAL: Receiving ...
    000000041 Debug: Initializing
    000000041 Debug: JOINING
    000004897 Debug: EV_TXSTART
    000004898 HAL: Sending ...
    000009960 HAL: Receiving ...
    000009961 HAL: Wait until 000009962 ms
    000010033 Debug: JOINED
    000010034 Debug: 54230
    000010034 Debug: Label 'val = ' value 0xd3d6
    000010034 Debug: EV_TXSTART
    000010034 HAL: Sending ...
    000011081 HAL: Receiving ...
    000011081 HAL: Wait until 000011082 ms
    000012128 HAL: Receiving ...
    000012128 HAL: Wait until 000012130 ms
    000016360 Debug: TXCOMPLETE
    000070068 Debug: 53692
    000070068 Debug: Label 'val = ' value 0xd1bc
    000070069 Debug: EV_TXSTART
    000070070 HAL: Sending ...
    000071117 HAL: Receiving ...
    000071117 HAL: Wait until 000071118 ms
    000072164 HAL: Receiving ...
    000072164 HAL: Wait until 000072165 ms
    000076734 Debug: TXCOMPLETE
