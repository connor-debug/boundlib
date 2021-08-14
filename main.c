#define _GNU_SOURCE
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)
static char *buffer;
#define METADATA_SIZE 131072 // 17 bit offset, 17 + 17 = 34
#define PAGE_NUM_BITS 8
//Bounds table is analagous to a page table. Addresses of these BTs are stored in a Bouds directory.
//We need a bound table Entry for everay possible pointer in the virtual address space
//We can do it in bigger chunks. Each page (or bounds) covers 4kb of data. 

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

struct bound_t_struct{ /*This structure stores the upper and lower bounds*/
    size_t upper, lower;
};

size_t get_boundt_number(size_t virtual){
    return (virtual >> PAGE_NUM_BITS);
}

size_t shift_bits_20(size_t virtual){
    return (virtual >> 20);
}

size_t shift_bits_3(size_t virtual){
    return (virtual >> 3);
}


size_t get_offset_16(size_t virtual){
    size_t mask = 65535;
    return virtual & mask;
}

size_t get_offset(size_t virtual){
    size_t mask = 255;
    return virtual & mask;
}

size_t get_offset_27 (size_t virtual){
    size_t mask = 134217727;
    return virtual & mask;
}

size_t get_bits_20_47(size_t virtual){
    size_t res = shift_bits_20(virtual);
    res = get_offset_27(res);
    return res;
}

size_t get_bits_3_19(size_t virtual){
    size_t res = shift_bits_3(virtual);
    res = get_offset_16(res);
    return res;
}


size_t get_bounds_location(size_t virtual){
    size_t bounds = get_offset(virtual) + get_boundt_number(virtual);
    return bounds;
}


size_t getAddr (void * s){
    size_t int_value = (size_t)s;
    return int_value;
}

struct bound_t_struct * create_table(){
    struct bound_t_struct * res = malloc(sizeof(struct bound_t_struct) * 32);
    return res;
}

int
main(int argc, char *argv[])
{
    int real_prot = PROT_READ|PROT_WRITE;
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

    printf("Location of the derived pointer address is %zu\n", addr);
    printf("This location in binary form is "PRINTF_BINARY_PATTERN_INT64 "\n", PRINTF_BYTE_TO_BINARY_INT64(addr));

    size_t first_chunk = get_bits_20_47(addr);
    printf("The first chunk is: "PRINTF_BINARY_PATTERN_INT64 "\n", PRINTF_BYTE_TO_BINARY_INT64(first_chunk));

    size_t second_chunk = get_bits_3_19(addr);
    printf("The second chunk is: "PRINTF_BINARY_PATTERN_INT64 "\n", PRINTF_BYTE_TO_BINARY_INT64(second_chunk));


    addr1 = get_offset(addr);
    printf("the offset (8 bytes ) is %d\n", addr1);
    printf("binary form of the offset is "PRINTF_BINARY_PATTERN_INT64 "\n", PRINTF_BYTE_TO_BINARY_INT64(addr1));

    size_t bound_page_number = get_boundt_number(addr);
    printf("The result is %zu\n", bound_page_number);
    printf("binary form of the boundtable number is "PRINTF_BINARY_PATTERN_INT64 "\n", PRINTF_BYTE_TO_BINARY_INT64(bound_page_number));

    
    size_t corrected_bits = get_offset_16(bound_page_number);
    printf("The result is %zu\n", corrected_bits);
    printf("these are the bits masked with the offset removed "PRINTF_BINARY_PATTERN_INT64 "\n", PRINTF_BYTE_TO_BINARY_INT64(corrected_bits));

    /*size_t boundlocal = get_bounds_location(addr);
    printf("The result is %zu\n", boundlocal);*/




   // int err_t = munmap(boundt_ptr, METADATA_SIZE);
    //int err_d = munmap(boundd_ptr, METADATA_SIZE);
    //assert(err_t >= 0);
    //assert(err_d >= 0);
    free(ptr_table);


    //pkey_set(pkey, 0);

    
}
