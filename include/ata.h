#ifndef ATA_H
#define ATA_H

//ata regs
#define ATA_REG_DATA          0x8
#define ATA_REG_ERROR         0x9
#define ATA_REG_WRITE_PRECOMP 0x9
#define ATA_REG_SECTOR_COUNT  0xa
#define ATA_REG_SECTOR_NUMBER 0xb
#define ATA_REG_CYL_LOW       0xc
#define ATA_REG_CYL_HIGH      0xd
#define ATA_REG_SDH           0xe
#define ATA_REG_STATUS        0xf
#define ATA_REG_COMMAND       0xf
#define ATA_REG_ALT_STATUS    0x16
#define ATA_REG_DIGITAL_OUT   0x16
#define ATA_REG_DRIVE_ADDR    0x17

//status bits
#define ATA_STATUS_BSY  7
#define ATA_STATUS_DRDY 6
#define ATA_STATUS_DF   5
#define ATA_STATUS_DSC  4
#define ATA_STATUS_DRQ  3
#define ATA_STATUS_CORR 2
#define ATA_STATUS_IDX  1
#define ATA_STATUS_ERR  0

//ata commands
#define ATA_CMD_IDENTIFY      0xEC
#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_CACHE_FLUSH   0xe7

//ata identify cmd return struct
typedef struct {
	uint16_t general_info;
	uint16_t num_cylinders;
	uint16_t reserved;
	uint16_t num_heads;
	uint16_t num_unform_b_per_track;
	uint16_t num_unform_b_per_sec;
	uint16_t num_sec_per_track;
	uint16_t vendor_unique[3];
	uint16_t serial_num[10];
	uint16_t buffer_type;
	uint16_t buffer_size_in_512b_inc;
	uint16_t num_ecc_avail;
	uint16_t firmware_rev[4];
	uint16_t model_num[20];
	uint16_t vendor_unique2;
	uint16_t can_perform_dw_io;
	uint16_t capabilities;
	uint16_t reserved2;
	uint16_t pio_data_transf_cyc_tim_mode;
	uint16_t dma_transf_cyc_tim_mode;
	uint16_t stuff;
	uint16_t num_current_cyl;
	uint16_t num_current_heads;
	uint16_t num_current_sec_per_track;
	uint16_t current_capacity_in_sec[2];
	uint16_t sector_settings;
	uint16_t num_user_addr_sec[2];
	uint16_t single_w_dma_transf_mode_active;
	uint16_t multi_w_dma_transf_mode_active;
	uint16_t reserved3[64];
	uint16_t vendor_unique3[32];
	uint16_t reserved4[96];
} __attribute((packed)) identify_info;

int is_drive_present();
void ata_write(uint32_t start_sector, uint32_t num_sectors, uint16_t* buf, uint32_t byteswap);
void ata_read(uint32_t start_sector, uint32_t num_sectors, uint16_t* buf, uint32_t byteswap);
void ata_read_dram(uint32_t start_sector, uint32_t num_sectors, uint32_t ram_addr, uint32_t byteswap);
void ata_identify_drive(identify_info *buf);
void ata_setreg(uint8_t reg);

int ata_drive_busy();
int ata_drive_ready();
int ata_drive_data_request();

uint16_t ata_read16();
uint32_t ata_read32();
void ata_write16(uint16_t val);

void ata_send_cmd(uint8_t cmd);

#endif // ATA_H
