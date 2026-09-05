#include <stdio.h>
#include <stdlib.h>
#include <iup.h>
#include <iupdraw.h>

#include "image.h"
#include "processing.h"

static Image* g_image = NULL;
static Image* g_undoImage = NULL;
static Ihandle* g_canvas = NULL;
static Ihandle* g_main_dialog = NULL;

static int ensure_image_loaded(void) {
    if(g_image == NULL || g_image->data == NULL) {
        IupMessage("Error", "No image loaded.");
        return 0;
    }
    return 1;
}

static void save_for_undo(void) {
    if(g_image != NULL) {
        if (g_undoImage != NULL) {
            free_image(g_undoImage);
        }
        g_undoImage = copy_image(g_image);
    }
}

static void refresh_display(void) {
    if(g_canvas != NULL) {
        IupUpdate(g_canvas);
    }
}

/*callback each time canvas redrawn*/
static int cb_action_canvas(Ihandle* self, float posx, float posy) {
    (void)posx;
    (void)posy;

    IupDrawBegin(self);

    int cw=0, ch=0;
    IupDrawGetSize(self, &cw, &ch);

    /*drawing the canvas background*/
    IupSetAttribute(self, "DRAWCOLOR", "230 230 230");
    IupSetAttribute(self, "DRAWSTYLE", "FILL");
    IupDrawRectangle(self, 0, 0, cw - 1, ch - 1);

    /*if no image is loaded */
    if (g_image == NULL || g_image->data == NULL) {
        IupSetAttribute(self, "DRAWCOLOR", "80 80 80");
        IupSetAttribute(self, "DRAWTEXTALIGNMENT", "ACENTER:ACENTER");

        const char* text = "No image loaded. Click 'Open BMP' to begin.";
        IupDrawText(self, text, -1, 0, 0, cw, ch);

        IupDrawEnd(self);
        return IUP_DEFAULT;
    }

    /*if image is loaded*/
    int img_w = g_image->width;
    int img_h = g_image->height;

    unsigned char* rgb_buf = (unsigned char*)malloc(img_w * img_h * 3);
    if(!rgb_buf) {
        IupDrawEnd(self);
        return IUP_DEFAULT;
    }

    for(int i=0; i < img_w * img_h; i++) {
        Pixel p = g_image->data[i];
        rgb_buf[i * 3 + 0] = p.red;
        rgb_buf[i * 3 + 1] = p.green;
        rgb_buf[i * 3 + 2] = p.blue;
    }

    Ihandle* img_handle = IupImageRGB(img_w, img_h, rgb_buf);
    free(rgb_buf);

    if(img_handle != NULL) {
        IupSetHandle("g_current_canvas_img", img_handle);

        /*scaling the image*/
        float scale_x = (float)cw/img_w;
        float scale_y = (float)ch/img_h;
        
        float scale;
        if(scale_x < scale_y) scale = scale_x;
        else { scale = scale_y; }
        if(scale > 1.0f) scale = 1.0f;

        int dest_w = (int)(img_w * scale);
        int dest_h = (int)(img_h * scale);
        int start_x = (cw - dest_w)/2;
        int start_y = (ch - dest_h)/2;

        IupDrawImage(self, "g_current_canvas_img", start_x, start_y, dest_w, dest_h);

        IupSetHandle("g_current_canvas_img", NULL);
        IupDestroy(img_handle);
    }

    IupDrawEnd(self);
    return IUP_DEFAULT;
}

/*callback the functions*/
static int cb_open(Ihandle* self) {
    (void)self;
    Ihandle* fileDlg = IupFileDlg();
    IupSetAttribute(fileDlg, "DIALOGTYPE", "OPEN");
    IupSetAttribute(fileDlg, "EXTFILTER", "BMP Files (*.bmp)|*.bmp|");
    IupPopup(fileDlg, IUP_CENTER, IUP_CENTER);

    if(IupGetInt(fileDlg, "STATUS") != -1) {
        char* path = IupGetAttribute(fileDlg, "VALUE");
        Image* newImg = load_bmp(path);

        if(newImg == NULL) {
            IupMessage("Error", "Invalid BMP file! Make sure it is a standard 24-bit uncompressed BMP.");
        }
        else {
            if(g_undoImage != NULL) {
                free_image(g_undoImage);
                g_undoImage = NULL;
            }
            if(g_image != NULL) {
                free_image(g_image);
            }

            g_image = newImg;
            refresh_display();
        }
    }
    IupDestroy(fileDlg);
    return IUP_DEFAULT;
}

