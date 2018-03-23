##
## Copyright (c) 2018 Ricardo Beck.
## 
## This file is part of temp_control
## (see https://github.com/Spritkopf/temp_control).
## 
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
## 
## You should have received a copy of the GNU Lesser General Public License
## along with this program. If not, see <http://www.gnu.org/licenses/>.
##

BIN_DIR ?= build
BINARY = miniblink
OPENCM3_DIR = ../libopencm3
LDSCRIPT = stm32f407vg.ld

TARGETS=stm32/f4


###############################################################################
# Source files

C_SOURCES = \
src/main.c \
lib/onewire/onewire.c \
lib/onewire/onewire_hal_usart.c \
lib/ds18b20/ds18b20.c \
lib/ssd1306/ssd1306_hal_i2c.c \
lib/ssd1306/ssd1306.c \
lib/ssd1306/fonts.c

###############################################################################
# Include paths

C_INCLUDES = \
-Ilib

###############################################################################
# Static libraries

include libopencm3.target.mk

