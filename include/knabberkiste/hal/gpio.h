#pragma once

#include <knabberkiste/io.h>
#include <stdbool.h>

typedef struct GPIO_Pin GPIO_Pin_t;

typedef enum {
    GPIO_CT_FLOATING,
    GPIO_CT_TIED_LOW,
    GPIO_CT_TIED_HIGH
} GPIO_ConnectivityTestResult_t;

typedef enum {
    GPIO_MODE_INPUT = 0b00,
    GPIO_MODE_OUTPUT = 0b01,
    GPIO_MODE_ALTERNATE = 0b10,
    GPIO_MODE_ANALOG = 0b11
} GPIO_Mode_t;

typedef enum {
    GPIO_OUTPUT_PUSHPULL = 0,
    GPIO_OUTPUT_OPENDRAIN = 1
} GPIO_OutputType_t;

typedef enum {
    GPIO_NOPULL = 0b00,
    GPIO_PULLUP = 0b01,
    GPIO_PULLDOWN = 0b10
} GPIO_PullConfiguration_t;

typedef enum {
    GPIO_AF0 = 0,
    GPIO_AF1 = 1,
    GPIO_AF2 = 2,
    GPIO_AF3 = 3,
    GPIO_AF4 = 4,
    GPIO_AF5 = 5,
    GPIO_AF6 = 6,
    GPIO_AF7 = 7,
    GPIO_AF8 = 8,
    GPIO_AF9 = 9,
    GPIO_AF10 = 10,
    GPIO_AF11 = 11,
    GPIO_AF12 = 12,
    GPIO_AF13 = 13,
    GPIO_AF14 = 14,
    GPIO_AF15 = 15
} GPIO_AlternateFunction_t;

void gpio_enable_port_clock(GPIO_Pin_t* pin);

void gpio_set_pin_mode(GPIO_Pin_t* pin, GPIO_Mode_t mode);
void gpio_set_output_type(GPIO_Pin_t* pin, GPIO_OutputType_t type);
void gpio_set_pull_configuration(GPIO_Pin_t* pin, GPIO_PullConfiguration_t type);
void gpio_set_alternate(GPIO_Pin_t* pin, GPIO_AlternateFunction_t alternate);

bool gpio_read_pin(GPIO_Pin_t* pin);
void gpio_write_pin(GPIO_Pin_t* pin, bool value);
void gpio_set_pin(GPIO_Pin_t* pin);
void gpio_clear_pin(GPIO_Pin_t* pin);
void gpio_toggle_pin(GPIO_Pin_t* pin);

GPIO_ConnectivityTestResult_t gpio_test_connectivity(GPIO_Pin_t* pin);

// GPIO pin definitions
#define GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, pin) extern GPIO_Pin_t* P##portLetter##pin
#define GPIO_SINGLE_PORT_DEFINITION(portLetter, portOffset) \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 0); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 1); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 2); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 3); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 4); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 5); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 6); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 7); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 8); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 9); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 10); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 11); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 12); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 13); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 14); \
    GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 15)

GPIO_SINGLE_PORT_DEFINITION(A, 0);
GPIO_SINGLE_PORT_DEFINITION(B, 1);
GPIO_SINGLE_PORT_DEFINITION(C, 2);
GPIO_SINGLE_PORT_DEFINITION(D, 3);
GPIO_SINGLE_PORT_DEFINITION(E, 4);
GPIO_SINGLE_PORT_DEFINITION(F, 5);

#undef GPIO_SINGLE_PIN_DEFINITON
#undef GPIO_SINGLE_PORT_DEFINITION