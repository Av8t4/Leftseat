// Left Seat: the second airport (Redrock Municipal), the highway to it and the river on the way.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* Drawing an airport in its own frame (turned to any runway heading)   */
/* ------------------------------------------------------------------ */

static void ap_quad(const Airport *a, float lx0, float lz0, float lx1, float lz1, unsigned int col) {
    V3 p[4] = { ap_pt(a, lx0, 0.0f, lz0), ap_pt(a, lx1, 0.0f, lz0), ap_pt(a, lx1, 0.0f, lz1), ap_pt(a, lx0, 0.0f, lz1) };
    draw_world_poly(p, 4, col);
}

// Marks measured from the threshold of one end: lateral l and distance a along the runway.
static void ap_rwy_rect(const Airport *ap, int north, float l0, float l1, float a0, float a1, unsigned int col) {
    float xa = north ? -l0 : l0, xb = north ? -l1 : l1;
    float za = north ? ap->hl - a0 : -ap->hl + a0, zb = north ? ap->hl - a1 : -ap->hl + a1;
    ap_quad(ap, fminf(xa, xb), fminf(za, zb), fmaxf(xa, xb), fmaxf(za, zb), col);
}
static void ap_digit(const Airport *ap, int d, int north, float lat0, unsigned int col) {
    static const int MASK[10] = { 63, 6, 91, 79, 102, 109, 125, 7, 127, 111 };      // seven segments per digit
    const float W = 7.0f, H = 18.0f, t = 1.6f, A0 = 45.0f;
    float seg[7][4] = { { 0, W, H - t, H }, { W - t, W, H / 2, H }, { W - t, W, 0, H / 2 }, { 0, W, 0, t },
                        { 0, t, 0, H / 2 }, { 0, t, H / 2, H }, { 0, W, H / 2 - t / 2, H / 2 + t / 2 } };
    for (int s = 0; s < 7; s++)
        if (MASK[d] & (1 << s))
            ap_rwy_rect(ap, north, lat0 + seg[s][0], lat0 + seg[s][1], A0 + seg[s][2], A0 + seg[s][3], col);
}

static void draw_ap_runway(const Airport *a) {
    ap_quad(a, -22.0f, -1000.0f, 22.0f, 1000.0f, RGBA8(72, 72, 78, 255));
    ap_quad(a, -22.0f, -1000.0f, -20.5f, 1000.0f, RGBA8(235, 235, 235, 255));
    ap_quad(a,  20.5f, -1000.0f,  22.0f, 1000.0f, RGBA8(235, 235, 235, 255));
    for (float z = -940.0f; z < 940.0f; z += 60.0f)
        ap_quad(a, -1.0f, z, 1.0f, z + 30.0f, RGBA8(235, 235, 235, 255));
    for (int e = 0; e < 2; e++) {
        float z0 = e ? 960.0f : -990.0f;
        for (float x = -16.0f; x <= 16.0f; x += 6.0f)
            ap_quad(a, x - 1.0f, z0, x + 1.0f, z0 + 30.0f, RGBA8(235, 235, 235, 255));
    }
}

