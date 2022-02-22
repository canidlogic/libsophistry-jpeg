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
 * Structure for declaring output dimension constraints.
 */
typedef struct {
  
  /*
   * The maximum "long" dimension value in pixels, or -1 if this is not
   * constrained.
   * 
   * The long dimension is the larger of the width and the height, or
   * the width if the width and height are equal.
   */
  int32_t max_long;
  
  /*
   * The maximum "short" dimension value in pixels, or -1 if this is not
   * constrained.
   * 
   * The short dimension is the smaller of the width and the height, or
   * the height if the width and height are equal.
   */
  int32_t max_short;
  
  /*
   * The maximum width in pixels, or -1 if this is not constrained.
   */
  int32_t max_width;
  
  /*
   * The maximum height in pixels, or -1 if this is not constrained.
   */
  int32_t max_height;
  
  /*
   * The maximum pixel count (width * height), or -1 if this is not
   * constrained.
   */
  int32_t max_pixels;
  
} JPEGSHRINK_BOUNDS;

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
 * pBounds points to an output image constraints structure, or is NULL
 * if there are no constraints.  Passing NULL is equivalent to passing
 * a bounds structure with all bounds set to -1.  After reading the
 * width and height of the input image, the dimensions of the output
 * image will be computed.  If these dimensions do not satisfy ALL of
 * the constraints that are present in the bounds structure, the
 * function will fail with an error code of -1.
 * 
 * The return value is a sophistry_jpeg status code, or -1 if the
 * function failed because the output constraints were not satisfied.
 * It is zero (or SPH_JPEG_ERR_OK) if the operation was successful, else
 * it is a sophistry_jpeg error code or -1.  sph_jpeg_errstr() can be
 * used to convert the error code (except for -1) into a message for the
 * user.  If pBounds is NULL, -1 will never be returned.
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
 *   pBounds - the output image constraints, or NULL
 * 
 * Return:
 * 
 *   SPH_JPEG_ERR_OK (0) if successful, otherwise a sophistry_jpeg error
 *   code, or -1 if the output constraints are not satisfied
 */
int jpegshrink(
          FILE              * pIn,
          FILE              * pOut,
          int                 sval,
          int                 q,
    const JPEGSHRINK_BOUNDS * pBounds);

#endif
