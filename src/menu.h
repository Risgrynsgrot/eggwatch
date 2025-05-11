#pragma once
#include <stdbool.h>

#define MENU_MAX_STRING_LENGTH 32
#define MENU_MAX_ITEMS 8
#define MENU_FONT_HEIGHT 13
#define MENU_PADDING 3

typedef struct u8g2_struct u8g2_t;

typedef struct menu_item {
  char title[MENU_MAX_STRING_LENGTH];
  int (*on_pressed)(void);
} menu_item_t;

typedef struct menu {
  u8g2_t* u8g2;
  menu_item_t items[MENU_MAX_ITEMS];
  int selected_index;
  int item_count;
} menu_t;

void menu_init(menu_t *menu, u8g2_t* u8g2);
void menu_draw(menu_t *menu);
void menu_add_item(menu_t *menu, char *title, int (*on_pressed_func)(void));
