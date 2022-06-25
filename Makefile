#Current make system
BIN=./bin/
SOURCE=./src/

LIST=$(BIN)/net2 $(BIN)/cmd2 $(BIN)/tp
CC=c++
CFLAGS=-std=c++17 -g

all: $(LIST)
$(BIN)/%:  $(SOURCE)%.C
	@mkdir -p $(@D)
	time $(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)
