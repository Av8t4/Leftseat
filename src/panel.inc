// Left Seat: the instrument panel (gauges, levers, status line).
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* Instrument panel: seven round gauges plus an engine-controls strip   */
/* ------------------------------------------------------------------ */

// Small helpers. Ticks, needles and bars are collected here and drawn in one go per gauge.
static vita2d_color_vertex pbuf[900];
static int pbuf_n;
static void pb_flush(void) { if (pbuf_n > 0) draw_triangles(pbuf, pbuf_n); pbuf_n = 0; }
static void pb_tri(P2 a, P2 b, P2 c, unsigned int col) {
    if (pbuf_n + 3 > 900) pb_flush();
    pbuf[pbuf_n++] = (vita2d_color_vertex){ a.x, a.y, 0.5f, col };
    pbuf[pbuf_n++] = (vita2d_color_vertex){ b.x, b.y, 0.5f, col };
    pbuf[pbuf_n++] = (vita2d_color_vertex){ c.x, c.y, 0.5f, col };
}
static void pb_quad(P2 a, P2 b, P2 c, P2 d, unsigned int col) { pb_tri(a, b, c, col); pb_tri(a, c, d, col); }
static void pb_line(float x0, float y0, float x1, float y1, float w, unsigned int col) {
    float dx = x1 - x0, dy = y1 - y0, l = sqrtf(dx * dx + dy * dy);
    if (l < 1e-4f) return;
    float nx = -dy / l * w * 0.5f, ny = dx / l * w * 0.5f;
    pb_quad((P2){ x0 + nx, y0 + ny }, (P2){ x1 + nx, y1 + ny }, (P2){ x1 - nx, y1 - ny }, (P2){ x0 - nx, y0 - ny }, col);
}
static void pb_needle(float cx, float cy, float ang_deg, float len, float tail, float hw, unsigned int col) {
    float a = ang_deg * PI_F / 180.0f, ca = cosf(a), sa = sinf(a);
    P2 tip = { cx + ca * len, cy + sa * len }, l = { cx - sa * hw, cy + ca * hw };
    P2 rr = { cx + sa * hw, cy - ca * hw }, tl = { cx - ca * tail, cy - sa * tail };
    pb_quad(tip, l, tl, rr, col);
}
// Colored band between two scale values.
static void pb_arc(float cx, float cy, float r0, float r1, float start, float sweep, float f0, float f1, unsigned int col) {
    int n = 2 + (int)(fabsf(sweep * (f1 - f0)) / 8.0f);
    for (int i = 0; i < n; i++) {
        float a = (start + sweep * (f0 + (f1 - f0) * i / n)) * PI_F / 180.0f;
        float b = (start + sweep * (f0 + (f1 - f0) * (i + 1) / n)) * PI_F / 180.0f;
        pb_quad((P2){ cx + cosf(a) * r0, cy + sinf(a) * r0 }, (P2){ cx + cosf(a) * r1, cy + sinf(a) * r1 },
                (P2){ cx + cosf(b) * r1, cy + sinf(b) * r1 }, (P2){ cx + cosf(b) * r0, cy + sinf(b) * r0 }, col);
    }
}

