#define _GNU_SOURCE
//#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)
static char *buffer;
#define TABLEENTRIES 125000 // tables are 4mb, 32 bytes means there are 125k entries
#define DIRECENTRIES 250000000 // statically init 2 gigs for the directory, 4g/8 = 250,000,000 entries

typedef unsigned long long size_t;

//Bounds table is analagous to a page table. Addresses of these BTs are stored in a Bouds directory.
//We need a bound table Entry for everay possible pointer in the virtual address space

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

struct bound_t_struct * create_table(){ // create a table of 32 128 bit structures that store upper and lower bounds
    struct bound_t_struct * res = malloc(sizeof(struct bound_t_struct) * TABLEENTRIES);
    return res;
} //^^^ send this pointer to pkey_mprotect to generate a pkey.

struct bound_d_struct * create_dir(){ // this will create a directory of 8 64 bit structures that store the location of a bound table
    //struct bound_d_struct * res =  mmap(NULL, (sizeof(struct bound_d_struct)) * DIRECENTRIES, prot_set, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
   // return res;
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

//need functions for checking the greater than or less then operator compared to a given data location. (read) also change them (write)
//intel TSX is used for these (it seems to be implemented in posix threads already)


int
main(int argc, char *argv[])
{
    //int real_prot = PROT_READ|PROT_WRITE;
    //int pkey = pkey_alloc(0, PKEY_DISABLE_WRITE);
    //need to set the pkey?

    struct bound_t_struct test_struct = {145, 567};
    printf("The size of the test obect is %d", sizeof(test_struct));

    struct bound_t_struct * ptr_table = create_table();

    ptr_table[12].upper = 126;
    ptr_table[12].lower = 130;

    /*char *boundt_ptr, *boundd_ptr;
    boundt_ptr = mmap(NULL, METADATA_SIZE, real_prot, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    assert(boundt_ptr != MAP_FAILED);
    boundd_ptr = mmap(NULL, METADATA_SIZE, real_prot, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    assert(boundd_ptr != MAP_FAILED);*/
    //int ret = pkey_mprotect(ptr, PAGE_SIZE, real_prot, pkey);

    //pkey_set(pkey, PKEY_DISABLE_WRITE);

    /*printf("second location: %c\n", ptr[1]);
    ptr[0] = 0xF4;
    ptr[1] = 0x25;
    printf("second location after assignment: %c\n", ptr[1]);
    printf("the location of this memory obect is: %p \n", ptr);*/

    //So now lets try to make some pointers and translate them into the metadat table
    char name [6] = "james";
    printf("printing string here %s\n", name);
    printf("Location of the pointer is %p\n", name);

    size_t addr = getAddr(name);
    size_t addr1;
    size_t addr2;

    /*size_t boundlocal = get_bounds_location(addr);
    printf("The result is %zu\n", boundlocal);*/




   // int err_t = munmap(boundt_ptr, METADATA_SIZE);
    //int err_d = munmap(boundd_ptr, METADATA_SIZE);
    //assert(err_t >= 0);
    //assert(err_d >= 0);
    free(ptr_table);


    //pkey_set(pkey, 0);

    
}