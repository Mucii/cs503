
# QEMU Support for Xinu (x86)

- NOTE: QEMU support for Xinu (x86) is experimental. While this QEMU
  version might be helpful if you want to debug Xinu, it may have
  several unknown bugs.  So use this QEMU version for debugging during
  your development, and you should always confirm that your submission
  is working as expected on Xinu's lab machines running Intel
  Galileo. Our evaluation on your submission also will be based on
  Intel Galileo boards.

- Tested on Ubuntu 16.04
    - Should be working on most of Linux-based machines.
    - If possible, please use your own desktop/labtop running Linux.
    - If you are running Windows/MacOS, you may install a virtual
      machine running Ubuntu 16.04.

## Prerequisites

- Run following commands (in Ubuntu 16.04)

```console
$ sudo apt install qemu-system-x86
$ sudo apt install terminator
$ sudo apt install gdb
$ sudo apt install git
```

## Download the source code

- Clone the source code from git repo `http://melon.cs.purdue.edu/blee/xinu-x86-qemu.git`.
    - Username: xinu-share
    - Password: share-xinu

```console
$ git clone http://melon.cs.purdue.edu/blee/xinu-x86-qemu.git
```

## How to build
- Execute `rebuild.sh` in `tools` directory.
    - This runs `QEMU=1 make` in `the compile` directory

```console
$ ./rebuild.sh
~/tmp/xinu/compile ~/tmp/xinu/tools
removing .o files
removing configuration files ...
make[1]: Entering directory '/home/blee/tmp/xinu/config'
rm -f config lex.yy.c y.tab.c
make[1]: Leaving directory '/home/blee/tmp/xinu/config'
removing xinu ...
making the config program
make[1]: Entering directory '/home/blee/tmp/xinu/config'
rm -f config lex.yy.c y.tab.c
/usr/bin/flex config.l
/usr/bin/bison -y        config.y

...

/usr/bin/gcc -march=i586 -m32 -fno-builtin -fno-stack-protector -nostdlib -c -Wall -O0 -DBSDURG -DVERSION=\""`cat version`"\" -I../include -DQEMU -g -o binaries/ethdispatch.o ../device/eth/ethdispatch.S

Loading object files to produce GRUB bootable xinu

Building ../compile/xinu.bin

Building ../compile/xinu
```

## Run Xinu with QEMU
- Execute `run-qemu.sh` in `tools` directory.

```console
$ ./run-qemu.sh
[*] Running: qemu-system-i386 -cpu qemu32 -m 256M  --nographic -kernel ../compile/xinu.bin


Xinu for galileo -- version #261  (blee)  Sun Sep 17 17:35:21 EDT 2017

 266993664 bytes of free memory.  Free list:
 [0x0013E000 to 0x0FFDDFFF]

...

xsh $
```

- To run with GDB, use `--gdb` option. This will automatically pop-up
  the gdb window, and QEMU stalls Xinu until it gets the `continue`
  command from GDB.

```console
$ ./run-qemu.sh --gdb
```

## QEMU Monitor Mode

- Since all keyboard inputs within the QEMU window will directly feed
  to the emulated OS (i.e., Xinu), any control command such as CTRL+c
  does not work.

- You should switch to `monitor mode` to exit QEMU. To enter `monitor
  mode`, type `CTRL+a` and then `c`. Once entered, you will see the QEMU
  prompt, `(qemu)`.

- For example,

```console
xsh $

# Type `CTRL+a` and then 'c'. Then you will see the prompt of the QEMU
  monitor mode.

QEMU 2.5.0 monitor - type 'help' for more information
(qemu)

# Now you are in the monitor mode. Type `quit` to exit. Type `help` to
  see more commands.
```

## References
- [QEMU manual](https://qemu.weilnetz.de/doc/qemu-doc.html)
- [GDB tutorial](https://www.cs.cmu.edu/~gilpin/tutorial/)