static void gauge_face(float cx, float cy, float r) {
    vita2d_draw_fill_circle(cx, cy, r + 4, RGBA8(92, 94, 102, 255));
    vita2d_draw_fill_circle(cx, cy, r + 1, RGBA8(36, 38, 44, 255));
    vita2d_draw_fill_circle(cx, cy, r,     RGBA8(12, 12, 14, 255));
}
static void gauge_ticks(float cx, float cy, float r, float start, float sweep, int n, int major_every,
                        float in_major, float in_minor, float out) {
    unsigned int white = RGBA8(232, 232, 232, 255);
    for (int i = 0; i <= n; i++) {
        float a = (start + sweep * i / n) * PI_F / 180.0f;
        int major = (i % major_every) == 0;
        float rin = (major ? in_major : in_minor) * r;
        pb_line(cx + cosf(a) * rin, cy + sinf(a) * rin, cx + cosf(a) * out * r, cy + sinf(a) * out * r,
                major ? 1.8f : 1.0f, white);
    }
}
static void gauge_text(vita2d_pgf *pgf, float cx, float cy, float r, float ang_deg, float rad, const char *t,
                       float scale, unsigned int col) {
    float a = ang_deg * PI_F / 180.0f, x = cx + cosf(a) * r * rad, y = cy + sinf(a) * r * rad;
    int tw = vita2d_pgf_text_width(pgf, scale, t);
    vita2d_pgf_draw_text(pgf, x - tw / 2, y + 8.0f * scale, col, scale, t);
}
static void gauge_label(vita2d_pgf *pgf, float cx, float y, const char *t) {
    int tw = vita2d_pgf_text_width(pgf, 0.5f, t);
    vita2d_pgf_draw_text(pgf, cx - tw / 2, y, RGBA8(150, 154, 166, 255), 0.5f, t);
}
// Little digital read-out window inside a gauge.
static void gauge_window(vita2d_pgf *pgf, float cx, float y, const char *t) {
    int tw = vita2d_pgf_text_width(pgf, 0.65f, t);
    float w = (float)tw + 10.0f;
    if (w < 34.0f) w = 34.0f;
    vita2d_draw_rectangle(cx - w / 2 - 1, y - 1, w + 2, 15, RGBA8(104, 106, 114, 255));
    vita2d_draw_rectangle(cx - w / 2, y, w, 13, RGBA8(0, 0, 0, 255));
    vita2d_pgf_draw_text(pgf, cx - tw / 2, y + 11, RGBA8(250, 250, 250, 255), 0.65f, t);
}
static void gauge_hub(float cx, float cy) {
    vita2d_draw_fill_circle(cx, cy, 4.0f, RGBA8(70, 72, 80, 255));
    vita2d_draw_fill_circle(cx, cy, 2.0f, RGBA8(150, 152, 160, 255));
}

static const unsigned int NEEDLE = RGBA8(246, 246, 246, 255);

static void draw_asi(vita2d_pgf *pgf, float cx, float cy, float r, float kt) {
    gauge_face(cx, cy, r);
    const float am = spec->asi_max;
    pb_arc(cx, cy, 0.83f * r, 0.88f * r, 135, 270, spec->asi_white0 / am, spec->asi_white1 / am, RGBA8(236, 236, 236, 255));     // flaps range
    pb_arc(cx, cy, 0.90f * r, 0.97f * r, 135, 270, spec->asi_green0 / am, spec->asi_green1 / am, RGBA8(60, 200, 80, 255));      // normal
    pb_arc(cx, cy, 0.90f * r, 0.97f * r, 135, 270, spec->asi_green1 / am, spec->asi_yellow1 / am, RGBA8(240, 210, 50, 255));    // caution
    pb_arc(cx, cy, 0.78f * r, 0.98f * r, 135, 270, (spec->asi_yellow1 - 0.6f) / am, (spec->asi_yellow1 + 0.6f) / am, RGBA8(230, 50, 40, 255));      // Vne
    gauge_ticks(cx, cy, r, 135, 270, (int)(am / 10.0f + 0.5f), (int)(spec->asi_major + 0.5f), 0.70f, 0.76f, 0.82f);
    pb_flush();
    for (int i = 1; i * spec->asi_step <= am + 0.5f; i++) {
        char t[8]; snprintf(t, sizeof(t), "%d", (int)(i * spec->asi_step + 0.5f));
        gauge_text(pgf, cx, cy, r, 135 + 270 * (i * spec->asi_step) / am, 0.56f, t, 0.5f, RGBA8(230, 230, 230, 255));
    }
    char b[16]; snprintf(b, sizeof(b), "%d KT", (int)(kt + 0.5f));
    gauge_window(pgf, cx, cy + 0.36f * r, b);
    pb_needle(cx, cy, 135 + 270 * clampf(kt, 0, am) / am, 0.78f * r, 0.12f * r, 1.8f, NEEDLE);
    pb_flush();
    gauge_hub(cx, cy);
}

