#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdio.h>
#include <string.h>
#include <u8g2.h>
#include "menu.h"

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 8
#define UART_RX_PIN 9

u8g2_t u8g2;

// ADD THE INIT DISPLAY
#define SPI_PORT spi0
#define PIN_CS 5
#define PIN_SCK 2
#define PIN_MOSI 3
#define SPI_SPEED 4000 * 1000
#define PIN_DC 4
#define PIN_RST 11

#define MAX_HUNGER 100
typedef struct char_stats {
  int hunger;
  int money;
} char_stats_t;

char_stats_t pet;
menu_t menu;

int menu_test(void) {
  return 1;
}

void init_game(u8g2_t* u8g2)
{
  pet = (char_stats_t){
      .hunger = 25,
      .money = 10,
  };

  menu_init(&menu, u8g2);
  menu_add_item(&menu, "bing", menu_test);
  menu_add_item(&menu, "swag", menu_test);
}

#define PET_MONEY_POS_X 10
#define PET_MONEY_POS_Y 10
#define PET_STAT_FONT_HEIGHT 14
#define PET_STAT_PADDING 3

void pet_draw(u8g2_t *draw, char_stats_t *pet) {
  char text_buf[128];
  sprintf(text_buf, "$: %d", pet->money);
  int y_pos = PET_STAT_FONT_HEIGHT;
  u8g2_DrawStr(draw, PET_MONEY_POS_X, y_pos, text_buf);
  y_pos += PET_STAT_FONT_HEIGHT + PET_STAT_PADDING;
  sprintf(text_buf, "H: %d", pet->hunger);
  u8g2_DrawStr(draw, PET_MONEY_POS_X, y_pos, text_buf);
  y_pos += PET_STAT_FONT_HEIGHT + PET_STAT_PADDING;

  u8g2_DrawFrame(draw, PET_MONEY_POS_X, y_pos, 100, 5);
  int percentage = (pet->hunger * 100 / MAX_HUNGER * 100) / 100;
  u8g2_DrawBox(draw, PET_MONEY_POS_X, y_pos, percentage, 5);

}

uint8_t u8x8_byte_pico_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int,
                              void *arg_ptr) {
  uint8_t *data;
  switch (msg) {
  case U8X8_MSG_BYTE_SEND:
    data = (uint8_t *)arg_ptr;
    spi_write_blocking(SPI_PORT, data, arg_int);
    break;
  case U8X8_MSG_BYTE_INIT:
    u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
    break;
  case U8X8_MSG_BYTE_SET_DC:
    u8x8_gpio_SetDC(u8x8, arg_int);
    break;
  case U8X8_MSG_BYTE_START_TRANSFER:
    u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
    u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO,
                            u8x8->display_info->post_chip_enable_wait_ns, NULL);
    break;
  case U8X8_MSG_BYTE_END_TRANSFER:
    u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO,
                            u8x8->display_info->pre_chip_disable_wait_ns, NULL);
    u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
    break;
  default:
    return 0;
  }
  return 1;
}

uint8_t u8x8_gpio_and_delay_pico(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int,
                                 void *arg_ptr) {
  (void)arg_ptr;
  switch (msg) {
  case U8X8_MSG_GPIO_AND_DELAY_INIT:
    spi_init(SPI_PORT, SPI_SPEED);
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_init(PIN_RST);
    gpio_init(PIN_DC);
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_RST, 1);
    gpio_put(PIN_CS, 1);
    gpio_put(PIN_DC, 0);
    break;
  case U8X8_MSG_DELAY_NANO: // delay arg_int * 1 nano second
    sleep_us(arg_int); // 1000 times slower, though generally fine in practice
                       // given rp2040 has no `sleep_ns()`
    break;
  case U8X8_MSG_DELAY_100NANO: // delay arg_int * 100 nano seconds
    sleep_us(arg_int);
    break;
  case U8X8_MSG_DELAY_10MICRO: // delay arg_int * 10 micro seconds
    sleep_us(arg_int * 10);
    break;
  case U8X8_MSG_DELAY_MILLI: // delay arg_int * 1 milli second
    sleep_ms(arg_int);
    break;
  case U8X8_MSG_GPIO_CS: // CS (chip select) pin: Output level in arg_int
    gpio_put(PIN_CS, arg_int);
    break;
  case U8X8_MSG_GPIO_DC: // DC (data/cmd, A0, register select) pin: Output level
    gpio_put(PIN_DC, arg_int);
    break;
  case U8X8_MSG_GPIO_RESET:     // Reset pin: Output level in arg_int
    gpio_put(PIN_RST, arg_int); // printf("U8X8_MSG_GPIO_RESET %d\n", arg_int);
    break;
  default:
    u8x8_SetGPIOResult(u8x8, 1); // default return value
    break;
  }
  return 1;
}

void draw_display() {
  u8g2_ClearBuffer(&u8g2);
  //u8g2_ClearDisplay(&u8g2);
  u8g2_SetDrawColor(&u8g2, 1);
  u8g2_SetFont(&u8g2, u8g2_font_fur14_tf);
  //*out_width = u8g2_DrawStr(&u8g2, x, y, text);
  /* pet_draw(&u8g2, &pet); */
  menu_draw(&menu);
  u8g2_UpdateDisplay(&u8g2);
}

void display_sequence() {
  u8g2_Setup_sh1107_seeed_128x128_f(&u8g2, U8G2_R0, u8x8_byte_pico_hw_spi,
                              u8x8_gpio_and_delay_pico);

  u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in
                           // sleep mode after this,
  u8g2_SetPowerSave(&u8g2, 0);
  /* int out_width; */
  /* draw_display(20, 20, "Hello world", &out_width); */
}

int main() {
  uart_init(UART_ID, BAUD_RATE);
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  init_game(&u8g2);

  // U8G2
  display_sequence();


  stdio_init_all();
  while (1) {
    pet.hunger++;
    pet.hunger %= MAX_HUNGER;
    /* uint32_t time = to_ms_since_boot(get_absolute_time()); */
    /* sprintf(text_buf, "time: %u", time); */
    draw_display();

    sleep_ms(10);
  }

  return 0;
}
