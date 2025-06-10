#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xbasic_types.h"
#include <unistd.h>
#include "fifo.h"
#include "interrupt_setup.h"
#include "gpio.h"
#include "wav_data.h"
#include "tone.h"
#include "stdlib.h"
#include "ppm.h"
#include "minfft.h"
#include "fft_helper.h"
#include "xtime_l.h"

#define BYTES_PER_TX 4

extern int wav_data[WAV_ELEMENTS];
int wav_index = 0;

// Counts to frequencies
int ch_id[NUM_CH]= {TRANS_X, TRANS_Y, THRUST, YAW};
int ch_max_cnts[NUM_CH] = {188300, 182700, 180400, 192700};//, 100800, 100800};
int ch_min_cnts[NUM_CH] = {109900, 115000, 100800, 112600};//, 203100, 203100};
float ch_min_freq[NUM_CH] = {120.0, 300.0, 460.0, 620.0};
float ch_max_freq[NUM_CH] = {220.0, 380.0, 540.0, 700.0};
float ch_mix_scale[NUM_CH] = {0.10, 0.45, 0.35, 0.10};

#define N 4096
#define NUM_PEAKS 4
minfft_aux* aux;
minfft_real x1[N], x2[N];
minfft_cmpl y[N];

int main() {

	init_platform();
	// FIFO
    int tx_vacancy = 0, rv_occupancy = 0;
    int bytes_to_tx = 0;
    int tx_latch_flag = 0;

    // Receive
    int rv_buffer[FIFO_SIZE];
    int rv_index = 0;
    int rv_tx_index = 0;
    int rv_fft_index = 0;

    // Tones
    int snd_buffer_mix[FIFO_SIZE];
    int snd_buffer_tones[NUM_CH][FIFO_SIZE];
    int snd_gen_index[NUM_CH] = {0, 0, 0, 0};
    int snd_mix_index = 0;
    int snd_tx_index = 0;
    float ch_freqs[NUM_CH];
    float phases[NUM_CH] = {0.0, 0.0, 0.0, 0.0};

    // PPM
    int ch_counts_in[NUM_CH];
    int ch_counts_out[NUM_CH];

    // FFT
    int x1_index = 0, x2_index = 0;
    int x_select = 0;
    int fft_x_select = 0;
    int fft_ready = 0;
    float magnitudes[N];
    float pk_ch_freq[NUM_CH];
    float pk_ch_mag[NUM_CH];
    float freq_res = fft_get_freq_res(SAMPLE_RATE, N);

    XTime start, end;
    double elapsed_time = 0.0;
    int sys_clock = 667000000 /2;
    char buff[50];

    aux = minfft_mkaux_realdft_1d(N);
    if(aux == NULL){
    	xil_printf("Failed to make aux.. \n\r");
    	return -1;
    }

    // Setupt interrupts - register isr(s)
    setup_interrupt_system();

    // Enable the AXI FIFO core interrupts
    fifo_set_en_int(FIFO_TFPE | FIFO_TFPF | FIFO_TC| FIFO_RC | FIFO_TSE | FIFO_TPOE);

    // Init ppm generator with period and valid channel counts for ch 5 & 6
    ppm_init_gen();

    // GUI Init
    fft_gui_send_cfg();
    //XTime_GetTime(&start);
    while(!gpio_btn_check(BTN_MID)){

		// Occupancy is # of 32 bit slots
		rv_occupancy = fifo_get_rv_occupancy();
		rv_occupancy = (rv_occupancy >= 500)? 500 : rv_occupancy; // Max reception 510
		//xil_printf("occupancy %d\n\r", rv_occupancy);
		if(rv_occupancy >= 10){
			// Read data from fifo into buffer
			xil_printf("reading %d\n\r", rv_occupancy);
			fifo_rv_rd_data(rv_buffer, rv_occupancy, &rv_index, FIFO_SIZE);
		}

    	// Receive data from receive FIFO and perform FFT when possible
    	if(gpio_sw_check(SW0)){
    		fft_x_select = fifo_rv_rd_fft(x1, x2, rv_occupancy, &x1_index, &x2_index, N, &x_select, &fft_ready, rv_buffer, &rv_fft_index, FIFO_SIZE);

    		if(fft_ready){
    			if(!fft_x_select){
    				xil_printf("FFT x1\n\r");
    				minfft_realdft (x1, y, aux);
    			}else{
    				xil_printf("FFT x2\n\r");
    				minfft_realdft (x2, y, aux);
    			}
    			fft_get_N_magnitudes(y, N, magnitudes);
    			fft_get_ch_peaks(freq_res, magnitudes, pk_ch_freq, pk_ch_mag, N);
    			fft_gui_send_peaks(NUM_CH, pk_ch_mag, pk_ch_freq);
	    	    //XTime_GetTime(&end);
	    		//xil_printf("clock end %d\n\r", end);
	    		//elapsed_time = ((double) (end - start) / sys_clock);
	    		//sprintf(buff, "time - %.9f\n\r",elapsed_time);
	    		//XTime_GetTime(&start);
        		// Map frequencies to counts
        		for(int i = 0; i < NUM_CH; i++){
        			ch_counts_out[i] = tone_map_freq_ppm(ch_min_cnts[i], ch_max_cnts[i], ch_min_freq[i], ch_max_freq[i], pk_ch_freq[i]);
        		}
        		ppm_set_gen_cnts(ch_counts_out);
    		}
    	}

		// Generate tone data & write to transmit FIFO
    	if(gpio_sw_check(SW1) && !tx_latch_flag){
    		// Preparation
    		//xil_printf("clock start %d\n\r", start);
    		tx_vacancy = fifo_get_tx_vacancy();
    		ppm_get_ch_counts(ch_counts_in);

    		// Map counts to frequencies
    		for(int i = 0; i < NUM_CH; i++){
    			ch_freqs[i] = tone_map_ppm_freq(ch_min_cnts[i], ch_max_cnts[i], ch_min_freq[i], ch_max_freq[i], ch_counts_in[i]);
    		}
    		// Generate tones
    		for(int i = 0; i < NUM_CH; i++){
    			tone_generate(ch_freqs[i], tx_vacancy, snd_buffer_tones[i], &snd_gen_index[i], &phases[i], FIFO_SIZE);
    		}
    		// Mix tones
    		tone_mixer(snd_buffer_tones, ch_mix_scale, NUM_CH, snd_buffer_mix, tx_vacancy, &snd_mix_index, FIFO_SIZE);

    		// Write block of data to the transmit FIFO
    		fifo_tx_wr_data_blk(snd_buffer_mix, tx_vacancy, &snd_tx_index, FIFO_SIZE);
    		// Data to transmit in bytes -- needed by "fifo_tx_strt_len"
    		bytes_to_tx = tx_vacancy * BYTES_PER_TX;
    		tx_latch_flag = 1;
    	}


    	// Receive data from receive FIFO and pass to transmit FIFO
    	if(gpio_sw_check(SW2) && !tx_latch_flag){
				fifo_tx_wr_data_blk(rv_buffer, rv_occupancy, &rv_tx_index, FIFO_SIZE);
				bytes_to_tx = rv_occupancy * 4;
				tx_latch_flag = 1;
    	}


    	// Fill entire FIFO with WAV data
    	if(gpio_sw_check(SW3) && !tx_latch_flag){
    		//xil_printf("Filling Entire FIFO\n\r");
    		bytes_to_tx = fifo_tx_fill_full(wav_data, &wav_index, WAV_ELEMENTS) * BYTES_PER_TX;
    	    tx_latch_flag = 1;
    	}

    	// Transmit data in transmit FIFO
    	if(gpio_sw_check(SW4) && tx_latch_flag){
    	    // Start transmission
    		//xil_printf("\n\rTransmitting %d bytes.. \n\r", bytes_to_tx);
    	    fifo_tx_strt_len(bytes_to_tx);
    	    bytes_to_tx = 0;
    	    tx_latch_flag = 0;
    	}

    	if(gpio_sw_check(SW7)){
    		ppm_set_sw_relay();
    		ppm_drone_sync();
    	}

    	// Print transmit vacancy
    	if(gpio_btn_check(BTN_UP)){
    	    tx_vacancy = fifo_get_tx_vacancy();
    	    xil_printf("tx vacancy : %d\n\r", tx_vacancy);
    	    sleep(2);
    	}
    	// Print receive occupancy
    	if(gpio_btn_check(BTN_DOWN)){
    	    rv_occupancy = fifo_get_rv_occupancy();
    	    xil_printf("rv occupancy : %d\n\r", rv_occupancy);
    	    sleep(2);
    	}


    	/*
        while(gpio_btn_check(BTN_LEFT)){
        	int period = *((int *) (AXI_PPM_BASE + PPM_GEN_PERIOD_OFFS));
        	period += 1;
        	*((int *) (AXI_PPM_BASE + PPM_GEN_PERIOD_OFFS)) = period;
    		sleep(0.3);
    		xil_printf("Period - %d\n\r", *((int *) (AXI_PPM_BASE + PPM_GEN_PERIOD_OFFS)));
        }

        while(gpio_btn_check(BTN_RIGHT)){
        	int period = *((int *) (AXI_PPM_BASE + PPM_GEN_PERIOD_OFFS));
        	period -= 1;
        	*((int *) (AXI_PPM_BASE + PPM_GEN_PERIOD_OFFS)) = period;
    		sleep(0.3);
    		xil_printf("Period - %d\n\r", *((int *) (AXI_PPM_BASE + PPM_GEN_PERIOD_OFFS)));
        }
        */

        while(gpio_btn_check(BTN_RIGHT)){
    		//ppm_get_ch_counts(ch_counts_in);
    		//ppm_print_ch_counts(ch_counts_in);
        	ppm_print_all();
    		sleep(2);
        }


    }

    xil_printf("Exiting..\n\r");
    //minfft_free_aux(aux);
    cleanup_platform();
    return 0;
}


/*
// Load tone data to transmit FIFO
if(gpio_sw_check(SW0) && !tx_latch_flag){
	//xil_printf("Loading Data : %d bytes..\n\r", NUM_TX*BYTES_PER_TX);
	fifo_tx_wr_data(data, NUM_TX);
    bytes_to_tx +=  NUM_TX * BYTES_PER_TX;
    tx_latch_flag = 1;
}

if(gpio_sw_check(SW3)){
	xil_printf("Resetting Interrupts..\n\r");
	fifo_clr_ints(0xFFFFFFFF);
	sleep(2);
	xil_printf("Done\n\r");
}
*/