static void draw_alt(vita2d_pgf *pgf, float cx, float cy, float r, float ft) {
    gauge_face(cx, cy, r);
    gauge_ticks(cx, cy, r, -90, 360, 50, 5, 0.80f, 0.88f, 0.96f);
    pb_flush();
    for (int i = 0; i < 10; i++) {
        char t[4]; snprintf(t, sizeof(t), "%d", i);
        gauge_text(pgf, cx, cy, r, -90 + 36.0f * i, 0.64f, t, 0.5f, RGBA8(240, 240, 240, 255));
    }
    char b[16]; snprintf(b, sizeof(b), "%d FT", (int)(ft + 0.5f));
    gauge_window(pgf, cx, cy + 0.38f * r, b);
    pb_needle(cx, cy, -90 + 360 * fmodf(ft, 10000.0f) / 10000.0f, 0.50f * r, 0.10f * r, 3.2f, RGBA8(220, 222, 226, 255));   // thousands
    pb_needle(cx, cy, -90 + 360 * fmodf(ft, 1000.0f) / 1000.0f, 0.86f * r, 0.14f * r, 1.6f, NEEDLE);                         // hundreds
    pb_flush();
    gauge_hub(cx, cy);
}

// Directional gyro: the compass card turns, the little aeroplane stays put.
static void draw_heading(vita2d_pgf *pgf, float cx, float cy, float r, float hdg_deg) {
    gauge_face(cx, cy, r);
    unsigned int white = RGBA8(236, 236, 236, 255);
    for (int d = 0; d < 360; d += 5) {
        float a = ((d - hdg_deg) - 90.0f) * PI_F / 180.0f, ca = cosf(a), sa = sinf(a);
        float len = (d % 30 == 0) ? 0.18f : ((d % 10 == 0) ? 0.12f : 0.07f);
        pb_line(cx + ca * r * 0.97f, cy + sa * r * 0.97f, cx + ca * r * (0.97f - len), cy + sa * r * (0.97f - len),
                (d % 30 == 0) ? 1.8f : 1.0f, white);
    }
    pb_flush();
    static const char *L[12] = { "N", "3", "6", "E", "12", "15", "S", "21", "24", "W", "30", "33" };
    for (int k = 0; k < 12; k++)
        gauge_text(pgf, cx, cy, r, (k * 30 - hdg_deg) - 90.0f, 0.66f, L[k], 0.5f,
                   k == 0 ? RGBA8(255, 110, 90, 255) : RGBA8(240, 240, 240, 255));
    unsigned int ac = RGBA8(255, 190, 60, 255);                       // fixed aeroplane symbol
    pb_line(cx, cy - 0.36f * r, cx, cy + 0.30f * r, 3.2f, ac);
    pb_line(cx - 0.34f * r, cy + 0.03f * r, cx + 0.34f * r, cy + 0.03f * r, 3.0f, ac);
    pb_line(cx - 0.12f * r, cy + 0.27f * r, cx + 0.12f * r, cy + 0.27f * r, 2.4f, ac);
    pb_tri((P2){ cx - 5, cy - r - 5 }, (P2){ cx + 5, cy - r - 5 }, (P2){ cx, cy - r + 6 }, RGBA8(255, 150, 40, 255));   // lubber mark
    pb_flush();
}

