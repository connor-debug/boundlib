#define _GNU_SOURCE
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)
static char *buffer;
#define METADATA_SIZE 17000000 // 17mb
//Prob wrong, I need to write a function that takes in a pointer address and maps it to an offset.
//this will essentially be the locatino in the data structure that it will live.

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

size_t getAddr (void * s){
    size_t int_value = (size_t)s;
    return int_value;
}

int
main(int argc, char *argv[])
{

    int real_prot = PROT_READ|PROT_WRITE;
    //int pkey = pkey_alloc(0, PKEY_DISABLE_WRITE);
    //need to set the pkey?

    char *boundt_ptr, *boundd_ptr;
    boundt_ptr = mmap(NULL, METADATA_SIZE, real_prot, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    assert(boundt_ptr != MAP_FAILED);
    boundd_ptr = mmap(NULL, METADATA_SIZE, real_prot, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    assert(boundd_ptr != MAP_FAILED);
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

    printf("Location of the derived pointer address is %zu\n", addr);
    printf("This location in binary form is "PRINTF_BINARY_PATTERN_INT64 "\n", PRINTF_BYTE_TO_BINARY_INT64(addr));

    int err_t = munmap(boundt_ptr, METADATA_SIZE);
    int err_d = munmap(boundd_ptr, METADATA_SIZE);
    assert(err_t >= 0);
    assert(err_d >= 0);


    //pkey_set(pkey, 0);

    
}