static void draw_ap_ground(const Airport *a) {
    const unsigned int tw = RGBA8(92, 88, 88, 255), apron = RGBA8(150, 146, 138, 255), yel = RGBA8(232, 202, 64, 255), wht = RGBA8(240, 240, 244, 255);
    ap_quad(a, 64.0f, -980.0f, 76.0f, 980.0f, tw);                          // taxiway on the right, connectors and apron
    ap_quad(a, 22.0f, -966.0f, 64.0f, -954.0f, tw);
    ap_quad(a, 22.0f, -6.0f, 64.0f, 6.0f, tw);
    ap_quad(a, 22.0f, 954.0f, 64.0f, 966.0f, tw);
    ap_quad(a, 76.0f, -330.0f, 178.0f, -110.0f, apron);
    ap_quad(a, 76.0f, 280.0f, 100.0f, 620.0f, apron);                       // ramp in front of the hangars
    ap_quad(a, 69.7f, -980.0f, 70.3f, 980.0f, yel);
    ap_quad(a, 178.0f, -230.0f, 330.0f, -210.0f, tw);                       // road to the terminal
    static const int NUM[2][2] = { { 0, 7 }, { 2, 5 } };                     // runway 07 at the south-west end, 25 at the other
    for (int north = 0; north < 2; north++) {
        ap_digit(a, NUM[north][0], north, -9.0f, wht);
        ap_digit(a, NUM[north][1], north,  2.0f, wht);
        ap_rwy_rect(a, north, -12.0f, -6.0f, 300.0f, 345.0f, wht);
        ap_rwy_rect(a, north,   6.0f, 12.0f, 300.0f, 345.0f, wht);
        for (int s = -1; s <= 1; s += 2) {
            ap_rwy_rect(a, north, s * 4.0f, s * 6.0f, 150.0f, 172.0f, wht);
            ap_rwy_rect(a, north, s * 8.0f, s * 10.0f, 150.0f, 172.0f, wht);
            ap_rwy_rect(a, north, s * 4.0f, s * 6.0f, 450.0f, 472.0f, wht);
        }
    }
    for (float z = -990.0f; z <= 990.0f; z += 90.0f)                        // edge lights by day
        for (int s = -1; s <= 1; s += 2) {
            V3 w = ap_pt(a, s * 24.5f, 0.0f, z);
            float dx = w.x - cam.pos.x, dz = w.z - cam.pos.z;
            if (dx * dx + dz * dz < 2200.0f * 2200.0f)
                ap_quad(a, s * 24.5f - 0.7f, z - 0.7f, s * 24.5f + 0.7f, z + 0.7f, RGBA8(255, 246, 190, 255));
        }
    for (float x = -20.0f; x <= 20.0f; x += 4.0f) {
        ap_quad(a, x - 0.6f, -1005.0f, x + 0.6f, -1003.8f, RGBA8(70, 230, 90, 255));
        ap_quad(a, x - 0.6f, 1003.8f, x + 0.6f, 1005.0f, RGBA8(235, 60, 50, 255));
    }
}

// Glide-slope lights, same as at home but in the airport's own frame.
static void draw_ap_papi(const Airport *a) {
    static const float THR[4] = { 3.50f, 3.17f, 2.83f, 2.50f };
    float plx, plz;
    ap_local(a, pl.pos.x, pl.pos.z, &plx, &plz);
    float eye = pl.pos.y + 2.0f;
    for (int end = 0; end < 2; end++) {
        float zp = end ? 700.0f : -700.0f;
        float d = end ? plz - zp : zp - plz;
        float ang = d > 5.0f ? atan2f(eye, d) * 57.2958f : -1.0f;
        bool lit = (ang > 0.0f && d < 6000.0f);
        for (int i = 0; i < 4; i++) {
            float x = (end ? 1.0f : -1.0f) * (30.0f + 4.0f * i);
            unsigned int col = RGBA8(110, 110, 116, 255);
            if (lit) col = ang > THR[i] ? RGBA8(255, 255, 255, 255) : RGBA8(255, 50, 40, 255);
            V3 q[4] = { ap_pt(a, x - 0.8f, 1.0f, zp), ap_pt(a, x + 0.8f, 1.0f, zp), ap_pt(a, x + 0.8f, 2.3f, zp), ap_pt(a, x - 0.8f, 2.3f, zp) };
            if (lit) draw_world_poly_emit(q, 4, col); else draw_world_poly(q, 4, col);
        }
    }
}

