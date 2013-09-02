#ifndef GFX_H
#define GFX_H

#include <libdragon.h>
#include <fat16.h>

#define SCREEN_W 320
#define SCREEN_H 240

#define NUM_LIST_ITEMS 9
#define DIRLIST_X 16
#define DIRLIST_Y 48
#define MARKER_MAX DIRLIST_Y+NUM_LIST_ITEMS*FONT_H-FONT_H
extern void *__safe_buffer[];
void print_text(display_context_t disp, int x, int y, char *str, uint32_t color);
void print_dirlist(display_context_t disp, int x, int y, fat_entry *dirlist, uint32_t list_offset);
void print_dirlist_dummy(display_context_t disp);
void draw_marker(display_context_t disp, int y, uint32_t color);
void draw_progress_bar(display_context_t disp, int y, uint32_t percent);
void draw_background(display_context_t disp);
void draw_window(display_context_t disp, int w, int h, char *title, int *r_x, int *r_y, uint32_t color);
void draw_alert(display_context_t disp, char *text);
#endif // GFX_H
