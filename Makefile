#Current make system
BIN=./bin/
SOURCE=./src/

LIST=$(BIN)/net2 $(BIN)/cmd2
CC=c++
CFLAGS=-std=c++17

all: $(LIST)
$(BIN)/%:  $(SOURCE)%.C
	@mkdir -p $(@D)
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)
