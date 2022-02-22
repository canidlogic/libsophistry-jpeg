/*
 * sophistry_jpeg.c
 * ================
 * 
 * Implementation of sophistry_jpeg.h
 * 
 * See the header for further information.
 */

#include "sophistry_jpeg.h"
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "jpeglib.h"

/*
 * Type declarations
 * =================
 */

/*
 * SPH_JPEG_WRITER
 * 
 * See the header for prototype.
 */
struct SPH_JPEG_WRITER_TAG {
  
  /*
   * The compressor object.
   */
  struct jpeg_compress_struct cinfo;
  
  /*
   * The error object.
   */
  struct jpeg_error_mgr jerr;
  
  /*
   * The width of the output image in pixels.
   */
  int32_t width;
  
  /*
   * The height of the output image in pixels.
   */ 
  int32_t height;
  
  /*
   * The number of scanlines written to output.
   */
  int32_t written;
  
  /*
   * The number of channels.
   * 
   * One for grayscale, three for RGB.
   */
  int chcount;
};

/*
 * The custom error manager object.
 */
typedef struct {
  
  /*
   * The common error fields.
   * 
   * This must be the first member of the structure.
   */
  struct jpeg_error_mgr pub;
  
  /*
   * The jump buffer used for error returns.
   * 
   * This must be set at the start of implementation procedures.
   */
  jmp_buf setjmp_buffer;
  
} SPH_JPEG_ERRMAN;

/*
 * SPH_JPEG_READER
 * 
 * See the header for prototype.
 */
struct SPH_JPEG_READER_TAG {
  
  /*
   * The decompressor object.
   */
  struct jpeg_decompress_struct cinfo;
  
  /*
   * The custom error manager object.
   */
  SPH_JPEG_ERRMAN errman;
  
  /*
   * The width of the input image in pixels.
   */
  int32_t width;
  
  /*
   * The height of the input image in pixels.
   */ 
  int32_t height;
  
  /*
   * The number of scanlines read from input.
   */
  int32_t readcount;
  
  /*
   * The number of channels.
   * 
   * One for grayscale, three for RGB.
   */
  int chcount;
  
  /*
   * The error status.
   */
  int status;
};

/*
 * Local functions
 * ===============
 */

/* Prototypes */
METHODDEF(void) sph_jpeg_error_exit(j_common_ptr cinfo);

/*
 * The custom error handler for libjpeg.
 */
METHODDEF(void) sph_jpeg_error_exit(j_common_ptr cinfo) {
  
  /* Get pointer to our custom fields */
  SPH_JPEG_ERRMAN *per = (SPH_JPEG_ERRMAN *) cinfo;
  
  /* Jump back to error handler callback */
  longjmp(per->setjmp_buffer, 1);
}

/*
 * Public function implementations
 * ===============================
 * 
 * See the header for specifications.
 */

/*
 * sph_jpeg_errstr function.
 */
const char *sph_jpeg_errstr(int status) {
  
  const char *pResult = NULL;
  
  switch (status) {
    case SPH_JPEG_ERR_OK:
      pResult = "No error";
      break;
    
    case SPH_JPEG_ERR_LIBJ:
      pResult = "Error reading header of JPEG file";
      break;
    
    case SPH_JPEG_ERR_IDIM:
      pResult = "Image dimensions out of range";
      break;
    
    case SPH_JPEG_ERR_CCNT:
      pResult = "Invalid number of color channels";
      break;
    
    case SPH_JPEG_ERR_READ:
      pResult = "Error decoding JPEG file";
      break;
    
    default:
      pResult = "Unknown error";
  }
  
  return pResult;
}

/*
 * sph_jpeg_writer_new function.
 */
