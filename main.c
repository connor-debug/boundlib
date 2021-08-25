#define _GNU_SOURCE
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)
static char *buffer;
#define TABLEENTRIES 125000 // tables are 4mb, 32 bytes means there are 125k entries
#define DIRECENTRIES 250000000 // statically init 2 gigs for the directory, 2g/8 = 250,000,000 entries

//typedef unsigned long long size_t;

//We need a bound table Entry for every possible pointer in the virtual address space(2^48)

/* --- PRINTF_BYTE_TO_BINARY macro's --- */

#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)    \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
    PRINTF_BINARY_PATTERN_INT8              PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
    PRINTF_BYTE_TO_BINARY_INT8((i) >> 8),   PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 \
    PRINTF_BINARY_PATTERN_INT16             PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
    PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64    \
    PRINTF_BINARY_PATTERN_INT32             PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
    PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), PRINTF_BYTE_TO_BINARY_INT32(i)
/* --- end macros --- */

/* These structs represent entries in their respective directory/tables*/

struct bound_t_struct{
    size_t upper, lower, reserved, pointer_value;
};

struct bound_d_struct{
    struct bound_t_struct * table_ptr;
};

size_t shift_bits_n(size_t virtual, int n){
    return (virtual >> n);
}

size_t get_offset_16(size_t virtual){
    size_t mask = 65535;
    return virtual & mask;
}

size_t get_offset_27 (size_t virtual){
    size_t mask = 134217727;
    return virtual & mask;
}

size_t get_bits_20_47(size_t virtual){
    size_t res = shift_bits_n(virtual, 20);
    res = get_offset_27(res);
    return res;
}

size_t get_bits_3_19(size_t virtual){
    size_t res = shift_bits_n(virtual, 3);
    res = get_offset_16(res);
    return res;
}

size_t get_offset_3_19(size_t virtual){
    size_t res = get_bits_3_19(virtual);
    res = shift_bits_n(res, 5);
    return res;
}

size_t get_offset_20_47(size_t virtual){
    size_t res = get_bits_20_47(virtual);
    res = shift_bits_n(res, 3);
    return res;
}

size_t getAddr (void * s){
    size_t int_value = (size_t)s;
    return int_value;
}

int prot_set = PROT_READ | PROT_WRITE;

struct bound_t_struct * create_table(){ // 
    struct bound_t_struct * res =  mmap(NULL, (sizeof(struct bound_t_struct)) * TABLEENTRIES, prot_set, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    perror("Checking for error");
    return res;
} //^^^ send this pointer to pkey_mprotect to generate a pkey.

struct bound_d_struct * create_dir(){ //
    struct bound_d_struct * res =  mmap(NULL, (sizeof(struct bound_d_struct)) * DIRECENTRIES, prot_set, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    perror("Checking for error");
    return res;
} //^^^ send this pointer to pkey_mprotect to generate a pkey.

void store_bound_t(struct bound_d_struct * dir, struct bound_t_struct * table, size_t loc){
    //pkeys to allow memory block to be written to (send the pkey along to the function)
    dir[loc].table_ptr = table;
    //disable write
} 

void insert_to_table(struct bound_t_struct * table, size_t loc, size_t upper, size_t lower, size_t pointer_value, size_t reserved){
    //pkeys to allow write, send the pkey along with the function.
    table[loc].upper = upper;
    table[loc].lower = lower;
    table[loc].pointer_value = pointer_value;
    table[loc].reserved = reserved;
    //disable write
}

bool does_table_exist(struct bound_d_struct *dir, size_t loc){
    if (dir[loc].table_ptr != NULL){
        return true;
    }
    else return false;
}

//need functions for checking the greater than or less then upper/lower bounds
//intel TSX is used for these (it seems to be implemented in posix threads already)


int
main(int argc, char *argv[])
{
    //int real_prot = PROT_READ|PROT_WRITE;
    //int pkey = pkey_alloc(0, PKEY_DISABLE_WRITE);
    //need to set the pkey?

    struct bound_d_struct * ptr_direc = create_dir();
    struct bound_t_struct * ptr_table = create_table();

    /*char *boundt_ptr, *boundd_ptr;
    boundt_ptr = mmap(NULL, METADATA_SIZE, real_prot, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    assert(boundt_ptr != MAP_FAILED);
    boundd_ptr = mmap(NULL, METADATA_SIZE, real_prot, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    assert(boundd_ptr != MAP_FAILED);*/
    //int ret = pkey_mprotect(ptr, PAGE_SIZE, real_prot, pkey);

    //pkey_set(pkey, PKEY_DISABLE_WRITE);

    /*Check is a table is at location 30k, no, then store table there and check*/

    bool result = does_table_exist(ptr_direc, 30000);
    printf("%d\n",result);

    store_bound_t(ptr_direc, ptr_table, 30000);

    bool result2 = does_table_exist(ptr_direc, 30000);
    printf("%d\n",result2);



    int err_t = munmap(ptr_table, (sizeof(struct bound_t_struct)) * TABLEENTRIES);
    int err_d = munmap(ptr_direc, (sizeof(struct bound_d_struct)) * DIRECENTRIES);
    assert(err_t >= 0);
    assert(err_d >= 0);
    //free(ptr_table); to free in mmap, you use munmap
    //free(ptr_direc);
    
}