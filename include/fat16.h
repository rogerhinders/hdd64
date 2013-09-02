#ifndef FAT16_H
#define FAT16_H

#define FAT16_TYPE_4  4
#define FAT16_TYPE_6  6
#define FAT16_TYPE_14 14

#define MBR_OFFSET_PART_TABLE 0x1be

#define FAT16_FILE_ATTRIB_READ_ONLY 0
#define FAT16_FILE_ATTRIB_HIDDEN    1
#define FAT16_FILE_ATTRIB_SYSTEM    2
#define FAT16_FILE_ATTRIB_VOL_LABEL 3
#define FAT16_FILE_ATTRIB_SUB_DIR   4
#define FAT16_FILE_ATTRIB_ARCHIVE   5
#define FAT16_FILE_ATTRIB_UNUSED1   6
#define FAT16_FILE_ATTRIB_UNUSED2   7

//first chars in filename
#define FAT16_FILE_UNUSED  0x0
#define FAT16_FILE_DELETED 0xe5
#define FAT16_FILE_IS_DIR  0x2e

//entry in partition table
typedef struct {
	uint8_t bootable;
	uint8_t start_chs[3];
	uint8_t type;
	uint8_t end_chs[3];
	uint32_t lba_offset;
	uint32_t lba_size;
} __attribute((packed)) partition_table;

typedef struct {
	uint8_t instr[3];
	char manufacturer_descr[8];
	uint16_t sector_size;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t number_of_fats;
    uint16_t root_dir_entries;
    uint16_t total_sectors_short;
    uint8_t media_descriptor;
    uint16_t fat_size_sectors;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_long;
    uint8_t drive_number;
    uint8_t current_head;
    uint8_t boot_signature;
    uint32_t volume_id;
    char volume_label[11];
    char fs_type[8];
    uint8_t boot_code[448];
    uint16_t boot_sector_signature;

} __attribute((packed)) fat_boot_sector;

typedef struct {
	char filename[8];
	char filename_ext[3];
	uint8_t file_attrib;
	uint8_t reserved[10];
	uint16_t time_created_updated;
	uint16_t date_created_updated;
	uint16_t start_cluster;
	uint32_t file_size_bytes;
} __attribute((packed)) fat_entry;

int fat16_init();
uint32_t get_num_root_entries();
void fat16_read_mbr(uint16_t *buf);
int fat16_find_partition(partition_table *pt, int num_table_entries);
void fat16_read_fat_boot(fat_boot_sector *boot);
void fat16_read_fat(uint16_t *fatbuf);
void fat16_read_root_dir(fat_entry *entry);
int is_valid_file(fat_entry *entry);
void generate_file_str(fat_entry *entry, char *str);
uint32_t fat16_cluster_to_lba_offset(uint16_t cluster);
uint32_t fat16_get_file_lba_size(fat_entry *entry);
void fat16_read_file_dram(fat_entry *entry, uint32_t dst_ram, uint32_t byteswap, void (*upd_progress)(uint32_t));
void fat16_read_file_array(fat_entry *entry, uint16_t *buf, uint32_t num_sectors, uint32_t byteswap);
uint32_t fat16_get_file_index(fat_entry *entry, uint32_t filtered_index);
int file_exist(char* name, fat_entry *dirlist);
int str_cmp(char *str1, char *str2, int len);
void get_free_clusters(uint32_t *buf, uint32_t len);
void write_file(char *name, uint8_t *buf, uint32_t len, fat_entry *dirinfo);
uint32_t test1();
uint32_t test2();
#endif // FAT16_H
