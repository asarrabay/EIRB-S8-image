#ifndef __DOMAIN_HH__
#define __DOMAIN_HH__
/**
 *  @file  domain.h
 *  @brief header for domain.c that defines functions for 
 *         image domain tranforms
 */

#include <bcl.h>

/**
 * @brief  compute a bilinear interpolation
 * @param  x: the pixel x coordinate
 * @param  y: the pixel y coordinate
 * @param  ims: the source image for the interpolation
 * @param  c: the channel number for the interpolation
 * @return the interpolation value
 */
unsigned short 
bilinear_interpolation(float x, float y, pnm ims, int c);

#endif
