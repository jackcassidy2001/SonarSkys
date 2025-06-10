#ifndef FFTHELP_H
#define FFTHELP_H

#include "minfft.h"

float fft_get_freq_res(float sampling_rate, int num_samples);
float fft_get_magnitude(minfft_cmpl val);
void fft_get_N_magnitudes(minfft_cmpl *vals, int num_vals, float *magnitudes);
void fft_get_ch_peaks(float freq_res, float* magnitudes, float* pk_ch_freq, float* pk_ch_mag, int transform_len);
void fft_magnitude_sort(float freq_res, float *magnitudes, int num_samples, float *frequencies);
void fft_find_ch_peaks(float* frequencies, float* magnitudes, float* pk_ch_freq, float* pk_ch_mag, int num_samples);
void fft_print_float(float num, int fraction_precision);
void fft_gui_send_cfg();
void fft_gui_send_peaks(int num_peaks, float *magnitudes, float *frequencies);


#endif /* FFTHELP_H */
