#include "image.h"

Image* create_image(int width, int height) {
    if(width<=0 || height<=0) {
        return NULL;
    }

    Image* image = (Image*)malloc(sizeof(Image));
    if(image==NULL) {
        return NULL;
    }

    image->width = width;
    image->height = height;
    image->data = (Pixel*)malloc(width*height*sizeof(Pixel));

    if(image->data==NULL) {
        free(image);
        return NULL;
    }

    return image;
}

void free_image(Image* image) {
    if(image!=NULL) {
        if(image->data != NULL) {
            free(image->data);
        }
        free(image);
    }
}

Image* copy_image(Image* source) {
    if(source==NULL || source->data==NULL) {
        return NULL;
    }

    Image* dest = create_image(source->width, source->height);
    if(dest==NULL) {
        return NULL;
    }

    int totalPixels = source->width * source->height;
    for(int i=0; i<totalPixels; i++) {
        dest->data[i] = source->data[i];
    }

    return dest;
}

Image* load_bmp(const char* filename) {
    FILE* in=fopen(filename, "rb");
    if(in==NULL) {
        return NULL;
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    if(fread(&fileHeader, sizeof(BMPFileHeader), 1, in) != 1) {
        fclose(in);
        return NULL;
    }
    if (fread(&infoHeader, sizeof(BMPInfoHeader), 1, in) != 1) { 
        fclose(in); 
        return NULL; 
    }

    if(fileHeader.type != 0x4D42) {
        fclose(in);
        return NULL;
    }
    if (infoHeader.bitsPerPixel != 24) { 
        fclose(in); 
        return NULL; 
    }
    if (infoHeader.compression != 0) { 
        fclose(in); 
        return NULL; 
    }

    int width = infoHeader.width;
    int height = infoHeader.height;

    if (width <= 0 || height == 0) { 
        fclose(in); 
        return NULL; 
    }

    /*negative height manipulations*/
    if(height<0) {
        height = -height;
    }

    Image* image = create_image(width, height);
    if(image==NULL) {
        fclose(in);
        return NULL;
    }

    if(fseek(in, fileHeader.pixelOffset, SEEK_SET) != 0) { 
        free_image(image); 
        fclose(in); 
        return NULL; 
    }

    int padding = (4-(width*3)%4)%4;

    for(int y=height-1; y>=0; y--) {
        for(int x=0; x<width; x++) {
            if(fread(&image->data[y * width + x], sizeof(Pixel), 1, in) != 1) { 
                free_image(image); 
                fclose(in); 
                return NULL; }
        }
        fseek(in, padding, SEEK_CUR);
    }

    fclose(in);
    return image;
}

int save_bmp(Image* image, const char* filename) {
    if(image==NULL || image->data==NULL) {
        return 0;
    }

    FILE* out=fopen(filename, "wb");
    if(out==NULL) {
        return 0;
    }

    int padding = (4-(image->width*3)%4)%4;

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    fileHeader.type = 0x4D42;
    fileHeader.filesize = 54+(image->width*3 + padding)*image->height;
    fileHeader.reserved1 = 0;
    fileHeader.reserved2 = 0;
    fileHeader.pixelOffset = 54;

    infoHeader.headerSize = 40;
    infoHeader.width = image->width;
    infoHeader.height = image->height;
    infoHeader.planes = 1;
    infoHeader.bitsPerPixel = 24;
    infoHeader.compression = 0;
    infoHeader.imageSize = (image->width*3 + padding)*image->height;
    infoHeader.xPixelsPerMeter = 0;
    infoHeader.yPixelsPerMeter = 0;
    infoHeader.colorsUsed = 0;
    infoHeader.colorsImportant = 0;

    fwrite(&fileHeader, sizeof(BMPFileHeader), 1, out);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, out);

    unsigned char zero=0;

    for(int y=image->height - 1; y>=0; y--) {
        for (int x=0; x<image->width; x++) {
            fwrite(&image->data[y * image->width + x], sizeof(Pixel), 1, out);
        }
        for (int p=0; p<padding; p++) {
            fwrite(&zero, sizeof(unsigned char), 1, out);
        }
    }

    fclose(out);
    return 1;
}