SOURCES = sim8086.c
FLAGS = -g
BIN=sim8086
LISTING=listings/listing_0041_add_sub_cmp_jnz

$(BIN): $(SOURCES)
	clang -o $@ $(FLAGS) $<

clean:
	rm -rf $(BIN)

nasm:
	nasm $(LISTING).asm

listing:
	./sim8086 $(LISTING)