// Left Seat: the chart. Hold Select to open it: the flight pauses and the chart fills the screen.
// Drag the map with a finger, pinch to zoom, tap an airport to select it. Part of Left Seat's single translation unit.

typedef struct { int n; float x[2], y[2]; int id[2]; } ChTouch;                 // front touch screen, in screen pixels (up to two fingers)
static ChTouch g_touch;                                                          // read once a frame in main(); the chart and the hangar use it

#define CH_MX 10.0f                                                              // the map
#define CH_MY 10.0f
#define CH_MW 600.0f
#define CH_MH 524.0f
#define CH_S0 (CH_MH / 48600.0f)                                                 // pixels per metre when the whole area is on show
#define CH_X0 (CH_MX + CH_MW + 16.0f)                                            // the side column
#define CH_LT 84.0f                                                              // the airport list window
#define CH_LB 398.0f
#define CH_STEP 78.0f
#define CH_BY 408.0f                                                             // the buttons
#define CH_BH 44.0f

static float chart_cx, chart_cz, chart_zm;                                       // what the map is looking at: world point at its middle, zoom (1 = whole area)
static int   chart_sel;                                                          // the airport picked on the chart (-1: none)

static float ch_s(void) { return CH_S0 * chart_zm; }
static float ch_sx(float x) { return CH_MX + CH_MW * 0.5f + (x - chart_cx) * ch_s(); }
static float ch_sy(float z) { return CH_MY + CH_MH * 0.5f - (z - chart_cz) * ch_s(); }

static void ch_clamp_view(void) {
    float zmin = ap_found ? 0.6f : 1.0f;
    chart_zm = clampf(chart_zm, zmin, 16.0f);
    chart_cx = clampf(chart_cx, -22000.0f, ap_found ? 40000.0f : 22000.0f);
    chart_cz = clampf(chart_cz, ap_found ? -34000.0f : -16000.0f, 34000.0f);
}
static void ch_zoom_at(float px, float py, float k) {                            // zoom, keeping the point under (px, py) where it is
    float s = ch_s(), wx = chart_cx + (px - CH_MX - CH_MW * 0.5f) / s, wz = chart_cz - (py - CH_MY - CH_MH * 0.5f) / s;
    chart_zm *= k;
    chart_zm = clampf(chart_zm, ap_found ? 0.6f : 1.0f, 16.0f);
    s = ch_s();
    chart_cx = wx - (px - CH_MX - CH_MW * 0.5f) / s;
    chart_cz = wz + (py - CH_MY - CH_MH * 0.5f) / s;
    ch_clamp_view();
}
static void ch_center_on(float x, float z) { chart_cx = x; chart_cz = z; ch_clamp_view(); }

static struct { int mode; int id; float sx, sy, mx, my; int age; bool moved; int btn; int cnt; float d; } ct;   // the finger that is down

static void chart_begin(void) {                                                  // called as the chart opens
    chart_scroll = 0.0f; chart_sel = -1; chart_zm = 1.0f;
    chart_cx = -1700.0f; chart_cz = 7000.0f;
    float hx = 0.5f * CH_MW / ch_s() - 1500.0f, hz = 0.5f * CH_MH / ch_s() - 1500.0f;     // flying outside the usual window (the far spaceport): look at you
    if (fabsf(pl.pos.x - chart_cx) > hx || fabsf(pl.pos.z - chart_cz) > hz) { chart_cx = pl.pos.x; chart_cz = pl.pos.z; }
    ch_clamp_view();
    memset(&ct, 0, sizeof ct);
}