// After dark: edge, threshold, approach, taxiway and apron lights, and lit windows.
static void draw_ap_night(const Airport *a) {
    unsigned int wht = RGBA8(255, 246, 214, 255), red = RGBA8(255, 62, 44, 255), grn = RGBA8(80, 245, 120, 255);
    unsigned int blu = RGBA8(80, 140, 255, 255), win = RGBA8(255, 224, 140, 255);
    for (float z = -1000.0f; z <= 1000.1f; z += 60.0f) {
        V3 l = ap_pt(a, -24.5f, 0.5f, z), r = ap_pt(a, 24.5f, 0.5f, z);
        draw_light(l.x, l.y, l.z, 1.5f, wht);
        draw_light(r.x, r.y, r.z, 1.5f, wht);
    }
    for (int i = 0; i < 6; i++) {
        float x = -18.0f + i * 7.2f;
        V3 g = ap_pt(a, x, 0.5f, -1004.0f), rr = ap_pt(a, x, 0.5f, 1004.0f);
        draw_light(g.x, g.y, g.z, 1.8f, grn);
        draw_light(rr.x, rr.y, rr.z, 1.8f, red);
    }
    for (int e = 0; e < 2; e++) {
        float sg = e ? 1.0f : -1.0f;
        for (int k = 1; k <= 20; k++) {
            float z = sg * (1000.0f + k * 30.0f);
            V3 c = ap_pt(a, 0.0f, 0.6f, z);
            draw_light(c.x, c.y, c.z, 1.7f, wht);
            if (k == 5 || k == 10)
                for (int j = 1; j <= 3; j++) {
                    V3 l = ap_pt(a, -3.0f * j, 0.6f, z), r = ap_pt(a, 3.0f * j, 0.6f, z);
                    draw_light(l.x, l.y, l.z, 1.7f, wht);
                    draw_light(r.x, r.y, r.z, 1.7f, wht);
                }
        }
    }
    for (float z = -960.0f; z <= 960.0f; z += 80.0f) { V3 t = ap_pt(a, 77.5f, 0.4f, z); draw_light(t.x, t.y, t.z, 1.4f, blu); }
    for (float z = -320.0f; z <= -120.0f; z += 50.0f) { V3 t = ap_pt(a, 178.5f, 0.6f, z); draw_light(t.x, t.y, t.z, 1.6f, wht); }    // apron
    for (int i = 0; i < 4; i++) {                                            // tower cab windows
        float o = -5.0f + i * 3.3f;
        V3 w1 = ap_pt(a, -80.0f + o, 33.0f, -107.2f), w2 = ap_pt(a, -80.0f + o, 33.0f, -92.8f);
        V3 w3 = ap_pt(a, -87.2f, 33.0f, -100.0f + o), w4 = ap_pt(a, -72.8f, 33.0f, -100.0f + o);
        draw_light(w1.x, w1.y, w1.z, 1.8f, win); draw_light(w2.x, w2.y, w2.z, 1.8f, win);
        draw_light(w3.x, w3.y, w3.z, 1.8f, win); draw_light(w4.x, w4.y, w4.z, 1.8f, win);
    }
    for (int i = 0; i < 8; i++) {                                            // terminal windows, runway side
        V3 w = ap_pt(a, 176.8f, 5.0f, -265.0f + i * 10.0f);
        draw_light(w.x, w.y, w.z, 1.8f, win);
    }
}

// The rotating airport beacon on the tower: visible from far away at night.
static void draw_beacon_at(const Airport *a, float lx, float lz, float hy) {
    bool on = ((frame_count / 24) & 1) != 0;
    V3 b = ap_pt(a, lx, hy, lz);
    light_far = 45000.0f;
    draw_light(b.x, b.y, b.z, 3.2f, on ? RGBA8(255, 246, 214, 255) : RGBA8(80, 245, 120, 255));
    light_far = 9000.0f;
}
static void draw_beacon(const Airport *a) { draw_beacon_at(a, -80.0f, -100.0f, 38.0f); }

static void draw_town_lights(void) {
    unsigned int win = RGBA8(255, 214, 130, 255);
    for (int i = 0; i < ntown_lights; i++) {
        V3 d = vsub(town_lights[i], cam.pos);
        if (d.x * d.x + d.z * d.z > 7000.0f * 7000.0f) continue;
        draw_light(town_lights[i].x, town_lights[i].y, town_lights[i].z, 1.5f, win);
    }
}

static void draw_redrock_all(void) {
    const Airport *a = &AIRPORTS[AP_REDROCK];
    float dc = sat_dist(cam.pos.x, cam.pos.z, a->cx, a->cz);
    if (dc < 14000.0f) draw_ap_runway(a);
    if (dc < 3800.0f)  draw_ap_ground(a);
    if (dc < 4500.0f)  draw_ap_papi(a);
    if (tod != TOD_DAY) {
        if (dc < 9000.0f) draw_ap_night(a);
        draw_beacon(a);
        draw_town_lights();
    }
}

