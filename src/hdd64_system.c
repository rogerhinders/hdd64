#include <libdragon.h>
#include <hdd64_system.h>

#define TEST_CONST_1 0x00000000
#define TEST_CONST_2 0xffffffff
#define TEST_CONST_3 0xAAAAAAAA
#define TEST_CONST_4 0x55555555

#define MSIZE_64M  0x800000
#define MSIZE_128M 0x1000000
#define MSIZE_256M 0x2000000
#define MSIZE_384M 0x3000000
#define MSIZE_512M 0x4000000

#define DRAM_START 0xb4000000

uint32_t quick_ram_test() {
	uint32_t ram_size = 0;
	uint32_t ram_addr = DRAM_START;
	uint32_t test_size = 0x2000;

	//first test, any ram at all?
	if(ram_test_increment(ram_addr, test_size)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_1)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_2)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_3)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_4)!=0)
		return ram_size;

	//128M found
    ram_size = MSIZE_128M;
	ram_addr = DRAM_START + MSIZE_128M;

	if(ram_test_increment(ram_addr, test_size)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_1)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_2)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_3)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_4)!=0)
		return ram_size;

	//256M found
    ram_size = MSIZE_256M;
	ram_addr = DRAM_START + MSIZE_256M;

	if(ram_test_increment(ram_addr, test_size)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_1)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_2)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_3)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_4)!=0)
		return ram_size;

	//384M found
    ram_size = MSIZE_384M;
	ram_addr = DRAM_START + MSIZE_384M;

	if(ram_test_increment(ram_addr, test_size)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_1)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_2)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_3)!=0)
		return ram_size;
	if(ram_test_constant(ram_addr, test_size,TEST_CONST_4)!=0)
		return ram_size;

	//512M found
	ram_size = MSIZE_512M;

	return ram_size;
}

int ram_test_increment(uint32_t ram_addr, uint32_t num_bytes) {
    
    uint32_t j=0,i;
    for(i=0;i<num_bytes;i+=4) {
		io_write(ram_addr+i, j);
        j+=4;
    }

    j=0;
    for(i=0;i<num_bytes;i+=4) {
        if(io_read(ram_addr+i) != j)
            return 1;
        j+=4;
    }
    return 0;
}

int ram_test_constant(uint32_t ram_addr, uint32_t num_bytes, uint32_t constant) {
    
    uint32_t i;
    for(i=0;i<num_bytes;i+=4) {
        io_write(ram_addr+i, constant);
    }

    for(i=0;i<num_bytes;i+=4) {
        if(io_read(ram_addr+i) != constant)
            return 1;
    }
    return 0;
}
/*
 * BUG FIXED get_ticks_ms and wait_ms routines from libdragon!!
 */
volatile unsigned long __get_ticks_ms(void)
{
    unsigned long count;
    asm volatile("mfc0 %0,$9\n\t nop \n\t" : "=r" (count) : );

    return count / (COUNTS_PER_SECOND / 1000);
}

void __wait_ms( unsigned long wait )
{
    unsigned int stop = wait + __get_ticks_ms();

    while( stop > __get_ticks_ms() );
}
