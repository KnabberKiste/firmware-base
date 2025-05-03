#include <knabberkiste/hal/gpio.h>
#include <knabberkiste/util/bit_manipulation.h>
void gpio_enable_port_clocks() {
    SET_MASK(RCC->AHBENR, 
        RCC_AHBENR_GPIOAEN | 
        RCC_AHBENR_GPIOBEN | 
        RCC_AHBENR_GPIOCEN |
        RCC_AHBENR_GPIODEN |
        RCC_AHBENR_GPIOEEN |
        RCC_AHBENR_GPIOFEN
    );
}

#define __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, pin) struct __GPIO_PinType##pin* const P##portLetter##pin = (void*)GPIO##portLetter##_BASE;
#define __GPIO_SINGLE_PORT_DEFINITION(portLetter, portOffset) \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 0); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 1); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 2); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 3); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 4); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 5); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 6); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 7); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 8); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 9); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 10); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 11); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 12); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 13); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 14); \
    __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, 15)

__GPIO_SINGLE_PORT_DEFINITION(A, 0);
__GPIO_SINGLE_PORT_DEFINITION(B, 1);
__GPIO_SINGLE_PORT_DEFINITION(C, 2);
__GPIO_SINGLE_PORT_DEFINITION(D, 3);
__GPIO_SINGLE_PORT_DEFINITION(E, 4);
__GPIO_SINGLE_PORT_DEFINITION(F, 5);

#undef __GPIO_SINGLE_PIN_DEFINITON
#undef __GPIO_SINGLE_PORT_DEFINITION