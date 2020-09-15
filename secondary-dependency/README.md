## Creating the libfoo.so:
```
    $ gcc -shared -o libfoo.so -fPIC foo.c
```

## Creating the libfoo.a:
```
    $ gcc -c foo.c
    $ ar rcs libfoo.a foo.o
```

## Creating the libbar.a:
```
    $ gcc -c bar.c
    $ ar rcs libbar.a bar.o
            foo.o is not included in the archive, so 
            0000000000000000 T bar
                            U foo
```

## Creating the libbar.so:
1) build with static libfoo.a:
```
    $ gcc -shared -o libbar_static_foo.so -fPIC bar.c libfoo.a -L$(pwd)
        00000000000005ba T bar
        00000000000005ca T foo
```
        
2) build with shared libfoo.so:
```
    $ gcc -shared -o libbar_shared_foo.so -fPIC bar.c libfoo.so -L$(pwd)
        00000000000005ca T bar
                         U foo
```

3) build with no libfoo (dumb):
```
    $ gcc -shared -o libbar_dumb.so -fPIC bar.c
        00000000000005ba T bar
                         U foo
```

## Linking against the libbar.a static library:
```
    $ gcc -o app_s main.c libbar.a
        libbar.a(bar.o): In function `bar':
        bar.c:(.text+0xa): undefined reference to `foo'
        collect2: error: ld returned 1 exit status
```
```
    $ gcc -o app main.c libbar.a -L$(pwd) libfoo.a
        $ nm app
        000000000000060a T bar
        000000000000061a T foo
        00000000000005fa T main

        $ readelf -d app
        0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]

        $ ./app
```

```
    $ gcc -o app main.c libbar.a -L$(pwd) libfoo.so
        $ nm app
        000000000000071a T bar
                         U foo
        000000000000070a T main

        $ readelf -d app
        0x0000000000000001 (NEEDED)             Shared library: [libfoo.so]
        0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]

        $ LD_LIBRARY_PATH=$(pwd) ./app
```

## Linking against the libbar_dumb.so shared library
```
    $ gcc -o app main.c -L$(pwd) -lbar_dumb
        /home/cuiy0006/Projects/shared/1/libbar_dumb.so: undefined reference to `foo'
        collect2: error: ld returned 1 exit status
```

```
    $ gcc -o app main.c -L$(pwd) -lbar_dumb libfoo.so
        $ readelf -d app
        0x0000000000000001 (NEEDED)             Shared library: [libbar_dumb.so]
        0x0000000000000001 (NEEDED)             Shared library: [libfoo.so]
        0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
        $ nm app
                         U bar
        000000000000071a T main

        $ LD_LIBRARY_PATH=$(pwd) ./app
```

```
    $ gcc -o app main.c -L$(pwd) -lbar_dumb libfoo.a
        $ readelf -d app
        0x0000000000000001 (NEEDED)             Shared library: [libbar_dumb.so]
        0x0000000000000001 (NEEDED)             Shared library: [libc.so.6] 
        $ nm app
                         U bar
        000000000000074a T foo
        000000000000073a T main

        $LD_LIBRARY_PATH=$(pwd) ./app
```

## Linking against the libbar_static_foo.so shared library
```
    $ gcc -o app main.c -L$(pwd) -lbar_static_foo
        $ readelf -d app
        0x0000000000000001 (NEEDED)             Shared library: [libbar_static_foo.so]
        0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
        $ nm app
                         U bar
        000000000000071a T main
        
        $ LD_LIBRARY_PATH=$(pwd) ./app
```

## Linking against the libbar_shared_foo.so shared liWbrary
```
    $ gcc -o app main.c -L$(pwd) -lbar_shared_foo
        /usr/bin/ld: warning: libfoo.so, needed by /home/cuiy0006/Projects/shared/1/libbar_shared_foo.so, not found (try using -rpath or -rpath-link)
        /home/cuiy0006/Projects/shared/1/libbar_shared_foo.so: undefined reference to `foo'
        collect2: error: ld returned 1 exit status
```

```
    $ gcc -o app main.c -L$(pwd) -lbar_shared_foo -Wl,-rpath-link=$(pwd)
        $ readelf -d app
        0x0000000000000001 (NEEDED)             Shared library: [libbar_shared_foo.so]
        0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
        $ nm app
                         U bar
        000000000000071a T main

    $ LD_LIBRARY_PATH=$(pwd) ./app
```

### Difference between rpath and rpath-link
    rpath-link is linking time path for dynamic libs, while rpath is for both linking time and runtime.  
    when rpath doesn't work, use -Wl,--disable-new-dtags  
