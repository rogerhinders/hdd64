#include <libdragon.h>
#include <fat16.h>
#include <ata.h>
#include <string.h>
#include <macro.h>
#include <stdlib.h>
#include <gfx.h>
#include <stdio.h>

#define TABLE_ENTRIES 4
partition_table part_table[TABLE_ENTRIES];
int selected_part;
uint8_t mbr[512];
fat_boot_sector fat_boot;
uint16_t tmp[256];
uint16_t tmp_dir[256*32];
int last_percent = 0;

//start of boot sector
uint32_t boot_start_lba = 0;

//start of cluster data
uint32_t data_start_lba = 0;

//start if FAT table
uint32_t fat_start_lba = 0;

//start of root dir
uint32_t root_dir_start_lba = 0;
//pointer to FAT table buffer
uint16_t *fat;

//size of cluster in bytes and lba
uint32_t cluster_size_bytes = 0;
uint32_t cluster_size_lba = 0;

//returns zero on failure
int fat16_init() {
	//read mbr sector
	fat16_read_mbr((uint16_t*)mbr);
	
	//read partition table
	memcpy(part_table, mbr+MBR_OFFSET_PART_TABLE,sizeof(partition_table)*4);
	selected_part = fat16_find_partition(part_table, TABLE_ENTRIES);

	if(selected_part==-1)
		return 0;
	
	//start of boot sector
	boot_start_lba = __SWAP32(part_table[selected_part].lba_offset);
	//read fat boot sector
	fat16_read_fat_boot(&fat_boot);
	
	//start of root dir
	root_dir_start_lba = (__SWAP16(fat_boot.reserved_sectors) + __SWAP16(fat_boot.fat_size_sectors) * fat_boot.number_of_fats) * __SWAP16(fat_boot.sector_size);
	root_dir_start_lba /= 512;
	root_dir_start_lba += boot_start_lba;

	//start of cluster data:
	data_start_lba = ((__SWAP16(fat_boot.reserved_sectors) 
							+ __SWAP16(fat_boot.fat_size_sectors) 
							* fat_boot.number_of_fats)
							* __SWAP16(fat_boot.sector_size));
	data_start_lba += __SWAP16(fat_boot.root_dir_entries)*0x20;
	data_start_lba /= 512;
	data_start_lba += boot_start_lba;
	
	//start of FAT data
	fat_start_lba = boot_start_lba+((__SWAP16(fat_boot.sector_size)*__SWAP16(fat_boot.reserved_sectors))/512);
	//read FAT
	fat = (uint16_t*) malloc(__SWAP16(fat_boot.fat_size_sectors)* __SWAP16(fat_boot.sector_size));
	fat16_read_fat(fat);

	cluster_size_bytes = __SWAP16(fat_boot.sector_size) * fat_boot.sectors_per_cluster;
	cluster_size_lba = cluster_size_bytes/512;
	return 1;
}

uint32_t get_num_root_entries() {
	return __SWAP16(fat_boot.root_dir_entries);
}

void fat16_read_mbr(uint16_t *buf) {
	ata_read(0,1,buf,1);
}

//returns selected partition, if -1, did not find suitable table
int fat16_find_partition(partition_table *pt, int num_table_entries) {
	int i;
	for(i=0;i<num_table_entries;i++) {
		switch(pt[i].type) {
			case FAT16_TYPE_4:
			case FAT16_TYPE_6:
			case FAT16_TYPE_14:
				return i;
		}
	}
	return -1;
}

void fat16_read_fat_boot(fat_boot_sector *boot) {
	ata_read(boot_start_lba,1,tmp,1);
	memcpy(boot, tmp, sizeof(tmp));
}

void fat16_read_fat(uint16_t *fatbuf) {
	ata_read(fat_start_lba, (__SWAP16(fat_boot.fat_size_sectors)* __SWAP16(fat_boot.sector_size))/512, fatbuf,1);
}
void fat16_read_root_dir(fat_entry *entry) {
	//uint32_t lba_offs = (__SWAP16(fat_boot.reserved_sectors) + __SWAP16(fat_boot.fat_size_sectors) * fat_boot.number_of_fats) * __SWAP16(fat_boot.sector_size);
	//ata_read((lba_offs/512)+__SWAP32(part_table[selected_part].lba_offset),32,tmp_dir,1);
	//memcpy(entry,tmp_dir,sizeof(fat_entry)*num_entries);
    ata_read(root_dir_start_lba,((__SWAP16(fat_boot.root_dir_entries))*0x20)/512,(uint16_t *)entry,1);
}

