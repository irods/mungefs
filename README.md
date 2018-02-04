# mungefs
A FUSE file system overlay inspired by [Charybdefs](https://github.com/scylladb/charybdefs), which is commanded by Avro and Zeromq. The file system may be instructed to misbehave in order to better test software which relies on file system access.

## Starting the file system overlay:
./mungefs /mount/dir/ -omodules=subdir,subdir=/target/directory

## mungefsctl

A command line utility used to modify the behavior of the filesystem.
```
Usage:
--help : show command usage
--operations : list of operations to apply a fault
--random : randomize error injection
--err_no : error number to force
--probability : 0-100 probability of random error to inject
--regexp : regexp matching operations
--kill_caller : kill the calling process
--delay_us : delay a method by a given number of microseconds
--auto_delay : set delay to simulate ssd
--corrupt_data : corrupt read or write data
--corrupt_size : report an invalid file size
```
## Valid Operations:
    getattr
    readlink
    mknod
    mkdir
    unlink
    rmdir
    symlink
    rename
    link
    chmod
    chown
    truncate
    open
    read
    write
    statfs
    flush
    release
    fsync
    setxattr
    getxattr
    listxattr
    removexattr
    opendir
    readdir
    releasedir
    fsyncdir
    access
    create
    ftruncate
    fgetattr
    lock
    bmap
    ioctl
    poll
    flock
    fallocate

# Examples:

## Corrupting write operations:
```mungefsctl --operations "write" --corrupt_data```

## Corrupting read operations:
```mungefsctl --operations "read" --corrupt_data```

## Corrupting the reporting of filesize:
```mungefsctl --operations "getattr" --corrupt_data```

## Resetting the operations:
```
mungefsctl --operations "write"
mungefsctl --operations "read"
mungefsctl --operations "getattr"
```
