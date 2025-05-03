#include <knabberkiste/hal/clock.h>
#include <knabberkiste/io.h>

void clock_configure64MHz() {
    // Disable the PLL
    CLEAR_MASK(RCC->CR, RCC_CR_PLLON);

    // Wait until PLL is disabled
    while(READ_MASK(RCC->CR, RCC_CR_PLLRDY));

    // PLL multiplication factor = 16
    WRITE_MASK_OFFSET(RCC->CFGR, 0b1111, 0b1110, RCC_CFGR_PLLMUL_Pos);

    // PLL source = HSI/2
    CLEAR_MASK(RCC->CFGR, RCC_CFGR_PLLSRC);

    // APB1 prescaler = 2 (max. frequency = 36 MHz)
    WRITE_MASK_OFFSET(RCC->CFGR, 0b111, 0b100, RCC_CFGR_PPRE1_Pos);

    // Set flash latency to 2 wait states, necessary for HCLK > 48 MHz
    WRITE_MASK_OFFSET(FLASH->ACR, 0b111, 0b010, FLASH_ACR_LATENCY_Pos);

    // Enable the PLL
    SET_MASK(RCC->CR, RCC_CR_PLLON);

    // Wait until PLL is ready
    while(!READ_MASK(RCC->CR, RCC_CR_PLLRDY));

    // Select PLL as system clock source
    WRITE_MASK_OFFSET(RCC->CFGR, 0b11, 0b10, RCC_CFGR_SW_Pos);

    // Wait until PLL is used as system clock source
    while(READ_MASK_OFFSET(RCC->CFGR, 0b11, RCC_CFGR_SWS_Pos) != 0b10) {}

    // Update the system core clock
    SystemCoreClockUpdate();
}