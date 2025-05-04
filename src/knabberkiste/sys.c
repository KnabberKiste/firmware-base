#include <knabberkiste/hal/clock.h>
#include <knabberkiste/hal/gpio.h>
#include <knabberkiste/hal/vcp_debug.h>
#include <knabberkiste/sys.h>
#include <knabberkiste/knabbercan.h>

void sys_init() {
    clock_configure64MHz();
    gpio_enable_port_clocks();
    vcp_init(921600);
    kc_init();
}