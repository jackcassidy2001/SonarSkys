#include "fft_helper.h"
#include "math.h"
#include "stdlib.h"
#include "xil_printf.h"
#include "ppm.h"
#include <stdio.h>

extern float ch_min_freq[NUM_CH];
extern float ch_max_freq[NUM_CH];

float fft_get_freq_res(float sampling_rate, int num_samples) {
    // Calculate the frequency resolution
    return sampling_rate / num_samples;
}

float fft_get_magnitude(minfft_cmpl val) {
    return sqrt(val[0] * val[0] + val[1] * val[1]);
}

void fft_get_N_magnitudes(minfft_cmpl *vals, int num_vals, float *magnitudes) {
    for (int i = 0; i < num_vals; i++) {
        magnitudes[i] = fft_get_magnitude(vals[i]);
    }
    return;
}

void fft_get_ch_peaks(float freq_res, float* magnitudes, float* pk_ch_freq, float* pk_ch_mag, int transform_len) {
    int ch_sel = 0;
    int i = 0;

    // Allocate memory for the array to hold the maximum magnitudes of each channel
    float *cur_ch_max = (float *)calloc(NUM_CH, sizeof(float));

    // Find start index. Lower bound of lowest frequency range for channels
    while (i * freq_res < ch_min_freq[ch_sel]) {
        i++;
    }

    // Init pk freqs and magnitudes
    for (int j = 0; j < NUM_CH; j++) {
        pk_ch_freq[j] = 0.0;
        pk_ch_mag[j] = 0.0;
    }

    for (; i < transform_len; i++) {
        float cur_freq = i * freq_res;
       // fft_print_float(cur_freq, 3);
        //xil_printf("\n\r");
        // If not in ch frequency range
        if (!(cur_freq >= ch_min_freq[ch_sel] - freq_res && cur_freq <= ch_max_freq[ch_sel] + freq_res)) {
        	// No more channels, exit
            if(ch_sel == NUM_CH - 1){
            	break;
            }
        	// Look for when we enter the next ch range
        	if((cur_freq >= ch_min_freq[ch_sel + 1] - freq_res && cur_freq <= ch_max_freq[ch_sel + 1] + freq_res))
        		ch_sel++;
        }else{
			//xil_printf("ch _sel %d\n\r", ch_sel);
			// Find max magnitude for the frequency range and populate pks
			if (magnitudes[i] > cur_ch_max[ch_sel]) {
				cur_ch_max[ch_sel] = magnitudes[i];
				pk_ch_mag[ch_sel] = magnitudes[i];
				pk_ch_freq[ch_sel] = cur_freq;
			}
        }
    }

    // Free dynamically allocated memory
    free(cur_ch_max);
}




void fft_magnitude_sort(float freq_res, float *magnitudes, int num_samples, float *frequencies) {
    // Create an array to store the indices of sorted magnitudes
    int *sorted_indices = (int *)malloc(num_samples * sizeof(int));
    if (sorted_indices == NULL) {
        xil_printf("Memory allocation failed\n");
        return;
    }

    // Initialize the sorted indices array
    for (int i = 0; i < num_samples; i++) {
        sorted_indices[i] = i;
    }

    // Bubble sort the magnitudes and corresponding frequencies (indices)
    for (int i = 0; i < num_samples; i++) {
        for (int j = num_samples - 1; j > i; j--) {
            if (magnitudes[j] > magnitudes[j - 1]) {
                // Swap magnitudes
                float temp_mag = magnitudes[j];
                magnitudes[j] = magnitudes[j - 1];
                magnitudes[j - 1] = temp_mag;

                // Swap indices
                int temp_index = sorted_indices[j];
                sorted_indices[j] = sorted_indices[j - 1];
                sorted_indices[j - 1] = temp_index;
            }
        }
    }

    // Populate frequencies with the sorted magnitudes
    for (int i = 0; i < num_samples; i++) {
        frequencies[i] = sorted_indices[i] * freq_res;
    }

    // Free allocated memory
    free(sorted_indices);
}

void fft_find_ch_peaks(float* frequencies, float* magnitudes, float* pk_ch_freq, float* pk_ch_mag, int num_samples) {
    for (int ch = 0; ch < NUM_CH; ch++) {
        float min_freq = ch_min_freq[ch];
        float max_freq = ch_max_freq[ch];
        pk_ch_freq[ch] = 0.0;
        pk_ch_mag[ch] = 0.0;
        // Find the first frequency within the range for this channel
        for (int i = 0; i < num_samples; i++) {
            if (frequencies[i] >= min_freq && frequencies[i] <= max_freq) {
                pk_ch_freq[ch] = frequencies[i];
                pk_ch_mag[ch] = magnitudes[i];
                break; // Found the first frequency, exit the loop
            }
        }
    }
}

void fft_gui_send_cfg(){
	xil_printf("gui cfg\n\r");
	for(int i = 0 ; i < NUM_CH; i++){
		xil_printf("(");
		fft_print_float(ch_min_freq[i], 2);
		xil_printf(", ");
		fft_print_float(ch_max_freq[i], 2);
		xil_printf(")\n\r");
	}
	xil_printf("end\n\r");
}

void fft_gui_send_peaks(int num_peaks, float *magnitudes, float *frequencies){
	xil_printf("peaks\n\r");
	for(int i = 0 ; i < num_peaks; i++){
		xil_printf("(");
		fft_print_float(frequencies[i], 2);
		xil_printf(", ");
		char *buff;
		sprintf(buff, "%.1f", magnitudes[i]/1000000);
		xil_printf(buff);
		//fft_print_float(magnitudes[i], 2);
		xil_printf(")\n\r");
	}
	xil_printf("end\n\r");
}


void fft_print_float(float num, int fraction_precision) {
    int whole = (int)num;
    float fraction = num - whole;

    int multiplier = 1;
    for (int i = 0; i < fraction_precision; i++) {
        multiplier *= 10;
    }
    xil_printf("%d.%d", whole, (int)(fraction * multiplier));
}