static void draw_tach(vita2d_pgf *pgf, float cx, float cy, float r, float rpm) {
    gauge_face(cx, cy, r);
    const float tm = spec->tach_max;
    pb_arc(cx, cy, 0.90f * r, 0.97f * r, 135, 270, spec->tach_green0 / tm, spec->tach_green1 / tm, RGBA8(60, 200, 80, 255));
    if (spec->tach_red > spec->tach_green1)
        pb_arc(cx, cy, 0.90f * r, 0.97f * r, 135, 270, spec->tach_green1 / tm, spec->tach_red / tm, RGBA8(240, 210, 50, 255));
    pb_arc(cx, cy, 0.78f * r, 0.98f * r, 135, 270, (spec->tach_red - spec->tach_mark) / tm, (spec->tach_red + spec->tach_mark) / tm, RGBA8(230, 50, 40, 255));
    gauge_ticks(cx, cy, r, 135, 270, (int)(tm / spec->tach_tick + 0.5f), (int)(spec->tach_major / spec->tach_tick + 0.5f), 0.70f, 0.76f, 0.82f);
    pb_flush();
    const float per = spec->tach_major / spec->tach_tick;                // (the numbers are in units of the tick: x100 rpm, or per cent)
    for (int i = 0; i * spec->tach_major <= tm + 0.5f; i++) {
        char t[8]; snprintf(t, sizeof(t), "%d", (int)(i * per * (spec->tach_tick >= 100.0f ? 1.0f : spec->tach_tick) + 0.5f));
        gauge_text(pgf, cx, cy, r, 135 + 270.0f * (i * spec->tach_major) / tm, 0.56f, t, 0.5f, RGBA8(230, 230, 230, 255));
    }
    char b[24]; snprintf(b, sizeof(b), "%d %s", (int)(rpm / spec->tach_round + 0.5f) * (int)spec->tach_round, spec->tach_unit);
    gauge_window(pgf, cx, cy + 0.36f * r, b);
    pb_needle(cx, cy, 135 + 270 * clampf(rpm, 0, tm) / tm, 0.78f * r, 0.12f * r, 1.8f, NEEDLE);
    pb_flush();
    gauge_hub(cx, cy);
}

static void draw_fuel(vita2d_pgf *pgf, float cx, float cy, float r, float gal) {
    gauge_face(cx, cy, r);
    const float S = 210.0f, W = 120.0f;                                // half dial across the top
    pb_arc(cx, cy, 0.84f * r, 0.94f * r, S, W, 0.0f, 0.10f, RGBA8(230, 50, 40, 255));       // nearly empty
    pb_arc(cx, cy, 0.84f * r, 0.94f * r, S, W, 0.10f, 1.0f, RGBA8(60, 200, 80, 255));
    gauge_ticks(cx, cy, r, S, W, 8, 2, 0.68f, 0.74f, 0.82f);
    pb_flush();
    gauge_text(pgf, cx, cy, r, S, 0.58f, "E", 0.55f, RGBA8(255, 120, 100, 255));
    gauge_text(pgf, cx, cy, r, S + W, 0.58f, "F", 0.55f, RGBA8(230, 230, 230, 255));
    gauge_text(pgf, cx, cy, r, S + W / 2, 0.58f, "1/2", 0.45f, RGBA8(230, 230, 230, 255));
    gauge_label(pgf, cx, cy + 0.20f * r, "FUEL");
    char b[24]; snprintf(b, sizeof(b), "%.1f GAL", gal);
    gauge_window(pgf, cx, cy + 0.36f * r, b);
    pb_needle(cx, cy, S + W * clampf(gal / FUEL_FULL, 0, 1), 0.76f * r, 0.10f * r, 1.8f, NEEDLE);
    pb_flush();
    gauge_hub(cx, cy);
}