// ---- clipping the map to its window --------------------------------------------------------------------------------------------------------
static bool ch_inmap(float x, float y, float m) { return x > CH_MX - m && x < CH_MX + CH_MW + m && y > CH_MY - m && y < CH_MY + CH_MH + m; }
static void ch_line(float x0, float y0, float x1, float y1, float w, unsigned int col) {     // Liang-Barsky
    float t0 = 0.0f, t1 = 1.0f, dx = x1 - x0, dy = y1 - y0;
    float p[4] = { -dx, dx, -dy, dy }, q[4] = { x0 - CH_MX, CH_MX + CH_MW - x0, y0 - CH_MY, CH_MY + CH_MH - y0 };
    for (int i = 0; i < 4; i++) {
        if (p[i] == 0.0f) { if (q[i] < 0.0f) return; }
        else {
            float t = q[i] / p[i];
            if (p[i] < 0.0f) { if (t > t1) return; if (t > t0) t0 = t; }
            else             { if (t < t0) return; if (t < t1) t1 = t; }
        }
    }
    pb_line(x0 + dx * t0, y0 + dy * t0, x0 + dx * t1, y0 + dy * t1, w, col);
}
static int ch_clip_side(const P2 *in, int n, P2 *out, int side) {                // Sutherland-Hodgman, one side of the window
    int m = 0;
    for (int i = 0; i < n; i++) {
        P2 a = in[i], b = in[(i + 1) % n];
        float da, db;
        switch (side) {
            case 0:  da = a.x - CH_MX;             db = b.x - CH_MX;             break;
            case 1:  da = CH_MX + CH_MW - a.x;     db = CH_MX + CH_MW - b.x;     break;
            case 2:  da = a.y - CH_MY;             db = b.y - CH_MY;             break;
            default: da = CH_MY + CH_MH - a.y;     db = CH_MY + CH_MH - b.y;     break;
        }
        if (da >= 0.0f) out[m++] = a;
        if ((da >= 0.0f) != (db >= 0.0f)) { float t = da / (da - db); out[m++] = (P2){ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t }; }
    }
    return m;
}
static void ch_poly(const P2 *p, int n, unsigned int col) {                      // a convex polygon, clipped to the window
    P2 a[24], b[24];
    if (n > 8) n = 8;
    memcpy(a, p, n * sizeof(P2));
    for (int s = 0; s < 4 && n >= 3; s++) {
        n = ch_clip_side(a, n, b, s);
        memcpy(a, b, n * sizeof(P2));
    }
    for (int i = 1; i + 1 < n; i++) pb_tri(a[0], a[i], a[i + 1], col);
}
static void ch_rect(float x0, float y0, float x1, float y1, unsigned int col) {
    P2 q[4] = { { x0, y0 }, { x1, y0 }, { x1, y1 }, { x0, y1 } };
    ch_poly(q, 4, col);
}

