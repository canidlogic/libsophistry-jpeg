/*
 * jpegshrink.c
 * ============
 * 
 * Implementation of jpegshrink.h
 * 
 * See the header for further information.
 */
#include "jpegshrink.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Local functions
 * ===============
 */

/* Prototypes */
static void jpegshrink_avgblit(
    const uint16_t * pAcc,
          uint8_t  * pOutScan,
          int32_t    out_samples,
          int        sval);

static void jpegshrink_mixscan(
    const uint8_t  * pInScan,
          uint16_t * pAcc,
          int32_t    out_width,
          int        sval,
          int        chcount);

static void jpegshrink_padscan(
    uint8_t * pInScan,
    int32_t   in_width,
    int32_t   pad_count,
    int       chcount);

/*
 * Transfer the accumulator into the output scanline buffer by averaging
 * each accumulator sample.
 * 
 * pAcc points to the accumulator.
 * 
 * pOutScan points to the output scanline.
 * 
 * out_samples is the total number of *samples* (not pixels!) to
 * transfer from accumulator to output buffer.  It must be in range
 * [1, SPH_JPEG_MAXDIM * 3].
 * 
 * sval is the scaling value, in range [1, JPEGSHRINK_MAXSHRINK].
 * 
 * Each sample in the accumulator is divided by (sval * sval) and then
 * clamped to range [0, 255] before being written to the output scaline.
 * The divisor of (sval * sval) is the number of pixels in the svalxsval
 * input pixel that maps to a single output pixel, so as to compute an
 * average value.
 * 
 * Parameters:
 * 
 *   pAcc - the accumulator
 * 
 *   pOutScan - the output scanline buffer
 * 
 *   out_samples - the number of samples to transfer from the
 *   accumulator to the output scanline buffer
 * 
 *   sval - the scaling value
 */
static void jpegshrink_avgblit(
    const uint16_t * pAcc,
          uint8_t  * pOutScan,
          int32_t    out_samples,
          int        sval) {
  
  int32_t div_val = 0;
  int32_t i = 0;
  int32_t sv = 0;
  
  /* Check parameters */
  if ((pAcc == NULL) || (pOutScan == NULL)) {
    abort();
  }
  if ((out_samples < 1) || (out_samples > SPH_JPEG_MAXDIM * 3)) {
    abort();
  }
  if ((sval < 1) || (sval > JPEGSHRINK_MAXSHRINK)) {
    abort();
  }
  
  /* Compute the divisor value */
  div_val = ((int32_t) sval) * ((int32_t) sval);
  
  /* Perform the transfer */
  for(i = 0; i < out_samples; i++) {
  
    /* Compute averaged sample value */
    sv = ((int32_t) pAcc[i]) / div_val;
    
    /* Clamp value */
    if (sv < 0) {
      sv = 0;
    } else if (sv > 255) {
      sv = 255;
    }
    
    /* Transfer to output buffer */
    pOutScan[i] = (uint8_t) sv;
  }
}

/*
 * Mix a (padded) input scanline into the accumulator.
 * 
 * pInScan points to the (padded) input scanline.
 * 
 * pAcc points to the accumulator.
 * 
 * out_width is the width in pixels of the accumulator.  pAcc must have
 * (out_width) pixels, and pInScan must have (out_width * sval) pixels.
 * out_width must be in range [1, SPH_JPEG_MAXDIM].
 * 
 * sval is the scaling value to use.  It must be in the range
 * [1, JPEGSHRINK_MAXSHRINK].
 * 
 * chcount is the number of channels per pixel.  It must be either one
 * or three.
 * 
 * Each run of (sval) pixels in the input scanline is mixed into a
 * single output pixel in the accumulator.  Mixing is done by adding
 * input sample values to the values already in the accumulator.
 * Undefined behavior occurs if the accumulator overflows.
 * 
 * Parameters:
 * 
 *   pInScan - the padded input scanline
 * 
 *   pAcc - the accumulator
 * 
 *   out_width - the output width in pixels
 * 
 *   sval - the scaling value
 * 
 *   chcount - the number of color channels
 */
