#include "processing.h"

void grayscale(Image* image) {
    if(image==NULL || image->data==NULL) return;

    int totalPixels = image->width * image->height;
    for(int i=0; i<totalPixels; i++) {
        int gray = (int)((0.299 * image->data[i].red)+(0.587 * image->data[i].green) + (0.114 * image->data[i].blue));

        image->data[i].red = (uint8_t)gray;
        image->data[i].green = (uint8_t)gray;
        image->data[i].blue = (uint8_t)gray;
    }
}

void brightness(Image* image, int value) {
    if(image==NULL || image->data==NULL) return;

    int totalPixels = image->width * image->height;
    for(int i=0; i<totalPixels; i++) {
        int r = image->data[i].red + value;
        int g = image->data[i].green + value;
        int b = image->data[i].blue + value;

        if(r>255) r = 255;
        if(r<0)   r = 0;
        if(g>255) g = 255;
        if(g<0)   g = 0;
        if(b>255) b = 255;
        if(b<0)   b = 0;

        image->data[i].red = (uint8_t)r;
        image->data[i].green = (uint8_t)g;
        image->data[i].blue = (uint8_t)b;
    }
}

void invert(Image* image) {
    if(image==NULL || image->data==NULL) return;

    int totalPixels = image->width * image->height;
    for(int i=0; i<totalPixels; i++) {
        image->data[i].red = 255 - image->data[i].red;
        image->data[i].green = 255 - image->data[i].green;
        image->data[i].blue = 255 - image->data[i].blue;
    }
}

void horizontal_flip(Image* image) {
    if(image==NULL || image->data==NULL) return;

    for(int y=0; y<image->height; y++) {
        for(int x=0; x<(image->width/2); x++) {
            Pixel temp = image->data[y*image->width + x];
            image->data[y*image->width + x] = image->data[y*image->width + (image->width-1-x)];
            image->data[y*image->width + (image->width-1-x)] = temp;
        }
    }
}

void vertical_flip(Image* image) {
    if(image==NULL || image->data==NULL) return;

    for(int y=0; y<(image->height/2); y++) {
        for(int x=0; x<image->width; x++) {
            Pixel temp = image->data[y*image->width + x];
            image->data[y*image->width + x] = image->data[(image->height-1-y)*image->width + x];
            image->data[(image->height-1-y)*image->width + x] = temp;
        }
    }
}

Image* rotate_90(Image* image) {
    if(image==NULL || image->data==NULL) return NULL;

    Image* rotated = create_image(image->height, image->width);
    if(rotated==NULL) return NULL;

    for(int y=0; y<image->height; y++) {
        for(int x=0; x<image->width; x++) {
            int targetX = image->height-1-y;
            int targetY = x;

            rotated->data[targetY*rotated->width + targetX] =
                image->data[y*image->width + x];
        }
    }

    return rotated;
}

/*crop a rectangular region starting at (x,y)*/
Image* crop(Image* image, int x, int y, int cropWidth, int cropHeight) {
    if(image==NULL || image->data==NULL) return NULL;

    if(x<0 || y<0 || cropWidth<=0 || cropHeight<=0) {
        return NULL;
    }
    if(x + cropWidth > image->width || y + cropHeight > image->height) {
        return NULL;
    }

    Image* cropped = create_image(cropWidth, cropHeight);
    if(cropped==NULL) return NULL;

    for(int j=0; j<cropHeight; j++) {
        for (int i=0; i<cropWidth; i++) {
            cropped->data[j * cropWidth + i] = image->data[(y + j) * image->width + (x + i)];
        }
    }

    return cropped;
}

Image* blur(Image* image) {
    if(image==NULL || image->data==NULL) return NULL;

    Image* blurred = create_image(image->width, image->height);
    if(blurred==NULL) return NULL;

    for(int y=0; y<image->height; y++) {
        for(int x=0; x<image->width; x++) {
            int red=0, green=0, blue=0;
            int count=0;

            for(int j=-1; j<=1; j++) {
                for(int i=-1; i<=1; i++) {
                    int nx = x+i;
                    int ny = y+j;

                    if(nx>=0 && nx<image->width && ny>=0 && ny<image->height) {
                        Pixel p = image->data[ny*image->width + nx];
                        red += p.red;
                        green += p.green;
                        blue += p.blue;
                        count++;
                    }
                }
            }

            Pixel* outPixel = &blurred->data[y*image->width + x];
            outPixel->red = red/count;
            outPixel->green = green/count;
            outPixel->blue = blue/count;
        }
    }

    return blurred;
}

Image* sharpen(Image* image) {
    if(image==NULL || image->data==NULL) return NULL;

    Image* sharpened = create_image(image->width, image->height);
    if(sharpened==NULL) return NULL;

    int kernel[3][3] = {
        { 0, -1,  0},
        {-1,  5, -1},
        { 0, -1,  0}
    };

    for(int y=0; y<image->height; y++) {
        for(int x=0; x<image->width; x++) {
            if(x==0 || x==image->width-1 || y==0 || y==image->height-1) {
                sharpened->data[y*image->width + x] = image->data[y*image->width + x];
                continue;
            }

            int red=0, green=0, blue=0;

            for(int j=-1; j<=1; j++) {
                for(int i=-1; i<=1; i++) {
                    Pixel p = image->data[(y+j)*image->width + (x+i)];
                    int weight = kernel[j+1][i+1];

                    red += p.red * weight;
                    green += p.green * weight;
                    blue += p.blue * weight;
                }
            }

            if(red>255) red=255; 
            if(red<0) red=0;
            if(green>255) green=255; 
            if(green<0) green=0;
            if(blue>255) blue=255; 
            if(blue<0) blue=0;

            Pixel* outPixel = &sharpened->data[y*image->width + x];
            outPixel->red = (uint8_t)red;
            outPixel->green = (uint8_t)green;
            outPixel->blue = (uint8_t)blue;
        }
    }

    return sharpened;
}