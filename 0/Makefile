# define compiler and compilation flags
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c17 -g
LDLIBS  := -lm

# target and sources
TARGET  := main
SRCS    := main.c bitmap.c
OBJS    := $(SRCS:.c=.o)
DEPS    := $(OBJS:.o=.d)

# Make the executable binary
all: $(TARGET)

# target recipe
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# catch-all object recipe
%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# catch updated deps
-include $(DEPS)

# clean up all files
clean:
	rm -f $(TARGET) after.bmp $(OBJS) $(DEPS)

.PHONY: all clean
