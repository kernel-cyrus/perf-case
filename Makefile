CC = gcc

INCLUDE = "./"
HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c cases/*.c cases/ustress/*.c)
OBJS = $(SOURCES:.c=.o)
TARGET = perf_case

CFLAGS = -O2 -g -Wall -I$(INCLUDE)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
