#ifndef BWFS_SUPER_H
#define BWFS_SUPER_H

#include <inttypes.h>
#include <stdlib.h>

#define BLOCK_SIZE 4096
#define MAX_BLOCKS 10000
#define MAX_FILE_SIZE 1638400

uint32_t jenkins_one_at_a_time_hash(char *key, size_t len);

void cifrar(void **data, uint32_t key);

void decifrar(void **data, uint32_t key);

#endif