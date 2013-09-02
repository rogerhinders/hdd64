#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

#include <gfx.h>
#include <cd64.h>
#include <ata.h>
#include <fat16.h>
#include <loader.h>
#include <macro.h>
#include <hdd64_system.h>
#include <header.h>
#include <global.h>
#include <savedata.h>

#define ROM_TYPE_V64  0
#define ROM_TYPE_Z64  1
#define ROM_TYPE_ERR -1

typedef struct {
	char name[21];
	uint32_t size_bytes;
} game_info;

typedef enum {
	DIR_LIST, BACKUP_SAVE, ALERT, CONFIRM_SAVE_WRITE
} active_wnd;

typedef struct {
	char name[9];
	uint8_t position;
} text_input;

void error_handler(char *msg);
void update_input();
int game_swapped();
void update_progress(uint32_t percent);
int read_game_info();
void show_backup_save(display_context_t disp);
void dump_save_ram();
void write_save_ram();
void show_confirm_save_write(display_context_t disp);

static resolution_t res = RESOLUTION_320x240;
static bitdepth_t bit = DEPTH_32_BPP;

uint8_t hdd_temp[4096];
fat_entry *dirlist;

//menu related stuff
int marker_pos = 0;
int marker_offs = 48;
int list_offs = 0;

//size of installed ram
uint32_t cd64_ram_size;

//temp stuff
char gname[21];
uint32_t lbasz = 0;
unsigned int tmparr[20];

//game info
game_info ginfo;
header game_header;
char save_type_str[20];

//UI state
active_wnd ui_active = DIR_LIST;

//text input for savename
text_input input_save;

//alert text
char alert_txt[30];

int main(void) {
    /* enable interrupts (on the CPU) */
    init_interrupts();
	controller_init();

    /* Initialize peripherals */
    display_init( res, bit, 2, GAMMA_NONE, ANTIALIAS_OFF );
	
	cd64_init();

	if(!is_drive_present()) {
		error_handler("ERR:NO DRIVE CONNECTED!");
	}

    if(!fat16_init()) {
		error_handler("ERR:NO PART. OR PART. TABLE?");
	}

	cd64_ram_size = quick_ram_test();

    char banner[] = "HDD64 v0.1 (c)";
	
    //identify drive
	identify_info device_id;
	ata_identify_drive(&device_id);
	device_id.model_num[19] = 0; //temp terminate str

	//read root dirlist
	dirlist = (fat_entry*)malloc(get_num_root_entries()*0x20);
	fat16_read_root_dir(dirlist);

	gname[0] = 0;
	gname[20] = 0;
	
	char ram_str[11];
	ram_str[10] = '\0';
	sprintf(ram_str,"Mem: %dM", (int)(cd64_ram_size>>17));
	
	read_game_info();
    get_save_type_str(save_type_str);

	//init savename input
	memset(input_save.name,0x20,8);
	input_save.name[8] = '\0';
	input_save.position = 0;

    //main loop
    while(1) 
    {
        
		static display_context_t disp = 0;
		update_input();
        
		/* Grab a render buffer */
        while( !(disp = display_lock()) );
       
        /*Fill the screen */
        //graphics_fill_screen( disp, 0x0 );
		draw_background(disp);


		//print game info
		print_text(disp,144,48,ginfo.name,0xff0000ff);
		char game_size[11];
		sprintf(game_size,"Size: %dM",(int)(ginfo.size_bytes>>17));
		print_text(disp,144,64,game_size,0xff0000ff);

		//misc

		print_text(disp,16,0,banner,0x00ff00ff);
		print_dirlist(disp,24,48,dirlist,list_offs);
		draw_marker(disp, marker_offs+marker_pos*16, 0x0000FFFF);
		

		//hardware info:
		((uint8_t*)device_id.model_num)[20] = 0; //temp fix size
		print_text(disp,144,160,(char*)device_id.model_num,0xff0000ff);
		print_text(disp,144,176,save_type_str,0xff0000ff);
		print_text(disp,144,192,ram_str,0xff0000ff);
        display_show(disp);

		//show save backup?
		if(ui_active==BACKUP_SAVE)
			show_backup_save(disp);

		//show alert?
		if(ui_active==ALERT)
			draw_alert(disp,alert_txt);

		//show save confirm?
		if(ui_active==CONFIRM_SAVE_WRITE)
			show_confirm_save_write(disp);

    }
}
void update_input_dirlist() {
        controller_scan();
        struct controller_data keys = get_keys_down();
		int i,old_marker,old_list;
        if( keys.c[0].up ) {
			if(marker_pos>0) {
				marker_pos--;
			} else {
				if(list_offs>0)
					list_offs--;
			}
			read_game_info();
        }

        if( keys.c[0].down ) {
			old_marker = marker_pos;
			old_list = list_offs;
			if(marker_pos<(NUM_LIST_ITEMS-1)) {
				marker_pos++;
			} else {
				list_offs++;
			}

			// prevent breaching the list bounds
			if(!read_game_info()) {
				marker_pos = old_marker;
				list_offs = old_list;				
			}
        }

		if(keys.c[0].A) {
			if(game_swapped()!= ROM_TYPE_ERR) {
				i = fat16_get_file_index(dirlist, marker_pos+list_offs);
				lbasz = fat16_get_file_lba_size(&dirlist[i]);
				fat16_read_file_dram(&dirlist[i], 0xb4000000, game_swapped(), update_progress);

				dma_read((void *)tmparr,0xb4700000,20);
				disable_interrupts();
				boot_dram();
			} else { //for now, -1 automagically means save game.
				ui_active = CONFIRM_SAVE_WRITE;
			}
		}

		if(keys.c[0].start) {
			ui_active = BACKUP_SAVE;
		}
}

