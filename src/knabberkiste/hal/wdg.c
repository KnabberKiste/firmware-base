#include <knabberkiste/io.h>
#include <knabberkiste/hal/wdg.h>
#include <knabberkiste/util/error.h>

#define IWDG_RELOAD_KEY 0xAAAA
#define IWDG_CONFIG_KEY 0x5555
#define IWDG_ENABLE_KEY 0xCCCC

#define IWDG_MAX_COUNTER_VALUE 4095

#define LSI_RC_FREQ 40000UL

void iwdg_init(uint32_t timeout) {
    const uint16_t prescalers[] = { 4, 8, 16, 32, 64, 128, 256 };

    // Calculate the prescaler and window register value
    uint32_t reload_value = 0;
    uint8_t prescaler_index = 0;

    // Iterate through the prescalers to find a matching prescaler
    for(uint8_t i = 0; i < sizeof(prescalers) / sizeof(*prescalers); i++) {
        uint16_t prescaler = prescalers[i];
        uint32_t max_timeout = (1000 * (uint32_t)prescaler * IWDG_MAX_COUNTER_VALUE) / LSI_RC_FREQ;

        if(max_timeout >= timeout) {
            // This is the matching prescaler
            reload_value = (LSI_RC_FREQ * timeout) / (1000 * prescaler);
            prescaler_index = i;
        }
    }

    if(!reload_value) {
        error_throw(ERR_RANGE, "IWDG timeout value out of range.");
    }
    
    // Enable the IWDG
    IWDG->KR = IWDG_ENABLE_KEY;
    
    // Unlock register access protection
    IWDG->KR = IWDG_CONFIG_KEY;

    // Write the values to the registers
    IWDG->PR = prescaler_index;
    IWDG->RLR = reload_value;

    // Wait for the values to be updated
    while(IWDG->SR != 0);

    // Reset the watchdog
    iwdg_reset();
}

void iwdg_reset() {
    IWDG->KR = IWDG_RELOAD_KEY;
}