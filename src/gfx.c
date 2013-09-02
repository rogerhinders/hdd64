#include <string.h>
#include <font1.h>
#include <gfx.h>




char dummy_list[15][20] = {
	"item 1",
	"item 2",
	"item 3",
	"item 4",
	"item 5",
	"item 6",
	"item 7",
	"item 8",
	"item 9",
	"item 10",
	"item 11",
	"item 12",
	"item 13",
	"item 14",
	"item 15"
};

char bg_text[] =
	"                                        \n"
	" +--------------+---------------------+ \n"
	" |              |                     | \n"
	" |              |                     | \n"
	" |              |                     | \n"
	" |              |                     | \n"
	" |              |                     | \n"
	" |              |                     | \n"
	" |              |                     | \n"
	" |              |                     | \n"
	" |              |                     | \n"
	" |              |                     | \n"
	" |              |                     | \n"
	" +--------------+---------------------+ \n"
	"   (S):dump save                        ";

void print_text(display_context_t disp, int x, int y, char *str, uint32_t color) {
	int i,_y,_x,font_offs;
	int org_x = x;
	char c;
	uint32_t *buf = (uint32_t*)__safe_buffer[disp-1];
	for(i=0;i<strlen(str);i++) {
		if(str[i]=='\n') {
			x = org_x;
			y+=FONT_H;
			continue;
		}

		c = str[i]-0x20;
		font_offs = c*(FONT_W*FONT_H);
		for(_y=0;_y<FONT_H;_y++) {
			for(_x=0;_x<FONT_W;_x++) {
				if(ami_font[font_offs+_y*FONT_W+_x]==1)
					buf[((_y+y)*SCREEN_W+(_x+x))] = color;
				else
					buf[((_y+y)*SCREEN_W+(_x+x))] = 0;
			}
		}
		x += FONT_W;
	}
}

void print_dirlist(display_context_t disp, int x, int y, fat_entry *dirlist, uint32_t list_offset) {
	int i;
	char dir_name[13];
	//for(i=list_offset;i<(NUM_LIST_ITEMS+list_offset);i++) {
	uint32_t num_entries = get_num_root_entries();
	uint32_t start = fat16_get_file_index(dirlist, list_offset);
	uint32_t num_printed = 0;
	for(i=start;i<num_entries;i++) {
		if(is_valid_file(&dirlist[i])==0){
			generate_file_str(&dirlist[i],dir_name);
			print_text(disp,x,y,dir_name,0x00ff00ff);
			y+=FONT_H;
			if(++num_printed==NUM_LIST_ITEMS)
				break;
		}				
	}
}

void print_dirlist_dummy(display_context_t disp) {
	int i;
	int y = DIRLIST_Y;
	for(i=0;i<NUM_LIST_ITEMS;i++) {
		print_text(disp,DIRLIST_X,y,dummy_list[i],0x00ff00ff);
		y+=FONT_H;	
	}
}

void draw_marker(display_context_t disp, int y, uint32_t color) {
	int _y,_x,buf_offs;
	if(y>(MARKER_MAX))
		y=MARKER_MAX; 
	uint32_t *buf = (uint32_t*)__safe_buffer[disp-1];
//	int buf_offs = y*SCREEN_W;
//	for(i=0;i<(SCREEN_W*FONT_H);i++) {
//		if(buf[i+buf_offs]!=0)
//			buf[i+buf_offs] = color;
//	}
	
	for(_y=y;_y<(y+FONT_H);_y++) {
		for(_x=0;_x<(16*FONT_W);_x++) {
			buf_offs = _y*SCREEN_W+_x;
			if(buf[buf_offs]!=0)
				buf[buf_offs] = color;
		}
	}

}

void draw_progress_bar(display_context_t disp, int y, uint32_t percent) {
	int rx,ry;
	draw_window(disp,204,20,"",&rx,&ry,0x00ff00ff);
	if(percent>100) percent = 100;
	uint32_t *buf = (uint32_t*)__safe_buffer[disp-1];
	int x;
	int y2;
	int prog_w = 200;
	int x_ofs = rx+2;//60;
	y = ry+2;
	int buf_offs;
	for(y2 = y; y2<(y+16);y2++) {
		buf_offs = y2*SCREEN_W;
		for(x=x_ofs;x<(x_ofs+prog_w);x++) {
					if((x-x_ofs)<=(percent<<1))
						buf[buf_offs+x] = 0x00ff00ff;
					else
						buf[buf_offs+x] = 0xff0000ff;
		}
	}
}

void draw_background(display_context_t disp) {
	print_text(disp, 0, 0, bg_text, 0x0000ffff);
}

void draw_window(display_context_t disp, int w, int h, char *title, int *r_x, int *r_y, uint32_t color) {
	int x,y;
	x = (SCREEN_W-w)/2;
	y = (SCREEN_H-h)/2;
	*r_x = x;
	*r_y = y;
	uint32_t *buf = (uint32_t*)__safe_buffer[disp-1];
	
	int _x,_y;
	for(_y=y;_y<(y+h);_y++) {
		for(_x=x;_x<(x+w);_x++) {
			if(
				(_y==y)||
				(_y==(y+h-1))||
				(_x==x)||
				(_x==(x+w-1)) //border
			)
				buf[_y*SCREEN_W+_x] = color;
			else
				buf[_y*SCREEN_W+_x] = 0x0;
		}
	}	
	
	print_text(disp,x+2,y+2,title,color);
}

void draw_alert(display_context_t disp, char *text) {
	int x,y;
	int w = (strlen(text)*FONT_W)+4;
	int h = 20;
	draw_window(disp,w,h,text,&x,&y, 0xff0000ff);
}
