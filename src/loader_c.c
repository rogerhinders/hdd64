#include <libdragon.h>
#include <loader.h>

uint32_t loader_blob1[] = {
    0x3C08A000,
    0x35080300,
    0x8D130004,
    0x8D140000,
    0x8D15000C,
    0x8D170014,
    0x3C0BA400,
    0x356B0040,
    0x3C1DA400,
    0x37BD1FF0, 
    0x3C1FA400,
    0x37FF1550,
    0x3C10A000,
    0x36100400,
    0x3C11003F,
    0x3631FC00,
    0x00001021,
    0xAE020000, 
    0x2631FFFC, 
    0x1411FFFD,
    0x26100004,
    0x3C08A400, 
    0x35080040,
    0x01000008,
    0x00000000
};

uint32_t loader_blob2[] = {
    0x3C0DBFC0,
    0x8DA807FC,
    0x25AD07C0, 
    0x31080080,
    0x5500FFFC,
    0x3C0DBFC0,  
    0x8DA80024,
    0x3C0BB000
};

void boot_dram() {
	disable_interrupts();
	asm volatile(
		"	li $v0,0x80000000\n"
		"	li $v1,0x80001ff0\n"
		"cache1:\n"
		"	cache 1, 0($v0)\n"
		"	bne $v0,$v1,cache1\n"
		"	addiu $v0,$v0,0x10\n"
		"	li $v0,0x80000000\n"
		"	li $v1,0x80003fe0\n"
		"cache2:\n"
		"	cache 0,0($v0)\n"
		"	bne $v0,$v1,cache2\n"
		"	addiu $v0,$v0,0x20\n"
		"	li $s0,0xa4000040\n"
		"	li $v1,0xa4600010\n"
		"	li $s6,0x3f\n"
		"	li $s1,0xb0000040\n"
		"	li $s2,0xb0001000\n"
		"copy_boot:\n"
		"	lw $v0,0($v1)\n"
		"	nop\n"
		"	andi $v0,3\n"
		"	bnez $v0,copy_boot\n"
		"	nop\n"
		"	lw $v0,0($s1)\n"
		"	addiu $s1,$s1,4\n"
		"	sw $v0,0($s0)\n"
		"	bne $s1,$s2,copy_boot\n"
		"	addiu $s0,$s0,4\n"
		"	li $s0,0xa0000000\n"
		"	la $s1,%0\n"
		"	la $s2,%0\n"
		"	addiu $s2,$s2,0x64\n"
		"copy_loader1:\n"
		"	lw $v0,0($s1)\n"
		"	addiu $s1,$s1,4\n"
		"	sw $v0,0($s0)\n"
		"	bne $s1,$s2,copy_loader1\n"
		"	addiu $s0,$s0,4\n"
		"	li $s0,0xa4001000\n"
		"	la $s1,%1\n"
		"	la $s2,%1\n"
		"	addiu $s2,$s2,0x20\n"
		"copy_loader2:\n"
		"	lw $v0,0($s1)\n"
		"	addiu $s1,$s1,4\n"
		"	sw $v0,0($s0)\n"
		"	bne $s1,$s2,copy_loader2\n"
		"	addiu $s0,$s0,4"
		:"=r"(loader_blob1),"=r"(loader_blob2):
	);
}
