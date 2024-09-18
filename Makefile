CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Ilib/sokol
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)  # macOS
    LDFLAGS = -framework Cocoa -framework Foundation -framework OpenGL -framework AudioToolbox
	OBJCFLAGS = -x objective-c
else ifeq ($(UNAME_S), Linux)
    LDFLAGS = -lGL -lm -ldl -lpthread
	OBJCFLAGS =
endif

OBJS = src/main.o lib/sokol.o

TARGET = heretic

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

main.o: src/main.c
	@$(CC) $(CFLAGS) -c main.c -o main.o

lib/sokol.o: lib/sokol.c
	@$(CC) $(CFLAGS) $(OBJCFLAGS) -c lib/sokol.c -o lib/sokol.o

clean:
	@rm -f $(OBJS) $(TARGET)

.PHONY: all clean
