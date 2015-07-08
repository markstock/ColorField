/*
 * colorfield.c
 *
 * (c) 2010-2011 Mark J. Stock
 *
 * 2010-11-15 MJS began project with copy of tracefield program
 * 2010-12-02 MJS modified to use HSV colors
 * 2011-01-26 MJS wrap hue around---don't bounce between [v0.1]
 *
 * A program to create a PNG image of bands of related colors
 *
 * gcc -o colorfield colorfield.c -lm -lpng -O3 -ffast-math -Wall
 * gcc -o colorfield colorfield.c -I/Developer/SDKs/MacOSX10.5.sdk/usr/X11/include -L/Developer/SDKs/MacOSX10.5.sdk/usr/X11/lib -lm -lpng -O3 -ffast-math -Wall
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <png.h>

#define TRUE 1
#define FALSE 0
#define FLOAT float

// not used yet, but should be!
enum colorspace {
  rgb, hsv, hsl, lab } colorspace = rgb;

// locally-defined subroutines
FLOAT** allocate_2d_array_F(int, int);
png_byte** allocate_2d_array_pb(int, int, int);
png_byte** allocate_2d_rgb_array_pb(int, int, int);
int write_png_image(int, FLOAT**, FLOAT**, FLOAT**, int, int, int);
int usage(char*, int);


/*
 *  main routine controls execution
 */
