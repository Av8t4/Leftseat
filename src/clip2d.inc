// Left Seat: 2D polygon clipping and filling.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

// Sutherland-Hodgman against one axis-aligned edge.
// sign=+1 keeps coord >= lim, sign=-1 keeps coord <= lim.
static int clip_pass(const P2 *in, int n, P2 *out, int axis, float lim, int sign) {
    int m = 0;
    for (int i = 0; i < n; i++) {
        P2 a = in[i], b = in[(i + 1) % n];
        float av = axis ? a.y : a.x, bv = axis ? b.y : b.x;
        float da = sign * (av - lim), db = sign * (bv - lim);
        bool ain = da >= 0.0f, bin = db >= 0.0f;
        if (ain) out[m++] = a;
        if (ain != bin) {
            float t = da / (da - db);
            out[m].x = a.x + (b.x - a.x) * t;
            out[m].y = a.y + (b.y - a.y) * t;
            m++;
        }
    }
    return m;
}

// Clip to the 3D window, then draw as a triangle fan.
static void fill_poly2d(const P2 *pts, int n, unsigned int color) {
    P2 a[24], b[24];
    float x0 = pts[0].x, x1 = x0, y0 = pts[0].y, y1 = y0;
    for (int i = 0; i < n; i++) {
        if (!isfinite(pts[i].x) || !isfinite(pts[i].y)) return;   // never feed NaN to the GPU
        if (pts[i].x < x0) x0 = pts[i].x; else if (pts[i].x > x1) x1 = pts[i].x;
        if (pts[i].y < y0) y0 = pts[i].y; else if (pts[i].y > y1) y1 = pts[i].y;
    }
    if (x1 < 0.0f || x0 > (float)SCREEN_W || y1 < 0.0f || y0 > (float)VIEW_H) return;      // entirely off the 3D window
    memcpy(a, pts, n * sizeof(P2));
    int m = n;
    if (x0 < 0.0f || x1 > (float)SCREEN_W || y0 < 0.0f || y1 > (float)VIEW_H) {           // only clip polygons that stick out
        m = clip_pass(a, n, b, 0, 0.0f, 1);              if (m < 3) return;
        m = clip_pass(b, m, a, 0, (float)SCREEN_W, -1);  if (m < 3) return;
        m = clip_pass(a, m, b, 1, 0.0f, 1);              if (m < 3) return;
        m = clip_pass(b, m, a, 1, (float)VIEW_H, -1);    if (m < 3) return;
    }

    int need = (m - 2) * 3;
    if (batch_n + need > MAX_BATCH) flush_batch();
    for (int i = 1; i < m - 1; i++) {
        batch[batch_n++] = (vita2d_color_vertex){ a[0].x,     a[0].y,     0.5f, color };
        batch[batch_n++] = (vita2d_color_vertex){ a[i].x,     a[i].y,     0.5f, color };
        batch[batch_n++] = (vita2d_color_vertex){ a[i + 1].x, a[i + 1].y, 0.5f, color };
    }
}

