# temp_control
Temperature control based on STM32L1 microcontroller and DS18B20 temperature sensor

## folder structure

#### code
STM32 Firmware projects based on libopencm3. There is one for the STM32F4-Discovery Board for prototyping and testing code and one for the finished custom PCB with a STM32L1.

- Firmware library: libopencm3. 
- Display driver: u8g2  (0.96" 128x64 OLED display, SSD1306, I2C)

#### cad
3D models of the housing as source (FreeCad) as well as the STLs for direct 3D printing

#### pcb
Design files for the PCB (made in KiCad) as well as the Gerber files

## Building the firmware
Make sure you have the GNU ARM Embedded toolchain installed. Also, building is much easier on a Linux machine. Windows is still porssible but requires a few more steps, see the linopencm3 repository for details)

After cloning the repository, make sure to run `git submodule init` and `git submodule update` in order to pull the libopencm3 submodule.
Then go to the code/libopencm3 directory and run `make TARGETS=stm32/f4`   (or `TARGETS=STM32/l1`, depending on which project you want to use). This builds the libopencm3 library.
After that, go to the firmware directory, for example ´code/f4discovery´ and run make