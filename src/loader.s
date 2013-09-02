.section .data
loader2:
    .word 0x3C0DBFC0
    .word 0x8DA807FC 
    .word 0x25AD07C0 
    .word 0x31080080
    .word 0x5500FFFC 
    .word 0x3C0DBFC0  
    .word 0x8DA80024 
    .word 0x3C0BB000
loader1:
    .word 0x3C08A000
    .word 0x35080300
    .word 0x8D130004
    .word 0x8D140000
    .word 0x8D15000C
    .word 0x8D170014 
    .word 0x3C0BA400
    .word 0x356B0040
    .word 0x3C1DA400
    .word 0x37BD1FF0  
    .word 0x3C1FA400
    .word 0x37FF1550
    .word 0x3C10A000
    .word 0x36100400 
    .word 0x3C11003F
    .word 0x3631FC00
    .word 0x00001021
    .word 0xAE020000  
    .word 0x2631FFFC 
    .word 0x1411FFFD
    .word 0x26100004 
    .word 0x3C08A400 
    .word 0x35080040 
    .word 0x01000008
    .word 0x00000000

.section .text

.align 4
.set nomips16
.set push
.set noreorder
.set noat

.global boot_dram
.ent boot_dram
boot_dram:
    li $v0,0x80000000
    li $v1,0x80001ff0

cache1:
    cache 1, 0($v0)
    bne $v0,$v1,cache1
    addiu $v0,$v0,0x10

    li $v0,0x80000000
    li $v1,0x80003fe0
cache2:
    cache 0,0($v0)
    bne $v0,$v1,cache2
    addiu $v0,$v0,0x20

    li $s0,0xa4000040
    li $v1,0xa4600010

    li $s6,0x3f
    li $s1,0xb0000040
    li $s2,0xb0001000

copy_boot:
    lw $v0,0($v1)
    nop
    andi $v0,3
    bnez $v0,copy_boot
    nop

    lw $v0,0($s1)
    addiu $s1,$s1,4
    sw $v0,0($s0)
    bne $s1,$s2,copy_boot
    addiu $s0,$s0,4

    li $s0,0xa0000000
    la $s1,loader1
    la $s2,loader1+0x64
copy_loader1:
    lw $v0,0($s1)
    addiu $s1,$s1,4
    sw $v0,0($s0)
    bne $s1,$s2,copy_loader1
    addiu $s0,$s0,4

    li $s0,0xa4001000
    la $s1,loader2
    la $s2,loader2+0x20

copy_loader2:
    lw $v0,0($s1)
    addiu $s1,$s1,4
    sw $v0,0($s0)
    bne $s1,$s2,copy_loader2
    addiu $s0,$s0,4

    li $a0,5
    jal cd64_setmode_asm
    nop

    jal cd64_disable_asm
    nop

    lui $s3,0x8000
    jr $s3
    nop
.end boot_dram

.global cd64_disable_asm
.ent cd64_disable_asm
cd64_disable_asm:
    addiu $sp,$sp,-4
    sw $ra,0($sp)
    
    jal wait_dma
    nop

    li $t0,0
    li $t1,0xb780000c
    sw $t0,0($t1)
    lw $ra,0($sp)
    addiu $sp,$sp,4

    jr $ra
    nop
.end cd64_disable_asm

.global cd64_setmode_asm
.ent cd64_setmode_asm
cd64_setmode_asm:
    addiu $sp,$sp,-4
    sw $ra,0($sp)

    jal wait_dma
    nop

    li $t0,0xb7800000
    sw $a0,0($t0)

    lw $ra,0($sp)
    addiu $sp,$sp,4

    jr $ra
    nop
.end cd64_setmode_asm

.global wait_dma
.ent wait_dma
wait_dma:
    li $t0,0xa4600010
wdma_loop:
    lw $t1,0($t0)
    nop
    andi $t1,$t1,3
    bnez $t1,wdma_loop
    nop
    jr $ra
    nop
.end wait_dma
