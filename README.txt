This program converts a 24-bit bmp image to black and white then to hexadecimal to display onto an LCD
To compile: make all
To test: make test
To choose your image and threshold of turning colors to black or white:
gcc main.c libbmp.c
.\a.exe -t 128 -i .\inputimage.bmp -o outputimage.bmp > output.h
-t for threshold from 0 to 255
> output.h the hexadecimal array of the converted image will be stored in image_data array of the output header file


