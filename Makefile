CC		:= gcc
C_FLAGS := -std=c11 -Wall -Wextra -g `pkg-config fuse3 --cflags --libs`

# For building as cpp code use:
#CC		:= g++
#C_FLAGS := -std=c++17 -Wall -Wextra -g `pkg-config fuse3 --cflags --libs`


BIN		:= bin
SRC		:= src
INCLUDE	:= include
LIB		:= lib

LIBRARIES	:=

EXECUTABLE	:= catalogfs

all: $(BIN)/$(EXECUTABLE)

clean:
	$(RM) $(BIN)/$(EXECUTABLE)

run: all
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.c
	$(CC) $(C_FLAGS) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES)