static int cb_save(Ihandle* self) {
    (void)self;
    if(!ensure_image_loaded()) return IUP_DEFAULT;

    Ihandle* fileDlg = IupFileDlg();
    IupSetAttribute(fileDlg, "DIALOGTYPE", "SAVE");
    IupSetAttribute(fileDlg, "EXTFILTER", "BMP Files (*.bmp)|*.bmp|");
    IupPopup(fileDlg, IUP_CENTER, IUP_CENTER);

    if(IupGetInt(fileDlg, "STATUS") != -1) {
        char* path = IupGetAttribute(fileDlg, "VALUE");
        if(!save_bmp(g_image, path)) {
            IupMessage("Error", "Failed to save the image file.");
        }
        else {
            IupMessage("Success", "Image saved successfully!");
        }
    }
    IupDestroy(fileDlg);
    return IUP_DEFAULT;
}

static int cb_undo(Ihandle* self) {
    (void)self;
    if(g_undoImage == NULL) {
        IupMessage("Information", "Nothing to undo!");
        return IUP_DEFAULT;
    }

    Image* temp = g_image;
    g_image = g_undoImage;
    g_undoImage = temp;

    refresh_display();
    return IUP_DEFAULT;
}

static int cb_grayscale(Ihandle* self) {
    (void)self;
    if(!ensure_image_loaded()) return IUP_DEFAULT;

    save_for_undo();
    grayscale(g_image);
    refresh_display();
    return IUP_DEFAULT;
}

static int cb_brightness(Ihandle* self) {
    (void)self;
    if(!ensure_image_loaded()) return IUP_DEFAULT;

    int value=30;
    if(IupGetParam("Adjust Brightness", NULL, NULL, "Brightness Offset (-255 to 255): %i\n", &value, NULL)) {
        if(value<-255 || value>255) {
            IupMessage("Error", "Brightness must be between -255 and 255.");
            return IUP_DEFAULT;
        }
        save_for_undo();
        brightness(g_image, value);
        refresh_display();
    }
    return IUP_DEFAULT;
}

static int cb_invert(Ihandle* self) {
    (void)self;
    if(!ensure_image_loaded()) return IUP_DEFAULT;

    save_for_undo();
    invert(g_image);
    refresh_display();
    return IUP_DEFAULT;
}

static int cb_flip_h(Ihandle* self) {
    (void)self;
    if(!ensure_image_loaded()) return IUP_DEFAULT;

    save_for_undo();
    horizontal_flip(g_image);
    refresh_display();
    return IUP_DEFAULT;
}

static int cb_flip_v(Ihandle* self) {
    (void)self;
    if(!ensure_image_loaded()) return IUP_DEFAULT;

    save_for_undo();
    vertical_flip(g_image);
    refresh_display();
    return IUP_DEFAULT;
}

static int cb_rotate(Ihandle* self) {
    (void)self;
    if(!ensure_image_loaded()) return IUP_DEFAULT;

    save_for_undo();
    Image* rotated = rotate_90(g_image);
    if(rotated != NULL) {
        free_image(g_image);
        g_image = rotated;
        refresh_display();
    }
    return IUP_DEFAULT;
}

static int cb_crop(Ihandle* self) {
    (void)self;
    if (!ensure_image_loaded()) return IUP_DEFAULT;

    int cropX = 0;
    int cropY = 0;
    int cropW = g_image->width / 2;
    int cropH = g_image->height / 2;

    if(IupGetParam("Crop Options", NULL, NULL, "X Position: %i\nY Position: %i\nCrop Width: %i\nCrop Height: %i\n", 
        &cropX, &cropY, &cropW, &cropH, NULL)) {

        if(cropX < 0 || cropY < 0 || cropW <= 0 || cropH <= 0 ||
            cropX + cropW > g_image->width || cropY + cropH > g_image->height) {
            IupMessage("Error", "Invalid crop area! Must fit inside image boundary.");
            return IUP_DEFAULT;
        }

        save_for_undo();
        Image* cropped = crop(g_image, cropX, cropY, cropW, cropH);
        if(cropped != NULL) {
            free_image(g_image);
            g_image = cropped;
            refresh_display();
        }
    }
    return IUP_DEFAULT;
}

