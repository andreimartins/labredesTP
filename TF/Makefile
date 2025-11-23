CC = gcc
CFLAGS = -std=gnu11 -D_GNU_SOURCE -Wall -Wextra -O2 -g -I.
LDFLAGS =
LDLIBS = 

SRCS = main.c analyze.c capture.c log.c stats.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

TARGET = traffic_monitor

.PHONY: all clean

all: $(TARGET)

-include $(DEPS)



$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	@rm -f $(OBJS) $(DEPS)


%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@


clean:
	rm -f $(OBJS) $(DEPS) $(TARGET) traffic_monitor
