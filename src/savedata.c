#include <libdragon.h>
#include <savedata.h>
#include <string.h>
#include <stdio.h>
#include <hdd64_system.h>

int get_save_type() {
	//check for presence of EEPROM
	unsigned char in[3];
	execute_raw_command(4,0,0,3,NULL,in);
	if(in[2]==0) { //if not 0, no eeprom present
		if((in[1]&0xc0)==0xc0)
			return SAVE_TYPE_EEPROM_16;
		if((in[1]&0xc0)==0x80)
			return SAVE_TYPE_EEPROM_4;
	}
	return 0;
}

void get_save_type_str(char *str) {
	switch(get_save_type()) {
		case SAVE_TYPE_EEPROM_4:
			sprintf(str,"Save: EEPROM 4k");
			break;
		case SAVE_TYPE_EEPROM_16:
			sprintf(str,"Save: EEPROM 16k");
			break;
		case SAVE_TYPE_NONE:
		case SAVE_TYPE_SRAM:
		case SAVE_TYPE_FLASHRAM:
		case SAVE_TYPE_UNKNOWN:
			sprintf(str,"Save: none");
	}
}

void read_eeprom4(uint8_t *buf) {
	int i;
	for(i=0;i<(SIZE_EEPROM_4/EEPROM_BLOCK_SIZE);i++) {
		eeprom_read(i,buf+(i*EEPROM_BLOCK_SIZE));
	}
}

void read_eeprom16(uint8_t *buf) {
	int i;
	for(i=0;i<(SIZE_EEPROM_16/EEPROM_BLOCK_SIZE);i++) {
		eeprom_read(i,buf+(i*EEPROM_BLOCK_SIZE));
    }
}

void write_eeprom4(uint8_t *buf) {
	int i;
	for(i=0;i<(SIZE_EEPROM_4/EEPROM_BLOCK_SIZE);i++) {
		eeprom_write(i,buf+(i*EEPROM_BLOCK_SIZE));
        __wait_ms(15);

	}
}

void write_eeprom16(uint8_t *buf) {
	int i;
	for(i=0;i<(SIZE_EEPROM_16/EEPROM_BLOCK_SIZE);i++) {
		eeprom_write(i,buf+(i*EEPROM_BLOCK_SIZE));
        __wait_ms(15);
	}
}
