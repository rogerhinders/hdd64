#include <libdragon.h>
#include <cd64.h>
#include <ata.h>
#include <macro.h>

//dirty function to check if any drive at all is present
int is_drive_present() {
	ata_setreg(ATA_REG_STATUS);
	uint8_t ret = ata_read16();
	if(ret==0xff)
		return 0;
	else
		return 1;
}

void ata_write(uint32_t start_sector, uint32_t num_sectors, uint16_t* buf, uint32_t byteswap) {
	while(ata_drive_busy());
	while(ata_drive_ready());
	
	//select drive
	ata_setreg(ATA_REG_SDH);
	ata_write16(0xE0|((start_sector>>24)&0x0F));

	//waste some time
	ata_setreg(ATA_REG_ERROR);
	ata_write16(0);
	
	//send number of sectors to read
	ata_setreg(ATA_REG_SECTOR_COUNT);
	ata_write16(num_sectors&0xff); //0==256 sectors
	
	//set LBAlo
	ata_setreg(ATA_REG_SECTOR_NUMBER);
	ata_write16(start_sector&0xff);

	//set LBAmid
	ata_setreg(ATA_REG_CYL_LOW);
	ata_write16((start_sector>>8)&0xff);

	//set LBAhi
	ata_setreg(ATA_REG_CYL_HIGH);
	ata_write16((start_sector>>16)&0xff);

	//send read cmd
	ata_send_cmd(ATA_CMD_WRITE_SECTORS);

	//wait
	while(ata_drive_busy());
	while(ata_drive_data_request());

	//read 256 words(uint16)
	ata_setreg(ATA_REG_DATA);

	int i;
//	uint16_t data;
	if(byteswap==1) {
		for(i=0;i<(256*num_sectors);i++) {
//			data = ata_read16();
//			buf[i] = __SWAP16(data);
			ata_write16(__SWAP16(buf[i]));
		}
	} else {
		for(i=0;i<(256*num_sectors);i++) {
//			data = ata_read16();
//			buf[i] = data;
			ata_write16(buf[i]);
		}
	}
	//flush cache
	ata_send_cmd(ATA_CMD_CACHE_FLUSH);
	while(ata_drive_busy());
}

void ata_read(uint32_t start_sector, uint32_t num_sectors, uint16_t* buf, uint32_t byteswap) {
	while(ata_drive_busy());
	while(ata_drive_ready());
	
	//select drive
	ata_setreg(ATA_REG_SDH);
	ata_write16(0xE0|((start_sector>>24)&0x0F));

	//waste some time
	ata_setreg(ATA_REG_ERROR);
	ata_write16(0);
	
	//send number of sectors to read
	ata_setreg(ATA_REG_SECTOR_COUNT);
	ata_write16(num_sectors&0xff); //0==256 sectors
	
	//set LBAlo
	ata_setreg(ATA_REG_SECTOR_NUMBER);
	ata_write16(start_sector&0xff);

	//set LBAmid
	ata_setreg(ATA_REG_CYL_LOW);
	ata_write16((start_sector>>8)&0xff);

	//set LBAhi
	ata_setreg(ATA_REG_CYL_HIGH);
	ata_write16((start_sector>>16)&0xff);

	//send read cmd
	ata_send_cmd(ATA_CMD_READ_SECTORS);

	//wait
	while(ata_drive_busy());
	while(ata_drive_data_request());

	//read 256 words(uint16)
	ata_setreg(ATA_REG_DATA);

	int i;
	uint32_t *p = (uint32_t*)buf;
/*	uint16_t data;
	if(byteswap==1) {
		for(i=0;i<(256*num_sectors);i++) {
			data = ata_read16();
			buf[i] = __SWAP16(data);
		}
	} else {
		for(i=0;i<(256*num_sectors);i++) {
			data = ata_read16();
			buf[i] = data;
		}
	}*/
	uint32_t data;
	if(byteswap==1) {
		for(i=0;i<(128*num_sectors);i++) {
			data = ata_read32();
			p[i] = __SWAP16(data) | (__SWAP16(data>>16)<<16);
		}
	} else {
		for(i=0;i<(128*num_sectors);i++) {
			data = ata_read32();
			p[i] = data;
		}
	}
}


