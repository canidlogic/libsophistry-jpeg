#ifndef SOPHISTRY_JPEG_H_INCLUDED
#define SOPHISTRY_JPEG_H_INCLUDED

/*
 * sophistry_jpeg.h
 * ================
 * 
 * The Sophistry JPEG library.
 * 
 * This library can read and write JPEG files using libjpeg.
 * 
 * Compilation
 * -----------
 * 
 * Requires libjpeg (or compatible).  Use -ljpeg to include the library.
 * 
 * Make sure the development package for libjpeg is installed.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Error status definitions.
 */
#define SPH_JPEG_ERR_OK   (0)   /* No error */
#define SPH_JPEG_ERR_LIBJ (1)   /* libjpeg internal error during load */
#define SPH_JPEG_ERR_IDIM (2)   /* Invalid image dimensions */
#define SPH_JPEG_ERR_CCNT (3)   /* Invalid color channel count */
#define SPH_JPEG_ERR_READ (4)   /* libjpeg read error */

/*
 * The maximum number of pixels for the width and height dimensions of
 * JPEG files that are read and written.
 */
#define SPH_JPEG_MAXDIM (INT32_C(32000))

/*
 * The minimum and maximum compression quality values.
 * 
 * Higher values indicate higher quality images but less compression,
 * while lower values indicate more compression but lower image quality.
 */
#define SPH_JPEG_MINQ (25)
#define SPH_JPEG_MAXQ (90)

/*
 * Structure prototype for SPH_JPEG_READER.
 * 
 * Definition given in implementation.
 */
struct SPH_JPEG_READER_TAG;
typedef struct SPH_JPEG_READER_TAG SPH_JPEG_READER;

/*
 * Structure prototype for SPH_JPEG_WRITER.
 * 
 * Definition given in implementation.
 */
struct SPH_JPEG_WRITER_TAG;
typedef struct SPH_JPEG_WRITER_TAG SPH_JPEG_WRITER;

/*
 * Return an error message for a given Sophistry JPEG error status code.
 * 
 * status is the status code.  If SPH_JPEG_ERR_OK is passed, the return
 * string is "No error".  If an unrecognized status is passed, the
 * return string is "Unknown error".
 * 
 * Error messages begin with a capital letter.  At the end of the
 * string, there is no punctuation and no line break.
 * 
 * The error message strings are statically defined and should NOT be
 * freed.
 * 
 * Parameters:
 * 
 *   status - the status code
 * 
 * Return:
 * 
 *   an error message
 */
const char *sph_jpeg_errstr(int status);

/*
 * Allocate a new JPEG writer object.
 * 
 * The returned object must eventually be freed by calling the function
 * sph_jpeg_writer_free().
 * 
 * pOut is the file to write the JPEG data to.  This file must be open
 * for binary writing or undefined behavior occurs.  The JPEG file will
 * be written sequentially starting at the current file pointer.
 * Undefined behavior occurs if the file pointer is modified externally
 * while the JPEG writer object is still allocated.
 * 
 * If the program ends or the JPEG writer object is freed before all the
 * scanlines have been written, only a partial JPEG file will have been
 * written to output.
 * 
 * Although the client must not touch the file handle while the writer
 * object is allocated, the client still remains the owner of the file
 * handle.  sph_jpeg_writer_free() does NOT close the file handle.
 * 
 * width and height are the width and height of the output image in
 * pixels.  Both dimensions must be at least one, and both dimensions
 * may not exceed SPH_JPEG_MAXDIM.
 * 
 * chcount is the number of color channels in the image to write.  This
 * must either be one (indicating a grayscale file) or three (indicating
 * an RGB file).
 * 
 * quality is the desired image quality.  Higher values indicate that
 * the image quality will be better but the compression will be lower.
 * Lower values indicate that the compression will be higher but the
 * image quality will be lower.  Passed values are automatically clamped
 * to the range [SPH_JPEG_MINQ, SPH_JPEG_MAXQ].
 * 
 * Errors cause faults.  Errors originating within libjpeg may print an
 * error message to stderr.
 * 
 * Parameters:
 * 
 *   pOut - the file handle to write the JPEG file to
 * 
 *   width - the width of the JPEG file in pixels
 * 
 *   height - the height of the JPEG file in pixels
 * 
 *   chcount - the number of color channels
 * 
 *   quality - the compression quality
 * 
 * Return:
 * 
 *   a new JPEG writer object
 */