static void jpegshrink_mixscan(
    const uint8_t  * pInScan,
          uint16_t * pAcc,
          int32_t    out_width,
          int        sval,
          int        chcount) {
  
  int32_t x = 0;
  int32_t pad_width = 0;
  
  /* Check parameters */
  if ((pInScan == NULL) || (pAcc == NULL)) {
    abort();
  }
  if ((out_width < 1) || (out_width > SPH_JPEG_MAXDIM)) {
    abort();
  }
  if ((sval < 1) || (sval > JPEGSHRINK_MAXSHRINK)) {
    abort();
  }
  if ((chcount != 1) && (chcount != 3)) {
    abort();
  }
  
  /* Iterate over all padded input pixels */
  pad_width = out_width * ((int32_t) sval);
  for(x = 0; x < pad_width; x++) {
    
    /* Add current input pixel into accumulator */
    if (chcount == 3) {
      /* RGB accumulation */
      pAcc[0] = pAcc[0] + ((uint16_t) pInScan[0]);
      pAcc[1] = pAcc[1] + ((uint16_t) pInScan[1]);
      pAcc[2] = pAcc[2] + ((uint16_t) pInScan[2]);
      
    } else if (chcount == 1) {
      /* Grayscale accumulation */
      *pAcc = *pAcc + ((uint16_t) *pInScan);
      
    } else {
      /* shouldn't happen */
      abort();
    }
    
    /* Update pointers if not at last pixel */
    if (x < pad_width - 1) {
      /* Advance the input pointer */
      pInScan += chcount;
      
      /* If sval is less than two, advance the accumulator pointer;
       * otherwise, advance accumulator pointer if (x % sval) is
       * (sval - 1) */
      if (sval < 2) {
        pAcc += chcount;
      } else if ((x % sval) >= sval - 1) {
        pAcc += chcount;
      }
    }
  }
}

/*
 * Pad a scanline appropriately by duplicating the last pixel.
 * 
 * pInScan points to the scanline to pad.
 * 
 * in_width is the unpadded input width of the scanline in pixels.  It
 * must be in range [1, SPH_JPEG_MAXDIM].
 * 
 * pad_count is the number of padding pixels to add to the end.  This
 * may be zero, in which case nothing is done.  The valid range is
 * [0, JPEGSHRINK_MAXSHRINK].
 * 
 * chcount is the number of channels per pixel.  This must be either one
 * or three.
 * 
 * Parameters:
 * 
 *   pInScan - the scanline
 * 
 *   in_width - the input width
 * 
 *   pad_count - the number of padding pixels
 * 
 *   chcount - the number of channels
 */
static void jpegshrink_padscan(
    uint8_t * pInScan,
    int32_t   in_width,
    int32_t   pad_count,
    int       chcount) {
  
  int32_t i = 0;
  uint8_t *plast = NULL;
  
  /* Check parameters */
  if (pInScan == NULL) {
    abort();
  }
  if ((in_width < 1) || (in_width > SPH_JPEG_MAXDIM)) {
    abort();
  }
  if ((pad_count < 0) || (pad_count > JPEGSHRINK_MAXSHRINK)) {
    abort();
  }
  if ((chcount != 1) && (chcount != 3)) {
    abort();
  }
  
  /* Only proceed if non-zero padding */
  if (pad_count > 0) {
  
    /* Get a pointer to the last input pixel in scanline */
    plast = pInScan + ((in_width - 1) * ((int32_t) chcount));
    
    /* Duplicate last pixel pad_count times */
    for(i = 1; i <= pad_count; i++) {
      if (chcount == 3) {
        /* RGB duplication */
        plast[(i * 3)    ] = plast[0];
        plast[(i * 3) + 1] = plast[1];
        plast[(i * 3) + 2] = plast[2];
        
      } else if (chcount == 1) {
        /* Grayscale duplication */
        plast[i] = plast[0];
        
      } else {
        /* shouldn't happen */
        abort();
      }
    }
  }
}

