#include <stdio.h>
#include "libbmp.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define WHITE 255
#define BLACK 0

#define COLOR_RGB(x) (x),(x),(x)

bool is_passing_threshold(bmp_pixel pixel, int threshold){

    int sum=pixel.red+pixel.green+pixel.blue;

    if (sum>=threshold*3) {
        return true;

    }
    return false;
}

bool parse_arg(unsigned int argc, char *argv[], char *inputFile, char *outFile, int *threshold)
{
    int opt;
    bool isIn = false ,isOut = false;
    while((opt = getopt(argc, argv, "t:i:o:h")) != -1)
    {
		switch(opt)
		{
		case 't':		
        	*threshold = atoi(optarg);
			if (*threshold > 255) {
				*threshold = 255;
			}
			break;
		case 'i':		
           	isIn = true;
           	strcpy(inputFile, optarg);
			break;
		case 'o':	
           	isOut = true;
           	strcpy(outFile, optarg);
			break;
		case 'h':
			printf("Example:%s.exe -t 128 -i inputimage.bmp -o outputimage.bmp > output.h\n", argv[0]);
			printf("-t to choose color threshold from 0 to 255\n");
			printf("-i choose input image to convert\n");
			printf("-o choose output image name\n");
			printf("> *.h to store the bitmap hexadecimal in header file where they can be input to an lcd to print\n");
			break;
		case ':':
			printf("option needs a value\n");
			break;
		case '?':
			printf("unknown option: %c\n write h for help", optopt);
			break;
		}
    }
	
    for(; optind < argc; optind++) {	
		printf("extra arguments: %s\n", argv[optind]);
    }
    return (isIn && isOut);
}

bool image_reader(bmp_img *img, char *file)
{
    enum bmp_error err = bmp_img_read (img, file);
    if (BMP_OK != err){
        printf("Error in reading image %d\r\n", err);
        return false;
    }
    return true;
}

uint8_t getByte(const bmp_img *img, int row, int col) 
{
	if (row % 8 != 0) {
		printf("Row Incorrect!\r\n");
		return 0;
	}
	uint8_t workingbyte = 0;
	int maxrow= row+8;
	if (img->img_header.biHeight<maxrow) {
		maxrow=img->img_header.biHeight;
	}

	for (int r=row;(r<maxrow);r++) {
		
		if (false == is_passing_threshold(img->img_pixels[r][col],127)) {
			
			workingbyte |= 1<<(r - row);
			
		}
		
	}
	return workingbyte;
} 

void print_page(const uint8_t *pageBytes, int cols, bool printNewLine)
{
	for (int i=0; i<cols; i++) {
		if (i>0 && i%16==0 && printNewLine) {
				printf("\r\n");
		}
		printf("0x%.2x ,", pageBytes[i]);	
	}
	printf("\r\n");
}

void get_page(const bmp_img *img, int page, int cols, uint8_t *pagebytes) 
{	
	int row = page * 8;
	for (int j=0;j<cols;j++) {
		pagebytes[j]=getByte(img, row, j);	
	}
}

void bmp_header_print(const bmp_img *img) 
{
	printf("************* Header *****************\r\n");
	printf("DataOffset: %d\r\n", img->img_header.bfOffBits);
	printf("FileSize: %d\r\n", img->img_header.bfSize);
	printf("BitCount: %d\r\n", img->img_header.biBitCount);
	printf("ColorsImportant: %d\r\n", img->img_header.biClrImportant);
	printf("ColorsUsed: %d\r\n", img->img_header.biClrUsed);
	printf("Compression: %d\r\n", img->img_header.biCompression);
	printf("Height: %d\r\n", img->img_header.biHeight);
	printf("Planes: %d\r\n", img->img_header.biPlanes);
	printf("Size: %d\r\n", img->img_header.biSize);
	printf("ImageSize: %d\r\n", img->img_header.biSizeImage);
	printf("Width: %d\r\n", img->img_header.biWidth);
	printf("XpixelsPerM: %d\r\n", img->img_header.biXPelsPerMeter);
	printf("YpixelsPerM: %d\r\n", img->img_header.biYPelsPerMeter);
	printf("************* End of Header *****************\r\n");
}

void headercmds1()
{
	printf("typedef struct uGFX_bitmap\n{\n\tconst uint8_t *data;\n\t");
	printf("enum uGFX_color color;\n\tuGFX_coord_t w;\n\tuGFX_coord_t h;\n}uGFX_bitmap;\r\n");
}

void headercmds2()
{
	printf("uGFX_bitmap image =\n{\n\t.data = image_data,\n\t");
	printf(".color = uGFX_PIXEL_SET;\n\t.w = WIDTH;\n\t.h = HEIGHT;\n};\r\n");
}

bool image_filter(bmp_img *in_img, bmp_img *out_img, int threshold )
{
	
    bmp_img_init_df (out_img, in_img->img_header.biWidth, in_img->img_header.biHeight);
	//Print to compare data between input and output image
	//bmp_header_print(in_img);
	//bmp_header_print(out_img);
	
	for (size_t y = 0, x; y < in_img->img_header.biHeight; y++)
	{
		for (x = 0; x < in_img->img_header.biWidth; x++)
		{
			if (is_passing_threshold(in_img->img_pixels[y][x], threshold))
			{
				bmp_pixel_init (&out_img->img_pixels[y][x], COLOR_RGB(WHITE));
			}
			else
			{
				bmp_pixel_init (&out_img->img_pixels[y][x], COLOR_RGB(BLACK));
			}
		}
		
	}
	//Header File Commands
	printf("#define HEIGHT = %d;\n", out_img->img_header.biHeight);
	printf("#define WIDTH = %d;\r\n", out_img->img_header.biWidth);
	headercmds1(); //Prints codes in header file
	return true;
}

struct string{
	char *str;
	int length;
};

bool convert_image(char *inputFile, char *outFile, int threshold)
{
	
	bmp_img myimage;
    image_reader(&myimage, inputFile);
	bmp_img saving_img;

	if (false == image_filter(&myimage, &saving_img, threshold)){
        printf("failed image_filter\r\n");
		bmp_img_free (&saving_img);
        return false;
    }
	int width = saving_img.img_header.biWidth;
	int pages = saving_img.img_header.biHeight / 8;
	if(saving_img.img_header.biHeight % 8 != 0) {
		pages++;
	}
	uint8_t bytes[width];
	printf("static char image_data[] = {\r\n");
	for(int i=0; i<pages; i++) {
		get_page(&saving_img, i, width, bytes);
		print_page(bytes, width, true);
	}
	printf("};\r\n");
	headercmds2();							//Prints codes in header file
	bmp_img_write (&saving_img, outFile);
	bmp_img_free (&saving_img);
    return true;
}

 int main(int argc, char *argv[]) 
{
	char inputFile[255];
    char outFile[255];
    int threshold = 127;
	
    if (false == parse_arg(argc, argv, inputFile, outFile, &threshold) ){
        printf("Wrong argument, try again.\n");
        return 1;
    }
	
    if (false == convert_image(inputFile, outFile, threshold)){
		printf("Failed image conversion\r\n");
        return 1;
    }
	
	bmp_img myimage;
	if (false == image_reader(&myimage, outFile)){
		printf("BW not reading, try again.\n");
        return 1;
    }
    return 0;
}
	
