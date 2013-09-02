#ifndef HEADER_H
#define HEADER_H

typedef struct {
    uint8_t init_PI_BSB_DOM1_LAT_REG;
    uint8_t init_PI_BSB_DOM1_PGS_REG;
    uint8_t init_PI_BSB_DOM1_PWD_REG;
    uint8_t init_PI_BSB_DOM1_PGS_REG2;
    uint32_t clock_rate;
    uint32_t program_count;
    uint32_t release;
    uint32_t CRC1;
    uint32_t CRC2;
    uint32_t unknown[2];
    uint8_t rom_name[0x20];
    uint32_t unknown2;
    uint32_t manufacturer_id;
    uint16_t cartridge_id;
    uint16_t country_code;
    uint8_t boot_code[0xfc0];
} __attribute((packed)) header;

void read_header(uint8_t *buf, header *hdr);
#endif // HEADER_H
