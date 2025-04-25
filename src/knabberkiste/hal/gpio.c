#include <knabberkiste/hal/gpio.h>
#include <knabberkiste/util/bit_manipulation.h>
#include <knabberkiste/hal/delay.h>

struct GPIO_Pin {
    uint8_t _portOffset;
    uint8_t _pin;
};

static GPIO_TypeDef* gpio_ports[] = {
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE,
    GPIOF
};

void gpio_enable_port_clock(GPIO_Pin_t* pin) {
    SET_BIT(RCC->AHBENR, RCC_AHBENR_GPIOAEN_Pos + pin->_portOffset);
}

void gpio_set_pin_mode(GPIO_Pin_t* pin, GPIO_Mode_t mode) {
    WRITE_MASK_OFFSET(gpio_ports[pin->_portOffset]->MODER, 0b11, mode, pin->_pin * 2);
}

void gpio_set_output_type(GPIO_Pin_t* pin, GPIO_OutputType_t type) {
    WRITE_MASK_OFFSET(gpio_ports[pin->_portOffset]->OTYPER, 0b1, type, pin->_pin);
}

void gpio_set_pull_configuration(GPIO_Pin_t* pin, GPIO_PullConfiguration_t pullConfiguration) {
    WRITE_MASK_OFFSET(gpio_ports[pin->_portOffset]->PUPDR, 0b11, pullConfiguration, pin->_pin * 2);
}

void gpio_set_alternate(GPIO_Pin_t* pin, GPIO_AlternateFunction_t alternate) {
    volatile uint32_t* afr_ptr = gpio_ports[pin->_portOffset]->AFR + (pin->_pin / 8);
    uint8_t bit_offset = (pin->_pin % 8) * 4;
    WRITE_MASK_OFFSET(*afr_ptr, 0b1111, alternate, bit_offset);
}

bool gpio_read_pin(GPIO_Pin_t* pin) {
    return READ_BIT(gpio_ports[pin->_portOffset]->IDR, pin->_pin);
}
void gpio_write_pin(GPIO_Pin_t* pin, bool value) {
    WRITE_BIT(gpio_ports[pin->_portOffset]->ODR, pin->_pin, value);
}

void gpio_set_pin(GPIO_Pin_t* pin) {
    gpio_write_pin(pin, true);
}
void gpio_clear_pin(GPIO_Pin_t* pin) {
    gpio_write_pin(pin, false);
}
void gpio_toggle_pin(GPIO_Pin_t* pin) {
    TOGGLE_BIT(gpio_ports[pin->_portOffset]->ODR, pin->_pin);
}

GPIO_ConnectivityTestResult_t gpio_test_connectivity(GPIO_Pin_t* pin) {
    gpio_enable_port_clock(pin);

    gpio_set_pin_mode(pin, GPIO_MODE_INPUT);
    
    gpio_set_pull_configuration(pin, GPIO_PULLDOWN);
    delay(10);
    if(gpio_read_pin(pin)) return GPIO_CT_TIED_HIGH;

    gpio_set_pull_configuration(pin, GPIO_PULLUP);
    delay(10);
    if(!gpio_read_pin(pin)) return GPIO_CT_TIED_LOW;

    return GPIO_CT_FLOATING;
}

// GPIO pin definitions

#define __GPIO_SINGLE_PIN_DEFINITON(portLetter, portOffset, pin) \
    static GPIO_Pin_t _P##portLetter##pin = { ._portOffset = portOffset, ._pin = pin }; \
    GPIO_Pin_t* P##portLetter##pin = &_P##portLetter##pin

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