SPH_JPEG_WRITER *sph_jpeg_writer_new(
    FILE    * pOut,
    int32_t   width,
    int32_t   height,
    int       chcount,
    int       quality) {
  
  SPH_JPEG_WRITER *pw = NULL;
  
  /* Check parameters */
  if (pOut == NULL) {
    abort();
  }
  if ((width < 1) || (width > SPH_JPEG_MAXDIM) ||
      (height < 1) || (height > SPH_JPEG_MAXDIM)) {
    abort();
  }
  if ((chcount != 1) && (chcount != 3)) {
    abort();
  }
  
  /* Clamp quality value */
  if (quality < SPH_JPEG_MINQ) {
    quality = SPH_JPEG_MINQ;
  }
  if (quality > SPH_JPEG_MAXQ) {
    quality = SPH_JPEG_MAXQ;
  }
  
  /* Allocate new object */
  pw = (SPH_JPEG_WRITER *) malloc(sizeof(SPH_JPEG_WRITER));
  if (pw == NULL) {
    abort();
  }
  memset(pw, 0, sizeof(SPH_JPEG_WRITER));
  
  /* Initialize simple fields */
  pw->width = width;
  pw->height = height;
  pw->written = 0;
  pw->chcount = chcount;
  
  /* Initialize JPEG compressor */
  (pw->cinfo).err = jpeg_std_error(&(pw->jerr));
  jpeg_create_compress(&(pw->cinfo));
  
  /* Set output destination */
  jpeg_stdio_dest(&(pw->cinfo), pOut);
  
  /* Initialize JPEG information */
  (pw->cinfo).image_width = (int) width;
  (pw->cinfo).image_height = (int) height;
  
  if (chcount == 3) {
    (pw->cinfo).input_components = 3;
    (pw->cinfo).in_color_space = JCS_RGB;
  
  } else if (chcount == 1) {
    (pw->cinfo).input_components = 1;
    (pw->cinfo).in_color_space = JCS_GRAYSCALE;
  
  } else {
    /* Shouldn't happen */
    abort();
  }
    
  jpeg_set_defaults(&(pw->cinfo));
    
  /* Set quality on scale 0-100, avoid < 25 */
  jpeg_set_quality(&(pw->cinfo), quality, TRUE);
  
  /* Start compression */
  jpeg_start_compress(&(pw->cinfo), TRUE);
  
  /* Return writer object */
  return pw;
}

/*
 * sph_jpeg_writer_free function.
 */
void sph_jpeg_writer_free(SPH_JPEG_WRITER *pw) {
  
  /* Only proceed if non-NULL passed */
  if (pw != NULL) {
    
    /* Free JPEG object */
    jpeg_destroy_compress(&(pw->cinfo));
    
    /* Free structure */
    free(pw);
  }
}

/*
 * sph_jpeg_writer_put function.
 */
void sph_jpeg_writer_put(SPH_JPEG_WRITER *pw, uint8_t *pscan) {
  
  JSAMPROW row_pointer[1];
  
  /* Initialize arrays */
  memset(&(row_pointer[0]), 0, sizeof(JSAMPROW));
  
  /* Check parameters */
  if ((pw == NULL) || (pscan == NULL)) {
    abort();
  }
  
  /* Check state */
  if (pw->written >= pw->height) {
    abort();
  }
  
  /* Set row pointer to scanline buffer */
  row_pointer[0] = (JSAMPROW) pscan;
  
  /* Write the scanline */
  jpeg_write_scanlines(&(pw->cinfo), row_pointer, 1);
  
  /* Increment the written count */
  (pw->written)++;
  
  /* If we just wrote the last scanline, finish the image */
  if (pw->written >= pw->height) {
    jpeg_finish_compress(&(pw->cinfo));
  }
}

/*
 * sph_jpeg_reader_new function.
 */
SPH_JPEG_READER *sph_jpeg_reader_new(FILE *pIn) {
  
  int status = 1;
  SPH_JPEG_READER *pr = NULL;
  
  /* Check parameter */
  if (pIn == NULL) {
    abort();
  }
  
  /* Allocate a new reader object */
  pr = (SPH_JPEG_READER *) malloc(sizeof(SPH_JPEG_READER));
  if (pr == NULL) {
    abort();
  }
  memset(pr, 0, sizeof(SPH_JPEG_READER));
  
  /* Initialize the simple fields with default values */
  pr->width = 1;
  pr->height = 1;
  pr->readcount = 0;
  pr->chcount = 1;
  pr->status = SPH_JPEG_ERR_OK;
  
  /* Set up the error handler */
  (pr->cinfo).err = jpeg_std_error(&((pr->errman).pub));
  ((pr->errman).pub).error_exit = &sph_jpeg_error_exit;
  
  /* Establish the callback error handler */
  if (setjmp(((pr->errman).setjmp_buffer))) {
    /* This is run if libjpeg indicates an error */
    pr->width = 1;
    pr->height = 1;
    pr->chcount = 1;
    pr->status = SPH_JPEG_ERR_LIBJ;
    return pr;
  }
  
  /* Create the decompressor object */
  jpeg_create_decompress(&(pr->cinfo));
  
  /* Specify file handle to read from */
  jpeg_stdio_src(&(pr->cinfo), pIn);
  
  /* Read file parameters */
  (void) jpeg_read_header(&(pr->cinfo), TRUE);
  
  /* Start decompression */
  (void) jpeg_start_decompress(&(pr->cinfo));
  
  /* Read the image information */
  pr->width = (int32_t) (pr->cinfo).output_width;
  pr->height = (int32_t) (pr->cinfo).output_height;
  pr->chcount = (int) (pr->cinfo).output_components;
  
  /* Range-check information */
  if ((pr->width < 1) || (pr->width > SPH_JPEG_MAXDIM) ||
      (pr->height < 1) || (pr->height > SPH_JPEG_MAXDIM)) {
    status = 0;
    pr->status = SPH_JPEG_ERR_IDIM;
  }
  
  if (status && (pr->chcount != 1) && (pr->chcount != 3)) {
    status = 0;
    pr->status = SPH_JPEG_ERR_CCNT;
  }
  
  /* If there was an error, reset the fields */
  if (!status) {
    pr->width = 1;
    pr->height = 1;
    pr->chcount = 1;
  }
  
  /* Return the new reader object */
  return pr;
  /* CAUTION: alternate return statement earlier! */
}