/* ------------------------------------------------------------------ */
/* Pinecrest: a mown grass strip in the forest                          */
/* ------------------------------------------------------------------ */

// A grass strip has no paint on it: just the mown stripes, bare earth by the buildings and, after dark, a few low lights.
static void draw_pinecrest_all(void) {
    const Airport *a = &AIRPORTS[AP_PINECREST];
    float dc = sat_dist(cam.pos.x, cam.pos.z, a->cx, a->cz);
    if (dc > 10000.0f) return;
    const float hl = a->hl, hw = a->hw;
    for (int i = 0; i < 6; i++) {                                           // mown stripes
        float z0 = -hl + 2.0f * hl * i / 6.0f, z1 = -hl + 2.0f * hl * (i + 1) / 6.0f;
        ap_quad(a, -hw, z0, hw, z1, (i & 1) ? RGBA8(122, 164, 78, 255) : RGBA8(132, 174, 86, 255));
    }
    ap_quad(a, 22.0f, -70.0f, 74.0f, 92.0f, RGBA8(140, 116, 82, 255));      // bare earth round the buildings
    if (tod != TOD_DAY && dc < 7000.0f) {                                     // after dark: low edge lights, thresholds and the cabin window
        unsigned int wht = RGBA8(255, 240, 200, 255), grn = RGBA8(80, 245, 120, 255), red = RGBA8(255, 62, 44, 255), win = RGBA8(255, 214, 130, 255);
        for (float z = -hl; z <= hl + 0.1f; z += 40.0f) {
            V3 l = ap_pt(a, -hw - 0.8f, 0.4f, z), r = ap_pt(a, hw + 0.8f, 0.4f, z);
            draw_light(l.x, l.y, l.z, 1.4f, wht);
            draw_light(r.x, r.y, r.z, 1.4f, wht);
        }
        for (int i = 0; i < 4; i++) {
            float x = -hw + 4.0f + i * 5.3f;
            V3 g = ap_pt(a, x, 0.4f, -hl - 3.0f), rr = ap_pt(a, x, 0.4f, hl + 3.0f);
            draw_light(g.x, g.y, g.z, 1.6f, grn);
            draw_light(rr.x, rr.y, rr.z, 1.6f, red);
        }
        V3 w1 = ap_pt(a, 47.8f, 2.0f, 18.0f), w2 = ap_pt(a, 47.8f, 2.0f, 22.0f);
        draw_light(w1.x, w1.y, w1.z, 1.6f, win);
        draw_light(w2.x, w2.y, w2.z, 1.6f, win);
    }
}


/* ------------------------------------------------------------------ */
/* Lana Lake Regional: a small paved airport beside a lake              */
/* ------------------------------------------------------------------ */

