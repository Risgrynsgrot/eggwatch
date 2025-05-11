#include "menu.h"
#include <stdio.h>
#include <string.h>
#include <u8g2.h>

void menu_init(menu_t *menu, u8g2_t *u8g2) {
  menu->u8g2 = u8g2;
  menu->item_count = 0;
  menu->selected_index = 0;
}
void menu_add_item(menu_t *menu, char *title,
                   int (*on_pressed_func)(void)) {
  menu->items[menu->item_count] = (menu_item_t){
      .on_pressed = on_pressed_func,
  };
  strcpy(menu->items[menu->item_count].title, title);
  menu->item_count++;
}

void menu_draw(menu_t *menu) {
  u8g2_SetFont(menu->u8g2, u8g2_font_7x13B_tf);
  u8g2_SetDrawColor(menu->u8g2, 2);
  u8g2_SetFontPosTop(menu->u8g2);
  for (int i = 0; i < menu->item_count; i++) {
    int x = 0;
    int y = (i) * (MENU_FONT_HEIGHT); //+ MENU_PADDING);
    int w = 128;
    int h = MENU_FONT_HEIGHT;// + MENU_PADDING;
    bool is_selected = i == menu->selected_index;
    u8g2_DrawStr(menu->u8g2, x, y, menu->items[i].title);
    if (is_selected) {
      u8g2_DrawBox(menu->u8g2, x, y, w, h);
    }
  }
}
