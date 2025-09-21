# GLXcube - GLX and X11 sanity check
# Copyright (C) 2025  Connor Thomson
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# C compiler
CC      = gcc
# Files
BIN     = main
SRC     = main.c
# CPU Flags
MARCH   = native
MTUNE   = $(MARCH)
# Optimization
OPT     = fast
# Compiler flags
FLAGS   = -static -D_DEFAULT_SOURCE
CFLAGS  = -march=$(MARCH) -mtune=$(MTUNE) -O$(OPT) $(FLAGS)
# Linker flags
LDFLAGS = -lX11 -lGL -lGLU -lm

# If you dont want to strip then remove "strip" from the line below
all: compile strip

compile:
	$(CC) -o $(BIN) $(SRC) $(CFLAGS) $(LDFLAGS)

strip:
	strip -s $(BIN)

clean:
	rm -f $(BIN)

.PHONY: all compile strip clean