static void draw_lana_all(void) {
    const Airport *a = &AIRPORTS[AP_LANA];
    const float hl = a->hl, hw = a->hw;
    const unsigned int wht = RGBA8(235, 235, 235, 255), tw = RGBA8(92, 88, 88, 255), apron = RGBA8(150, 146, 138, 255);
    float dc = sat_dist(cam.pos.x, cam.pos.z, a->cx, a->cz);
    if (dc > 14000.0f) return;
    ap_quad(a, -hw, -hl, hw, hl, RGBA8(72, 72, 78, 255));                    // runway
    ap_quad(a, -hw, -hl, -hw + 1.5f, hl, wht);
    ap_quad(a, hw - 1.5f, -hl, hw, hl, wht);
    for (float z = -hl + 60.0f; z < hl - 60.0f; z += 60.0f) ap_quad(a, -1.0f, z, 1.0f, z + 30.0f, wht);
    for (int e = 0; e < 2; e++) {
        float z0 = e ? hl - 40.0f : -hl + 10.0f;
        for (float x = -13.0f; x <= 13.0f; x += 6.0f) ap_quad(a, x - 1.0f, z0, x + 1.0f, z0 + 30.0f, wht);
    }
    if (dc < 3800.0f) {
        ap_quad(a, 38.0f, -hl + 20.0f, 50.0f, hl - 20.0f, tw);              // parallel taxiway, connectors and aprons
        ap_quad(a, 18.0f, -hl + 8.0f, 38.0f, -hl + 26.0f, tw);
        ap_quad(a, 18.0f, hl - 26.0f, 38.0f, hl - 8.0f, tw);
        ap_quad(a, 18.0f, -6.0f, 38.0f, 6.0f, tw);
        ap_quad(a, 50.0f, -250.0f, 150.0f, -60.0f, apron);
        ap_quad(a, 50.0f, 130.0f, 110.0f, 420.0f, apron);
        ap_quad(a, 43.7f, -hl + 20.0f, 44.3f, hl - 20.0f, RGBA8(232, 202, 64, 255));
        static const int NUM[2][2] = { { 1, 6 }, { 3, 4 } };                   // runway 16 at the north end of the frame, 34 at the other
        for (int north = 0; north < 2; north++) {
            ap_digit(a, NUM[north][0], north, -9.0f, wht);
            ap_digit(a, NUM[north][1], north,  2.0f, wht);
            for (int s = -1; s <= 1; s += 2) ap_rwy_rect(a, north, s * 4.0f, s * 6.0f, 280.0f, 302.0f, wht);
        }
    }
    if (dc < 4500.0f) draw_ap_papi(a);
    if (tod != TOD_DAY) {
        unsigned int lw = RGBA8(255, 246, 214, 255), red = RGBA8(255, 62, 44, 255), grn = RGBA8(80, 245, 120, 255);
        unsigned int blu = RGBA8(80, 140, 255, 255), win = RGBA8(255, 224, 140, 255);
        if (dc < 9000.0f) {
            for (float z = -hl; z <= hl + 0.1f; z += 60.0f) {
                V3 l = ap_pt(a, -hw - 1.0f, 0.5f, z), r = ap_pt(a, hw + 1.0f, 0.5f, z);
                draw_light(l.x, l.y, l.z, 1.5f, lw);
                draw_light(r.x, r.y, r.z, 1.5f, lw);
            }
            for (int i = 0; i < 6; i++) {
                float x = -14.0f + i * 5.6f;
                V3 g = ap_pt(a, x, 0.5f, -hl - 4.0f), rr = ap_pt(a, x, 0.5f, hl + 4.0f);
                draw_light(g.x, g.y, g.z, 1.8f, grn);
                draw_light(rr.x, rr.y, rr.z, 1.8f, red);
            }
            for (float z = -hl + 20.0f; z <= hl - 20.0f; z += 80.0f) { V3 t = ap_pt(a, 51.0f, 0.4f, z); draw_light(t.x, t.y, t.z, 1.4f, blu); }
            for (float z = -230.0f; z <= -80.0f; z += 50.0f) { V3 t = ap_pt(a, 151.0f, 0.6f, z); draw_light(t.x, t.y, t.z, 1.6f, lw); }
            for (int i = 0; i < 6; i++) { V3 w = ap_pt(a, 149.8f, 4.5f, -190.0f + i * 10.0f); draw_light(w.x, w.y, w.z, 1.8f, win); }
            for (int i = 0; i < 3; i++) { V3 w = ap_pt(a, 156.0f, 22.0f, -43.0f + i * 3.0f); draw_light(w.x, w.y, w.z, 1.8f, win); }
        }
        draw_beacon_at(a, 160.0f, -40.0f, 28.0f);
    }
}

// Windsocks at the outlying airports, when you are near enough to see them.
static void build_outlying_windsocks(void) {
    V3 p[3] = { ap_pt(&AIRPORTS[AP_REDROCK], -34.0f, 0.0f, -700.0f), ap_pt(&AIRPORTS[AP_PINECREST], 24.0f, 0.0f, -110.0f),
                ap_pt(&AIRPORTS[AP_LANA], -34.0f, 0.0f, -560.0f) };
    for (int i = 0; i < 3; i++) {
        float dx = p[i].x - cam.pos.x, dz = p[i].z - cam.pos.z;
        if (dx * dx + dz * dz < 3000.0f * 3000.0f) build_windsock_at(p[i].x, p[i].z);
    }
}

