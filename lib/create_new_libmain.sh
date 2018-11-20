#!/bin/sh
mkdir libmain_tmp
cp libmain.a libmain_tmp
cd libmain_tmp
xtensa-lx106-elf-ar -x libmain.a
LIST=$(ls *.o)
for I in $LIST ; do 
	xtensa-lx106-elf-objcopy --redefine-sym call_user_start=call_user_start_sys $I 
done
xtensa-lx106-elf-ar -rcs libsys.a $(xtensa-lx106-elf-ar -t libmain.a)
cd ..
cp libmain_tmp/libsys.a .
rm -rf libmain_tmp