// Vertical speed: expanded scale near zero so small climbs and descents are readable.
static float vsi_frac(float fpm) {
    float m = clampf(fabsf(fpm) / 2000.0f, 0.0f, 1.0f);
    m = powf(m, 0.6f);
    return 0.5f + (fpm < 0.0f ? -0.5f : 0.5f) * m;
}
static void draw_vsi(vita2d_pgf *pgf, float cx, float cy, float r, float fpm) {
    gauge_face(cx, cy, r);
    static const int marks[] = { -2000, -1500, -1000, -500, 0, 500, 1000, 1500, 2000 };
    for (int i = 0; i < 9; i++) {
        float a = (135.0f + 270.0f * vsi_frac((float)marks[i])) * PI_F / 180.0f;
        float in = (marks[i] % 1000 == 0) ? 0.72f : 0.80f;
        pb_line(cx + cosf(a) * r * in, cy + sinf(a) * r * in, cx + cosf(a) * r * 0.95f, cy + sinf(a) * r * 0.95f,
                (marks[i] % 1000 == 0) ? 1.8f : 1.0f, RGBA8(232, 232, 232, 255));
    }
    pb_flush();
    for (int i = 0; i < 9; i++)
        if (marks[i] % 1000 == 0) {
            char t[8]; snprintf(t, sizeof(t), "%d", abs(marks[i]) / 1000);
            gauge_text(pgf, cx, cy, r, 135.0f + 270.0f * vsi_frac((float)marks[i]), 0.56f, t, 0.5f, RGBA8(230, 230, 230, 255));
        }
    char b[16]; snprintf(b, sizeof(b), "%+d FPM", (int)(fpm / 10.0f + (fpm >= 0 ? 0.5f : -0.5f)) * 10);
    gauge_window(pgf, cx, cy + 0.36f * r, b);
    pb_needle(cx, cy, 135.0f + 270.0f * vsi_frac(fpm), 0.78f * r, 0.12f * r, 1.8f, NEEDLE);
    pb_flush();
    gauge_hub(cx, cy);
}

// Vertical slider with a lever: throttle, flaps or trim.
static void slider_track(float x, float y0, float y1) {
    vita2d_draw_rectangle(x - 4, y0 - 3, 8, y1 - y0 + 6, RGBA8(100, 102, 110, 255));
    vita2d_draw_rectangle(x - 3, y0 - 2, 6, y1 - y0 + 4, RGBA8(10, 10, 12, 255));
}
static void slider_knob(float x, float y, unsigned int col) {
    vita2d_draw_rectangle(x - 9, y - 5, 18, 10, RGBA8(30, 30, 34, 255));
    vita2d_draw_rectangle(x - 8, y - 4, 16, 8, col);
    vita2d_draw_rectangle(x - 8, y - 4, 16, 2, RGBA8(255, 255, 255, 90));
}

