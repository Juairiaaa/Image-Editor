#ifndef IMAGE_H
#define IMAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#pragma pack(push, 1)

/*BMP file header*/
typedef struct {
    uint16_t type;
    uint32_t filesize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t pixelOffset;
} BMPFileHeader;

/*BMP info header*/
typedef struct {
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
} BMPInfoHeader;

/*BGR pixel*/
typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} Pixel;

#pragma pack(pop)

/*image*/
typedef struct {
    int width;
    int height;
    Pixel* data;
} Image;

/*function declarations*/
Image* create_image(int width, int height);
void free_image(Image* image);
Image* copy_image(Image* source);
Image* load_bmp(const char* filename);
int save_bmp(Image* image, const char* filename);

#endif