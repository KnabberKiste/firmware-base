#include <knabberkiste/hal/clock.h>
#include <knabberkiste/io.h>

// Prescaler mapping table
static const uint16_t ahb_prescaler_mapping[] = {
    1, // 0000
    1, // 0001
    1, // 0010
    1, // 0011
    1, // 0100
    1, // 0101
    1, // 0110
    1, // 0111
    2, // 1000
    4, // 1001
    8, // 1010
    16, // 1011
    64, // 1100
    128, // 1101
    256, // 1110
    512// 1111
};
static const uint8_t apb_prescaler_mapping[] = {
    1, // 000
    1, // 001
    1, // 010
    1, // 011
    2, // 100
    4, // 101
    8, // 110
    16, // 111
};


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

uint16_t clock_getAHBPrescaler() {
    return ahb_prescaler_mapping[READ_MASK_OFFSET(RCC->CFGR, 0b1111, RCC_CFGR_HPRE_Pos)];
}

uint8_t clock_getAPB1Prescaler() {
    return apb_prescaler_mapping[READ_MASK_OFFSET(RCC->CFGR, 0b111, RCC_CFGR_PPRE1_Pos)];
}

uint8_t clock_getAPB2Prescaler() {
    return apb_prescaler_mapping[READ_MASK_OFFSET(RCC->CFGR, 0b111, RCC_CFGR_PPRE2_Pos)];
}

uint32_t clock_getAHBFrequency() {
    return SystemCoreClock / clock_getAHBPrescaler();
}

uint32_t clock_getAPB1Frequency() {
    return clock_getAHBFrequency() / clock_getAPB1Prescaler();
}

uint32_t clock_getAPB2Frequency() {
    return clock_getAHBFrequency() / clock_getAPB2Prescaler();
}