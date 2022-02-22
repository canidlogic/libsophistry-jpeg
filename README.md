# libsophistry-jpeg README

## 1. Introduction

libsophistry-jpeg is a simple wrapper around [libjpeg](https://ijg.org/) that supports encoding and decoding JPEG files.

This wrapper library is built on the 6B version release of libjpeg.  It should also be compatible with derivative libraries, such as [libjpeg-turbo](https://www.libjpeg-turbo.org/).  libsophistry-jpeg avoids using any special or alternate interfaces, and attempts to keep the interface with libjpeg as widely compatible as possible.

libsophistry-jpeg only supports grayscale and RGB images, and no support is guaranteed for anything beyond the most basic of JPEG features.  Reading and writing is scanline by scanline.  Reading of progressive JPEG files may or may not be supported, depending on whether or not the underlying JPEG library is able to automatically convert such images to allow for sequential scanline reading.

Also included is an optional extension library for libsophistry-jpeg that is able to perform quick reductions of JPEG images.  This library can be ignored if it is not required.

### 1.1 Compilation

In order to use libsophistry-jpeg in a program, it is first of all necessary to include libjpeg (or a compatible library) in the program compilation.  To do this on UNIX-like platforms, make sure that the development libraries for libjpeg are installed &mdash; it is __not__ sufficient to just have libjpeg without the development files.  Then, your best bet is to try `pkg-config` as follows:

    pkg-config --cflags --libs libjpeg

If `pkg-config` doesn't recognize libjpeg, check once again that the _development libraries_ of libjpeg (or compatible) are installed.  If `pkg-config` is not available, see below for something else to try.

In order to include libjpeg in the program compilation, the best way is to include the following in the compiler invocation:

    `pkg-config --cflags --libs libjpeg`

Note that the above invocation is surrounded by grave accent characters.  This will invoke `pkg-config` and get it to generate the appropriate compiler and linker options that need to be inserted.  If you have other `pkg-config` libraries, you can of course include them in the invocation above.

Depending on the compilation line, you may need to split up `pkg-config` into two separate invocations on the same line, the first one only with the `--cflags` option and library name(s) and the second only with the `--libs` option and library name(s).  Place the first invocation near the start of the C compiler options, and the second invocation near the end of the command line.

If `pkg-config` is not available, try including the following linker option:

    -ljpeg

If that doesn't work either, you'll need to figure out how to compile and link against libjpeg before using libsophistry-jpeg.  If you are compiling on Windows, you might also consider using [Cygwin](https://cygwin.com/) which will provide a way to get development packages for libjpeg as well as the `pkg-config` utility in a UNIX-like shell.  Otherwise, you will need to figure out how to get libjpeg to work with Windows build tools.

Once you figure out how to compile and link against libjpeg, all you need to do is include `sophistry_jpeg.h` and `sophistry_jpeg.c` in the project directory and compile `sophistry_jpeg.c` together with your program.  Provided that those two libsophistry-jpeg source files are present in the source directory of a project, compilation might look like this (all one line):

    gcc
      -o example
      `pkg-config --cflags libjpeg`
      example.c
      sophistry_jpeg.c
      `pkg-config --libs libjpeg`

To use the libsophistry-jpeg library functions within your program, just `#include` the `sophistry_jpeg.h` header file.  libsophistry-jpeg is a complete wrapper around libjpeg, so you do not need to directly interact with libjpeg in your program.

If you wish to use the `jpegshrink` extension library for libsophistry-jpeg, copy the `jpegshrink.c` and `jpegshrink.h` source files into your project directory, `#include` the `jpegshrink.h` header file in your program source file, and compile `jpegshrink.c` with your program _in addition to_ `sophistry_jpeg.c` and libjpeg.

You may wish to generate static library files for libsophistry-jpeg.  You can do this by first compiling libsophistry-jpeg (with optimizations) as follows:

    gcc -c -O2 sophistry_jpeg.c

You can then put the generated object file into a library, as follows:

    ar rcs libsophistry_jpeg.a sophistry_jpeg.o

(Note that we use an underscore in the library name rather than a hyphen.)

### 1.2 Sample programs

libsophistry-jpeg includes two sample programs that demonstrate the use of the library in practice.

`jpeg_echo.c` simply decodes a JPEG file from standard input and then encodes the JPEG file to standard output.  It optionally takes a single program argument, which is the JPEG compression quality in range 0-100, with higher values meaning more image quality but less compression (default 90).  One possible invocation for compiling this program is (all on one line):

    gcc
      -o jpeg_echo
      `pkg-config --cflags --libs libjpeg`
      jpeg_echo.c sophistry_jpeg.c

See &sect;1.1 "Compilation" for further information about compilation.  Note that the `jpeg_echo` program completely re-encodes the JPEG file, and it does _not_ carry over metadata from the original image file.

`jpeg_reduce.c` demonstrates the optional `jpegshrink` extension library.  It takes a required reduction value parameter in range [1, 16], where one means that no reduction is performed, and values greater than one mean that the width and height of the input image are each divided by that reduction value.  The program has an optional second parameter that is a JPEG quality value in range 0-100, with the same meanings as for the `jpeg_echo` program above.  One possible invocation for compiling this program is (all on one line):

    gcc
      -o jpeg_reduce
      `pkg-config --cflags --libs libjpeg`
      jpeg_reduce.c jpegshrink.c sophistry_jpeg.c

See &sect;1.1 "Compilation" for further information about compilation.

## 2. Error handling functions

libsophistry-jpeg simplifies the error handling system from the complex `longjmp` system that libjpeg uses.

All errors during write operations simply cause faults with the standard library `abort()` function.  This means that clients of the JPEG writing functions do not need to worry about error checking and error handling.

JPEG reader objects have a _status code_ that indicates whether they are OK or whether an error has been encountered while reading the JPEG file.  For reporting errors to the user, the following function is available:

    const char *
    sph_jpeg_errstr(
      int status
    );

This function takes a status code as input and returns an error message corresponding to that status code.  The error message is statically allocated, so the client should not try to free it.  The returned pointer remains valid throughout the whole process.

If an unknown status code is passed, `Unknown error` is returned.  If a status code corresponding to a non-error OK state is passed, `No error` is returned.  All returned messages begin with a capital letter but do not have any punctuation or line breaks at the end.  They are intended to be used as a `printf` string parameter like this:

    printf("Sophistry error: %s!\n", sph_jpeg_errstr(code));

All error messages returned by libsophistry-jpeg are in the English language.  If you require localization in another language, you use your own function for translating status codes into strings.  All the possible status codes are given as `SPH_JPEG_ERR` constants near the top of the `sophistry_jpeg.h` header.

The underlying libjpeg library may print its own error messages to standard error if something goes wrong.

## 3. JPEG reading functions

To read a JPEG file, the first step is to create a `SPH_JPEG_READER` object with the following function:

    SPH_JPEG_READER *
    sph_jpeg_reader_new(
      FILE * pIn
    );

The provided parameter is the `stdio` file handle to read the file from.  Reading is fully sequential starting at the current file position.  On platforms that distinguish between binary and text streams, the input file should be opened in binary mode.  The client remains the owner of the file handle (libsophistry-jpeg will _not_ close the file handle), but the client must not use the file handle while a JPEG reader is using it or undefined behavior occurs.

This allocation function always succeeds even if there was an error.  To check whether opening the JPEG file was actually successful, you can check the status of the JPEG reader using the following function:

    int
    sph_jpeg_reader_status(
      SPH_JPEG_READER * pr
    );

The return value is one of the `SPH_JPEG_ERR` constants.  If the JPEG reader is OK and no error has occurred, the special `SPH_JPEG_ERR_OK` status code will be returned.  For more about error handling and for a function to convert a status code into an error message, see &sect;2 "Error handling functions" for further information.

Once a JPEG reader object is open, you can query for basic information about the file.  The following three functions are available:

    int32_t
    sph_jpeg_reader_width(
      SPH_JPEG_READER * pr
    );

    int32_t
    sph_jpeg_reader_height(
      SPH_JPEG_READER * pr
    );

    int
    sph_jpeg_reader_channels(
      SPH_JPEG_READER * pr
    );

These function return the width in pixels, the height in pixels, and the color channel count, respectively.  The width and height are both at least one and both have a maximum value of `SPH_JPEG_MAXDIM`.  The color channel count is either one or three, with one meaning a grayscale image and three meaning an RGB image.

The width, height, and color channel count are always available for a JPEG reader object and never change for the object.  If there was an error opening the JPEG file, the width, height, and color channel count will all return valid but meaningless values.

libsophistry-jpeg allows the image to be read sequentially, scanline by scanline.  The first scanline that is read is the top scanline of the image, and the last scanline that is read is the bottom scanline of the image.  Within scanlines, pixels are stored sequentially in order from left to right.

For grayscale images, each pixel is an unsigned byte value where zero means black and 255 means white.  For RGB images, each pixel is three unsigned byte values, where the first is the red channel, the second is the green channel, and the third is the blue channel.

The following function is used to read a scanline from a JPEG reader object:

    int
    sph_jpeg_reader_get(
      SPH_JPEG_READER * pr,
      uint8_t         * pscan
    );

The maximum number of times this function may be called for a particular JPEG reader object is equal to the height of the image.  If the function succeeds, a non-zero value is returned.  If the function fails, zero is returned and `sph_jpeg_reader_status()` can be used to get more information about the failure.  The function fails immediately if it is called with a JPEG reader object that is already in an error status state.

The provided buffer `pscan` must be large enough to hold the entire scanline.  The required size in bytes is equal to the width of the image multiplied by the color channel count.

Once you are done reading a JPEG file, you should close the JPEG reader object using the following function:

    void
    sph_jpeg_reader_free(
      SPH_JPEG_READER * pr
    );

If you successfully read all scanlines of the JPEG image, the file position after closing the reader will be immediately after the JPEG file that was just read.  Note that since the client is the owner of the file handle, this function does __not__ close the file handle the JPEG reader object was reading from.

Note that since JPEG reader objects start reading from the current file position and they end after a successful read-through immediately after the JPEG file that was just read, it is possible to read through sequences of JPEG images in a single file, such as occurs with raw Motion-JPEG (M-JPEG) streams.

## 4. JPEG writing functions

To write a JPEG file, the first step is to create a `SPH_JPEG_WRITER` object using the following function:

    SPH_JPEG_WRITER *
    sph_jpeg_writer_new(
      FILE    * pOut,
      int32_t   width,
      int32_t   height,
      int       chcount,
      int       quality
    );

The `pOut` parameter is the `stdio` file handle to write the file to.  Writing is fully sequential starting at the current file position.  On platforms that distinguish between binary and text streams, the output file should be opened in binary mode.  The client remains the owner of the file handle (libsophistry-jpeg will _not_ close the file handle), but the client must not use the file handle while a JPEG writer is using it or undefined behavior occurs.

The `width` and `height` parameters give the width and height in pixels of the JPEG file to write.  Both must be at least one and may not exceed `SPH_JPEG_MAXDIM`.

The `chcount` parameter is the number of color channels, and it must be either one for grayscale or three for RGB.  ARGB images with an alpha channel are not possible.

The `quality` parameter is the JPEG encoding quality.  A value of zero means high compression but low image quality, while a value of 100 means low compression but high image quality.  A value of 90 is a good default value to use.  Any numeric value may be specified for this parameter &mdash; libsophistry-jpeg will automatically clamp the quality to the range [`SPH_JPEG_MINQ`, `SPH_JPEG_MAXQ`].

libsophistry-jpeg never returns errors when writing JPEG files.  Any kind of error will result in a fault by calling the standard `abort()` function.

Once a JPEG writer object is created, you must write each image scanline sequentially.  The top scanline of the image is written first, and the bottom scanline of the image is written last.  Within each scanline, pixels are ordered from left to right.  For grayscale images, each pixel is an unsigned byte value, where zero means black and 255 means white.  For RGB images, each pixel is three bytes, where the first byte is the red channel, the second byte is the green channel, and the third byte is the blue channel.

The following function is used to write a scanline to a JPEG writer object:

    void
    sph_jpeg_writer_put(
      SPH_JPEG_WRITER * pw,
      uint8_t         * pscan
    );

To properly write the whole JPEG file, you must call this function exactly once for each scanline in the image.

Once all the image scanlines have been written, use the following function to close the JPEG writer object:

    void
    sph_jpeg_writer_free(
      SPH_JPEG_WRITER * pw
    );

To write a complete JPEG file, you must write all the scanlines using the `sph_jpeg_writer_put()` function before calling this function; otherwise, only a partial JPEG file will be written.

This function does _not_ close the file handle it is writing to.  The file handle is owned by the client.  After writing a full JPEG file and closing the JPEG writer object, the file pointer will be positioned immediately after the JPEG file that was just written.  This allows a sequence of JPEG images to be written to a single file, as in a raw Motion-JPEG (M-JPEG) stream.

## 5. JPEG shrink library

If the optional `jpegshrink` library is included (see &sect;1.1 "Compilation"), then the following shrink function is available:

    int
    jpegshrink(
            FILE              * pIn,
            FILE              * pOut,
            int                 sval,
            int                 q,
      const JPEGSHRINK_BOUNDS * pBounds
    );

The input file `pIn` is read sequentially starting at the current file position and decoded as a JPEG file.  The output file `pOut` is written sequentially starting at the current file position.  For platforms that distinguish between binary and text files, both streams should be opened in binary mode.

The shrink algorithm works scanline by scanline without reading the whole image into memory, so it should be safe to use even on large images.

The parameter `sval` is the scaling value.  A value of one means no scaling, in which case scanlines are copied directly from input to output.  Note that the image will still be re-encoded in this case, and image metadata will not be transferred.  Scaling values greater than one are used to divide both the width and height of the input image.  For example, if the input image is 256 by 128 pixels and the scaling value is four, the output image will be 64 by 32.

If the dimensions of the input image are not evenly divisible by the scaling value, then the image is padded up to the next divisible boundary by duplicating pixel values at the ends of rows and columns.

The scaling value must be in range one up to and including `JPEGSHRINK_MAXSHRINK`.

The `q` value is the JPEG encoding quality to use for the output image.  The valid range is [0, 100] where zero is high compression but low image quality while 100 is low compression but high image quality.  90 is a good default value.

The return value is zero (`SPH_JPEG_ERR_OK`) if the operation is successful, -1 if the output constraints (see below) are not satisfied, and else a libsophistry-jpeg error status code that can be translated to an error message with `sph_jpeg_errstr()` as described in &sect;2 "Error handling functions".

The optional `pBounds` parameter may be set to `NULL` if it is not needed.  Otherwise, it must point to a structure of the following type:

    typedef struct {
    
      int32_t max_long;
    
      int32_t max_short;
    
      int32_t max_width;
    
      int32_t max_height;
    
      int32_t max_pixels;
    
    } JPEGSHRINK_BOUNDS;

Each field in this structure is a separate constraint.  Constraints that have the special value of `-1` mean that the constraint should be ignored.  Passing one of these structures where every field is set to `-1` is equivalent to passing `NULL` for the structure, since in both cases no constraints will be enforced.

When a constraint is specified with a value other than `-1` then it will be enforced.  Constraints are enforced after the input image is read but before the output image is written.  The dimensions of the output image are computed based on the dimensions of the input image and the given `sval` scaling value.  It is these computed dimensions that are then checked against the constraints.  If the constraints are not satisfied, the `jpegshrink()` function will fail with the special `-1` return value, indicating output constraints were not satisfied.

The `max_width` and `max_height` constraints apply to the computed width and height of the output image, respectively.  The `max_long` and `max_short` constraints apply to the computed dimension that is longer and the computed dimension that is shorter, respectively.  The `max_pixels` constraint applies to the computed width multiplied by the computed height.

A constraint is satisfied if the corresponding computed dimension or pixel count is less than or equal to the given constraint.

## 6. Further information

Further documentation is available in the `sophistry_jpeg.h` and `jpegshrink.h` header files.  You may also consult the source code of the included `jpeg_echo` and `jpeg_reduce` sample programs for examples of how to use this library in practice.
