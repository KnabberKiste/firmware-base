#pragma once

typedef enum {
    DELAYRES_100MS = 10,
    DELAYRES_10MS = 100,
    DELAYRES_1MS = 1000,
    DELAYRES_100US = 10000
} TickRate_t;

void delay_init(TickRate_t res);
void delay(float milliseconds);