// ---- the picture ---------------------------------------------------------------------------------------------------------------------------
static void draw_chart(vita2d_pgf *pgf) {
    const unsigned int gold = RGBA8(255, 206, 130, 255), white = RGBA8(255, 255, 255, 255), dim = RGBA8(170, 184, 214, 255), card = RGBA8(8, 18, 40, 255);
    const float S = ch_s(), MX = CH_MX, MY = CH_MY, MW = CH_MW, MH = CH_MH;
    float gs = sqrtf(pl.vel.x * pl.vel.x + pl.vel.z * pl.vel.z);

    vita2d_draw_rectangle(0, 0, SCREEN_W, SCREEN_H, card);
    vita2d_draw_rectangle(MX, MY, MW, MH, RGBA8(40, 74, 44, 255));                // farmland
    {                                                                              // the desert around Redrock
        const Airport *b = &AIRPORTS[AP_REDROCK];
        float bx = ch_sx(b->cx), by = ch_sy(b->cz), r = 6500.0f * S;
        P2 o[8];
        for (int i = 0; i < 8; i++) { float a = i * PI_F / 4.0f + 0.39f; o[i] = (P2){ bx + cosf(a) * r, by + sinf(a) * r }; }
        ch_poly(o, 8, RGBA8(112, 80, 58, 255));
    }
    if (ap_found) {                                                                // the sea down the east side, once you have been out there
        for (float z = -60000.0f; z < 60000.0f; z += 1500.0f) {
            float c0 = sea_coast_x(z), c1 = sea_coast_x(z + 1500.0f);
            float y0 = ch_sy(z), y1 = ch_sy(z + 1500.0f);
            if ((y0 < MY && y1 < MY) || (y0 > MY + MH && y1 > MY + MH)) continue;
            P2 sh[4] = { { ch_sx(c0), y0 }, { ch_sx(c0 + 1800.0f), y0 }, { ch_sx(c1 + 1800.0f), y1 }, { ch_sx(c1), y1 } };
            P2 dp[4] = { { ch_sx(c0 + 1800.0f), y0 }, { ch_sx(c0 + 80000.0f), y0 }, { ch_sx(c1 + 80000.0f), y1 }, { ch_sx(c1 + 1800.0f), y1 } };
            ch_poly(sh, 4, RGBA8(84, 158, 190, 255));
            ch_poly(dp, 4, RGBA8(36, 84, 150, 255));
            ch_line(ch_sx(c0), y0, ch_sx(c1), y1, 1.6f, RGBA8(226, 208, 150, 255));    // the beach
        }
    }
    pb_flush();
    for (int n = 0; n < NLAKES; n++) {                                                // lakes
        float lx = ch_sx(LAKES[n].x), ly = ch_sy(LAKES[n].z), r = 3.0f + LAKES[n].rx * S;
        if (ch_inmap(lx, ly, -r)) vita2d_draw_fill_circle(lx, ly, r, RGBA8(52, 108, 172, 255));
    }
    for (int i = 0; i + 1 < nriver; i++)                                           // river
        ch_line(ch_sx(river_x[i]), ch_sy(river_z[i]), ch_sx(river_x[i + 1]), ch_sy(river_z[i + 1]), 2.0f, RGBA8(70, 130, 200, 255));
    for (int i = 0; i + 1 < nroadp; i++) {                                         // country roads
        if (road_new[i + 1]) continue;
        ch_line(ch_sx(road_x[i]), ch_sy(road_z[i]), ch_sx(road_x[i + 1]), ch_sy(road_z[i + 1]), 1.2f, road_dirt[i] ? RGBA8(176, 146, 100, 255) : RGBA8(168, 168, 174, 255));
    }
    {                                                                              // highway
        V3 e = ap_pt(&AIRPORTS[AP_REDROCK], 330.0f, 0.0f, -220.0f);
        ch_line(ch_sx(704.0f), ch_sy(-300.0f), ch_sx(e.x), ch_sy(e.z), 1.5f, RGBA8(150, 150, 156, 255));
    }
    {
        float zs = sqrtf(chart_zm);
        for (int i = 0; i < nmounts; i++) {                                        // mountains and hills
            float x = ch_sx(mounts[i].x), y = ch_sy(mounts[i].z);
            bool hill = mounts[i].h < 220.0f;
            float s = (hill ? 3.5f : 5.0f + mounts[i].hb * CH_S0 * 0.45f) * zs;
            if (!ch_inmap(x, y, -s)) continue;
            unsigned int c = hill ? RGBA8(70, 106, 58, 255) : RGBA8(150, 146, 140, 255);
            pb_tri((P2){ x - s, y + s * 0.7f }, (P2){ x + s, y + s * 0.7f }, (P2){ x, y - s }, c);
        }
        for (int i = 0; i < nmesas; i++) {
            float x = ch_sx(mesas[i].x), y = ch_sy(mesas[i].z), w = 4.0f * zs, h = 3.0f * zs;
            if (ch_inmap(x, y, -w)) pb_quad((P2){ x - w, y - h }, (P2){ x + w, y - h }, (P2){ x + w, y + h }, (P2){ x - w, y + h }, RGBA8(190, 100, 66, 255));
        }
        for (int i = 0; i < ntowns; i++) {
            float x = ch_sx(town_x[i]), y = ch_sy(town_z[i]), w = 3.0f * zs;
            if (ch_inmap(x, y, -w)) pb_quad((P2){ x - w, y - w }, (P2){ x + w, y - w }, (P2){ x + w, y + w }, (P2){ x - w, y + w }, RGBA8(240, 214, 120, 255));
        }
    }
    for (int i = 0; i < ntrk; i++) {                                               // where you have been
        float x = ch_sx(trk_x[i]), y = ch_sy(trk_z[i]);
        if (ch_inmap(x, y, -2.0f)) pb_quad((P2){ x - 1.2f, y - 1.2f }, (P2){ x + 1.2f, y - 1.2f }, (P2){ x + 1.2f, y + 1.2f }, (P2){ x - 1.2f, y + 1.2f }, RGBA8(120, 220, 230, 255));
    }
    for (int k = 0; k < AP_COUNT; k++) {                                           // runways
        if (!ap_shown(k)) continue;
        V3 a = ap_pt(&AIRPORTS[k], 0.0f, 0.0f, -AIRPORTS[k].hl), b = ap_pt(&AIRPORTS[k], 0.0f, 0.0f, AIRPORTS[k].hl);
        ch_line(ch_sx(a.x), ch_sy(a.z), ch_sx(b.x), ch_sy(b.z), 3.0f, white);
    }
    if (game_mode == MODE_LANDING && ch_trip()) {                                  // straight line to where you are going
        float x0 = ch_sx(pl.pos.x), y0 = ch_sy(pl.pos.z), x1 = ch_sx(AIRPORTS[ch_dest_ap()].cx), y1 = ch_sy(AIRPORTS[ch_dest_ap()].cz);
        float dx = x1 - x0, dy = y1 - y0, len = sqrtf(dx * dx + dy * dy);
        for (float t = 0.0f; t + 5.0f < len; t += 11.0f)
            ch_line(x0 + dx / len * t, y0 + dy / len * t, x0 + dx / len * (t + 5.0f), y0 + dy / len * (t + 5.0f), 1.5f, RGBA8(255, 120, 220, 255));
    }
    if (chart_sel >= 0 && ap_shown(chart_sel)) {                                   // the airport you picked: a line to it from you
        float x0 = ch_sx(pl.pos.x), y0 = ch_sy(pl.pos.z), x1 = ch_sx(AIRPORTS[chart_sel].cx), y1 = ch_sy(AIRPORTS[chart_sel].cz);
        float dx = x1 - x0, dy = y1 - y0, len = sqrtf(dx * dx + dy * dy);
        if (len > 1.0f && len < 20000.0f)
            for (float t = 0.0f; t + 6.0f < len; t += 14.0f)
                ch_line(x0 + dx / len * t, y0 + dy / len * t, x0 + dx / len * (t + 7.0f), y0 + dy / len * (t + 7.0f), 1.8f, RGBA8(120, 240, 255, 255));
    }
    {                                                                              // you (at the edge of the map when you are off it)
        float x = clampf(ch_sx(pl.pos.x), MX + 8.0f, MX + MW - 8.0f), y = clampf(ch_sy(pl.pos.z), MY + 10.0f, MY + MH - 8.0f);
        float h = pl.hdg, sx = sinf(h), sy = -cosf(h);
        pb_tri((P2){ x + sx * 11.0f, y + sy * 11.0f }, (P2){ x - sx * 6.0f - sy * 6.0f, y - sy * 6.0f + sx * 6.0f },
               (P2){ x - sx * 6.0f + sy * 6.0f, y - sy * 6.0f - sx * 6.0f }, RGBA8(255, 220, 60, 255));
    }
    pb_flush();
    if (game_mode == MODE_LANDING && ch_rings() && ch_state == CH_FLYING)                  // the rings still to fly
        for (int i = ch_ring_i; i < NRINGS; i++) {
            V3 rp = ring_pos(i);
            float x = ch_sx(rp.x), y = ch_sy(rp.z);
            if (ch_inmap(x, y, -4.0f)) vita2d_draw_fill_circle(x, y, i == ch_ring_i ? 3.5f : 2.2f, i == ch_ring_i ? gold : dim);
        }
    for (int k = 0; k < AP_COUNT; k++) {                                           // airport markers
        if (!ap_shown(k)) continue;
        float x = ch_sx(AIRPORTS[k].cx), y = ch_sy(AIRPORTS[k].cz);
        if (!ch_inmap(x, y, -6.0f)) continue;
        if (k == chart_sel) vita2d_draw_fill_circle(x, y, 11.0f, RGBA8(120, 240, 255, 255));
        vita2d_draw_fill_circle(x, y, k == chart_sel ? 9.0f : 5.0f, RGBA8(8, 18, 40, 255));
        vita2d_draw_fill_circle(x, y, k == chart_sel ? 6.0f : 3.0f, gold);
    }

    // The window frame: cover whatever spilled over the edge of the map, then draw the frame.
    vita2d_draw_rectangle(0, 0, SCREEN_W, MY, card);
    vita2d_draw_rectangle(0, MY + MH, SCREEN_W, SCREEN_H - MY - MH, card);
    vita2d_draw_rectangle(0, 0, MX, SCREEN_H, card);
    vita2d_draw_rectangle(MX + MW, 0, SCREEN_W - MX - MW, SCREEN_H, card);
    vita2d_draw_rectangle(MX, MY, MW, 1, gold); vita2d_draw_rectangle(MX, MY + MH - 1, MW, 1, gold);
    vita2d_draw_rectangle(MX, MY, 1, MH, gold); vita2d_draw_rectangle(MX + MW - 1, MY, 1, MH, gold);

    for (int k = 0; k < AP_COUNT; k++) {                                           // labels
        if (!ap_shown(k)) continue;
        float x = ch_sx(AIRPORTS[k].cx), y = ch_sy(AIRPORTS[k].cz);
        if (!ch_inmap(x, y, -40.0f)) continue;
        vita2d_pgf_draw_text(pgf, (int)x + 10, (int)y - 8, k == chart_sel ? RGBA8(120, 240, 255, 255) : white, 0.85f, AIRPORTS[k].id);
        if (chart_zm >= 2.5f) vita2d_pgf_draw_text(pgf, (int)x + 10, (int)y + 10, dim, 0.65f, AIRPORTS[k].name);
    }
    vita2d_pgf_draw_text(pgf, MX + 10, MY + 26, dim, 0.8f, "N");
    {                                                                              // scale bar (a round number of nautical miles, about 100 pixels)
        static const float NMS[7] = { 0.5f, 1.0f, 2.0f, 5.0f, 10.0f, 20.0f, 50.0f };
        float nm = NMS[0];
        for (int i = 0; i < 7; i++) if (NMS[i] * 1852.0f * S <= 150.0f) nm = NMS[i];
        float w = nm * 1852.0f * S;
        vita2d_draw_rectangle(MX + 14, MY + MH - 22, w, 2, white);
        vita2d_draw_rectangle(MX + 14, MY + MH - 26, 2, 10, white); vita2d_draw_rectangle(MX + 14 + w - 2, MY + MH - 26, 2, 10, white);
        vita2d_pgf_draw_textf(pgf, MX + 22 + (int)w, MY + MH - 14, white, 0.7f, nm < 1.0f ? "%.1f NM" : "%.0f NM", nm);
    }

    // The side column.
    const float x0 = CH_X0;
    vita2d_pgf_draw_text(pgf, x0, 42, gold, 1.2f, "CHART");
    vita2d_pgf_draw_text(pgf, x0 + 150, 42, dim, 0.8f, "PAUSED");
    vita2d_pgf_draw_textf(pgf, x0, 68, white, 0.75f, "HDG %03d   ALT %d FT   GS %d KT", ((int)(pl.hdg * 57.2958f + 0.5f)) % 360,
                          (int)(fmaxf(0.0f, pl.pos.y) * 3.281f), (int)(gs * 1.944f));
    int row = -1;
    for (int k = 0; k < AP_COUNT; k++) {
        if (!ap_shown(k)) continue;
        row++;
        float brg, dist;
        ap_bearing(&AIRPORTS[k], &brg, &dist);
        float y = CH_LT + 34.0f + row * CH_STEP - chart_scroll;
        if (k == chart_sel) {                                                      // the picked airport: a bar behind its lines (kept inside the window)
            float t = fmaxf(y - 24.0f, CH_LT), b = fminf(y + 50.0f, CH_LB);
            if (b > t) vita2d_draw_rectangle(x0 - 8.0f, t, 324.0f, b - t, RGBA8(24, 56, 104, 255));
        }
        if (y - 20.0f >= CH_LT && y <= CH_LB)
            vita2d_pgf_draw_textf(pgf, x0, y, gold, 0.95f, "%s  %s", AIRPORTS[k].id, AIRPORTS[k].name);
        if (y + 24.0f - 18.0f >= CH_LT && y + 24.0f <= CH_LB)
            vita2d_pgf_draw_textf(pgf, x0, y + 24, white, 0.85f, "BEARING %03d    %.1f NM", (int)(brg + 0.5f) % 360, dist / 1852.0f);
        if (y + 46.0f - 18.0f >= CH_LT && y + 46.0f <= CH_LB) {
            int r1 = ((int)(AIRPORTS[k].hdg * 57.2958f / 10.0f + 0.5f) + 36) % 36, r2 = ((int)(AIRPORTS[k].hdg * 57.2958f / 10.0f + 0.5f) + 18) % 36;
            if (gs > 8.0f) vita2d_pgf_draw_textf(pgf, x0, y + 46, dim, 0.8f, "%d:%02d to go at this speed", (int)(dist / gs / 60.0f), (int)(dist / gs) % 60);
            else           vita2d_pgf_draw_textf(pgf, x0, y + 46, dim, 0.8f, "Runway %02d / %02d", r1 == 0 ? 36 : r1, r2 == 0 ? 36 : r2);
        }
    }
    if (CHART_SCROLL_MAX > 0.0f) {                                                 // scroll bar
        float full = ap_nshown() * CH_STEP, view = CH_LB - CH_LT, th = fmaxf(24.0f, view * view / full);
        float ty = CH_LT + (view - th) * (chart_scroll / CHART_SCROLL_MAX);
        vita2d_draw_rectangle(SCREEN_W - 8.0f, CH_LT, 3.0f, view, RGBA8(40, 56, 90, 255));
        vita2d_draw_rectangle(SCREEN_W - 8.0f, ty, 3.0f, th, gold);
    }
    {                                                                              // buttons
        static const float BX[4] = { 0.0f, 68.0f, 136.0f, 224.0f }, BW[4] = { 60.0f, 60.0f, 80.0f, 100.0f };
        static const char *BT[4] = { "+", "-", "ME", "CLOSE" };
        for (int i = 0; i < 4; i++) {
            float bx = x0 + BX[i];
            vita2d_draw_rectangle(bx, CH_BY, BW[i], CH_BH, RGBA8(30, 52, 96, 255));
            vita2d_draw_rectangle(bx, CH_BY, BW[i], 1, gold); vita2d_draw_rectangle(bx, CH_BY + CH_BH - 1, BW[i], 1, gold);
            vita2d_draw_rectangle(bx, CH_BY, 1, CH_BH, gold); vita2d_draw_rectangle(bx + BW[i] - 1, CH_BY, 1, CH_BH, gold);
            if (i == 2) {                                                          // "centre on me": the same arrow that marks you on the map
                float cx = bx + BW[i] * 0.5f, cy = CH_BY + CH_BH * 0.5f;
                pb_tri((P2){ cx, cy - 15.0f }, (P2){ cx - 8.0f, cy + 12.0f }, (P2){ cx + 8.0f, cy + 12.0f }, RGBA8(255, 220, 60, 255));
                pb_flush();
                continue;
            }
            int tw = vita2d_pgf_text_width(pgf, 1.0f, BT[i]);
            vita2d_pgf_draw_text(pgf, (int)(bx + (BW[i] - tw) * 0.5f), (int)(CH_BY + 30.0f), white, 1.0f, BT[i]);
        }
    }
    vita2d_pgf_draw_text(pgf, x0, 482, dim, 0.68f, "Touch: drag the map, pinch to zoom, tap an airport");
    vita2d_pgf_draw_text(pgf, x0, 502, dim, 0.68f, "D-pad / stick: move   L / R: zoom   X: centre on me");
    vita2d_pgf_draw_text(pgf, x0, 522, dim, 0.68f, "Square: next airport   O / SELECT / START: resume");
}