// The rings of the lake-rings challenge: the one you are flying to is gold, the next two paler. Flat glowing bands, with a
// little thickness so a ring is still there when you see it edge-on.
static void build_rings(void) {
    if (game_mode != MODE_LANDING || !ch_rings() || (ch_state != CH_FLYING && ch_state != CH_TOUCHED)) return;
    for (int i = ch_ring_i; i < NRINGS && i <= ch_ring_i + 2; i++) {
        V3 C = ring_pos(i), N = ring_nrm(i);
        float dx = C.x - cam.pos.x, dz = C.z - cam.pos.z;
        if (dx * dx + dz * dz > 7000.0f * 7000.0f) continue;
        V3 R = vnorm(vcross(v3(0.0f, 1.0f, 0.0f), N)), U = vcross(N, R);
        float rin = ch_ring_r - 1.0f, rout = ch_ring_r + 4.0f, hh = 1.5f;
        int cr = i == ch_ring_i ? 255 : 190, cg = i == ch_ring_i ? 206 : 226, cb = i == ch_ring_i ? 50 : 255;
        for (int j = 0; j < 20; j++) {
            float a0 = j * 2.0f * PI_F / 20.0f, a1 = (j + 1) * 2.0f * PI_F / 20.0f;
            V3 d0 = vadd(vscale(R, cosf(a0)), vscale(U, sinf(a0))), d1 = vadd(vscale(R, cosf(a1)), vscale(U, sinf(a1)));
            V3 q[4] = { vadd(C, vscale(d0, rin)), vadd(C, vscale(d0, rout)), vadd(C, vscale(d1, rout)), vadd(C, vscale(d1, rin)) };
            pf_poly(4, q, NULL, cr, cg, cb, PF_TWO | PF_EMIT, 255);
            V3 o0 = vadd(C, vscale(d0, rout)), o1 = vadd(C, vscale(d1, rout));
            V3 s[4] = { vsub(o0, vscale(N, hh)), vadd(o0, vscale(N, hh)), vadd(o1, vscale(N, hh)), vsub(o1, vscale(N, hh)) };
            pf_poly(4, s, NULL, cr * 0.8f, cg * 0.8f, cb * 0.8f, PF_TWO | PF_EMIT, 255);
        }
    }
}

/* ------------------------------------------------------------------ */
/* The highway between the airports and the river                       */
/* ------------------------------------------------------------------ */

// A flat strip of given width between two ground points.
static void ground_strip(float x0, float z0, float x1, float z1, float half, unsigned int col) {
    float dx = x1 - x0, dz = z1 - z0, len = sqrtf(dx * dx + dz * dz);
    if (len < 0.01f) return;
    float nx = -dz / len * half, nz = dx / len * half;
    V3 q[4] = { v3(x0 + nx, 0.03f, z0 + nz), v3(x1 + nx, 0.03f, z1 + nz), v3(x1 - nx, 0.03f, z1 - nz), v3(x0 - nx, 0.03f, z0 - nz) };
    draw_world_poly(q, 4, col);
}

static void draw_highway(void) {
    const Airport *b = &AIRPORTS[AP_REDROCK];
    V3 end = ap_pt(b, 330.0f, 0.0f, -220.0f);
    float x0 = 704.0f, z0 = -300.0f, dx = end.x - x0, dz = end.z - z0, L = sqrtf(dx * dx + dz * dz);
    int n = (int)(L / 500.0f);
    const unsigned int road = RGBA8(84, 84, 90, 255), line = RGBA8(226, 200, 70, 255);
    for (int i = 0; i < n; i++) {
        float t0 = (float)i / n, t1 = (float)(i + 1) / n;
        float ax = x0 + dx * t0, az = z0 + dz * t0, bx = x0 + dx * t1, bz = z0 + dz * t1;
        float mx = (ax + bx) * 0.5f - cam.pos.x, mz = (az + bz) * 0.5f - cam.pos.z;
        float d2 = mx * mx + mz * mz;
        if (d2 > 8000.0f * 8000.0f) continue;
        ground_strip(ax, az, bx, bz, 5.5f, road);
        if (d2 < 2000.0f * 2000.0f) {                                         // dashed centre line
            for (int k = 0; k < 4; k++) {
                float u0 = t0 + (t1 - t0) * (k * 0.25f + 0.02f), u1 = t0 + (t1 - t0) * (k * 0.25f + 0.13f);
                ground_strip(x0 + dx * u0, z0 + dz * u0, x0 + dx * u1, z0 + dz * u1, 0.25f, line);
            }
        }
    }
}