/*
 * sph_jpeg_reader_free function.
 */
void sph_jpeg_reader_free(SPH_JPEG_READER *pr) {
  
  /* Only proceed if non-NULL passed */
  if (pr != NULL) {
    
    /* Establish the callback error handler */
    if (setjmp(((pr->errman).setjmp_buffer))) {
      /* This is run if libjpeg indicates an error */
      free(pr);
      return;
    }
    
    /* Release the decompressor object */
    jpeg_destroy_decompress(&(pr->cinfo));
    
    /* Release the structure */
    free(pr);
  }
  /* CAUTION: alternate return statement earlier! */
}

/*
 * sph_jpeg_reader_status function.
 */
int sph_jpeg_reader_status(SPH_JPEG_READER *pr) {
  
  if (pr == NULL) {
    abort();
  }
  return pr->status;
}

/*
 * sph_jpeg_reader_width function.
 */
int32_t sph_jpeg_reader_width(SPH_JPEG_READER *pr) {
  
  if (pr == NULL) {
    abort();
  }
  return pr->width;
}

/*
 * sph_jpeg_reader_height function.
 */
int32_t sph_jpeg_reader_height(SPH_JPEG_READER *pr) {
  
  if (pr == NULL) {
    abort();
  }
  return pr->height;
}

/*
 * sph_jpeg_reader_channels function.
 */
int sph_jpeg_reader_channels(SPH_JPEG_READER *pr) {
  
  if (pr == NULL) {
    abort();
  }
  return pr->chcount;
}

/*
 * sph_jpeg_reader_get function.
 */
int sph_jpeg_reader_get(SPH_JPEG_READER *pr, uint8_t *pscan) {
  
  int status = 1;
  JSAMPROW row_pointer[1];
  
  /* Initialize arrays */
  memset(&(row_pointer[0]), 0, sizeof(JSAMPROW));
  
  /* Check parameters */
  if ((pr == NULL) || (pscan == NULL)) {
    abort();
  }
  
  /* Check state */
  if (pr->readcount >= pr->height) {
    abort();
  }
  
  /* Update the read counter */
  (pr->readcount)++;
  
  /* Set row pointer to scanline buffer */
  row_pointer[0] = (JSAMPROW) pscan;
  
  /* Fail if already an error */
  if (pr->status != SPH_JPEG_ERR_OK) {
    status = 0;
  }
  
  /* Proceed only if not already in error state */
  if (status) {
    
    /* Establish the callback error handler */
    if (setjmp(((pr->errman).setjmp_buffer))) {
      /* This is run if libjpeg indicates an error */
      pr->status = SPH_JPEG_ERR_READ;
      memset(pscan, 0, (size_t) (pr->width * ((int32_t) pr->chcount)));
      return 0;
    }
    
    /* Read a scanline */
    (void) jpeg_read_scanlines(&(pr->cinfo), row_pointer, 1);
    
    /* If we just finished reading the last scanline, finish
     * decompression */
    if (pr->readcount >= pr->height) {
      (void) jpeg_finish_decompress(&(pr->cinfo));
    }
  }
  
  /* If there was an error, blank the buffer */
  if (!status) {
    memset(pscan, 0, (size_t) (pr->width * ((int32_t) pr->chcount)));
  }
  
  /* Return status */
  return status;
  /* CAUTION: alternate return statement earlier! */
}
