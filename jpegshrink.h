#ifndef JPEGSHRINK_H_INCLUDED
#define JPEGSHRINK_H_INCLUDED

/*
 * jpegshrink.h
 * ============
 * 
 * Self-contained module that performs an efficient reduction of JPEG
 * images.
 * 
 * Compilation
 * -----------
 * 
 * Compile with sophistry_jpeg.
 */

#include <stddef.h>
#include <stdio.h>
#include "sophistry_jpeg.h"

/*
 * The maximum shrink value.
 * 
 * The maximum value of sixteen means that the width and height of the
 * input image are both divided by sixteen, with duplication padding
 * used to round the input image up to 16-pixel boundaries.
 * 
 * JPEGSHRINK_MAXSHRINK must be chosen such that the function
 * jpegshrink_mixscan() in the implementation never encounters an
 * overflow during accumulation.  The value of 16 is the maximum
 * possible using unsigned 16-bit accumulator samples.
 */
#define JPEGSHRINK_MAXSHRINK (16)

/*
 * Perform a shrink operation.
 * 
 * pIn is the handle to the input JPEG file to shrink.  The handle must
 * be open for reading or undefined behavior occurs.  The JPEG image
 * will be read sequentially starting at the current file pointer.
 * After a successful operation, the file pointer will be positioned
 * immediately after the JPEG file that was just read.  After an error,
 * the state of the file pointer is undefined.
 * 
 * pOut is the handle to the output JPEG file to write.  The handle must
 * be open for writing or undefined behavior occurs.  The JPEG image
 * will be written sequentially starting at the current file pointer.
 * After a successful operation, the file pointer will be positioned
 * immediately after the JPEG file that was just written.  After an
 * error, the state of the file pointer is undefined.
 * 
 * If the input file is grayscale, the output file will be grayscale.
 * If the input file is RGB, the output file will be RGB.
 * 
 * sval is the scaling value.  The minimum value of one means that no
 * scaling is performed.  Otherwise, the width and height of the input
 * image are padded up to a multiple of the scaling value by duplicating
 * the last pixel of scanlines and the last scanline as necessary, and
 * then the width and the height of the input image are then divided by
 * the scaling value.  The maximum scaling value is
 * JPEGSHRINK_MAXSHRINK.
 * 
 * There is special code to perform an efficient copy in the special
 * case that sval is one.
 * 
 * q is the quality value of the output compressed JPEG file.  It is
 * passed through to sophistry_jpeg.  See sph_jpeg_writer_new() for the
 * meaning of this parameter.
 * 
 * The return value is a sophistry_jpeg status code.  It is zero (or
 * SPH_JPEG_ERR_OK) if the operation was successful, else it is a
 * sophistry_jpeg error code.  sph_jpeg_errstr() can be used to convert
 * the error code into a message for the user.
 * 
 * Parameters:
 * 
 *   pIn - the input JPEG file
 * 
 *   pOut - the output JPEG file
 * 
 *   sval - the scaling value
 * 
 *   q - the compression quality
 * 
 * Return:
 * 
 *   SPH_JPEG_ERR_OK (0) if successful, otherwise a sophistry_jpeg error
 *   code
 */
int jpegshrink(FILE *pIn, FILE *pOut, int sval, int q);

#endif
