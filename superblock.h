#ifndef BWFS_SUPER_H
#define BWFS_SUPER_H

#include <inttypes.h>
#include <stdlib.h>

#define BLOCK_SIZE 1024
#define MAX_BLOCKS 1000
#define MAX_FILE_SIZE 102400

uint32_t jenkins_one_at_a_time_hash(char *key, size_t len);

void cifrar(void **data, uint32_t key);

void decifrar(void **data, uint32_t key);

#endif