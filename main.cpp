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

#define FUSE_USE_VERSION 29

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "mungefs_operations.hpp"

static struct fuse_operations mungefs_oper = {
	.getattr     = mungefs_getattr,
    .readlink    = mungefs_readlink,
    .mknod       = mungefs_mknod,
    .mkdir       = mungefs_mkdir,
    .unlink      = mungefs_unlink,
    .rmdir       = mungefs_rmdir,
    .symlink     = mungefs_symlink,
    .rename      = mungefs_rename,
    .link        = mungefs_link,
    .chmod       = mungefs_chmod,
    .chown       = mungefs_chown,
    .truncate    = mungefs_truncate,
	.open		 = mungefs_open,
	.read		 = mungefs_read,
    .write       = mungefs_write,
    .statfs      = mungefs_statfs,
    .flush       = mungefs_flush,
    .release     = mungefs_release,
    .fsync       = mungefs_fsync,
    .setxattr    = mungefs_setxattr,
    .getxattr    = mungefs_getxattr,
    .listxattr   = mungefs_listxattr,
    .removexattr = mungefs_removexattr,
    .opendir     = mungefs_opendir,
    .readdir     = mungefs_readdir,
    .releasedir  = mungefs_releasedir,
    .fsyncdir    = mungefs_fsyncdir,

    .init        = mungefs_init,
    .destroy     = mungefs_destroy,

    .access      = mungefs_access,
    .create      = mungefs_create,
    .ftruncate   = mungefs_ftruncate,
    .fgetattr    = mungefs_fgetattr,
    .lock        = mungefs_lock,
    .utimens     = mungefs_utimens,
    .bmap        = mungefs_bmap,
    .ioctl       = mungefs_ioctl,
    .poll        = mungefs_poll,
    .flock       = mungefs_flock,
    .fallocate   = mungefs_fallocate,
};

int main(int argc, char *argv[]) {
    printf("starting fuse filesystem\n");
	return fuse_main(argc, argv, &mungefs_oper, NULL);
}