SPH_JPEG_WRITER *sph_jpeg_writer_new(
    FILE    * pOut,
    int32_t   width,
    int32_t   height,
    int       chcount,
    int       quality);

/*
 * Release an allocated JPEG writer object.
 * 
 * pw is the writer object to release.  The function call is ignored if
 * pw is NULL.
 * 
 * This function does NOT close the file handle that was used to
 * initialize the JPEG writer object.  The client retains ownership of
 * the file handle.
 * 
 * If all scanlines have been written, then after this function call the
 * file pointer will be at the location just after the JPEG file that
 * was written.  If not all scanlines have been written, then only a
 * partial JPEG file will be present.
 * 
 * The client may safely use the file handle again after the writer
 * object has been released.
 * 
 * Errors cause faults.  Errors originating within libjpeg may print an
 * error message to stderr.
 * 
 * Parameters:
 * 
 *   pw - the JPEG writer object to free, or NULL
 */
void sph_jpeg_writer_free(SPH_JPEG_WRITER *pw);

/*
 * Write an image scanline row to the JPEG file.
 * 
 * pw is the JPEG writer object.  sph_jpeg_writer_put() must be called
 * once for each scanline row in the image, in top-down order.  If the
 * sph_jpeg_writer_free() function is called before all rows have been
 * written, the JPEG file will be incomplete and invalid.  If more than
 * (height) scanlines are written to a JPEG writer object, a fault
 * occurs.
 * 
 * pscan points to the scanline data to write.  Each pixel either has
 * one byte indicating a grayscale value (zero is black and 255 is
 * white), or three bytes in Red, Green, Blue order, depending on the
 * chcount parameter that was used to initialize the writer object.  The
 * pixels must be ordered from left to right.  The total number of
 * pixels in a scanline is given by the (width) parameter passed to the
 * initialization routine.
 * 
 * Errors cause faults.  Errors originating within libjpeg may print an
 * error message to stderr.
 * 
 * Parameters:
 * 
 *   pw - the JPEG writer object
 * 
 *   pscan - the scanline data to write
 */
void sph_jpeg_writer_put(SPH_JPEG_WRITER *pw, uint8_t *pscan);

/*
 * Allocate a new JPEG reader object.
 * 
 * The returned object must eventually be freed by calling the function
 * sph_jpeg_reader_free().
 * 
 * pIn is the file to read the JPEG data from.  This file must be open
 * for binary reading or undefined behavior occurs.  The JPEG file will
 * be read sequentially starting at the current file pointer.  Undefined
 * behavior occurs if the file pointer is modified externally while the
 * JPEG reader object is still allocated.
 * 
 * Although the client must not touch the file handle while the reader
 * object is allocated, the client still remains the owner of the file
 * handle.  sph_jpeg_reader_free() does NOT close the file handle.
 * 
 * If there is any problem reading the file, if the color format is not
 * supported, or if the image dimensions exceed the SPH_JPEG_MAXDIM
 * limit, then a new reader object is returned with an error status set
 * (see sph_jpeg_reader_status()).
 * 
 * Parameters:
 * 
 *   pIn - the file handle to read the JPEG file from
 * 
 * Return:
 * 
 *   a new JPEG reader object
 */
SPH_JPEG_READER *sph_jpeg_reader_new(FILE *pIn);

/*
 * Free an allocated JPEG reader object.
 * 
 * pr is the reader object to release.  The function call is ignored if
 * pr is NULL.
 * 
 * This function does NOT close the file handle that was used to
 * initialize the JPEG reader object.  The client retains ownership of
 * the file handle.
 * 
 * The client may safely use the file handle again after the reader 
 * object has been released.  If reading was complete and successful,
 * the file pointer is positioned immediately after the JPEG file that
 * was read.
 * 
 * Parameters:
 * 
 *   pr - the JPEG reader object to free, or NULL
 */