int main (int argc, char **argv) {

  int debug = FALSE;
  int i,y;
  int seed = 235125;
  int x_resolution = 128;
  int y_resolution = 512;
  int depth = 8;
  int use_color = TRUE;
  int use_rgb = TRUE;
  int basecolorset = FALSE;
  int num[3];
  int imaxrgb,iminrgb;
  FLOAT **red = NULL;
  FLOAT **grn = NULL;
  FLOAT **blu = NULL;
  FLOAT **seq = NULL;
  FLOAT f,n,m,v;
  FLOAT basecolor[3];
  FLOAT maxrgb,minrgb,chroma,hue,saturation,value;
  FLOAT scale[3];
  FLOAT variance[3];
  char progname[80];
  //char infilename[80];

  // default number string length (like a time constant)
  num[0] = 100;
  num[1] = 100;
  num[2] = 100;

  // default base color
  basecolor[0] = 0.5;
  basecolor[1] = 0.5;
  basecolor[2] = 0.5;

  // default color scale (larger more pronounced changes)
  scale[0] = 1.0;
  scale[1] = 1.0;
  scale[2] = 1.0;

  // parse command-line args
  (void) strcpy(progname,argv[0]);
  if (argc < 1) (void) usage(progname,0);
  for (i=1; i<argc; i++) {
    if (strncmp(argv[i], "-8", 2) == 0) {
      depth = 8;
    } else if (strncmp(argv[i], "-16", 3) == 0) {
      depth = 16;
    } else if (strncmp(argv[i], "-rgb", 4) == 0) {
      use_rgb = TRUE;
    } else if (strncmp(argv[i], "-hsv", 4) == 0) {
      use_rgb = FALSE;
    } else if (strncmp(argv[i], "-color", 3) == 0) {
      use_color = TRUE;
    } else if (strncmp(argv[i], "-grey", 3) == 0) {
      use_color = FALSE;
    } else if (strncmp(argv[i], "-x", 2) == 0) {
      x_resolution = atoi(argv[++i]);
    } else if (strncmp(argv[i], "-y", 2) == 0) {
      y_resolution = atoi(argv[++i]);
    } else if (strncmp(argv[i], "-seed", 3) == 0) {
      seed = atoi(argv[++i]);
    } else if (strncmp(argv[i], "-scale", 2) == 0) {
      scale[0] = atof(argv[++i]);
      scale[1] = atof(argv[++i]);
      scale[2] = atof(argv[++i]);
    } else if (strncmp(argv[i], "-base", 2) == 0) {
      basecolor[0] = atof(argv[++i]);
      basecolor[1] = atof(argv[++i]);
      basecolor[2] = atof(argv[++i]);
      basecolorset = TRUE;
    } else if (strncmp(argv[i], "-n", 2) == 0) {
      num[0] = atoi(argv[++i]);
      num[1] = atoi(argv[++i]);
      num[2] = atoi(argv[++i]);
    } else {
      // it's not an arg, so it must be the input file
      //(void) strcpy(infilename,argv[i]);
      // ahhh, no input files yet, just dump help
      fprintf(stderr,"Unknown option (%s)\n",argv[i]);
      (void) usage(progname,0);
    }
  }

  // allocate the array for the image
  red = allocate_2d_array_F(1,y_resolution);
  grn = allocate_2d_array_F(1,y_resolution);
  blu = allocate_2d_array_F(1,y_resolution);

  // seed the rng
  (void) srand((unsigned int)seed);

  // if there's no basecolor, set it
  if (!basecolorset) {
    if (use_rgb) {
      // pure gray---lame!
      basecolor[0] = 0.5;
      basecolor[1] = 0.5;
      basecolor[2] = 0.5;
    } else {
      // random hue, middle saturation and value
      basecolor[0] = 6.0 * (FLOAT)rand()/RAND_MAX;
      basecolor[1] = 0.5;
      basecolor[2] = 0.5;
    }
  }

  // convert basecolor to hsv if necessary (basecolor is always rgb)
  if (basecolorset && !use_rgb) {
    maxrgb = -1.0;
    imaxrgb = -1;
    minrgb = 2.0;
    iminrgb = -1;
    for (i=0; i<3; i++) {
      if (basecolor[i] > maxrgb) {
        maxrgb = basecolor[i];
        imaxrgb = i;
      }
      if (basecolor[i] < minrgb) {
        minrgb = basecolor[i];
        iminrgb = i;
      }
    }
    chroma = maxrgb - minrgb;
    hue = 0.0;
    if (chroma < 1.e-5) {
      // hue really is undefined in this case
    } else if (imaxrgb == 0) {
      hue = fmod( (basecolor[1]-basecolor[2])/chroma, 6.0);
    } else if (imaxrgb == 1) {
      hue = 2 + (basecolor[2]-basecolor[0]) / chroma;
    } else if (imaxrgb == 2) {
      hue = 4 + (basecolor[0]-basecolor[1]) / chroma;
    }
    value = maxrgb;
    if (chroma < 1.e-5) {
      saturation = 0.;
    } else {
      saturation = chroma/value;
    }
    basecolor[0] = hue;
    basecolor[1] = saturation;
    basecolor[2] = value;
    //fprintf(stderr,"base color in HSV is %g %g %g\n",basecolor[0],basecolor[1],basecolor[2]);
  }
  // set hue again to remind us
  hue = basecolor[0];

  // sequences are r,g,b or h,s,v
  for (i=0; i<3; i++) if (num[i] < 1) num[i] = 1;
  for (i=0; i<3; i++) if (num[i] > 1000000) num[i] = 1000000;

  // find a multiplier that proportional to 1/RMS
  for (i=0; i<3; i++) variance[i] = 1./sqrt((FLOAT)num[i]);

  // allocate array for random number sequence
  i = num[0];
  if (num[1] > i) i = num[1];
  if (num[2] > i) i = num[2];
  seq = allocate_2d_array_F(i,3);

  // sequences are r,g,b or h,s,v
  for (i=0; i<num[0]; i++) seq[i][0] = (FLOAT)rand()/RAND_MAX;
  for (i=0; i<num[1]; i++) seq[i][1] = (FLOAT)rand()/RAND_MAX;
  for (i=0; i<num[2]; i++) seq[i][2] = (FLOAT)rand()/RAND_MAX;

  // march through y-direction (rows) and set colors
  for (y=0; y<y_resolution; y++) {

    // update the number list
    seq[y%num[0]][0] = (FLOAT)rand()/RAND_MAX;
    seq[y%num[1]][1] = (FLOAT)rand()/RAND_MAX;
    seq[y%num[2]][2] = (FLOAT)rand()/RAND_MAX;

    // use sums of random numbers to select three values
    red[0][y] = 0.0;
    for (i=0; i<num[0]; i++) red[0][y] += seq[i][0];
    red[0][y] = (red[0][y]-0.5*num[0])*variance[0];

    grn[0][y] = 0.0;
    for (i=0; i<num[1]; i++) grn[0][y] += seq[i][1];
    grn[0][y] = (grn[0][y]-0.5*num[1])*variance[1];

    blu[0][y] = 0.0;
    for (i=0; i<num[2]; i++) blu[0][y] += seq[i][2];
    blu[0][y] = (blu[0][y]-0.5*num[2])*variance[2];
    // now, red,grn,blu are correlated Gaussian random numbers centered on 0

    // switch on rgb-vs-hsv
    if (use_rgb) {

      // scale then add to the baseline
      // these will be made range-bound when writing the image
      red[0][y] = basecolor[0] + scale[0]*red[0][y];
      grn[0][y] = basecolor[1] + scale[1]*grn[0][y];
      blu[0][y] = basecolor[2] + scale[2]*blu[0][y];

      // compress down to single channel
      if (!use_color) grn[0][y] = 0.3*red[0][y] + 0.6*grn[0][y] + 0.1*blu[0][y];

    } else { // 3 numbers represent hsv

      if (use_color) {
        // convert hsv into rgb

        // first, spread the numbers across a range (0-6 for hue, 0-1 for sv)
        hue += 0.1*scale[0]*red[0][y];
        saturation = basecolor[1] + scale[1]*grn[0][y];
        value = basecolor[2] + scale[2]*blu[0][y];
        //fprintf(stderr,"this color in HSV is %g %g %g\n",hue,saturation,value);

        // then convert hsv to rgb
        if (hue < 0.0) hue += 6.;
        if (hue > 6.0) hue -= 6.;
        i = ((int)hue) % 6;
        f = hue - (FLOAT)i;
        if ( !(i&1) ) f = 1 - f; // if i is even
        m = value * (1 - saturation);
        n = value * (1 - saturation * f);
        v = value;
        //if (fabs(hue)<0.1) fprintf(stderr,"%d %g %d %g\n",y,hue,i,f);
        //if (y>65 && y<75) fprintf(stderr,"%d %g %d %g\n",y,hue,i,f);

        switch (i) {
          case 6:
          case 0:
            red[0][y] = v;
            grn[0][y] = n;
            blu[0][y] = m;
            break;
          case 1:
            red[0][y] = n;
            grn[0][y] = v;
            blu[0][y] = m;
            break;
          case 2:
            red[0][y] = m;
            grn[0][y] = v;
            blu[0][y] = n;
            break;
          case 3:
            red[0][y] = m;
            grn[0][y] = n;
            blu[0][y] = v;
            break;
          case 4:
            red[0][y] = n;
            grn[0][y] = m;
            blu[0][y] = v;
            break;
          case 5:
            red[0][y] = v;
            grn[0][y] = m;
            blu[0][y] = n;
            break;
        }

      } else { // no color
        // just use value channel
        grn[0][y] = blu[0][y];
      }

    } // hsv

    // debug print
    if (debug) fprintf(stdout,"%g %g %g\n",red[0][y],grn[0][y],blu[0][y]);
  }

  // convert the single column into a PNG file
  if (!debug) write_png_image (use_color,red,grn,blu,x_resolution,y_resolution,depth);

  // quit, we must be done.
  exit(0);
}


