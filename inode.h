#ifndef BWFS_INODE_H
#define BWFS_INODE_H


#include <inttypes.h>
#include <sys/stat.h>
#include "superblock.h"

#define N_DIRECT 6

typedef struct inode
{
    uint16_t userID;
    uint16_t groupID;
    uint32_t modo;
    uint32_t creationTime;
    uint32_t modiTime;
    int32_t size;
    uint32_t direct[N_DIRECT];
    uint32_t indir_1;
    uint32_t indir_2;

    uint32_t pad;


}inode;

typedef struct dirent
{
    uint32_t valid;
    uint32_t isDir;
    uint32_t inode;
    char filename[25];
}dirent;

enum {
    DIR_ENTS_PER_BLK = BLOCK_SIZE / sizeof(struct dirent),
    INODES_PER_BLK = BLOCK_SIZE / sizeof(struct inode),
    PTRS_PER_BLK = BLOCK_SIZE / sizeof(uint32_t)
};


void copy_stat(inode *inodo, struct stat * sb);


inode *crear_inodo(int modo, int size, int inodoID, int direct_array[N_DIRECT], int indir1, int indir2);


dirent *crear_dirent(int valid, int isDir, int inodoID, char *filename);


int set_update(dirent *de, char *name, mode_t modo, bool isDir);

#endif 