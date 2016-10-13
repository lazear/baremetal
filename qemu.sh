#!/bin/bash

if [[ $(uname -a) =~ "arch" ]]; then
	echo ARCH
	qemu-system-x86_64 -kernel bin/kernel.bin -hdb ext2 -m 512 -smp cpus=4 -serial stdio -ctrl-grab | tee serial.txt
else
	echo NO
	qemu-system-x86_64 -kernel bin/kernel.bin -curses -hdb ext2 -m 512 -smp cpus=4 -serial file:serial.txt
	cat serial.txt

fi
