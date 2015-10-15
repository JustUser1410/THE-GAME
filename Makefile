CC = gcc
CFLAGS = -W -Wall
LDFLAGS = -lmenu -lncurses
SOURCES = THE_GAME.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = THE_GAME

.PHONY: all clean

all: $(TARGET)

$(TARGET) : $(OBJECTS) 
	$(CC) $(CFLAGS) -o $@  $^ $(LDFLAGS)
 
clean:
	rm -rf $(TARGET) $(OBJECTS)
