#include <knabberkiste/util/critical.h>
#include <knabberkiste/io.h>
#include <stdint.h>

static uint32_t critical_counter;

void critical_enter() {
    __disable_irq();
    critical_counter++;
}

void critical_exit() {
    critical_counter--;
    if(critical_counter == 0) __enable_irq();
}