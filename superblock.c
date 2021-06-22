#include <stdlib.h>
#include "superblock.h"


uint32_t jenkins_one_at_a_time_hash(char *key, size_t len) {

    uint32_t hash, i;
    for(hash = i = 0; i < len; ++i) {
        hash += key[i];
        hash += (hash << 10); 
        hash ^= (hash >> 6); 
    }
    
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return abs(hash); 
}

void cifrar (void **data, uint32_t key){

    char *data_cifrar = *data;

    for(int i=0; i<BLOCK_SIZE;++i){
        data_cifrar[i] = data_cifrar[i]^ key;
    }
}

void decifrar(void **data, uint32_t key){

    char *data_cifrar = *data;

    for(int i=0; i<BLOCK_SIZE; ++i){
        data_cifrar[i] = data_cifrar[i] ^ key;
    }
}
