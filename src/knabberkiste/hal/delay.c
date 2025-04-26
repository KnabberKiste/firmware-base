#if !__has_include("FreeRTOS.h")

    #include <knabberkiste/hal/delay.h>
    #include <knabberkiste/io.h>

    static volatile uint64_t tick_cnt = 0;
    static TickRate_t tick_rate;

    void SysTick_Handler() {
        tick_cnt++;
    }

    void delay_init(TickRate_t res) {
        SystemCoreClockUpdate();
    
        tick_rate = res;
    
        uint32_t ticks = SystemCoreClock / res;
        SysTick_Config(ticks);
        __enable_irq();
        NVIC_EnableIRQ(SysTick_IRQn);
    }

    void delay(float milliseconds) {
        uint32_t diffTicks = (milliseconds * (float)tick_rate) / (1000.0);
        uint64_t endTick = tick_cnt + diffTicks;

        while(tick_cnt < endTick);
    }

#endif