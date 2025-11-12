#include <knabberkiste/drivers/st7066u.h>
#include <FreeRTOS.h>
#include "st7066u_config.h"

/* Macro checks  */
#ifndef ST7066U_GPIO_RW
    #error Please define ST7066U_GPIO_RW when using the ST7066u driver.
#endif
#ifndef ST7066U_GPIO_E
    #error Please define ST7066U_GPIO_E when using the ST7066u driver.
#endif
#ifndef ST7066U_GPIO_RS
    #error Please define ST7066U_GPIO_RS when using the ST7066u driver.
#endif
#ifndef ST7066U_GPIO_DB0
    #error Please define ST7066U_GPIO_DB0 when using the ST7066u driver.
#endif
#ifndef ST7066U_GPIO_DB1
    #error Please define ST7066U_GPIO_DB1 when using the ST7066u driver.
#endif
#ifndef ST7066U_GPIO_DB2
    #error Please define ST7066U_GPIO_DB2 when using the ST7066u driver.
#endif
#ifndef ST7066U_GPIO_DB3
    #error Please define ST7066U_GPIO_DB3 when using the ST7066u driver.
#endif
#ifndef ST7066U_GPIO_DB4
    #error Please define ST7066U_GPIO_DB4 when using the ST7066u driver.
#endif
#ifndef ST7066U_GPIO_DB5
    #error Please define ST7066U_GPIO_DB5 when using the ST7066u driver.
#endif
#ifndef ST7066U_GPIO_DB6
    #error Please define ST7066U_GPIO_DB6 when using the ST7066u driver.
#endif
#ifndef ST7066U_GPIO_DB7
    #error Please define ST7066U_GPIO_DB7 when using the ST7066u driver.
#endif

#define ST7066U_MODE_WRITE 0
#define ST7066U_MODE_READ 1
#define ST7066U_REGISTER_COMMAND 0
#define ST7066U_REGISTER_DATA 1

void st7066u_init() {
    /* Initialize the GPIO ports */
    ST7066U_GPIO_RW->mode = GPIO_MODE_OUTPUT;
    ST7066U_GPIO_E->mode = GPIO_MODE_OUTPUT;
    ST7066U_GPIO_RS->mode = GPIO_MODE_OUTPUT;
    ST7066U_GPIO_DB0->mode = GPIO_MODE_OUTPUT;
    ST7066U_GPIO_DB1->mode = GPIO_MODE_OUTPUT;
    ST7066U_GPIO_DB2->mode = GPIO_MODE_OUTPUT;
    ST7066U_GPIO_DB3->mode = GPIO_MODE_OUTPUT;
    ST7066U_GPIO_DB4->mode = GPIO_MODE_OUTPUT;
    ST7066U_GPIO_DB5->mode = GPIO_MODE_OUTPUT;
    ST7066U_GPIO_DB6->mode = GPIO_MODE_OUTPUT;
    ST7066U_GPIO_DB7->mode = GPIO_MODE_OUTPUT;

    ST7066U_GPIO_RW->output_data = ST7066U_MODE_WRITE;
    ST7066U_GPIO_RS->output_data = ST7066U_REGISTER_COMMAND;

    st7066u_write_command(0x30); // Function set []
    vTaskDelay(5);
    st7066u_write_command(0x30); // Function set []
    vTaskDelay(1);
    st7066u_write_command(0x38);
    vTaskDelay(1);
}

static void st7066u_send_byte(uint8_t byte) {
    ST7066U_GPIO_DB0->output_data = (byte & (1 << 0)) ? 1 : 0;
    ST7066U_GPIO_DB1->output_data = (byte & (1 << 1)) ? 1 : 0;
    ST7066U_GPIO_DB2->output_data = (byte & (1 << 2)) ? 1 : 0;
    ST7066U_GPIO_DB3->output_data = (byte & (1 << 3)) ? 1 : 0;
    ST7066U_GPIO_DB4->output_data = (byte & (1 << 4)) ? 1 : 0;
    ST7066U_GPIO_DB5->output_data = (byte & (1 << 5)) ? 1 : 0;
    ST7066U_GPIO_DB6->output_data = (byte & (1 << 6)) ? 1 : 0;
    ST7066U_GPIO_DB7->output_data = (byte & (1 << 7)) ? 1 : 0;

    ST7066U_GPIO_E->output_data = 1;
    vTaskDelay(1);
    ST7066U_GPIO_E->output_data = 0;
    vTaskDelay(1);
}

void st7066u_write_byte(uint8_t byte) {
    ST7066U_GPIO_RW->output_data = ST7066U_MODE_WRITE;
    ST7066U_GPIO_RS->output_data = ST7066U_REGISTER_DATA;

    st7066u_send_byte(byte);
}

void st7066u_write_chunk(uint8_t* buf, size_t size) {
    for(size_t i = 0; i < size; i++) {
        st7066u_write_byte(buf[i]);
    }
}
void st7066u_write_string(char* str) {
    do {
        st7066u_write_byte(*str);
    } while(*++str);
}
void st7066u_write_command(uint8_t command) {
    ST7066U_GPIO_RW->output_data = ST7066U_MODE_WRITE;
    ST7066U_GPIO_RS->output_data = ST7066U_REGISTER_COMMAND;

    st7066u_send_byte(command);
}

void st7066u_clear_display() {
    st7066u_write_command(0x01);
}

void st7066u_return_home() {
    st7066u_write_command(0x02);
}
void st7066u_set_entry_mode(ST7066U_MoveDirection_t direction, bool display_shift) {
    st7066u_write_command(0x04 | direction << 1 | display_shift ? 1 : 0);
}
void st7066u_on_off_control(bool display, bool cursor, bool cursor_blinking) {
    st7066u_write_command(0x08 | (display ? 4 : 0) | (cursor ? 2 : 0) | (cursor_blinking ? 1 : 0));
}
void st7066u_shift_cursor(ST7066U_MoveDirection_t direction) {
    st7066u_write_command(0x10 | direction << 2);
}
void st7066u_shift_display(ST7066U_MoveDirection_t direction) {
    st7066u_write_command(0x18 | direction << 2);
}
void st7066u_set_cgram_address(uint8_t address) {
    st7066u_write_command(0x40 | (address & 0x3F));
}
void st7066u_set_ddram_address(uint8_t address) {
    st7066u_write_command(0x80 | (address & 0x7F));
}