void update_input_backup_save() {
    controller_scan();
    struct controller_data keys = get_keys_down();

	if( keys.c[0].left ) {
		input_save.position = (input_save.position-1)&7;
	}
	if( keys.c[0].right ) {
		input_save.position = (input_save.position+1)&7;
	}
	if( keys.c[0].up ) {
		char c = input_save.name[input_save.position];
		switch(c) {
			case 'Z':
				c = ' ';
				break;
			case ' ':
				c = '0';
				break;
			case '9':
				c = 'A';
				break;
			default:
				c++;
		}
		input_save.name[input_save.position] = c;
		
	}
	if( keys.c[0].down ) {
		char c = input_save.name[input_save.position];
		switch(c) {
			case ' ':
				c = 'Z';
				break;
			case '0':
				c = ' ';
				break;
			case 'A':
				c = '9';
				break;
			default:
				c--;
		}
		input_save.name[input_save.position] = c;
	}

	if(keys.c[0].A) {
		ui_active = ALERT;

		if(file_exist(input_save.name,dirlist)) {
			sprintf(alert_txt,"File exists!");
		} else {
			dump_save_ram();
			//test1();
			sprintf(alert_txt,"Save backed up!");
		}
	}

	if(keys.c[0].B) {
		ui_active = DIR_LIST;
	}
}

void update_input_alert() {
    controller_scan();
    struct controller_data keys = get_keys_down();
	if(keys.c[0].A || keys.c[0].B)
		ui_active = DIR_LIST;
}
void update_input_confirm_save_write() {
    controller_scan();
    struct controller_data keys = get_keys_down();
	if(keys.c[0].B)
		ui_active = DIR_LIST;

	if(keys.c[0].A) { 
		ui_active = ALERT;
		write_save_ram();
		sprintf(alert_txt,"Save data flashed to chip!");
	}
}
void update_input() {
	switch(ui_active) {
		case DIR_LIST:
			update_input_dirlist();
			break;
		case BACKUP_SAVE:
			update_input_backup_save();
			break;
		case ALERT:
			update_input_alert();
			break;
		case CONFIRM_SAVE_WRITE:
			update_input_confirm_save_write();
			break;
	}
}

