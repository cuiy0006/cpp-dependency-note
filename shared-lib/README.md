
### nm displays undefined symbols
```
    $ nm -Du /bin/ls

    w _ITM_deregisterTMCloneTable
    w _ITM_registerTMCloneTable
    U __assert_fail
    U __ctype_b_loc
    U __ctype_get_mb_cur_max
    U __ctype_tolower_loc
    U __ctype_toupper_loc
    U __cxa_atexit
    w __cxa_finalize
    ......
    ......
```

### objdump displays needed shared libs
```
    $ objdump -p /bin/ls

    Dynamic Section:
        NEEDED               libselinux.so.1
        NEEDED               libc.so.6
    ......
    ......
```

### Search path of shared lib for executable
1. Name of shared library contains '/' absolute path.  
2. Executable contains DT_RPATH without defining DT_RUNPATH, then search DT_RPATH.
3. LD_LIBRARY_PATH 
4. DT_RUNPATH 
5. ldconfig generated /etc/ld.so.cache
6. /lib and /usr/lib


### Search path of undefined symbols
1. in executable
2. in shared libraries in order (same order of ldd)

```
$ gcc -g -c -fPIC -Wall -c foo.c
$ gcc -g -c -fPIC -Wall -c bar.c
$ gcc -g -shared -o libfoo.so foo.o
$ gcc -g -shared -o libbar.so bar.o
```

```
$ gcc -g -o prog prog.c libfoo.so libbar.so
$ objdump -p prog | grep NEEDED
  NEEDED               libfoo.so
  NEEDED               libbar.so
  NEEDED               libc.so.6

$ ldd prog
    linux-vdso.so.1 (0x00007ffff96ef000)
    libfoo.so => not found
    libbar.so => not found
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f8ac4510000)
    /lib64/ld-linux-x86-64.so.2 (0x00007f8ac4c00000)

$ LD_LIBRARY_PATH=. ./prog
    foo
    foo-xyz
    bar
    foo-xyz
```

```
$ gcc -g -o prog prog.c libbar.so libfoo.so
$ objdump -p prog | grep NEEDED
  NEEDED               libbar.so
  NEEDED               libfoo.so
  NEEDED               libc.so.6

$ ldd prog
    linux-vdso.so.1 (0x00007fffdb3bf000)
    libbar.so => not found
    libfoo.so => not found
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fc0f4200000)
    /lib64/ld-linux-x86-64.so.2 (0x00007fc0f4a00000)

$ LD_LIBRARY_PATH=. ./prog
    foo
    bar-xyz
    bar
    bar-xyz
```


### Display the process of searching for symbol when run execuable
```
    LD_DEBUG=symbols LD_LIBRARY_PATH=. ./prog

    foo
        3684:     symbol=xyz;  lookup in file=./prog [0]
        3684:     symbol=xyz;  lookup in file=./libbar.so [0]
        3684:     symbol=puts;  lookup in file=./prog [0]
        3684:     symbol=puts;  lookup in file=./libbar.so [0]
        3684:     symbol=puts;  lookup in file=./libfoo.so [0]
        3684:     symbol=puts;  lookup in file=/lib/x86_64-linux-gnu/libc.so.6 [0]
    bar-xyz
    bar
        3684:     symbol=xyz;  lookup in file=./prog [0]
        3684:     symbol=xyz;  lookup in file=./libbar.so [0]
    bar-xyz
        3684:
        3684:     calling fini: ./prog [0]
        3684:
        3684:
        3684:     calling fini: ./libbar.so [0]
        3684:
        3684:
        3684:     calling fini: ./libfoo.so [0]
        3684:
```

### Display the process of loading libraries
```
    $ LD_DEBUG=libs LD_LIBRARY_PATH=. ./prog

    4102:     find library=libbar.so [0]; searching
    4102:      search path=./tls/haswell/x86_64:./tls/haswell:./tls/x86_64:./tls:./haswell/x86_64:./haswell:./x86_64:.                (LD_LIBRARY_PATH)
    4102:       trying file=./tls/haswell/x86_64/libbar.so
    4102:       trying file=./tls/haswell/libbar.so
    4102:       trying file=./tls/x86_64/libbar.so
    4102:       trying file=./tls/libbar.so
    4102:       trying file=./haswell/x86_64/libbar.so
    4102:       trying file=./haswell/libbar.so
    4102:       trying file=./x86_64/libbar.so
    4102:       trying file=./libbar.so
    ......
    ......
```


### LD_PRELOAD: symbol search path

LD_PRELOAD or /etc/ld.so.preload are searched first for symbols, can use it to overwrite the symbols

```
$ gcc -Wall -fPIC -shared -o libmylib.so mylib.c
$ gcc -o main main.c
./main
    X

LD_PRELOAD=./libmylib.so ./main
    call putchar() with 88
    call putchar() with 10
```


### Debug: cannot find symbol?

1. use nm -D to check whether symbol is in lib
2. use LD_DEBUG=libs to see whether the lib is loaded
3. use LD_DEBUG=symbols to see whether the symbol is loaded
3. use LD_LIBRARY_PATH to specify the path