/*
 * write a png file
 */
int write_png_image (int use_color, FLOAT **red, FLOAT **grn, FLOAT **blu,
       int xres, int yres, int depth) {

   static png_byte **image;
   static png_byte **imgrgb;
   png_uint_32 height,width;
   FILE *fp;
   png_structp png_ptr;
   png_infop info_ptr;
   int i,j;
   float thisGamma = 1.0;
   int printval,thisRed,thisGrn,thisBlu;

   height=yres;
   width=xres;

   // allocate the space for the image
   if (use_color) {
      imgrgb = allocate_2d_rgb_array_pb(xres,yres,depth);
   } else {
      image = allocate_2d_array_pb(xres,yres,depth);
   }

   // convert the double array into png_bytes!
   fprintf(stderr,"Writing PNG image\n"); fflush(stderr);
   // gamma-correct and check for peak value
   for (i=0; i<1; i++) {
      for (j=yres-1; j>-1; j--) {
         grn[i][j] = (FLOAT)exp((1./thisGamma)*log((double)grn[i][j]));
      }
   }
   if (use_color) {
      for (i=0; i<1; i++) {
         for (j=yres-1; j>-1; j--) {
            red[i][j] = (FLOAT)exp((1./thisGamma)*log((double)red[i][j]));
         }
      }
      for (i=0; i<1; i++) {
         for (j=yres-1; j>-1; j--) {
            blu[i][j] = (FLOAT)exp((1./thisGamma)*log((double)blu[i][j]));
         }
      }
   }

   // color image, read data from red array
   if (use_color) {

     if (depth == 16) {
       for (j=yres-1; j>-1; j--) {
         // red
         printval = (int)(red[0][j]*65536.0);
         if (printval<0) printval = 0;
         if (printval>65535) printval = 65535;
         thisRed = printval;
         // green
         printval = (int)(grn[0][j]*65536.0);
         if (printval<0) printval = 0;
         if (printval>65535) printval = 65535;
         thisGrn = printval;
         // blue
         printval = (int)(blu[0][j]*65536.0);
         if (printval<0) printval = 0;
         if (printval>65535) printval = 65535;
         thisBlu = printval;
         for (i=0; i<xres; i++) {
           imgrgb[yres-1-j][6*i] = (png_byte)(thisRed/256);
           imgrgb[yres-1-j][6*i+1] = (png_byte)(thisRed%256);
           imgrgb[yres-1-j][6*i+2] = (png_byte)(thisGrn/256);
           imgrgb[yres-1-j][6*i+3] = (png_byte)(thisGrn%256);
           imgrgb[yres-1-j][6*i+4] = (png_byte)(thisBlu/256);
           imgrgb[yres-1-j][6*i+5] = (png_byte)(thisBlu%256);
         }
       }
     } else { // depth is 8 (hopefully)
       for (j=yres-1; j>-1; j--) {
         // red
         printval = (int)(red[0][j]*256.0);
         if (printval<0) printval = 0;
         if (printval>255) printval = 255;
         thisRed = printval;
         // green
         printval = (int)(grn[0][j]*256.0);
         if (printval<0) printval = 0;
         if (printval>255) printval = 255;
         thisGrn = printval;
         // blue
         printval = (int)(blu[0][j]*256.0);
         if (printval<0) printval = 0;
         if (printval>255) printval = 255;
         thisBlu = printval;
         //fprintf(stderr,"%d %d %d\n",thisRed,thisGrn,thisBlu); fflush(stderr);
         for (i=0; i<xres; i++) {
           imgrgb[yres-1-j][3*i] = (png_byte)thisRed;
           imgrgb[yres-1-j][3*i+1] = (png_byte)thisGrn;
           imgrgb[yres-1-j][3*i+2] = (png_byte)thisBlu;
         }
       }
     }

   // monochrome image, read data from grn array
   } else {

     if (depth == 16) {
       for (j=yres-1; j>-1; j--) {
         printval = (int)(grn[0][j]*65536.0);
         if (printval<0) printval = 0;
         if (printval>65535) printval = 65535;
         for (i=0; i<xres; i++) {
           image[yres-1-j][2*i] = (png_byte)(printval/256);
           image[yres-1-j][2*i+1] = (png_byte)(printval%256);
         }
       }
     } else { // depth is 8 (hopefully)
       for (j=yres-1; j>-1; j--) {
         printval = (int)(grn[0][j]*256.0);
         if (printval<0) printval = 0;
         if (printval>255) printval = 255;
         for (i=0; i<xres; i++) {
           image[yres-1-j][i] = (png_byte)printval;
         }
       }
     }
   }

   /* open the file */
   // fp = fopen(file_name, "wb");
   fp = stdout;
   if (fp == NULL)
      return (-1);

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also check that
    * the library version is compatible with the one used at compile time,
    * in case we are using dynamically linked libraries.  REQUIRED.
    */
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);
      // user_error_ptr, user_error_fn, user_warning_fn);

   if (png_ptr == NULL)
   {
      //fclose(fp);
      fprintf(stderr,"Could not create png struct\n");
      fflush(stderr);
      exit(0);
      return (-1);
   }

   /* Allocate/initialize the image information data.  REQUIRED */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      //fclose(fp);
      png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
      return (-1);
   }

   /* Set error handling.  REQUIRED if you aren't supplying your own
    * error handling functions in the png_create_write_struct() call.
    */
   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* If we get here, we had a problem reading the file */
      //fclose(fp);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return (-1);
   }

   /* set up the output control if you are using standard C streams */
   png_init_io(png_ptr, fp);

   /* Set the image information here.  Width and height are up to 2^31,
    * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
    * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
    * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
    * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
    * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
    * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
    */
   //png_set_IHDR(png_ptr, info_ptr, height, width, depth, PNG_COLOR_TYPE_GRAY,
   //   PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
   if (use_color) {
      png_set_IHDR(png_ptr, info_ptr, width, height, depth,
         PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
         PNG_FILTER_TYPE_BASE);
   } else {
      png_set_IHDR(png_ptr, info_ptr, width, height, depth,
         PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
         PNG_FILTER_TYPE_BASE);
   }

   /* Optional gamma chunk is strongly suggested if you have any guess
    * as to the correct gamma of the image.
    */
   png_set_gAMA(png_ptr, info_ptr, thisGamma);

   /* Write the file header information.  REQUIRED */
   png_write_info(png_ptr, info_ptr);

   /* One of the following output methods is REQUIRED */
   // png_write_image(png_ptr, row_pointers);
   if (use_color) {
      png_write_image(png_ptr, imgrgb);
   } else {
      png_write_image(png_ptr, image);
   }

   /* It is REQUIRED to call this to finish writing the rest of the file */
   png_write_end(png_ptr, info_ptr);

   /* clean up after the write, and free any memory allocated */
   png_destroy_write_struct(&png_ptr, &info_ptr);

   /* that's it */
   return (0);
}


