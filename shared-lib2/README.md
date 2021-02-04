### general shared lib

```
$ gcc -g -fPIC -c foo.c
$ gcc -shared foo.o -o libfoo.so

$ gcc -g -o main main.c libfoo.so
$ gcc -g -o main main.c -lfoo -L.

$ LD_LIBRARY_PATH=. ./main
```

```
$ ldd main
        linux-vdso.so.1 (0x00007fffdcd4b000)
        libfoo.so => not found
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f36e3010000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f36e3800000)
```

```
$ LD_LIBRARY_PATH=. ldd main
        linux-vdso.so.1 (0x00007fffe91c5000)
        libfoo.so => ./libfoo.so (0x00007f8368a70000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f8368670000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f8369000000)
```

### circular dependency
A.a uses B.a, B.a uses A.a

```
$ gcc -g -o main main.o A.a B.a A.a
```

Or

```
$ gcc -g -o main main.o -Wl,--start-group A.a B.a -Wl,--end-group
```

### Search path of shared lib for executable

1. Name of shared library contains '/' absolute path.  
2. Executable contains DT_RPATH without defining DT_RUNPATH, then search DT_RPATH.
3. LD_LIBRARY_PATH 
4. DT_RUNPATH 
5. ldconfig generated /etc/ld.so.cache
6. /lib and /usr/lib

**/usr/local/lib is in /etc/ld.so.cache.**

```
$ gcc -fPIC -g -shared -o libfoo.so foo.c
$ sudo cp libfoo.so /usr/local/lib
$ gcc -g -o main main.c -lfoo
```

```
$ ldd main
        linux-vdso.so.1 (0x00007fffc34c7000)
        libfoo.so => not found
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f35ec880000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f35ed000000)

```

```
$ sudo ldconfig
$ ldd main
        linux-vdso.so.1 (0x00007fffc467b000)
        libfoo.so => /usr/local/lib/libfoo.so (0x00007fd5d6d70000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fd5d6970000)
        /lib64/ld-linux-x86-64.so.2 (0x00007fd5d7200000)
```

### linker name, real name, soname

libX.so.A.B.C: real name
libX.so.A: soname
libX.so: linker name

1. generate libX.so.A.B.C
2. soft link libX.so.A → libX.A.B.C
3. soft link libX.so → libX.so.A

```
$ gcc -fPIC -g -shared -Wl,-soname,libfoo.so.1 -o libfoo.so.1.0.0 foo.c
$ sudo cp libfoo.so.1.0.0 /usr/local/lib
$ cd /usr/local/lib
$ sudo ldconfig # will generate soft link libfoo.so.1 -> libfoo.so.1.0.0
$ sudo ln -s libfoo.so.1 libfoo.so


$ cd -
$ gcc -g -o main main.c -lfoo
$ ldd main
        linux-vdso.so.1 (0x00007fffe47ec000)
        libfoo.so.1 => /usr/local/lib/libfoo.so.1 (0x00007f44f87a0000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f44f83a0000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f44f8c00000)
```

**why is this libfoo.so.1?**
ld uses libfoo.so -> libfoo.so.1
ld read soname(libfoo.so.1) from realname lib(libfoo.so.1.0.0)

**How to get soname from a shared lib?**
objdump -p libfoo.so.1.0.0 | grep SONAME  
objdump -p libfoo.so | grep SONAME  

```
$ gcc -g -o main main.c -lfoo
```

### Whenever needed to update the shared lib, no need to relink shared link with executable
```
$ gcc -fPIC -g -shared -Wl,-soname,libfoo.so.1 -o libfoo.so.1.2.3 foo2.c
$ sudo cp libfoo.so.1.2.3 /usr/local/lib
$ sudo ldconfig
```

### When need to generate new shared lib
```
$ gcc -fPIC -g -shared -Wl,-soname,libfoo.so.2 -o libfoo.so.2.0.0 foo3.c
$ sudo cp libfoo.so.2.0.0 /usr/local/lib
$ sudo ldconfig
$ cd /usr/local/lib
$ sudo ln -fs libfoo.so.2 libfoo.so
```


### rpath
```
$ gcc -fPIC -g -shared  -o libfoo.so foo.c
$ gcc -g -o main main.c -lfoo -L. -Wl,-rpath,`pwd`
$ ldd main
        linux-vdso.so.1 (0x00007fffe5287000)
        libfoo.so => /home/cuiy0006/Projects/shared/shared-lib2/libfoo.so (0x00007f4d9e840000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f4d9e440000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f4d9ee00000)

$ objdump -p main | grep RPATH(RUNPATH?)
  RUNPATH              /home/cuiy0006/Projects/shared/shared-lib2
```

