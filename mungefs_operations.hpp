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

#ifndef MUNGEFS_OPERATIONS_HPP
#define MUNGEFS_OPERATIONS_HPP

#include <stdint.h>
#include <sys/types.h>

#include <fuse.h>

int mungefs_getattr(const char *, struct stat *);
int mungefs_readlink(const char *, char *, size_t);
int mungefs_mknod(const char *, mode_t, dev_t);
int mungefs_mkdir(const char *, mode_t);
int mungefs_unlink(const char *);
int mungefs_rmdir(const char *);
int mungefs_symlink(const char *, const char *);
int mungefs_rename(const char *, const char *);
int mungefs_link(const char *, const char *);
int mungefs_chmod(const char *, mode_t);
int mungefs_chown(const char *, uid_t, gid_t);
int mungefs_truncate(const char *, off_t);
int mungefs_open(const char *, struct fuse_file_info *);
int mungefs_read(const char *, char *, size_t, off_t,
                 struct fuse_file_info *);
int mungefs_write(const char *, const char *, size_t, off_t,
                  struct fuse_file_info *);
int mungefs_statfs(const char *, struct statvfs *);
int mungefs_flush(const char *, struct fuse_file_info *);
int mungefs_release(const char *, struct fuse_file_info *);
int mungefs_fsync(const char *, int, struct fuse_file_info *);
int mungefs_setxattr(const char *, const char *, const char *, size_t, int);
int mungefs_getxattr(const char *, const char *, char *, size_t);
int mungefs_listxattr(const char *, char *, size_t);
int mungefs_removexattr(const char *, const char *);
int mungefs_opendir(const char *, struct fuse_file_info *);
int mungefs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                    off_t offset, struct fuse_file_info *fi);
int mungefs_releasedir(const char *, struct fuse_file_info *);
int mungefs_fsyncdir(const char *, int, struct fuse_file_info *);

void *mungefs_init(struct fuse_conn_info *conn);
void mungefs_destroy(void *);

int mungefs_access(const char *, int);
int mungefs_create(const char *, mode_t, struct fuse_file_info *);
int mungefs_ftruncate(const char *, off_t, struct fuse_file_info *);
int mungefs_fgetattr(const char *, struct stat *, struct fuse_file_info *);
int mungefs_lock(const char *, struct fuse_file_info *, int cmd,
                 struct flock *);
int mungefs_utimens(const char *, const struct timespec tv[2]);
int mungefs_bmap(const char *, size_t blocksize, uint64_t *idx);
int mungefs_ioctl(const char *, int cmd, void *arg,
                  struct fuse_file_info *, unsigned int flags, void *data);
int mungefs_poll(const char *path, struct fuse_file_info *fi,
                 struct fuse_pollhandle *ph, unsigned *reventsp);
int mungefs_flock(const char *, struct fuse_file_info *, int op);
int mungefs_fallocate(const char *, int, off_t, off_t,
                      struct fuse_file_info *);

#endif // MUNGEFS_OPERATIONS_HPP


