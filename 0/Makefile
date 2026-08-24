CC      := gcc
CFLAGS  := -Wall -Wextra -std=c17 -g
LDLIBS  := -lm

TARGET  := main
SRCS    := main.c bitmap.c
OBJS    := $(SRCS:.c=.o)
DEPS    := $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(TARGET) after.bmp $(OBJS) $(DEPS)

.PHONY: all clean