/*
 * Public function implementations
 * ===============================
 * 
 * See the header for specifications.
 */

/*
 * jpegshrink function.
 */
int jpegshrink(
          FILE              * pIn,
          FILE              * pOut,
          int                 sval,
          int                 q,
    const JPEGSHRINK_BOUNDS * pBounds) {
  
  int status = 1;
  int retval = SPH_JPEG_ERR_OK;
  
  int32_t in_width = 0;
  int32_t in_height = 0;
  int chcount = 0;
  int32_t out_width = 0;
  int32_t out_height = 0;
  int32_t out_samples = 0;
  int32_t pad_height = 0;
  int32_t pad_count = 0;
  
  int32_t long_dim = 0;
  int32_t short_dim = 0;
  int64_t pix_count = 0;
  
  int32_t y = 0;
  int32_t i = 0;
  
  SPH_JPEG_READER *pr = NULL;
  SPH_JPEG_WRITER *pw = NULL;
  
  uint8_t *pInScan = NULL;
  uint16_t *pAcc = NULL;
  uint8_t *pOutScan = NULL;
  
  /* Check parameters */
  if ((pIn == NULL) || (pOut == NULL)) {
    abort();
  }
  if ((sval < 1) || (sval > JPEGSHRINK_MAXSHRINK)) {
    abort();
  }

  /* Open the input file */
  pr = sph_jpeg_reader_new(pIn);
  if (sph_jpeg_reader_status(pr) != SPH_JPEG_ERR_OK) {
    status = 0;
  }
  
  /* Get the width, height, and channel count */
  if (status) {
    in_width = sph_jpeg_reader_width(pr);
    in_height = sph_jpeg_reader_height(pr);
    chcount = sph_jpeg_reader_channels(pr);
  }
  
  /* Determine output width and height */
  if (status && (sval <= 1)) {
    /* No scaling, so output dimensions equal to input */
    out_width = in_width;
    out_height = in_height;
    
  } else if (status) {
    /* Scaling, so first divide by scaling value */
    out_width = in_width / ((int32_t) sval);
    out_height = in_height / ((int32_t) sval);
    
    /* Next, if input padding required, increment corresponding
     * dimensions */
    if ((in_width % sval) != 0) {
      out_width++;
    }
    if ((in_height % sval) != 0) {
      out_height++;
    }
  }
  
  /* If there are constraints, check them */
  if (status && (pBounds != NULL)) {
    
    /* Figure out long dimension and short dimension */
    if (out_height > out_width) {
      long_dim = out_height;
      short_dim = out_width;
    } else {
      long_dim = out_width;
      short_dim = out_height;
    }
    
    /* Figure out total pixel count */
    pix_count = ((int64_t) out_width) * ((int64_t) out_height);
    
    /* Check constraints */
    if (pBounds->max_long >= 0) {
      if (long_dim > pBounds->max_long) {
        status = 0;
      }
    }
    if (pBounds->max_short >= 0) {
      if (short_dim > pBounds->max_short) {
        status = 0;
      }
    }
    if (pBounds->max_width >= 0) {
      if (out_width > pBounds->max_width) {
        status = 0;
      }
    }
    if (pBounds->max_height >= 0) {
      if (out_height > pBounds->max_height) {
        status = 0;
      }
    }
    if (pBounds->max_pixels >= 0) {
      if (pix_count > pBounds->max_pixels) {
        status = 0;
      }
    }
    
    /* If failure, set retval to -1 */
    if (!status) {
      retval = -1;
    }
  }
  
  /* Open the output file */
  if (status) {
    pw = sph_jpeg_writer_new(pOut, out_width, out_height, chcount, q);
  }
  
  /* Allocate input scanline buffer with necessary padding */
  if (status) {
    pInScan = (uint8_t *) calloc(
                            (size_t) (out_width * ((int32_t) sval)),
                            (size_t) chcount);
    if (pInScan == NULL) {
      abort();
    }
  }
  
  /* Transfer scanlines with appropriate scaling */
  if (status && (sval <= 1)) {
    /* No scaling required, so just copy from input to output */
    for(y = 0; y < in_height; y++) {
      
      /* Read a scanline */
      if (!sph_jpeg_reader_get(pr, pInScan)) {
        status = 0;
      }
      
      /* Write a scanline */
      if (status) {
        sph_jpeg_writer_put(pw, pInScan);
      }
      
      /* Leave loop if error */
      if (!status) {
        break;
      }
    }
    
  } else if (status) {
    /* Scaling required -- allocate an accumulator with 16-bit channels
     * and an output scanline buffer */
    pAcc = (uint16_t *) calloc(
                            (size_t) out_width,
                            ((size_t) chcount) * sizeof(uint16_t));
    if (pAcc == NULL) {
      abort();
    }
    
    pOutScan = (uint8_t *) calloc(
                            (size_t) out_width,
                            (size_t) chcount);
    if (pOutScan == NULL) {
      abort();
    }
    
    /* Padded height is the input height plus any necessary padding;
     * compute this by multiplying the output height by the scaling
     * factor */
    pad_height = out_height * ((int32_t) sval);
    
    /* The amount of padding per scanline is the difference between the
     * padded input width and the actual input width; compute the padded
     * input width by multiplying the output width by the scaling
     * factor */
    pad_count = (out_width * ((int32_t) sval)) - in_width;
    
    /* The amount of samples per output scanline is the output width
     * multiplied by the channel count */
    out_samples = out_width * ((int32_t) chcount);
    
    /* Go through all scanlines in the padded input Y space */
    for(y = 0; y < pad_height; y++) {
      
      /* If we have not exceeded the input dimensions yet, read another
       * scanline from input to the input scanline buffer and end-pad
       * scanline by duplication as necessary; otherwise, leave input
       * scanline buffer as-is so as to duplicate the last scanline in
       * the image */
      if (y < in_height) {
        /* There is an actual input line -- read into input buffer */
        if (!sph_jpeg_reader_get(pr, pInScan)) {
          status = 0;
        }
        
        /* Pad scanline as necessary */
        if (status) {
          jpegshrink_padscan(pInScan, in_width, pad_count, chcount);
        }
      }
      
      /* If (y % divisor) is zero, zero out the accumulator */
      if (status && ((y % sval) == 0)) {
        for(i = 0; i < out_samples; i++) {
          pAcc[i] = (uint16_t) 0;
        }
      }
      
      /* Mix padded input scanline into accumulator */
      if (status) {
        jpegshrink_mixscan(pInScan, pAcc, out_width, sval, chcount);
      }
      
      /* If (y % divisor) is (divisor - 1), copy accumulator to output
       * buffer (averaging components) and output scanline */
      if (status && ((y % sval) >= (sval - 1))) {
        jpegshrink_avgblit(pAcc, pOutScan, out_samples, sval);
        sph_jpeg_writer_put(pw, pOutScan);
      }
      
      /* Leave loop if error */
      if (!status) {
        break;
      }
    }
  }
  
  /* If there is an error, get the return value as the error status from
   * the reader, unless the retval is set to -1; otherwise, leave the
   * return value set to OK or -1 */
  if (!status) {
    if (retval != -1) {
      retval = sph_jpeg_reader_status(pr);
    }
  }
  
  /* Free reader and writer if allocated */
  sph_jpeg_reader_free(pr);
  sph_jpeg_writer_free(pw);
  
  pr = NULL;
  pw = NULL;
  
  /* Free buffers if allocated */
  if (pInScan != NULL) {
    free(pInScan);
    pInScan = NULL;
  }
  if (pAcc != NULL) {
    free(pAcc);
    pAcc = NULL;
  }
  if (pOutScan != NULL) {
    free(pOutScan);
    pOutScan = NULL;
  }
  
  /* Return retval */
  return retval;
}