static void draw_panel(vita2d_pgf *pgf) {
    unsigned int txt = RGBA8(222, 224, 230, 255), dim = RGBA8(150, 154, 166, 255);
    vita2d_draw_rectangle(0, VIEW_H, SCREEN_W, SCREEN_H - VIEW_H, RGBA8(48, 48, 54, 255));
    vita2d_draw_rectangle(0, VIEW_H, SCREEN_W, 3, RGBA8(120, 120, 128, 255));

    float kt  = pl.speed * 1.944f;
    float ft  = fmaxf(0.0f, pl.pos.y * 3.281f);
    float fpm = pl.vs * 196.85f;
    float hdg_deg = pl.hdg * 57.2958f;
    const float R = 46.0f, dy = VIEW_H + 57.0f;

    draw_asi(pgf,    62, dy, R, kt);
    draw_attitude(  180, dy, R);
    draw_alt(pgf,   298, dy, R, ft);
    draw_heading(pgf, 416, dy, R, hdg_deg);
    draw_vsi(pgf,   534, dy, R, fpm);
    draw_tach(pgf,  652, dy, R, pl.rpm);
    draw_fuel(pgf,  770, dy, R, pl.fuel);

    // Engine controls: throttle, flaps and trim levers.
    const float y0 = VIEW_H + 18.0f, y1 = VIEW_H + 98.0f;
    float tx = 852.0f, fx = 892.0f, mx = 932.0f;
    slider_track(tx, y0, y1);
    vita2d_draw_rectangle(tx - 2, y1 - (y1 - y0) * pl.throttle, 4, (y1 - y0) * pl.throttle, RGBA8(70, 190, 90, 255));
    slider_knob(tx, y1 - (y1 - y0) * pl.throttle, RGBA8(210, 50, 44, 255));
    slider_track(fx, y0, y1);
    for (int i = 0; i < 4; i++) {
        float y = y0 + (y1 - y0) * i / 3.0f;
        vita2d_draw_rectangle(fx - 7, y, 3, 1, RGBA8(200, 200, 206, 255));
        char t[4]; snprintf(t, sizeof(t), "%d", i * 10);
        vita2d_pgf_draw_text(pgf, fx - 25, y + 4, dim, 0.45f, t);
    }
    slider_knob(fx, y0 + (y1 - y0) * pl.flaps / 3.0f, RGBA8(70, 130, 210, 255));
    slider_track(mx, y0, y1);
    vita2d_draw_rectangle(mx - 7, (y0 + y1) / 2, 14, 1, RGBA8(200, 200, 206, 255));
    slider_knob(mx, (y0 + y1) / 2 - pl.trim / 0.5f * (y1 - y0) / 2, RGBA8(230, 190, 60, 255));
    vita2d_pgf_draw_text(pgf, tx - 11, VIEW_H + 118, dim, 0.5f, "THR");
    vita2d_pgf_draw_text(pgf, fx - 14, VIEW_H + 118, dim, 0.5f, "FLAPS");
    vita2d_pgf_draw_text(pgf, mx - 11, VIEW_H + 118, dim, 0.5f, "TRIM");

    // Slip indicator (a ball in a tube) under the attitude indicator.
    vita2d_draw_rectangle(138, VIEW_H + 113, 84, 14, RGBA8(100, 102, 110, 255));
    vita2d_draw_rectangle(139, VIEW_H + 114, 82, 12, RGBA8(8, 8, 10, 255));
    vita2d_draw_rectangle(171, VIEW_H + 114, 1, 12, RGBA8(160, 160, 168, 255));
    vita2d_draw_rectangle(189, VIEW_H + 114, 1, 12, RGBA8(160, 160, 168, 255));
    vita2d_draw_fill_circle(180.5f + clampf(pl.beta / 0.15f, -1.0f, 1.0f) * 30.0f, VIEW_H + 120, 5.0f, RGBA8(255, 200, 60, 255));

    // Weather line along the bottom (weather, wind, ground/air state).
    int wd = ((int)(wx_dir * 57.2958f + 360.5f)) % 360;
    vita2d_pgf_draw_textf(pgf, 250, VIEW_H + 129, txt, 0.65f, "%s  WIND %03d/%d KT   %s%s%s",
                          wx_label(), wd, (int)(wx.wind * 1.944f + 0.5f),
                          pl.on_ground ? "ON GROUND" : "AIRBORNE", invert_pitch ? "  INV" : "", gy_mode ? "  GYRO" : "");

    // Jet-style gear lights (retractable aircraft only): 3 green = down and locked, 3 red = moving, dark = up.
    if (spec->retract) {
        int cx = 711, cy = VIEW_H + 126;
        bool down = pl.gear >= 0.999f, moving = (pl.gear > 0.001f && pl.gear < 0.999f);
        vita2d_draw_rectangle(cx - 34, cy - 17, 68, 34, RGBA8(90, 92, 98, 255));
        vita2d_draw_rectangle(cx - 32, cy - 15, 64, 30, RGBA8(14, 14, 16, 255));
        vita2d_pgf_draw_text(pgf, cx - 29, cy + 3, dim, 0.45f, "GEAR");
        int lx[3] = { cx + 14, cx + 5, cx + 23 }, ly[3] = { cy - 8, cy + 7, cy + 7 };
        unsigned int col  = down ? RGBA8(40, 230, 70, 255)  : moving ? RGBA8(240, 40, 35, 255) : RGBA8(35, 50, 38, 255);
        unsigned int glow = down ? RGBA8(20, 110, 35, 255)  : RGBA8(120, 20, 18, 255);
        for (int i = 0; i < 3; i++) {
            if (down || moving) vita2d_draw_fill_circle(lx[i], ly[i], 6.5f, glow);
            vita2d_draw_fill_circle(lx[i], ly[i], 4.5f, col);
        }
    }
}

