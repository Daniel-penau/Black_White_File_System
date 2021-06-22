#ifndef BWFS_SUPER_H
#define BWFS_SUPER_H

#include <inttypes.h>
#include <stdlib.h>

#define BLOCK_SIZE 1024

#define MAGIC 0xf0f03410

#define N_INODES 64

#define SUPERB_NUM 0

#define N_DATABLOCKS 1024

#define SUPERB_SIZE 24

typedef struct superblock
{
    uint32_t magic;
    uint32_t inode_map_sz;
    uint32_t block_map_sz;
    uint32_t inode_region_sz;
    uint32_t n_blocks;
    uint32_t root_inode;

}superblock;

uint32_t jenkins_one_at_a_time_hash(char *key, size_t len);

void cifrar(void **data, uint32_t key);

void decifrar(void **data, uint32_t key);

#endif