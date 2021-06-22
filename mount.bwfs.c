#include "mount.bwfs.h"
#include "superblock.h"
#include "inode.h"
#include "dataStorage.h"

#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>



static int DIR_SIZE = BLOCK_SIZE * N_DIRECT;

static int INDIR1_SIZE = (BLOCK_SIZE/ sizeof(uint32_t))* BLOCK_SIZE;

static int INDIR2_SIZE = (BLOCK_SIZE / sizeof(uint32_t)) * (BLOCK_SIZE / sizeof(uint32_t)) * BLOCK_SIZE;

void *myinit(struct fuse_conn_info *conn){
    return NULL;
}


char *strmode(char *buf, int mode){

    int mask = 0400;
    char *str = "rwxrwxrw" , *retval = buf;
    *buf++ = S_ISDIR(mode) ? 'd': '-';
    for (mask = 0400; mask != 0; str++, mask = mask >>1){

        *buf++ = (mask & mode) ? *str : '-';
    }
    *buf++ = 0;

    return retval;

}




int getattr(const char *path, struct stat *stbuf){

    char *dup_path = strdup(path);
    int inode_id = get



}