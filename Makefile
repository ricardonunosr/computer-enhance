SOURCES = sim8086.c
FLAGS = -g
BIN=sim8086

$(BIN): $(SOURCES)
	clang -o $@ $(FLAGS) $< 

clean:
	rm -rf $(BIN)
