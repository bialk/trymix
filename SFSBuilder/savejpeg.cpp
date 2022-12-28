#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>    

#ifndef WIN32
extern "C" {
#include <jpeglib.h>
}
#endif
//#include "eventlog.h"

extern "C" int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    return 0;
}

void savejpeg(const char *filename, const unsigned char *image_buf, 
	      long image_width, long image_height){
#ifndef WIN32
  /* Points to large array of R,G,B-order data */

  JSAMPLE *image_buffer = (JSAMPLE *)image_buf;  
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  FILE *outfile;
  if ((outfile = fopen(filename, "wb")) == NULL) {
    err_printf(("can't open %s\n", filename));
    return;
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = image_width;      /* image width and height, in pixels */
  cinfo.image_height = image_height;
  cinfo.input_components = 3;           /* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB;       /* colorspace of input image */
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality (&cinfo, 100, true); // the value 75 is default value
  jpeg_start_compress(&cinfo, TRUE);

  /* pointer to rows */
  JSAMPROW *row_pointer= (JSAMPROW *)alloca(image_height*sizeof(JSAMPROW));
  long row_stride = image_width * 3;
  long i;
  for(i=0;i<cinfo.image_height;i++){
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    // inverse order of rows in image array expected 
    long inverse_order = cinfo.image_height-i-1;
    row_pointer[i] = & image_buffer[inverse_order * row_stride];
  }
  jpeg_write_scanlines(&cinfo, row_pointer, cinfo.image_height);
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
#endif
}

