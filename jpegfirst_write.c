/*
 * jpegfirst_write.c
 * =================
 * 
 * Test program for writing a JPEG file using Sophistry JPEG.
 * 
 * Compile with sophistry_jpeg
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sophistry_jpeg.h"

#define IMAGE_WIDTH 320
#define IMAGE_HEIGHT 240
#define PIXEL_BYTES 3   /* bytes per pixel */

int main(int argc, char *argv[]) {
  
  int status = 1;
  FILE *fp = NULL;
  int i = 0;
  unsigned char *pRow = NULL;
  SPH_JPEG_WRITER *pw = NULL;
  
  /* Ignore arguments */
  (void) argc;
  (void) argv;
  
  /* Open output file */
  fp = fopen("test_out.jpeg", "wb");
  if (fp == NULL) {
    fprintf(stderr, "Can't open output file!\n");
    status = 0;
  }
  
  /* Allocate scanline buffer and clear it */
  if (status) {
    pRow = (unsigned char *) malloc(IMAGE_WIDTH * PIXEL_BYTES);
    if (pRow == NULL) {
      abort();
    }
    memset(pRow, 0, IMAGE_WIDTH * PIXEL_BYTES);
  }
  
  /* Write a pattern into the row */
  if (status) {
    for(i = 0; i < IMAGE_WIDTH; i++) {
      pRow[(i * PIXEL_BYTES)] = (unsigned char) (i % 256);
      pRow[(i * PIXEL_BYTES) + 1] = (unsigned char) (i % 256);
      pRow[(i * PIXEL_BYTES) + 2] = (unsigned char) (i % 256);
    }
  }
  
  /* Get a writer object */
  if (status) {
    pw = sph_jpeg_writer_new(
          fp,
          IMAGE_WIDTH,
          IMAGE_HEIGHT,
          PIXEL_BYTES,
          90);
  }
  
  /* Write each scanline */
  if (status) {
    for(i = 0; i < IMAGE_HEIGHT; i++) {
      sph_jpeg_writer_put(pw, pRow);
    }
  }
  
  /* Free writer object if allocated */
  sph_jpeg_writer_free(pw);
  pw = NULL;
  
  /* Close file if open */
  if (fp != NULL) {
    fclose(fp);
    fp = NULL;
  }
  
  /* Free scanline buffer if allocated */
  if (pRow != NULL) {
    free(pRow);
    pRow = NULL;
  }
  
  /* Invert status and return */
  if (status) {
    status = 0;
  } else {
    status = 1;
  }
  return status;
}
