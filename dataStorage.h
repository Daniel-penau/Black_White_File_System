#ifndef BWFS_DATASTORAGE_H
#define BWFS_DATASTORAGE_H

#include "superblock.h"

static int size = 0;
static int freeblocks[MAX_BLOCKS] = { 0 };

void getFreeBlocks();
void setFreeBlocks(int block);
void addFreeBlocks(int block);
int getOneBlock();
int getThatBlock(const char *path);


#endif