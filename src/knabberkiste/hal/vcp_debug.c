#include <knabberkiste/hal/vcp_debug.h>
#include <knabberkiste/io.h>
#include <knabberkiste/hal/gpio.h>
#include <knabberkiste/util/bit_manipulation.h>
#include <stdio.h>

#define VCP_USART_IF USART1

#define VCP_USART_RX_PIN PA9
#define VCP_USART_RX_PIN_AF GPIO_AF7
#define VCP_USART_TX_PIN PA10
#define VCP_USART_TX_PIN_AF GPIO_AF7

void vcp_init(long long baudrate) {
    // Configure the GPIO pins
    VCP_USART_RX_PIN->mode = GPIO_MODE_ALTERNATE;
    VCP_USART_RX_PIN->alternate = VCP_USART_RX_PIN_AF;

    VCP_USART_TX_PIN->mode = GPIO_MODE_ALTERNATE;
    VCP_USART_TX_PIN->alternate = VCP_USART_TX_PIN_AF;

    // Enable the USART clock
    SET_MASK(RCC->APB2ENR, RCC_APB2ENR_USART1EN);

    // Disable the UART so configurations can be made 
    CLEAR_MASK(VCP_USART_IF->CR1, USART_CR1_UE);

    // Configure the UART control register as needed by the driver
    VCP_USART_IF->CR1 = 
        USART_CR1_RXNEIE | // RXNE interrupt enable
        USART_CR1_TE | // Transmitter enable
        USART_CR1_RE // Receiver enable
    ;
    VCP_USART_IF->CR2 = 0;
    VCP_USART_IF->CR3 = 
        USART_CR3_DMAR // Enable DMA for receiver
    ;

    uint32_t brrValue = SystemCoreClock / baudrate;
    VCP_USART_IF->BRR = brrValue;

    // Enable the UART
    SET_MASK(VCP_USART_IF->CR1, USART_CR1_UE);

    vcp_println("VCP serial port initialized successfully!");
}

void vcp_putchar(char c) {
    VCP_USART_IF->TDR = c;
        
    // Wait for the transmit register to be emtpy
    while(!READ_MASK(VCP_USART_IF->ISR, USART_ISR_TXE));
}

void vcp_print(const char* str) {
    str--;
    while(*(++str) != 0) {
        vcp_putchar(*str);
    }
}

void vcp_println(const char* str) {
    vcp_print(str);
    vcp_putchar('\r');
    vcp_putchar('\n');
}