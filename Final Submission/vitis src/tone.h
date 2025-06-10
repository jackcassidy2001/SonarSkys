#ifndef TONE_H
#define TONE_H

#include "stdint.h"
#include "fifo.h"
#define SAMPLE_RATE 48000
#define AMPLITUDE_24BIT 16777215 // Maximum amplitude for a 24-bit DAC

void tone_generate(float frequency, int num_samples, int* buffer, int* buffer_index, float* phase, int buffer_length);
void tone_mixer(int (*buffers)[FIFO_SIZE], float* scales, int num_buffers, int* out_buffer, int num_samples, int* buffer_index, int buffer_length);
int tone_map_freq_ppm(int ppm_cnt_min, int ppm_cnt_max, float freq_min, float freq_max, float frequency);
float tone_map_ppm_freq(int ppm_cnt_min, int ppm_cnt_max, float freq_min, float freq_max, int ppm_cnt);
int tone_period_samples(float frequency);
int tone_is_rising(int current_sample, int last_sample);
int tone_is_positive(int current_sample);
float tone_find_phase(int current_sample, int last_sample, float desired_freq, int amplitude_thresh);

#endif /* TONE_H */
