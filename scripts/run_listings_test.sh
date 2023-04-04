#!/bin/bash

set -e

for listing in $(ls $1 | grep asm); do
	result=${listing%.*}
	echo "running $listing";
	./sim8086 "$1/$result" > test.asm
	nasm test.asm
	diff test $1/$result
done

rm test test.asm
