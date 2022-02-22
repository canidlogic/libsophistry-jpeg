/*
 * jpeg_reduce.c
 * =============
 * 
 * Read a JPEG file from standard input and then reduce it with
 * jpegshrink and write the result as a JPEG file on standard output.
 * 
 * Note that metadata from the input JPEG file is NOT carried over to
 * the output file.  Also note that the JPEG image is fully re-encoded.
 * 
 * Syntax
 * ------
 * 
 *   jpeg_reduce [rval]
 *   jpeg_reduce [rval] [q]
 * 
 * [rval] is the reduction value.  It must be in range [1, 16].  A value
 * of one means that no reduction is performed.  Values greater than one
 * mean that the width and height of the input image are each divided by
 * that reduction value to get the dimensions of the shrunk output
 * image.  The input image is padded at the end of each scanlines and
 * beyond the bottom of the image by duplication as necessary to get the
 * width and height up to a multiple of [rval].
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
 * Compile with sophistry_jpeg and jpegshrink.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sophistry_jpeg.h"
#include "jpegshrink.h"

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
  int retval = 0;
  const char *pModule = NULL;
  int32_t rval = 0;
  int32_t qval = DEFAULT_Q_VAL;
  
  /* Get the module name */
  if (argc > 0) {
    if (argv != NULL) {
      if (argv[0] != NULL) {
        pModule = argv[0];
      }
    }
  }
  if (pModule == NULL) {
    pModule = "jpeg_reduce";
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
  
  /* Check that either one extra parameter or two extra parameters */
  if ((argc != 2) && (argc != 3)) {
    fprintf(stderr, "%s: Wrong number of parameters!\n", pModule);
    status = 0;
  }
  
  /* Parse the reduction value parameter */
  if (status) {
    /* Parse value */
    if (!parseInt(argv[1], &rval)) {
      fprintf(stderr, "%s: Can't parse reduction value!\n", pModule);
      status = 0;
    }
    
    /* Range-check reduction value */
    if (status && ((rval < 1) || (rval > JPEGSHRINK_MAXSHRINK))) {
      fprintf(stderr, "%s: Reduction value out of range!\n", pModule);
      status = 0;
    }
  }
  
  /* If a second extra parameter is there, parse it as a quality
   * value */
  if (status && (argc > 2)) {
    
    /* Parse quality value */
    if (!parseInt(argv[2], &qval)) {
      fprintf(stderr, "%s: Can't parse quality value!\n", pModule);
      status = 0;
    }
    
    /* Range-check quality value */
    if (status && ((qval < 0) || (qval > 100))) {
      fprintf(stderr, "%s: Quality value out of range!\n", pModule);
      status = 0;
    }
  }
  
  /* Perform the shrink operation */
  if (status) {
    retval = jpegshrink(stdin, stdout, (int) rval, (int) qval);
    if (retval != SPH_JPEG_ERR_OK) {
      fprintf(stderr, "%s: %s!\n", pModule, sph_jpeg_errstr(retval));
      status = 0;
    }
  }

  /* Invert status and return */
  if (status) {
    status = 0;
  } else {
    status = 1;
  }
  return status;
}
