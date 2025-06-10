#include "tone.h"
#include "fifo.h"
#include "math.h"
#include "xil_io.h"

void tone_generate(float frequency, int num_samples, int* buffer, int* buffer_index, float* phase, int buffer_length) {
    float increment = 2.0 * M_PI * frequency / SAMPLE_RATE;
    for (uint32_t i = 0; i < num_samples; i++) {
        buffer[*buffer_index] = (int)(0.5 * AMPLITUDE_24BIT * sin(*phase));
        *buffer_index = (*buffer_index + 1) % buffer_length; // Update buffer index

        *phase += increment;
        if (*phase >= 2.0 * M_PI){ // Wrap phase after reaching 2*pi
            *phase -= 2.0 * M_PI;
        }
    }
}

void tone_mixer(int (*buffers)[FIFO_SIZE], float* scales, int num_buffers, int* out_buffer, int num_samples, int* buffer_index, int buffer_length){
    // Iterate over each sample
    for (int i = 0; i < num_samples; i++) {
        // Initialize the mixed sample value
        float mixed_sample = 0.0;

        // Iterate over each buffer
        for (int j = 0; j < num_buffers; j++) {
            // Add scaled sample from each buffer
            mixed_sample += scales[j] * buffers[j][*buffer_index];
        }

        // Store the mixed sample in the output buffer
        out_buffer[*buffer_index] = (int)mixed_sample;

        // Update buffer index
        *buffer_index = (*buffer_index + 1) % buffer_length;
    }
}

float tone_map_ppm_freq(int ppm_cnt_min, int ppm_cnt_max, float freq_min, float freq_max, int ppm_cnt) {
    // Calculate the slope of the linear mapping
	//xil_printf("Count min %d, Count max %d, PPM cnt %d\n\r", ppm_cnt_min, ppm_cnt_max, ppm_cnt);
    float slope = (freq_max - freq_min) / (ppm_cnt_max - ppm_cnt_min);

    // Calculate the frequency using linear interpolation
    float frequency = freq_min + slope * (ppm_cnt - ppm_cnt_min);

    return frequency;
}

int tone_map_freq_ppm(int ppm_cnt_min, int ppm_cnt_max, float freq_min, float freq_max, float frequency) {
    // Calculate the slope of the linear mapping
    float slope = (ppm_cnt_max - ppm_cnt_min) / (freq_max - freq_min);

    // Calculate the ppm count using linear interpolation
    int ppm_cnt = ppm_cnt_min + slope * (frequency - freq_min);

    return ppm_cnt;
}


// Function to calculate the number of samples per period for a given frequency
int tone_period_samples(float frequency) {
    return (uint32_t)SAMPLE_RATE / frequency;
}

int tone_is_rising(int current_sample, int last_sample) {
    return current_sample > last_sample; // Rising
}
int tone_is_positive(int current_sample) {
    return current_sample > 0; // Positive
}

// Binary search to find the phase of a frequency that matches given conditions
float tone_find_phase(int current_sample, int last_sample, float desired_freq, int amplitude_thresh) {
    uint32_t period_samples = tone_period_samples(desired_freq);
    uint32_t quarter_period_samples = period_samples / 4;
    uint32_t is_rising = tone_is_rising(current_sample, last_sample);
    uint32_t is_positive = tone_is_positive(current_sample);
    double increment = 2.0 * M_PI * desired_freq / SAMPLE_RATE;

    // Determine the correct quarter of the period based on last_sample
    uint32_t quarter = 0;
    if (is_rising && is_positive)
        quarter = 0;
    else if (!is_rising && is_positive)
        quarter = 1;
    else if (!is_rising && !is_positive)
        quarter = 2;
    else
        quarter = 3;

    xil_printf("---------------------------------------------------------------- \n\r");
    xil_printf("Binary Search - Phase \n\r");
    xil_printf("Samples: %d, Quarter: %d, Amplitude Thresh : [%d, %d] \n\r", quarter_period_samples, quarter, current_sample - amplitude_thresh, current_sample + amplitude_thresh);
    xil_printf("---------------------------------------------------------------- \n\r\n\r");

    // Perform binary search within the determined quarter period
    uint32_t min_sample = quarter_period_samples * quarter;
    uint32_t max_sample = quarter_period_samples * (quarter + 1);

    while (min_sample < max_sample) {
        uint32_t mid_sample = (min_sample + max_sample) / 2;
        double mid_phase = mid_sample * increment;
        int32_t mid_amplitude = (int32_t)AMPLITUDE_24BIT * sin(mid_phase);

        // Print mid_phase and mid_amplitude for debugging
        xil_printf("mid_phase: %f, mid_amplitude: %d\n\r", mid_phase, mid_amplitude);

        // If conditions are met, return the phase
        if (mid_amplitude < (current_sample + amplitude_thresh) && mid_amplitude > (current_sample - amplitude_thresh))
            return mid_phase;
        else if (mid_amplitude < (current_sample + amplitude_thresh))
            min_sample = mid_sample + 1;
        else
            max_sample = mid_sample;
    }

    xil_printf("No phase found!\n\r");
    // If no suitable phase is found, return 0
    return 0.0;
}