// ---- the chart is open: the flight is paused, this runs instead -----------------------------------------------------------------------------
static void chart_update(const SceCtrlData *pad, unsigned int pressed, const ChTouch *t) {
    const float x0 = CH_X0;
    if (pressed & (SCE_CTRL_SELECT | SCE_CTRL_CIRCLE | SCE_CTRL_START)) { chart_open = false; return; }
    if (pressed & SCE_CTRL_CROSS) ch_center_on(pl.pos.x, pl.pos.z);
    if (pressed & SCE_CTRL_SQUARE) {                                               // next airport on the list, and look at it
        int k = chart_sel;
        for (int i = 0; i < AP_COUNT; i++) { k = (k + 1 + AP_COUNT) % AP_COUNT; if (ap_shown(k)) break; }
        chart_sel = k;
        ch_center_on(AIRPORTS[k].cx, AIRPORTS[k].cz);
        chart_zm = fmaxf(chart_zm, 3.0f); ch_clamp_view();
    }
    {                                                                              // buttons and sticks
        float ax = ((pad->buttons & SCE_CTRL_RIGHT) ? 1.0f : 0.0f) - ((pad->buttons & SCE_CTRL_LEFT) ? 1.0f : 0.0f);
        float ay = ((pad->buttons & SCE_CTRL_DOWN) ? 1.0f : 0.0f) - ((pad->buttons & SCE_CTRL_UP) ? 1.0f : 0.0f);
        float sx = ((int)pad->lx - 128) / 128.0f, sy = ((int)pad->ly - 128) / 128.0f;
        if (fabsf(sx) > 0.2f) ax += sx;
        if (fabsf(sy) > 0.2f) ay += sy;
        if (ax != 0.0f || ay != 0.0f) { chart_cx += ax * 300.0f * DT / ch_s(); chart_cz -= ay * 300.0f * DT / ch_s(); ch_clamp_view(); }
        float z = ((pad->buttons & SCE_CTRL_RTRIGGER) ? 1.0f : 0.0f) - ((pad->buttons & SCE_CTRL_LTRIGGER) ? 1.0f : 0.0f);
        if (z != 0.0f) ch_zoom_at(CH_MX + CH_MW * 0.5f, CH_MY + CH_MH * 0.5f, expf(z * 1.6f * DT));
    }

    // Fingers. The first one down decides what a touch does: move the map, scroll the list, or press a button.
    static const float BX[4] = { 0.0f, 68.0f, 136.0f, 224.0f }, BW[4] = { 60.0f, 60.0f, 80.0f, 100.0f };
    int pi = -1;
    for (int i = 0; i < t->n; i++) if (ct.mode && t->id[i] == ct.id) pi = i;
    if (ct.mode && pi < 0) {                                                       // the finger came up
        if (!ct.moved && ct.age < 30) {
            if (ct.mode == 1) {                                                    // tap on the map: pick the nearest airport under the finger
                int best = -1; float bd = 34.0f * 34.0f;
                for (int k = 0; k < AP_COUNT; k++) {
                    if (!ap_shown(k)) continue;
                    float dx = ch_sx(AIRPORTS[k].cx) - ct.mx, dy = ch_sy(AIRPORTS[k].cz) - ct.my, d = dx * dx + dy * dy;
                    if (d < bd) { bd = d; best = k; }
                }
                chart_sel = best;
            } else if (ct.mode == 2) {                                             // tap on the list: pick that airport and look at it
                int want = (int)floorf((ct.my - (CH_LT + 34.0f - 26.0f) + chart_scroll) / CH_STEP), row = -1;
                for (int k = 0; k < AP_COUNT; k++) {
                    if (!ap_shown(k)) continue;
                    if (++row == want) {
                        chart_sel = k;
                        ch_center_on(AIRPORTS[k].cx, AIRPORTS[k].cz);
                        chart_zm = fmaxf(chart_zm, 3.0f); ch_clamp_view();
                        break;
                    }
                }
            }
        }
        if (ct.mode == 3 && ct.btn >= 2) {                                         // ME / CLOSE act when the finger lifts (and is still on the button)
            float bx = x0 + BX[ct.btn];
            if (ct.mx >= bx && ct.mx <= bx + BW[ct.btn] && ct.my >= CH_BY && ct.my <= CH_BY + CH_BH) {
                if (ct.btn == 2) ch_center_on(pl.pos.x, pl.pos.z);
                else { chart_open = false; ct.mode = 0; return; }
            }
        }
        ct.mode = 0;
    }
    if (!ct.mode && t->n > 0) {                                                    // a finger went down
        float fx = t->x[0], fy = t->y[0];
        ct.id = t->id[0]; ct.sx = ct.mx = fx; ct.sy = ct.my = fy; ct.age = 0; ct.moved = false; ct.cnt = 1; ct.d = 0.0f; ct.btn = -1;
        if (fx >= CH_MX && fx <= CH_MX + CH_MW && fy >= CH_MY && fy <= CH_MY + CH_MH) ct.mode = 1;
        else if (fy >= CH_BY && fy <= CH_BY + CH_BH) {
            for (int i = 0; i < 4; i++) if (fx >= x0 + BX[i] && fx <= x0 + BX[i] + BW[i]) { ct.mode = 3; ct.btn = i; }
        }
        else if (fx >= x0 - 10.0f && fy >= CH_LT && fy <= CH_LB) ct.mode = 2;
        pi = 0;
    }
    if (ct.mode && pi >= 0) {
        ct.age++;
        float fx = t->x[pi], fy = t->y[pi];
        if (ct.mode == 1) {
            int other = -1;
            for (int i = 0; i < t->n; i++) if (i != pi) other = i;
            float mx = fx, my = fy, d = 0.0f;
            int cnt = 1;
            if (other >= 0) {                                                      // a second finger: pinch (and move with the middle of the two)
                mx = 0.5f * (fx + t->x[other]); my = 0.5f * (fy + t->y[other]);
                d = sqrtf((fx - t->x[other]) * (fx - t->x[other]) + (fy - t->y[other]) * (fy - t->y[other]));
                cnt = 2;
            }
            if (cnt == ct.cnt) {
                float s = ch_s();
                chart_cx -= (mx - ct.mx) / s; chart_cz += (my - ct.my) / s;
                if (cnt == 2 && ct.d > 10.0f && d > 10.0f) ch_zoom_at(mx, my, d / ct.d);
                ch_clamp_view();
            }
            ct.cnt = cnt; ct.d = d; ct.mx = mx; ct.my = my;
            if (cnt == 2 || fabsf(fx - ct.sx) + fabsf(fy - ct.sy) > 10.0f) ct.moved = true;
        } else if (ct.mode == 2) {
            chart_scroll = clampf(chart_scroll - (fy - ct.my), 0.0f, CHART_SCROLL_MAX);
            ct.mx = fx; ct.my = fy;
            if (fabsf(fy - ct.sy) > 10.0f) ct.moved = true;
        } else {
            ct.mx = fx; ct.my = fy;
            if (ct.btn == 0) ch_zoom_at(CH_MX + CH_MW * 0.5f, CH_MY + CH_MH * 0.5f, 1.035f);
            if (ct.btn == 1) ch_zoom_at(CH_MX + CH_MW * 0.5f, CH_MY + CH_MH * 0.5f, 1.0f / 1.035f);
        }
    }
}
