#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdio.h>
#include <string.h>
#include <u8g2.h>

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

void draw_display(int x, int y, char* text, int* out_width) {
  u8g2_ClearBuffer(&u8g2);
  //u8g2_ClearDisplay(&u8g2);
  u8g2_SetDrawColor(&u8g2, 1);
  u8g2_SetFont(&u8g2, u8g2_font_t0_11_te);
  *out_width = u8g2_DrawStr(&u8g2, x, y, text);
  u8g2_DrawFrame(&u8g2, x + 50, y, 10, 10);
  u8g2_UpdateDisplay(&u8g2);
}

void display_sequence() {
  u8g2_Setup_sh1107_seeed_128x128_f(&u8g2, U8G2_R0, u8x8_byte_pico_hw_spi,
                              u8x8_gpio_and_delay_pico);

  u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in
                           // sleep mode after this,
  u8g2_SetPowerSave(&u8g2, 0);
  int out_width;
  draw_display(20, 20, "Hello world", &out_width);
}

int main() {
  uart_init(UART_ID, BAUD_RATE);
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  // U8G2
  display_sequence();
  char text_buf[128] = "Hello world";

  stdio_init_all();
  int x = 30;
  int y = 50;
  int x_dir = 1;
  int y_dir = -1;
  int out_width = 0;

  while (1) {
    //x += x_dir;
    //y += y_dir;
    if (x > 128 - out_width || x < 0) {
      //x_dir *= -1;
    }
    if (y > 128 || y < 11) {
      //y_dir *= -1;
    }

    uint32_t time = to_ms_since_boot(get_absolute_time());
    sprintf(text_buf, "time: %d", time);
    draw_display(x, y, text_buf, &out_width);

    sleep_ms(1000);
  }

  return 0;
}