static int cb_blur(Ihandle* self) {
    (void)self;
    if(!ensure_image_loaded()) return IUP_DEFAULT;

    save_for_undo();
    Image* blurred = blur(g_image);
    if(blurred != NULL) {
        free_image(g_image);
        g_image = blurred;
        refresh_display();
    }
    return IUP_DEFAULT;
}

static int cb_sharpen(Ihandle* self) {
    (void)self;
    if(!ensure_image_loaded()) return IUP_DEFAULT;

    save_for_undo();
    Image* sharpened = sharpen(g_image);
    if(sharpened != NULL) {
        free_image(g_image);
        g_image = sharpened;
        refresh_display();
    }
    return IUP_DEFAULT;
}

int main(int argc, char** argv) {
    IupOpen(&argc, &argv);

    g_canvas = IupCanvas(NULL);
    IupSetCallback(g_canvas, "ACTION", (Icallback)cb_action_canvas);
    IupSetAttribute(g_canvas, "EXPAND", "YES");

    /*make buttons*/
    Ihandle* btn_open = IupButton("Open BMP", NULL);
    Ihandle* btn_save = IupButton("Save BMP", NULL);
    Ihandle* btn_undo = IupButton("Undo", NULL);
    Ihandle* btn_gray = IupButton("Grayscale", NULL);
    Ihandle* btn_bright = IupButton("Brightness", NULL);
    Ihandle* btn_invert = IupButton("Invert", NULL);
    Ihandle* btn_fliph = IupButton("Flip H", NULL);
    Ihandle* btn_flipv = IupButton("Flip V", NULL);
    Ihandle* btn_rotate = IupButton("Rotate 90", NULL);
    Ihandle* btn_crop = IupButton("Crop", NULL);
    Ihandle* btn_blur = IupButton("Blur", NULL);
    Ihandle* btn_sharpen = IupButton("Sharpen", NULL);

    IupSetCallback(btn_open, "ACTION", (Icallback)cb_open);
    IupSetCallback(btn_save, "ACTION", (Icallback)cb_save);
    IupSetCallback(btn_undo, "ACTION", (Icallback)cb_undo);
    IupSetCallback(btn_gray, "ACTION", (Icallback)cb_grayscale);
    IupSetCallback(btn_bright, "ACTION", (Icallback)cb_brightness);
    IupSetCallback(btn_invert, "ACTION", (Icallback)cb_invert);
    IupSetCallback(btn_fliph, "ACTION", (Icallback)cb_flip_h);
    IupSetCallback(btn_flipv, "ACTION", (Icallback)cb_flip_v);
    IupSetCallback(btn_rotate, "ACTION", (Icallback)cb_rotate);
    IupSetCallback(btn_crop, "ACTION", (Icallback)cb_crop);
    IupSetCallback(btn_blur, "ACTION", (Icallback)cb_blur);
    IupSetCallback(btn_sharpen, "ACTION", (Icallback)cb_sharpen);

    /*layout*/
    Ihandle* row1 = IupHbox(btn_open, btn_save, btn_undo, btn_gray, btn_bright, btn_invert, NULL);
    Ihandle* row2 = IupHbox(btn_fliph, btn_flipv, btn_rotate, btn_crop, btn_blur, btn_sharpen, NULL);
    IupSetAttribute(row1, "GAP", "5");
    IupSetAttribute(row2, "GAP", "5");

    Ihandle* toolbar = IupVbox(row1, row2, NULL);
    IupSetAttribute(toolbar, "MARGIN", "5x5");
    IupSetAttribute(toolbar, "GAP", "5");

    Ihandle* vbox = IupVbox(toolbar, g_canvas, NULL);
    IupSetAttribute(vbox, "ALIGNMENT", "ACENTER");

    g_main_dialog = IupDialog(vbox);
    IupSetAttribute(g_main_dialog, "TITLE", "Image Editor");
    IupSetAttribute(g_main_dialog, "RASTERSIZE", "800x600");

    IupShowXY(g_main_dialog, IUP_CENTER, IUP_CENTER);
    IupMainLoop();

    /*free memory*/
    if(g_image != NULL) free_image(g_image);
    if(g_undoImage != NULL) free_image(g_undoImage);

    IupClose();
    return 0;
}