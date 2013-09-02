#ifndef SAVEDATA_H
#define SAVEDATA_H

#define SAVE_TYPE_NONE      0
#define SAVE_TYPE_SRAM      1
#define SAVE_TYPE_EEPROM_4  2
#define SAVE_TYPE_EEPROM_16 3
#define SAVE_TYPE_FLASHRAM  4
#define SAVE_TYPE_UNKNOWN   5

#define SIZE_EEPROM_4  512
#define SIZE_EEPROM_16 2048

#define EEPROM_BLOCK_SIZE 8

int get_save_type();
void get_save_type_str(char *str);

void read_eeprom4(uint8_t *buf);
void read_eeprom16(uint8_t *buf);
void write_eeprom4(uint8_t *buf);
void write_eeprom16(uint8_t *buf);
#endif // SAVEDATA_H
