BIN := fbBreakout
CC  := gcc

OBJS := main.o game.o
CFLAGS := -Wall -O2

.phony: clean

$(BIN): $(OBJS)
	gcc $(CFLAGS) -o $@ $^

clean:
	-rm -f $(OBJS) $(BIN)
