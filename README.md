# rpi2_quad_core
An attempt to use all for cores of Raspberry Pi2.

This source code uses all four cores of RPI2 to display a pattern on 640x480 screen over HDMI.


It is made from armc-014 tutorial from Brian Sidebotham's  Raspberry-Pi Bare Metal Tutorials.

Build steps:
1. Download source
2. Install arm-none-gnu-eabi toolchain
3. Install make if it is build on Windows
4. Run source\scripts\configure_rpi2.bat or sh depending on OS
5. Run make in source\scripts\

Loading steps:
1. Use standard binaries for RPI2 i.e. bootcode.bin and start.elf, copy to microSD card formatted with FAT32.
2. Copy kernel.img from source\scripts\ to microSD card.
3. Safe remove from PC.
4. Put it in RPI2.
5. Connect HDMI cable to Monitor / LCD.
6. Power on RPI2.
7. Each core colors one quadrant, and rotates to different quadrants.