//returns nonzero if not valid
int is_valid_file(fat_entry *entry) {
	if(entry->file_attrib&((1<<FAT16_FILE_ATTRIB_HIDDEN)|
                           (1<<FAT16_FILE_ATTRIB_SYSTEM)|
                           (1<<FAT16_FILE_ATTRIB_VOL_LABEL)|
                           (1<<FAT16_FILE_ATTRIB_SUB_DIR))) {
		return 1;
	} 

	switch((uint8_t)entry->filename[0]) {
		case FAT16_FILE_UNUSED:
		case FAT16_FILE_DELETED:
		case FAT16_FILE_IS_DIR:
			return 1;
	}
	return 0;
}

void generate_file_str(fat_entry *entry, char *str) {
	int len = 8;
    int i;
    for(i=len-1;i>0;i--) {
        if((entry->filename[i])==0x20)
            len=i;
        else
            break;
    }

	for(i=0;i<len;i++)
		str[i] = entry->filename[i];

	str[len] = '.';

	for(i=len+1;i<(len+4);i++)
		str[i] = entry->filename_ext[i-len-1];

	str[len+4] = '\0';
}

uint32_t fat16_cluster_to_lba_offset(uint16_t cluster) {
//start byte: (start_cluster-2)*sector_size*sectors_per_cluster+(rootdir_offs+rootdir_entries*0x20)


	uint32_t file_start_lba = (
								(__SWAP16(cluster)-2) 
								*__SWAP16(fat_boot.sector_size)
								*fat_boot.sectors_per_cluster
								)/512;
	return file_start_lba+data_start_lba;
}

uint32_t fat16_get_file_lba_size(fat_entry *entry) {
	uint32_t size_b = __SWAP32(entry->file_size_bytes);
	uint32_t mod = size_b%512;
	if(mod==0) {
		return size_b/512;
	}

	return (size_b + (512-mod))/512;
}

uint32_t fat16_bytes_to_cluster_size(uint32_t num_bytes) {
	//calculate number of fat sectors needed
	uint32_t sec_size = __SWAP16(fat_boot.sector_size);
	uint32_t mod = num_bytes % sec_size;
	uint32_t num_sec;
	if(mod==0)
		num_sec = num_bytes/sec_size;
	else
		num_sec = (num_bytes+(sec_size-mod))/sec_size;

	//get number of clusters needed
	mod = num_sec % fat_boot.sectors_per_cluster;
	if(mod==0)
		return num_sec/fat_boot.sectors_per_cluster;

	return (num_sec+(fat_boot.sectors_per_cluster-mod))/fat_boot.sectors_per_cluster;
}

void fat16_read_file_dram(fat_entry *entry, uint32_t dst_ram, uint32_t byteswap, void (*upd_progress)(uint32_t)) {


	int percent = 0;
	uint16_t num_cluster = entry->start_cluster;
	int num_sec_read = 0;

	//display initial progress
	upd_progress(0);

	uint32_t lba_offset = fat16_cluster_to_lba_offset(num_cluster);
	uint32_t file_size_lba = fat16_get_file_lba_size(entry);

	while(1) {
		ata_read_dram(lba_offset,cluster_size_lba,dst_ram,byteswap);
		dst_ram += cluster_size_bytes;
		num_sec_read += cluster_size_lba;

		num_cluster = fat[__SWAP16(num_cluster)];
		//anymore clusters?
		if(num_cluster == 0xFFFF)
			break;

		
		lba_offset = fat16_cluster_to_lba_offset(num_cluster);

		//update progress bar
		percent = ((float)num_sec_read/file_size_lba)*100;
		if(percent>=(last_percent)) {
			last_percent = percent;
			upd_progress(percent);
		}
	}
	upd_progress(100); //make sure bar is filled

}
uint32_t test1() {

//ata_read(boot_start_lba,1,tmp,1);
	ata_write(boot_start_lba,1,(uint16_t *)&fat_boot,1);
	//ata_write(uint32_t start_sector, uint32_t num_sectors, uint16_t* buf, uint32_t byteswap)
	return 0;//fat[0x10];//__SWAP16(fat_boot.fat_size_sectors)* __SWAP16(fat_boot.sector_size);
}
uint32_t test2() {
	return fat[0x82];
}
//crappy function that blindly reads n sectors to array..
void fat16_read_file_array(fat_entry *entry, uint16_t *buf, uint32_t num_sectors, uint32_t byteswap) {
	ata_read(fat16_cluster_to_lba_offset(entry->start_cluster),num_sectors,buf,byteswap);
}

