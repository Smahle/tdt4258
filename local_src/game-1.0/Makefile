CC := arm-cortexm3-uclinuxeabi-gcc

OBJS := main.o game.o
CFLAGS := -std=gnu99 -Wall -O2

.phony: clean

game: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

install:


clean:
	-rm -f game $(OBJS)
