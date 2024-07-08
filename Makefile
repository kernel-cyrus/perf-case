CC = gcc
CFLAGS = -O2 -g -Wall
TARGET = perf_case
SRCS = perf_stat.c cases/memset.c cases/membw.c cases/memlat.c perf_case.c
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