void sph_jpeg_reader_free(SPH_JPEG_READER *pr);

/*
 * Return the error status of a JPEG reader object.
 * 
 * pr is the reader object to query.  The return value is the status
 * code, which is one of the SPH_JPEG_ERR constants.  To get an error
 * message for an error status, use sph_jpeg_errstr().
 * 
 * If the reader object hasn't encountered an error yet, the return
 * status will be SPH_JPEG_ERR_OK.  Once an error status occurs, the
 * reader object no longer tries to read the JPEG file any further and
 * further calls to do so are ignored.  The error status therefore
 * remains once it is set.
 * 
 * Parameters:
 * 
 *   pr - the JPEG reader object
 * 
 * Return:
 * 
 *   the current status code 
 */
int sph_jpeg_reader_status(SPH_JPEG_READER *pr);

/*
 * Get the width in pixels of the JPEG file being read.
 * 
 * pr is the reader object to query.  The width is determined during the
 * allocation of the reader object, and it never changes for an object.
 * If the allocation routine was not able to read a valid width, this
 * function returns one.
 * 
 * The width returned is always in range [1, SPH_JPEG_MAXDIM].
 * 
 * Parameters:
 * 
 *   pr - the JPEG reader object
 * 
 * Return:
 * 
 *   the width in pixels
 */
int32_t sph_jpeg_reader_width(SPH_JPEG_READER *pr);

/*
 * Get the height in pixels of the JPEG file being read.
 * 
 * pr is the reader object to query.  The height is determined during
 * the allocation of the reader object, and it never changes for an
 * object.  If the allocation routine was not able to read a valid
 * height, this function returns one.
 * 
 * The height returned is always in range [1, SPH_JPEG_MAXDIM].
 * 
 * Parameters:
 * 
 *   pr - the JPEG reader object
 * 
 * Return:
 * 
 *   the height in pixels
 */
int32_t sph_jpeg_reader_height(SPH_JPEG_READER *pr);

/*
 * Get the number of color channels in the JPEG file being read.
 * 
 * pr is the reader object to query.  The color channels are determined
 * during the allocation of the reader object, and they never change for
 * an object.  If the allocation routine was not able to determine a
 * valid color channels value, this function returns one.
 * 
 * The return value is always either one or three.  A return value of
 * one means a grayscale image where zero is black and 255 is white.  A
 * return value of three means a Red, Green, Blue image (with channels
 * in that order).
 * 
 * Parameters:
 * 
 *   pr - the JPEG reader object
 * 
 * Return:
 * 
 *   the number of color channels
 */
int sph_jpeg_reader_channels(SPH_JPEG_READER *pr);

/*
 * Read a scanline from the JPEG file.
 * 
 * pr is the reader object.  sph_jpeg_reader_get() may be called at most
 * (height) times for a reader.  A fault occurs if it is called more
 * than that.  Scanlines are returned in top to bottom order.
 * 
 * pscan points to the scanline buffer to write the pixels into.  The
 * length of the buffer is at least (width * channels).  If the number
 * of channels is one, each pixel is a grayscale byte with zero meaning
 * black and 255 meaning white.  If the number of channels is three,
 * each pixel is three bytes, with the first byte being red, the next
 * being green, and the third being blue.  Pixels are ordered from left
 * to right within the scanline.
 * 
 * The function fails immediately if there is already an error status
 * present in the reader object.  If the function fails, the buffer will
 * be cleared to zero values.  sph_jpeg_reader_status() can be used to
 * check the error status code.
 * 
 * Parameters:
 * 
 *   pr - the JPEG reader object
 * 
 *   pscan - pointer to the scanline buffer
 * 
 * Return:
 * 
 *   non-zero if successful, zero if error
 */
int sph_jpeg_reader_get(SPH_JPEG_READER *pr, uint8_t *pscan);

#endif
