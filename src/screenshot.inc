// Left Seat: save a screenshot as a PNG. Hold SELECT and press START: the picture lands in ux0:data/ as leftseat_001.png,
// leftseat_002.png and so on (copy them off the Vita with VitaShell). Part of Left Seat's single translation unit.

#include <png.h>
#include <psp2/display.h>

#ifndef SHOT_DIR
#define SHOT_DIR "ux0:data/"
#endif

static bool shot_pending;          // a screenshot was asked for this frame: take it once the frame has been drawn
static int  shot_msg_t;            // frames left showing the "saved" message
static char shot_msg[48];

// Write the picture that is on the screen. Returns true on success.
static bool take_screenshot(void) {
    SceDisplayFrameBuf fb;
    memset(&fb, 0, sizeof(fb));
    fb.size = sizeof(fb);
    if (sceDisplayGetFrameBuf(&fb, 0) < 0 || !fb.base || fb.width == 0 || fb.height == 0 || fb.pitch < fb.width) {
        snprintf(shot_msg, sizeof(shot_msg), "SCREENSHOT FAILED");
        return false;
    }
    char path[96], name[32];
    int n;
    for (n = 1; n < 1000; n++) {                                   // the first free number
        snprintf(name, sizeof(name), "leftseat_%03d.png", n);
        snprintf(path, sizeof(path), SHOT_DIR "%s", name);
        FILE *t = fopen(path, "rb");
        if (!t) break;
        fclose(t);
    }
    FILE *f = (n < 1000) ? fopen(path, "wb") : NULL;
    if (!f) { snprintf(shot_msg, sizeof(shot_msg), "SCREENSHOT FAILED"); return false; }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png ? png_create_info_struct(png) : NULL;
    unsigned char *row = (unsigned char *)malloc(fb.width * 3);
    if (!png || !info || !row || setjmp(png_jmpbuf(png))) {
        if (png) png_destroy_write_struct(&png, info ? &info : NULL);
        free(row);
        fclose(f);
        remove(path);
        snprintf(shot_msg, sizeof(shot_msg), "SCREENSHOT FAILED");
        return false;
    }
    png_init_io(png, f);
    png_set_compression_level(png, 3);                              // quick: this happens in the middle of a flight
    png_set_IHDR(png, info, fb.width, fb.height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    for (unsigned int y = 0; y < fb.height; y++) {                 // the screen holds 4 bytes a pixel (red, green, blue, unused)
        const unsigned char *src = (const unsigned char *)fb.base + (size_t)y * fb.pitch * 4;
        for (unsigned int x = 0; x < fb.width; x++) {
            row[x * 3] = src[x * 4]; row[x * 3 + 1] = src[x * 4 + 1]; row[x * 3 + 2] = src[x * 4 + 2];
        }
        png_write_row(png, row);
    }
    png_write_end(png, info);
    png_destroy_write_struct(&png, &info);
    free(row);
    fclose(f);
    snprintf(shot_msg, sizeof(shot_msg), "SAVED  %s", name);
    return true;
}

static void draw_shot_msg(vita2d_pgf *pgf) {
    if (shot_msg_t <= 0) return;
    shot_msg_t--;
    int tw = vita2d_pgf_text_width(pgf, 0.85f, shot_msg);
    vita2d_draw_rectangle((SCREEN_W - tw) / 2 - 14, 8, tw + 28, 34, RGBA8(10, 22, 56, 220));
    vita2d_pgf_draw_text(pgf, (SCREEN_W - tw) / 2, 32, RGBA8(255, 206, 130, 255), 0.85f, shot_msg);
}