// Country roads (built in build_countryside): paved, or gravel.
static void draw_country_roads(void) {
    for (int i = 0; i + 1 < nroadp; i++) {
        if (road_new[i + 1]) continue;
        float mx = (road_x[i] + road_x[i + 1]) * 0.5f - cam.pos.x, mz = (road_z[i] + road_z[i + 1]) * 0.5f - cam.pos.z;
        if (mx * mx + mz * mz > 7000.0f * 7000.0f) continue;
        bool dirt = road_dirt[i] != 0;
        ground_strip(road_x[i], road_z[i], road_x[i + 1], road_z[i + 1], dirt ? 3.0f : 4.5f, dirt ? RGBA8(150, 124, 86, 255) : RGBA8(86, 86, 92, 255));
    }
}

// Two main streets and a little square in every town.
static void draw_town_streets(void) {
    const unsigned int st = RGBA8(88, 88, 94, 255), sq = RGBA8(172, 160, 128, 255);
    for (int n = 0; n < ntowns; n++) {
        float dx = town_x[n] - cam.pos.x, dz = town_z[n] - cam.pos.z;
        if (dx * dx + dz * dz > 6000.0f * 6000.0f) continue;
        float x = town_x[n], z = town_z[n];
        ground_strip(x - 430.0f, z, x + 430.0f, z, 4.5f, st);
        ground_strip(x, z - 430.0f, x, z + 430.0f, 4.5f, st);
        ground_strip(x - 20.0f, z, x + 20.0f, z, 20.0f, sq);
        ground_strip(x - 430.0f, z, x + 430.0f, z, 0.25f, RGBA8(226, 200, 70, 255));
        ground_strip(x, z - 430.0f, x, z + 430.0f, 0.25f, RGBA8(226, 200, 70, 255));
    }
}

