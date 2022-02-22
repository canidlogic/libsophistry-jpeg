/*
 * jpeg_echo.c
 * ===========
 * 
 * Read a JPEG file from standard input and then re-encode it as a JPEG
 * file on standard output.
 * 
 * Note that metadata from the input JPEG file is NOT carried over to
 * the output file.  Also note that the JPEG image is fully re-encoded.
 * 
 * Syntax
 * ------
 * 
 *   jpeg_echo
 *   jpeg_echo [q]
 * 
 * The optional [q] parameter is an integer specifying the compression
 * quality to use for output.  This is in range 0-100, with higher
 * values meaning more image quality but less compression and lower
 * values meaning less image quality but more compression.  If not
 * specified, it defaults to 90.
 * 
 * Compilation
 * -----------
 * 
 * Compile with sophistry_jpeg.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sophistry_jpeg.h"

/* 
 * The default quality value if none is specified.
 */
#define DEFAULT_Q_VAL (90)

/*
 * Parse the given string as a signed integer.
 * 
 * pstr is the string to parse.
 * 
 * pv points to the integer value to use to return the parsed numeric
 * value if the function is successful.
 * 
 * In two's complement, this function will not successfully parse the
 * least negative value.
 * 
 * Parameters:
 * 
 *   pstr - the string to parse
 * 
 *   pv - pointer to the return numeric value
 * 
 * Return:
 * 
 *   non-zero if successful, zero if failure
 */
static int parseInt(const char *pstr, int32_t *pv) {
  
  int negflag = 0;
  int32_t result = 0;
  int status = 1;
  int32_t d = 0;
  
  /* Check parameters */
  if ((pstr == NULL) || (pv == NULL)) {
    abort();
  }
  
  /* If first character is a sign character, set negflag appropriately
   * and skip it */
  if (*pstr == '+') {
    negflag = 0;
    pstr++;
  } else if (*pstr == '-') {
    negflag = 1;
    pstr++;
  } else {
    negflag = 0;
  }
  
  /* Make sure we have at least one digit */
  if (*pstr == 0) {
    status = 0;
  }
  
  /* Parse all digits */
  if (status) {
    for( ; *pstr != 0; pstr++) {
    
      /* Make sure in range of digits */
      if ((*pstr < '0') || (*pstr > '9')) {
        status = 0;
      }
    
      /* Get numeric value of digit */
      if (status) {
        d = (int32_t) (*pstr - '0');
      }
      
      /* Multiply result by 10, watching for overflow */
      if (status) {
        if (result <= INT32_MAX / 10) {
          result = result * 10;
        } else {
          status = 0; /* overflow */
        }
      }
      
      /* Add in digit value, watching for overflow */
      if (status) {
        if (result <= INT32_MAX - d) {
          result = result + d;
        } else {
          status = 0; /* overflow */
        }
      }
    
      /* Leave loop if error */
      if (!status) {
        break;
      }
    }
  }
  
  /* Invert result if negative mode */
  if (status && negflag) {
    result = -(result);
  }
  
  /* Write result if successful */
  if (status) {
    *pv = result;
  }
  
  /* Return status */
  return status;
}

/*
 * Program entrypoint.
 */
int main(int argc, char *argv[]) {
  
  int status = 1;
  int i = 0;
  const char *pModule = NULL;
  int32_t qval = DEFAULT_Q_VAL;
  int32_t y = 0;
  int32_t ih = 0;
  
  SPH_JPEG_READER *pr = NULL;
  SPH_JPEG_WRITER *pw = NULL;
  uint8_t *pscan = NULL;
  
  /* Get the module name */
  if (argc > 0) {
    if (argv != NULL) {
      if (argv[0] != NULL) {
        pModule = argv[0];
      }
    }
  }
  if (pModule == NULL) {
    pModule = "jpeg_echo";
  }
  
  /* Check that parameters are present */
  if (argv == NULL) {
    abort();
  }
  for(i = 0; i < argc; i++) {
    if (argv[i] == NULL) {
      abort();
    }
  }
  
  /* Check that either no parameters or one parameter */
  if ((argc != 1) && (argc != 2)) {
    fprintf(stderr, "%s: Wrong number of parameters!\n", pModule);
    status = 0;
  }
  
  /* If a parameter is there, parse it as a quality value */
  if (status && (argc > 1)) {
    
    /* Parse quality value */
    if (!parseInt(argv[1], &qval)) {
      fprintf(stderr, "%s: Can't parse quality value!\n", pModule);
      status = 0;
    }
    
    /* Range-check quality value */
    if (status && ((qval < 0) || (qval > 100))) {
      fprintf(stderr, "%s: Quality value out of range!\n", pModule);
      status = 0;
    }
  }
  
  /* Establish a reader object on standard input */
  if (status) {
    pr = sph_jpeg_reader_new(stdin);
    i = sph_jpeg_reader_status(pr);
    if (i != SPH_JPEG_ERR_OK) {
      fprintf(stderr, "%s: %s!\n", pModule, sph_jpeg_errstr(i));
      status = 0;
    }
  }
  
  /* Establish a writer object on standard output */
  if (status) {
    pw = sph_jpeg_writer_new(
            stdout,
            sph_jpeg_reader_width(pr),
            sph_jpeg_reader_height(pr),
            sph_jpeg_reader_channels(pr),
            (int) qval);
  }
  
  /* Allocate scanline buffer */
  if (status) {
    pscan = (uint8_t *) calloc(
                          (size_t) sph_jpeg_reader_width(pr),
                          (size_t) sph_jpeg_reader_channels(pr));
    if (pscan == NULL) {
      abort();
    }
  }
  
  /* Transfer scanlines to output */
  if (status) {
    ih = sph_jpeg_reader_height(pr);
    for(y = 0; y < ih; y++) {
      
      /* Read a scanline */
      if (!sph_jpeg_reader_get(pr, pscan)) {
        status = 0;
        fprintf(stderr, "%s: %s!\n",
                  pModule,
                  sph_jpeg_errstr(sph_jpeg_reader_status(pr)));
      }
      
      /* Write a scanline */
      if (status) {
        sph_jpeg_writer_put(pw, pscan);
      }
      
      /* Leave loop if error */
      if (!status) {
        break;
      }
    }
  }

  /* Free reader and writer if allocated */
  sph_jpeg_reader_free(pr);
  sph_jpeg_writer_free(pw);
  pr = NULL;
  pw = NULL;

  /* Free buffer if allocated */
  if (pscan != NULL) {
    free(pscan);
    pscan = NULL;
  }

  /* Invert status and return */
  if (status) {
    status = 0;
  } else {
    status = 1;
  }
  return status;
}
