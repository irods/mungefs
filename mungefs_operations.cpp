/*
 * ** 27-12-2015
 * **
 * ** The author disclaims copyright to this source code.  In place of
 * ** a legal notice, here is a blessing:
 * **
 * **    May you do good and not evil.
 * **    May you find forgiveness for yourself and forgive others.
 * **    May you share freely, never taking more than you give.
 * **
 */


#include <iostream>
#include <fstream>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <dirent.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include <string.h>

#include "mungefs_operations.hpp"
#include "mungefs_server.hpp"

static std::ofstream err_log;

#define ERR_LOG err_log << __FUNCTION__ << ":" << __LINE__ << " - "

int mungefs_getattr(const char *path, struct stat *buf) {
    bool corrupt_flag = false;
    int ret = evaluate_fault_for_operation(
                  path,
                  "getattr",
                  corrupt_flag);
    if (ret) {
        return ret;
    }

    ret = stat(path, buf);
    if (ret < 0) {
        return -errno;
    }
   
    if(corrupt_flag) {
       buf->st_size = buf->st_size / 2;
    }
    
    return 0;    
}

int mungefs_readlink(const char *path, char *buf, size_t bufsiz) {
    int ret = evaluate_fault_for_operation(path, "readlink");
    if (ret) {
        return ret;
    }

    ret = readlink(path, buf, bufsiz);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_mknod(const char *path, mode_t mode, dev_t dev) {
    int ret = evaluate_fault_for_operation(path, "mknod");
    if (ret) {
        return ret;
    }

    ret = mknod(path, mode, dev);    
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_mkdir(const char *path, mode_t mode) {
    int ret = evaluate_fault_for_operation(path, "mkdir");
    if (ret) {
        return ret;
    }

    ret = mkdir(path, mode);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_unlink(const char *path) {
    int ret = evaluate_fault_for_operation(path, "unlink");
    if (ret) {
        return ret;
    }

    ret = unlink(path); 
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_rmdir(const char *path) {
    int ret = evaluate_fault_for_operation(path, "rmdir");
    if (ret) {
        return ret;
    }

    ret = rmdir(path); 
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_symlink(const char *target, const char *linkpath) {
    int ret = evaluate_fault_for_operation(target, "symlink");
    if (ret) {
        return ret;
    }

    ret = evaluate_fault_for_operation(linkpath, "symlink");
    if (ret) {
        return ret;
    }

    ret = symlink(target, linkpath);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_rename(const char *oldpath, const char *newpath) {
    int ret = evaluate_fault_for_operation(oldpath, "rename");
    if (ret) {
        return ret;
    }

    ret = evaluate_fault_for_operation(newpath, "rename");
    if (ret) {
        return ret;
    }

    ret = rename(oldpath, newpath);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_link(const char *oldpath, const char *newpath) {
    int ret = evaluate_fault_for_operation(oldpath, "link");
    if (ret) {
        return ret;
    }

    ret = evaluate_fault_for_operation(newpath, "link");
    if (ret) {
        return ret;
    }
    
    ret = link(oldpath, newpath);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_chmod(const char *path, mode_t mode) {
    int ret = evaluate_fault_for_operation(path, "chmod");
    if (ret) {
        return ret;
    }
    
    ret = chmod(path, mode);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_chown(
    const char *path,
    uid_t owner,
    gid_t group) {
    int ret = evaluate_fault_for_operation(path, "chown");
    if (ret) {
        return ret;
    }

    ret = chown(path, owner, group);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_truncate(
    const char *path,
    off_t length) {
    int ret = evaluate_fault_for_operation(path, "truncate");
    if (ret) {
        return ret;
    }

    ret = truncate(path, length); 
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_open(
    const char *path,
    struct fuse_file_info *fi) {
    int ret = evaluate_fault_for_operation(path, "open");
    if (ret) {
        return ret;
    }

    ret = open(path, fi->flags);
    if (ret < 0) {
        return -errno;
    }

    fi->fh = ret;
    return 0;
}

int mungefs_read(
    const char*            path,
    char*                  buf,
    size_t                 size,
    off_t                  offset,
    struct fuse_file_info* fi) {
    bool corrupt_flag = false;
    int ret = evaluate_fault_for_operation(
                  path,
                  "read",
                  corrupt_flag);
    if (ret) {
        return ret;
    }

    ret = pread(fi->fh, buf, size, offset);
    if (ret == -1) {
        ret = -errno;
    }

    if(corrupt_flag) {
       memset(buf, 'x', size);
    }

    return ret;
}

int mungefs_write(
    const char*            path,
    const char*            buf,
    size_t                 size,
    off_t                  offset,
    struct fuse_file_info* fi) {

    bool corrupt_flag = false;
    int ret = evaluate_fault_for_operation(
                  path,
                  "write",
                  corrupt_flag);
    if(ret) {
        return ret;
    }

    if(corrupt_flag) {
        char bad_buf[size];
        memset(bad_buf, 'x', size);
        ret = pwrite(fi->fh, bad_buf, size, offset);
    }
    else {
        ret = pwrite(fi->fh, buf, size, offset);
    }

    if(-1 == ret) {
        ret = -errno;
    }

    return ret;
}

int mungefs_statfs(
    const char *path,
    struct statvfs *buf) {
    int ret = evaluate_fault_for_operation(
                  path,
                  "statfs");
    if(ret) {
        return ret;
    }

    ret = statvfs(path, buf);
    if(ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_flush(
    const char *path,
    struct fuse_file_info *fi) {
    int ret = evaluate_fault_for_operation(path, "flush");
    if (ret) {
        return ret;
    }

    /* Took from fuse examples */
    ret = close(dup(fi->fh));
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_release(
    const char *path,
    struct fuse_file_info *fi) {
    int ret = evaluate_fault_for_operation(path, "release");
    if (ret) {
        return ret;
    }

    close(fi->fh);

    return 0;    
}

int mungefs_fsync(
    const char *path,
    int datasync,
    struct fuse_file_info *fi) {
    int ret = evaluate_fault_for_operation(path, "fsync");
    if (ret) {
        return ret;
    }

    if (datasync) {
        ret = fdatasync(fi->fh);
        if (ret < 0) {
            return -errno;
        }
    } else {
        ret = fsync(fi->fh);
        if (ret < 0) {
            return -errno;
        }
    }

    return 0;
}

int mungefs_setxattr(
    const char *path,
    const char *name,
    const char *value,
    size_t size,
    int flags) {
    int ret = evaluate_fault_for_operation(path, "setxattr");
    if (ret) {
        return ret;
    }

    ret = setxattr(path, name, value, size, flags);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int mungefs_getxattr(
     const char *path,
     const char *name,
     char *value,
     size_t size) {
    int ret = evaluate_fault_for_operation(path, "getxattr");
    if (ret) {
        return ret;
    }

    ret = getxattr(path, name, value, size);
    if (ret < 0) {
        return -errno;
    }
    
    return ret;
}

int mungefs_listxattr(
    const char *path,
    char *list,
    size_t size) {
    int ret = evaluate_fault_for_operation(path, "listxattr");
    if (ret) {
        return ret;
    }

    ret = listxattr(path, list, size);
    if (ret < 0) {
        return -errno;
    }
    
    return ret;
}

int mungefs_removexattr(
   const char *path,
    const char *name) {
    int ret = evaluate_fault_for_operation(path, "removexattr");
    if (ret) {
        return ret;
    }


    ret = removexattr(path, name);
    if (ret < 0) {
        return -errno;
    }
    
    return 0;    
}

int mungefs_opendir(
    const char *path,
    struct fuse_file_info *fi) {
    int ret = evaluate_fault_for_operation(path, "opendir");
    if (ret) {
        return ret;
    }

    auto dir = opendir(path);

    if (!dir) {
        return -errno;
    }
    
    fi->fh = (int64_t) dir;
    return 0;    
}

int mungefs_readdir(
    const char *path,
    void *buf,
    fuse_fill_dir_t filler,
    off_t offset,
    struct fuse_file_info *fi) {
    int ret = evaluate_fault_for_operation(path, "readdir");
    if (ret) {
        return ret;
    }

    DIR *dp = (DIR *) fi->fh;
    struct dirent *de;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }
    
    return 0;    
}


int mungefs_releasedir(
    const char *path,
    struct fuse_file_info *fi) {
    int ret = evaluate_fault_for_operation(path, "releasedir");
    if (ret) {
        return ret;
    }

    DIR *dir = (DIR *) fi->fh;

    ret = closedir(dir);
    if (ret < 0) {
        return -errno;
    }
    
    return 0;    
}

int mungefs_fsyncdir(
    const char *path, int datasync,
    struct fuse_file_info *fi) {
    int ret = evaluate_fault_for_operation(path, "fsyncdir");
    if (ret) {
        return ret;
    }

    auto dir = opendir(path);
    if (!dir) {
        return -errno;
    }

    if (datasync) {
        ret = fdatasync(dirfd(dir));
        if (ret < 0) {
            return -errno;
        }
    } else {
        ret = fsync(dirfd(dir));
        if (ret < 0) {
            return -errno;
        }
    }

    closedir(dir);

    return 0;
}

void *mungefs_init(struct fuse_conn_info *conn) {
    //err_log.open("/tmp/mungefs_operation_log.txt", std::fstream::in|std::fstream::out|std::fstream::app);
    start_server_thread();
    return NULL;
}

void mungefs_destroy(void *) {
    //err_log.close();
    stop_server_thread();
}

int mungefs_access(
    const char *path,
    int mode) {
    int ret = evaluate_fault_for_operation(path, "access");
    if (ret) {
        return ret;
    }

    ret = access(path, mode); 
    if (ret < 0) {
        return -errno;
    }
    
    return 0;    
}

int mungefs_create(
    const char *path,
    mode_t mode,
    struct fuse_file_info *fi) {
    int ret = evaluate_fault_for_operation(path, "create");
    if (ret) {
        return ret;
    }

    ret = creat(path, mode);
    if (ret < 0) {
        return -errno;
    }

    fi->fh = ret;

    return 0;    
}

int mungefs_ftruncate(
    const char *path,
    off_t length,
    struct fuse_file_info *fi) {
    int ret = evaluate_fault_for_operation(path, "ftruncate");
    if (ret) {
        return ret;
    }

    ret = truncate(path, length);
    if (ret < 0) {
        return -errno;
    }
    
    return 0;    
}

int mungefs_fgetattr(
    const char *path,
    struct stat *buf,
    struct fuse_file_info *fi) {

    bool corrupt_flag = false;
    int ret = evaluate_fault_for_operation(
                  path,
                  "fgetattr",
                  corrupt_flag);
    if (ret) {
        return ret;
    }

    ret = fstat((int) fi->fh, buf);
    if (ret < 0) {
        return -errno;
    }

    if(corrupt_flag) {
        buf->st_size = buf->st_size / 2;
    }
    
    return 0;    
}

int mungefs_lock(
    const char *path,
    struct fuse_file_info *,
    int cmd,
    struct flock *) {
    int ret = evaluate_fault_for_operation(path, "lock");
    if (ret) {
        return ret;
    }

    std::cout << "mungefs_lock: unimplemented." << std::endl;
    
    return 0;    
}

int mungefs_utimens(
    const char *path,
    const struct timespec tv[2]) {
    int ret = evaluate_fault_for_operation(path, "bmap");
    if (ret) {
        return ret;
    }
    
    std::cout << "mungefs_utimens: unimplemented." << std::endl;

    return 0;    
}

int mungefs_bmap(
    const char *path,
    size_t blocksize,
    uint64_t *idx) {
    int ret = evaluate_fault_for_operation(path, "bmap");
    if (ret) {
        return ret;
    }

    std::cout << "mungefs_bmap: unimplemented." << std::endl;
    
    return 0;    
}

int mungefs_ioctl(
    const char *path,
    int cmd,
    void *arg,
    struct fuse_file_info *fi,
    unsigned int flags,
    void *data) {
    int ret = evaluate_fault_for_operation(path, "ioctl");
    if (ret) {
        return ret;
    }

    ret = ioctl(fi->fh, cmd, arg);
    if (ret < 0) {
        return -errno;
    }
    
    return 0;    
}

int mungefs_poll(
    const char *path,
    struct fuse_file_info *fi,
    struct fuse_pollhandle *ph,
    unsigned *reventsp) {
    int ret = evaluate_fault_for_operation(path, "poll");
    if (ret) {
        return ret;
    }

    std::cout << "mungefs_poll: unimplemented." << std::endl;
    
    return 0;    
}

int mungefs_flock(const char *path, struct fuse_file_info *fi, int op) {
    int ret = evaluate_fault_for_operation(path, "flock");
    if (ret) {
        return ret;
    }

    ret = flock(((int) fi->fh), op);
    if (ret < 0) {
        return -errno;
    }
    
    return 0;    
}

int mungefs_fallocate(
    const char *path,
    int mode,
    off_t offset,
    off_t len,
    struct fuse_file_info *fi) {
    int ret = evaluate_fault_for_operation(path, "fallocate");
    if (ret) {
        return ret;
    }

    ret = fallocate((int) fi->fh, mode, offset, len);
    if (ret < 0) {
        return -errno;
    }
    
    return 0;    
}