// Bridges: a raised deck on piers, abutments at both ends, sloping approach ramps with earth banks, parapets and a centre line.
static void draw_bridges(void) {
    const float H = 2.4f, RAMP = 30.0f;
    const unsigned int road = RGBA8(90, 90, 96, 255), conc = RGBA8(150, 150, 156, 255), conc_d = RGBA8(112, 112, 118, 255),
                       earth = RGBA8(112, 100, 74, 255), line = RGBA8(226, 200, 70, 255);
    for (int i = 0; i < nbridges; i++) {
        float dx = br_x[i] - cam.pos.x, dz = br_z[i] - cam.pos.z;
        if (dx * dx + dz * dz > 6000.0f * 6000.0f) continue;
        float L = br_len[i] - 36.0f, hw = br_hw[i];                                    // L = the deck over the river and its banks
        float ux = br_dx[i], uz = br_dz[i], px = -uz, pz = ux, cx = br_x[i], cz = br_z[i];
        #define BP(a, b, y) v3(cx + ux * (a) + px * (b), (y), cz + uz * (a) + pz * (b))
        // Piers: two rows of two square columns standing in the river.
        for (int r = -1; r <= 1; r += 2)
            for (int c = -1; c <= 1; c += 2) {
                float a0 = r * L * 0.24f - 0.9f, a1 = a0 + 1.8f, b0 = c * (hw - 2.2f) - 0.9f, b1 = b0 + 1.8f;
                V3 f0[4] = { BP(a0, b0, -1.0f), BP(a1, b0, -1.0f), BP(a1, b0, H), BP(a0, b0, H) };
                V3 f1[4] = { BP(a0, b1, -1.0f), BP(a1, b1, -1.0f), BP(a1, b1, H), BP(a0, b1, H) };
                V3 f2[4] = { BP(a0, b0, -1.0f), BP(a0, b1, -1.0f), BP(a0, b1, H), BP(a0, b0, H) };
                V3 f3[4] = { BP(a1, b0, -1.0f), BP(a1, b1, -1.0f), BP(a1, b1, H), BP(a1, b0, H) };
                draw_world_poly(f0, 4, conc_d); draw_world_poly(f1, 4, conc_d); draw_world_poly(f2, 4, conc); draw_world_poly(f3, 4, conc);
            }
        for (int e = -1; e <= 1; e += 2) {                                             // abutments and approach ramps
            float a0 = e * L * 0.5f, a1 = e * (L * 0.5f + RAMP);
            V3 ab[4] = { BP(a0, -hw, 0.0f), BP(a0, hw, 0.0f), BP(a0, hw, H), BP(a0, -hw, H) };            // end wall under the deck
            draw_world_poly(ab, 4, conc);
            V3 ramp[4] = { BP(a1, -hw, 0.03f), BP(a1, hw, 0.03f), BP(a0, hw, H), BP(a0, -hw, H) };
            draw_world_poly(ramp, 4, road);
            for (int s = -1; s <= 1; s += 2) {                                          // earth banks at the sides of the ramp
                V3 bk[3] = { BP(a1, s * hw, 0.03f), BP(a0, s * hw, H), BP(a0, s * hw, 0.0f) };
                draw_world_poly(bk, 3, earth);
                V3 bk2[3] = { BP(a1, s * hw, 0.03f), BP(a0, s * hw, H), BP(a0, s * (hw + 3.0f), 0.0f) };
                draw_world_poly(bk2, 3, earth);
            }
        }
        for (int s = -1; s <= 1; s += 2) {                                              // girders along both sides of the deck
            V3 g[4] = { BP(-L * 0.5f, s * hw, H - 1.1f), BP(L * 0.5f, s * hw, H - 1.1f), BP(L * 0.5f, s * hw, H), BP(-L * 0.5f, s * hw, H) };
            draw_world_poly(g, 4, s < 0 ? conc : conc_d);
        }
        V3 deck[4] = { BP(-L * 0.5f, -hw, H), BP(L * 0.5f, -hw, H), BP(L * 0.5f, hw, H), BP(-L * 0.5f, hw, H) };
        draw_world_poly(deck, 4, road);
        for (int k = 0; k < 6; k++) {                                                   // dashed centre line
            float a0 = -L * 0.5f + (k + 0.15f) * L / 6.0f, a1 = a0 + L / 6.0f * 0.5f;
            V3 d[4] = { BP(a0, -0.25f, H + 0.03f), BP(a1, -0.25f, H + 0.03f), BP(a1, 0.25f, H + 0.03f), BP(a0, 0.25f, H + 0.03f) };
            draw_world_poly(d, 4, line);
        }
        for (int s = -1; s <= 1; s += 2) {                                              // parapets: a low rail along each edge
            float b0 = s * (hw - 0.5f);
            V3 r1[4] = { BP(-L * 0.5f, b0 - 0.35f, H + 0.9f), BP(L * 0.5f, b0 - 0.35f, H + 0.9f), BP(L * 0.5f, b0 + 0.35f, H + 0.9f), BP(-L * 0.5f, b0 + 0.35f, H + 0.9f) };
            V3 r2[4] = { BP(-L * 0.5f, b0 + s * 0.35f, H), BP(L * 0.5f, b0 + s * 0.35f, H), BP(L * 0.5f, b0 + s * 0.35f, H + 0.9f), BP(-L * 0.5f, b0 + s * 0.35f, H + 0.9f) };
            draw_world_poly(r2, 4, conc_d);
            draw_world_poly(r1, 4, conc);
        }
        #undef BP
    }
}

static void draw_river(void) {
    if (nriver < 2) return;
    for (int i = 0; i + 1 < nriver; i++) {
        float mx = (river_x[i] + river_x[i + 1]) * 0.5f - cam.pos.x, mz = (river_z[i] + river_z[i + 1]) * 0.5f - cam.pos.z;
        if (mx * mx + mz * mz > 9000.0f * 9000.0f) continue;
        float t = (float)i / (float)(nriver - 2);                                   // narrow at the source, wider towards the mouth
        float hw = 11.0f + 24.0f * t;
        ground_strip(river_x[i], river_z[i], river_x[i + 1], river_z[i + 1], hw + 10.0f, RGBA8(196, 184, 130, 255));    // sandy banks
        ground_strip(river_x[i], river_z[i], river_x[i + 1], river_z[i + 1], hw, RGBA8(52, 108, 172, 255));
    }
}

