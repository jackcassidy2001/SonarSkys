#ifndef PPM_H
#define PPM_H

#define AXI_PPM_BASE XPAR_AXI_PPM_0_S00_AXI_BASEADDR
#define PPM_REG_OFFS 0x4
#define PPM_GEN_PERIOD_OFFS (PPM_REG_OFFS * 2)
#define PPM_DET_SLV0_OFFS (PPM_REG_OFFS * 4)
#define PPM_GEN_SLV0_OFFS (PPM_REG_OFFS * 10)
#define PPM_GEN_PERIOD 2005600
#define PPM_CH_5_6_VALS 200000
#define NUM_CH 4

#define THRUST 2
#define YAW 3
#define TRANS_X 0
#define TRANS_Y 1
#define KNOB_LEFT 5
#define KNOB_RIGHT 4

void ppm_init_gen();
void ppm_set_gen_cnts(int* ch_counts);
void ppm_drone_sync();
void ppm_set_sw_relay();
void ppm_set_hw_relay();
int ppm_get_frame_num();
void ppm_get_ch_counts(int* ch_counts);
void ppm_print_ch_counts(int* ch_counts);
void ppm_print_all();

#endif /* PPM_H */
