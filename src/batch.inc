// Left Seat: collecting triangles and sending them to the GPU.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* 2D polygon clipping + drawing                                       */
/* ------------------------------------------------------------------ */

// All scene triangles are collected here and sent to the GPU in one draw call
// (hundreds of tiny draw calls per frame can overwhelm the GXM command buffers).
#define MAX_BATCH 16000
static vita2d_color_vertex batch[MAX_BATCH];
static int batch_n;

// Copy triangles into GPU-visible memory (the vita2d frame pool) and draw them.
// Ordinary program memory is not readable by the GPU and would crash it.
static void draw_triangles(const vita2d_color_vertex *src, int n) {
    static int logged = 0;
    if (n <= 0) return;
    vita2d_color_vertex *gpu = (vita2d_color_vertex *)vita2d_pool_memalign(
        n * sizeof(vita2d_color_vertex), sizeof(vita2d_color_vertex));
    if (logfile && logged < 3) {
        fprintf(logfile, "flush %d verts, pool %s\n", n, gpu ? "ok" : "FULL");
        fflush(logfile);
        logged++;
    }
    if (gpu) {
        memcpy(gpu, src, n * sizeof(vita2d_color_vertex));
        vita2d_draw_array(SCE_GXM_PRIMITIVE_TRIANGLES, gpu, n);
    }
}

static void flush_batch(void) {
    draw_triangles(batch, batch_n);
    batch_n = 0;
}