/*
 * allocate memory for a two-dimensional array of png_byte
 */
png_byte** allocate_2d_array_pb(int nx, int ny, int depth) {

   int i,bytesperpixel;
   png_byte **array;

   if (depth <= 8) bytesperpixel = 1;
   else bytesperpixel = 2;
   array = (png_byte **)malloc(ny * sizeof(png_byte *));
   array[0] = (png_byte *)malloc(bytesperpixel * nx * ny * sizeof(png_byte));

   for (i=1; i<ny; i++)
      array[i] = array[0] + i * bytesperpixel * nx;

   return(array);
}

png_byte** allocate_2d_rgb_array_pb(int nx, int ny, int depth) {

   int i,bytesperpixel;
   png_byte **array;

   if (depth <= 8) bytesperpixel = 3;
   else bytesperpixel = 6;
   array = (png_byte **)malloc(ny * sizeof(png_byte *));
   array[0] = (png_byte *)malloc(bytesperpixel * nx * ny * sizeof(png_byte));

   for (i=1; i<ny; i++)
      array[i] = array[0] + i * bytesperpixel * nx;

   return(array);
}


/*
 * allocate memory for a two-dimensional array of floats or doubles
 */
FLOAT** allocate_2d_array_F(int nx, int ny) {

   int i;
   FLOAT **array = (FLOAT **)malloc(nx * sizeof(FLOAT *));

   array[0] = (FLOAT *)malloc(nx * ny * sizeof(FLOAT));
   for (i=1; i<nx; i++)
      array[i] = array[0] + i * ny;

   return(array);
}