//returns real file index from filtered index 
uint32_t fat16_get_file_index(fat_entry *entry, uint32_t filtered_index) {
	int i;
	int j=0;
	for(i=0;i<__SWAP16(fat_boot.root_dir_entries);i++) {
		if(is_valid_file(&entry[i])==0){
			if(j==filtered_index)
				return i;
			j++;
		}				
	}
	return 0;
}

//0 = doesnt exist, nonzero = exist
int file_exist(char* name, fat_entry *dirlist) {
	int i;
	for(i=0;i<__SWAP16(fat_boot.root_dir_entries);i++) {
		if(str_cmp(name,dirlist[i].filename,8))
			return 1;
	}
	return 0;
}

//0 = not equal, nonzero=equal
int str_cmp(char *str1, char *str2, int len) {
	int i;
	for(i=0;i<len;i++) {
		if(str1[i]!=str2[i])
			return 0;
	}
	return 1;
}

void get_free_clusters(uint32_t *buf, uint32_t len) {
	uint32_t fat_len = __SWAP16(fat_boot.fat_size_sectors)* __SWAP16(fat_boot.sector_size);
	fat_len /= 2; //16bit entries
	int i,j;
	int start_search = 3; //first 2 clusters are reserved
	//search FAT for unused clusters
	for(i=0;i<len;i++) { //clusters needed
		for(j=start_search;j<fat_len;j++) { //number of fat entries
			if(fat[j]==0x0000) {
				buf[i] = j;
				start_search = j+1; //prevent same cluster being marked more than once
				break;
			}
		}
	}
}

// returns -1 if failed to find an entry
int get_free_root_dir_space(fat_entry *dirinfo) {
	int i;
	for(i=0;i<__SWAP16(fat_boot.root_dir_entries);i++) {
		if((dirinfo[i].filename[0] == FAT16_FILE_UNUSED)||(dirinfo[i].filename[0] == FAT16_FILE_DELETED))
			return i;
	}
	return -1;
}
void write_file(char *name, uint8_t *buf, uint32_t len, fat_entry *dirinfo) {
	uint32_t num_clusters = fat16_bytes_to_cluster_size(len);
	uint32_t *clusters = (uint32_t*)malloc(num_clusters*sizeof(uint32_t));
	get_free_clusters(clusters, num_clusters);

	//create an entry in the root dir
	int root_idx = get_free_root_dir_space(dirinfo);
	if(root_idx==-1)
		return; //no free space in rootdir..
	
	fat_entry *entry = &dirinfo[root_idx];
	memcpy(entry->filename,name,8);
	entry->filename_ext[0]= 'S';
	entry->filename_ext[1]= 'A';
	entry->filename_ext[2]= 'V';
	entry->file_attrib = 0;
	entry->time_created_updated = 0;
	entry->date_created_updated = 0;
	entry->start_cluster = __SWAP16(clusters[0]);
	entry->file_size_bytes = __SWAP32(len);

	//update dirlist on disk
	int k;
	uint16_t *p = (uint16_t *)dirinfo;
	for(k=0;k<(((__SWAP16(fat_boot.root_dir_entries))*0x20)/512);k++) {
		ata_write(root_dir_start_lba+k,1,p,1);
		p+=256;
	}
	

	//ONLY WRITES ONE CLUSTER FOR NOW!!
	uint32_t cluster_offs = fat16_cluster_to_lba_offset(entry->start_cluster);
	for(k=0;k<fat16_get_file_lba_size(entry);k++) {
		ata_write(cluster_offs+k,1,(uint16_t *)buf,1);
		buf += 512;
	}

	//write FAT to disk
	p = &fat[0];
	fat[clusters[0]] = 0xFFFF; //end of file
	for(k=0;k<((__SWAP16(fat_boot.fat_size_sectors)* __SWAP16(fat_boot.sector_size))/512);k++) {
		ata_write(fat_start_lba+k,1,p,1);
		p +=256;
    }

}
