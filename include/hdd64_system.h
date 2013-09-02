#ifndef HDD64_SYSTEM_H
#define HDD64_SYSTEM_H

#define COUNTS_PER_SECOND (93750000/2)

uint32_t quick_ram_test();
int ram_test_increment(uint32_t ram_addr, uint32_t num_bytes);
int ram_test_constant(uint32_t ram_addr, uint32_t num_bytes, uint32_t constant);
volatile unsigned long __get_ticks_ms(void);
void __wait_ms( unsigned long wait );
#endif // HDD64_SYSTEM_H
