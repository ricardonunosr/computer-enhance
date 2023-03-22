# Computer Enhance

```bash
# Use nasm assembler to output decode assembly.
nasm listing_0037_single_register_mov.asm

# Compare original assembly output with the decoder output.
diff listing_0037_single_register_mov test

# Build sim8086, run it, output to test.asm and assemble the new test to compare with original
./build.sh && ./sim8086 listing_0039_more_movs > test.asm && nasm test.asm && diff listing_0039_more_movs test
```


