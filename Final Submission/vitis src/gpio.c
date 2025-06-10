#include "gpio.h"
#include "xil_io.h"

int gpio_sw_check(int switch_vals) {
    int switch_state = Xil_In32(SW_BASE);
    return (switch_state & switch_vals) == switch_vals;
}

int gpio_btn_check(int btn_vals) {
    int btn_state = Xil_In32(BTN_BASE);
    return (btn_state & btn_vals) == btn_vals;
}
