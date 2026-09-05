#ifndef PROCESSING_H
#define PROCESSING_H

#include "image.h"

/*function declarations*/
void grayscale(Image* image);
void brightness(Image* image, int value);
void invert(Image* image);
void horizontal_flip(Image* image);
void vertical_flip(Image* image);

Image* rotate_90(Image* image);
Image* crop(Image* image, int x, int y, int cropWidth, int cropHeight);
Image* blur(Image* image);
Image* sharpen(Image* image);

#endif