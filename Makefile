SOURCES = sim8086.c
FLAGS = -g
BIN=sim8086
LISTING=listings/listing_0044_register_movs

$(BIN): $(SOURCES)
	clang -o $@ $(FLAGS) $<

clean:
	rm -rf $(BIN)

nasm:
	nasm $(LISTING).asm

listing:
	./sim8086 $(LISTING)