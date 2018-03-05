/**
 * @file fft.h
 * @brief header for fft module that implements functions for Fourier direct
 *        and inverse transforms and, amplitude and phase spectrum        
 */

#ifndef FFT_H
#define FFT_H

#include <complex.h>
#include <fftw3.h>
#include <bcl.h>

/**
 * @brief perform a discrete 2D Fourier transform from grayscale image
 * @param int rows: the input image height
 * @param int cols: the input image widht
 * @param unsigned short* gray_image: the grayscale input image
 * @return the Fourrier transform in frequency domain
 */
fftw_complex *
forward(int rows, int cols, unsigned short *gray_image);

/**
 * @brief perform a discrete inverse 2D Fourier transform from freqency domain
 * @param int rows: the input complex height
 * @param int cols: the input complex width
 * @param fft_complex* freq_repr: the frequency complex input 
 * @return the grayscale image from inverse Fourier transform
 */
unsigned short *
backward(int rows, int cols, fftw_complex *freq_repr);

/**
 * @brief compute amplitude and phase spectrum from frequency domain
 * @param int rows: the input complex height
 * @param int cols: the input complex width
 * @param fft_complex* freq_repr: the frequency complex input 
 * @param float* as: the amplitude spectrum output
 * @param float* ps: the phase spectrum output
 */
void 
freq2spectra(int rows, int cols, fftw_complex *freq_repr, float *as, float *ps);

/**
 * @brief compute the complex frequency from amplitude and phase spectrum
 * @param int rows: the input complex height
 * @param int cols: the input cemplex width
 * @param float* as: the amplitude spectrum input
 * @param float* ps: the phase spectrum input
 * @param fft_complex* freq_repr: the frequency complex output
 */
void 
spectra2freq(int rows, int cols, float* as, float* ps, fftw_complex* freq_repr);

#endif /* FFT_H */