/*
 * This function writes basic usage information to stderr,
 * and then quits. Too bad.
 */
int usage (char progname[80],int status) {

   static char **cpp, *help_message[] =
   {
       "where [-options] are one or more of the following:                         ",
       "                                                                           ",
       "   -rgb        march through RGB colorspace (default)                      ",
       "                                                                           ",
       "   -hsv        march through HSV colorspace                                ",
       "                                                                           ",
       "   -x res      resolution in x dimension (just scales image), default=128  ",
       "                                                                           ",
       "   -y res      resolution in y dimension (number of colors), default=512   ",
       "                                                                           ",
       "   -color      write a color image (otherwise just a grey image is made)   ",
       "                                                                           ",
       "   -b r g b    baseline color, r, g, b from 0.0 to 1.0                     ",
       "                                                                           ",
       "   -n r g b    integer time constant, from 1 to 2 billion                  ",
       "                                                                           ",
       "   -s r g b    real variance, default is 1.0                               ",
       "                                                                           ",
       "   -seed val   integer random seed value                                   ",
       "                                                                           ",
       "   -8          output 8-bit png image                                      ",
       "                                                                           ",
       "   -16         output 16-bit png (default)                                 ",
       "                                                                           ",
       "   -help       returns this help information                               ",
       " ",
       "Output image is to stdout, so make sure to redirect to a PNG!",
       NULL
   };

   //fprintf(stderr, "usage:\n  %s [-options|infile] > out.png\n\n", progname);
   fprintf(stderr, "usage:\n  %s [-options] > out.png\n\n", progname);
   for (cpp = help_message; *cpp; cpp++)
      fprintf(stderr, "%s\n", *cpp);
      fflush(stderr);
   exit(status);
   return(0);
}