//ata_read version for storing result directly to DRAM addr
void ata_read_dram(uint32_t start_sector, uint32_t num_sectors, uint32_t ram_addr, uint32_t byteswap) {
	while(ata_drive_busy());
	while(ata_drive_ready());
	
	//select drive
	ata_setreg(ATA_REG_SDH);
	ata_write16(0xE0|((start_sector>>24)&0x0F));

	//waste some time
	ata_setreg(ATA_REG_ERROR);
	ata_write16(0);
	
	//send number of sectors to read
	ata_setreg(ATA_REG_SECTOR_COUNT);
	ata_write16(num_sectors&0xff); //0==256 sectors
	
	//set LBAlo
	ata_setreg(ATA_REG_SECTOR_NUMBER);
	ata_write16(start_sector&0xff);

	//set LBAmid
	ata_setreg(ATA_REG_CYL_LOW);
	ata_write16((start_sector>>8)&0xff);

	//set LBAhi
	ata_setreg(ATA_REG_CYL_HIGH);
	ata_write16((start_sector>>16)&0xff);

	//send read cmd
	ata_send_cmd(ATA_CMD_READ_SECTORS);

	//wait
	while(ata_drive_busy());
	while(ata_drive_data_request());

	//read 256 words(uint16)
	ata_setreg(ATA_REG_DATA);

	int i;
	/*uint16_t data,data2;
	if(byteswap==1) {
		for(i=0;i<((256*num_sectors)/2);i++) {
			data = ata_read16() & 0xffff;
			data2 = ata_read16() & 0xffff;
			io_write(ram_addr, (__SWAP16(data)<<16)|(__SWAP16(data2) & 0xffff));
			ram_addr+=4;
		}
	} else {
		for(i=0;i<((256*num_sectors)/2);i++) {
			data = ata_read16() & 0xffff;
			data2 = ata_read16() & 0xffff;
			io_write(ram_addr, (data<<16)|data2);
			ram_addr+=4;
		}
	}*/

	uint32_t data;
	if(byteswap==1) {
		for(i=0;i<(128*num_sectors);i++) {
			data = ata_read32();
			io_write(ram_addr, __SWAP16(data) | (__SWAP16(data>>16)<<16));
			ram_addr+=4;
		}
	} else {
		for(i=0;i<(128*num_sectors);i++) {
			data = ata_read32();
			io_write(ram_addr, data);
			ram_addr+=4;
		}
	}

}

void ata_identify_drive(identify_info *ids) {
	while(ata_drive_busy());
	while(ata_drive_ready());
	
	//select drive
	ata_setreg(ATA_REG_SDH);
	ata_write16(0xA0);

	//send command identify
	ata_send_cmd(ATA_CMD_IDENTIFY);

	//wait for data to be requested
	while(ata_drive_data_request());

	//read 256 words(uint16)
	ata_setreg(ATA_REG_DATA);

	int i;
	for(i=0;i<256;i++) {
		((uint16_t*)ids)[i] = ata_read16();
	}
}

void ata_setreg(uint8_t reg) {
	io_write(CD64_REG_CD_COMM_IO,reg&0xff);
}

/*
	returns nonzero if drive is busy
*/
int ata_drive_busy() {
	ata_setreg(ATA_REG_STATUS);
	uint16_t stat = ata_read16();
	return stat&(1<<ATA_STATUS_BSY);
	
}

/*
	returns nonzero if drive is not ready
*/
int ata_drive_ready() {
	ata_setreg(ATA_REG_STATUS);
	uint16_t stat = ata_read16();

	if( (stat&(1<<ATA_STATUS_DRDY)) ==0 )
		return 1;
	else
		return 0;
}

/*
	returns nonzero if drive is not ready for data req
*/
int ata_drive_data_request() {
	ata_setreg(ATA_REG_STATUS);
	uint16_t stat = ata_read16();

	if( (stat&(1<<ATA_STATUS_DRQ)) ==0 )
		return 1;
	else
		return 0;
}
uint16_t ata_read16() {
	return (uint16_t) io_read(CD64_REG_CD_DATA_16);
}

uint32_t ata_read32() {
    return (uint32_t) io_read(CD64_REG_CD_DATA_32);
}

void ata_write16(uint16_t val) {
	io_write(CD64_REG_CD_DATA_16, val);
}

void ata_send_cmd(uint8_t cmd) {
	ata_setreg(ATA_REG_COMMAND);
	ata_write16(cmd);
}
