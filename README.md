# ColorField
Create a PNG image of bands of related colors


### Compile

You'll need the libpng development libraries. On Fedora/RHEL Linuxes, you can get them with:

    sudo yum install libpng-devel

Then, from the command-line on Linux, build colorfield with:

    gcc -o colorfield colorfield.c -lm -lpng -O3 -ffast-math -Wall

Or from the command line on OSX, use something like:

    gcc -o colorfield colorfield.c -I/Developer/SDKs/MacOSX10.5.sdk/usr/X11/include -L/Developer/SDKs/MacOSX10.5.sdk/usr/X11/lib -lm -lpng -O3 -ffast-math -Wall

### Usage

Finally, run the program with:

    ./colorfield -h

This will dump the help text, which is repeated here.

    usage:
        ./colorfield [-options] > out.png

    where [-options] are one or more of the following:                         
                                                                           
        -rgb        march through RGB colorspace (default)                      
                                                                           
        -hsv        march through HSV colorspace                                
                                                                           
        -x res      resolution in x dimension (just scales image), default=128  
                                                                           
        -y res      resolution in y dimension (number of colors), default=512   
                                                                           
        -color      write a color image (otherwise just a grey image is made)   
                                                                           
        -b r g b    baseline color, r, g, b from 0.0 to 1.0                     
                                                                           
        -n r g b    integer time constant, from 1 to 2 billion                  
                                                                           
        -s r g b    real variance, default is 1.0                               
                                                                           
        -seed val   integer random seed value                                   
                                                                           
        -8          output 8-bit png image                                      
                                                                           
        -16         output 16-bit png (default)                                 
                                                                           
        -help       returns this help information                               
 
    Output image is to stdout, so make sure to redirect to a PNG!

### License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

### Citing ColorField

I don't get paid for writing or maintaining this, so if you find this tool useful or mention it in your writing, please please cite it by using the following BibTeX entry.

```
@Misc{ColorField2023,
  author =       {Mark J.~Stock},
  title =        {ColorField:  Create a PNG image of bands of related colors},
  howpublished = {\url{https://github.com/markstock/ColorField}},
  year =         {2023}
}
```
