#ifndef GPIO_H
#define GPIO_H

#define BTN_BASE XPAR_AXI_GPIO_0_BASEADDR
#define SW_BASE XPAR_AXI_GPIO_1_BASEADDR

#define BTN_MID         0x1
#define BTN_LEFT        0x4
#define BTN_RIGHT       0x8
#define BTN_UP          0x10
#define BTN_DOWN        0x2

#define SW0            0x1
#define SW1            0x2
#define SW2            0x4
#define SW3            0x8
#define SW4            0x10
#define SW5            0x20
#define SW6            0x40
#define SW7            0x80

int gpio_sw_check(int switch_vals);
int gpio_btn_check(int btn_vals);

#endif /* GPIO_H */