//0 = V64, 1 = Z64, -1 = err
int game_swapped() {
	int i = fat16_get_file_index(dirlist, marker_pos+list_offs);
	fat16_read_file_array(&dirlist[i], (uint16_t *)hdd_temp,1,1);

	//is v64?
	//if(((uint32_t*)hdd_temp)[0]==0x37804012)
	if((hdd_temp[0]==0x37)&&(hdd_temp[1]==0x80)&&(hdd_temp[2]==0x40)&&(hdd_temp[3]==0x12))
		return 0;

	//is z64?
	//if(((uint32_t*)hdd_temp)[0]==0x80371240)
	if((hdd_temp[0]==0x80)&&(hdd_temp[1]==0x37)&&(hdd_temp[2]==0x12)&&(hdd_temp[3]==0x40))
		return 1;

	//else err
	return -1;

}

void update_progress(uint32_t percent) {
        static display_context_t disp = 0;
        while( !(disp = display_lock()) );
		draw_progress_bar(disp, 224, percent);
        display_show(disp);
}
/* 0 = invalid file, 1 = valid file*/
int read_game_info() {
	int game_type = game_swapped();
	int i = fat16_get_file_index(dirlist, marker_pos+list_offs);

	if(is_valid_file(&dirlist[i])) {
		return 0;
	}

	fat16_read_file_array(&dirlist[i], (uint16_t *)hdd_temp,8,game_type);			
	if(game_type!=ROM_TYPE_ERR) {
		read_header(hdd_temp, &game_header);
		memcpy(ginfo.name,game_header.rom_name,20);
		ginfo.name[20] = '\0';
	} else {
		ginfo.name[0]='\0';
	}

	ginfo.size_bytes = __SWAP32(dirlist[i].file_size_bytes);

	return 1;
}


void show_backup_save(display_context_t disp) {
	int wnd_x,wnd_y;
	draw_window(disp,100,80,"Backup Save",&wnd_x,&wnd_y,0x00ff00ff);
	print_text(disp,wnd_x+8,wnd_y+16*2,input_save.name,0xff0000ff);
	print_text(disp,wnd_x+8+input_save.position*8,wnd_y+16*3,"^",0xff0000ff);
}

void show_confirm_save_write(display_context_t disp) {
	draw_alert(disp,"WARNING: Overwrite save chip?");
}
void dump_save_ram() {
	uint8_t *buf;
	switch(get_save_type()) {
		case SAVE_TYPE_EEPROM_4:
//        io_write(CD64_REG_MODE,CD64_MODE_7);
//cd64_disable();
			buf = (uint8_t*)malloc(SIZE_EEPROM_4);
			read_eeprom4(buf);
//            cd64_enable();
//io_write(CD64_REG_MODE,CD64_MODE_0);

			write_file(input_save.name, buf, SIZE_EEPROM_4, dirlist);
			break;
		case SAVE_TYPE_EEPROM_16:
			buf = (uint8_t*)malloc(SIZE_EEPROM_16);
			read_eeprom16(buf);
			write_file(input_save.name, buf, SIZE_EEPROM_16, dirlist);
			break;
	}
	
}

void write_save_ram() {

	uint8_t *buf;
	int i = fat16_get_file_index(dirlist, marker_pos+list_offs);
	switch(get_save_type()) {
		case SAVE_TYPE_EEPROM_4:
			buf = (uint8_t*)malloc(SIZE_EEPROM_4);
			fat16_read_file_array(&dirlist[i], (uint16_t *)buf,SIZE_EEPROM_4/512,1);
//io_write(CD64_REG_MODE,CD64_MODE_7);
//cd64_disable();
			write_eeprom4(buf);
//cd64_enable();
//io_write(CD64_REG_MODE,CD64_MODE_0);

			break;
		case SAVE_TYPE_EEPROM_16:
			buf = (uint8_t*)malloc(SIZE_EEPROM_16);
			fat16_read_file_array(&dirlist[i], (uint16_t *)buf,SIZE_EEPROM_16/512,1);
			write_eeprom16(buf);

			break;
	}

}

void error_handler(char *msg) {
        static display_context_t disp = 0;
        while( !(disp = display_lock()) );
		print_text(disp,16,32,msg,0xff0000ff);
        display_show(disp);
}
