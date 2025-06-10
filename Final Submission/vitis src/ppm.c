#include "ppm.h"
#include "xil_io.h"
#include "xil_printf.h"

extern int ch_min_cnts[NUM_CH];
extern int ch_max_cnts[NUM_CH];
int sync_counts[NUM_CH] = {154340, 155204, 100791, 152324};


void ppm_init_gen(){
	// Set ppm gen period
	*((int *) (AXI_PPM_BASE + PPM_GEN_PERIOD_OFFS)) = (int)PPM_GEN_PERIOD;
	// Set channel 5 & 6 values to a valid count
	*((int *)(AXI_PPM_BASE + PPM_GEN_SLV0_OFFS + PPM_REG_OFFS * 4)) = (int)PPM_CH_5_6_VALS;
	*((int *)(AXI_PPM_BASE + PPM_GEN_SLV0_OFFS + PPM_REG_OFFS * 5)) = (int)PPM_CH_5_6_VALS;
}

void ppm_set_gen_cnts(int* ch_counts){
	for(int i = 0; i < NUM_CH; i++){
		*((int *)(AXI_PPM_BASE + PPM_GEN_SLV0_OFFS + PPM_REG_OFFS * i)) = ch_counts[i];
	}
	return;
}

void ppm_drone_sync(){
	for(int i = 0; i < NUM_CH; i++){
		*((int *)(AXI_PPM_BASE + PPM_GEN_SLV0_OFFS + PPM_REG_OFFS * i)) = sync_counts[i];
	}
}


void ppm_set_sw_relay(){
	*((int *)(AXI_PPM_BASE)) |= 0x00000001;
	return;
}

void ppm_set_hw_relay(){
	*((int *)(AXI_PPM_BASE)) &= 0xFFFFFFFE;
	return;
}

int ppm_get_frame_num(){
	return *((int *)(AXI_PPM_BASE + PPM_REG_OFFS));;
}

void ppm_get_ch_counts(int* ch_counts){
	for (int i = 0; i < NUM_CH; i++) {
		int *det_slv_addr_i = (int *)(AXI_PPM_BASE + PPM_REG_OFFS * 4 + PPM_REG_OFFS * i);
		ch_counts[i] = *det_slv_addr_i;
	}
	return;
}


void ppm_print_all(){
	xil_printf("\033[2J");  // Clear entire screen
	xil_printf("\033[1;1H");  // Move cursor to row 1, column 1

	xil_printf("Registers \n\r");
	xil_printf("----------------\n\r");
	for (int i = 0; i < 16; i++) {
		xil_printf("Reg %d - %d\n\r", i, *(int *)(AXI_PPM_BASE + PPM_REG_OFFS * i));
	}

	return;
}

void ppm_print_ch_counts(int* ch_counts){
	xil_printf("\033[2J");  // Clear entire screen
	xil_printf("\033[1;1H");  // Move cursor to row 1, column 1

	xil_printf("Channel Counts \n\r");
	xil_printf("----------------\n\r");
	for (int i = 0; i < NUM_CH; i++) {
		xil_printf("Ch. %d - %d\n\r", i+1, ch_counts[i]);
	}

	return;
}
