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
    {                                                                                       // the lighthouse: a long white flash, then dark
        float lx = sea_coast_x(LH_Z) - 240.0f, dx = lx - cam.pos.x, dz = LH_Z - cam.pos.z;
        if (dx * dx + dz * dz < 40000.0f * 40000.0f && (frame_count % 150) < 40) {
            light_far = 40000.0f;
            draw_light(lx, 31.5f, LH_Z, 3.4f, RGBA8(255, 250, 220, 255));
            light_far = 9000.0f;
        }
    }
    unsigned int sodium = RGBA8(255, 196, 120, 255);                                       // lamps at both ends of every bridge
    for (int i = 0; i < nbridges; i++) {
        float dx = br_x[i] - cam.pos.x, dz = br_z[i] - cam.pos.z;
        if (dx * dx + dz * dz > 6000.0f * 6000.0f) continue;
        float L = br_len[i] - 36.0f;
        for (int e = -1; e <= 1; e += 2)
            for (int sd = -1; sd <= 1; sd += 2)
                draw_light(br_x[i] + br_dx[i] * e * L * 0.5f - br_dz[i] * sd * br_hw[i], 6.5f, br_z[i] + br_dz[i] * e * L * 0.5f + br_dx[i] * sd * br_hw[i], 1.6f, sodium);
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
    V3 p[4] = { ap_pt(&AIRPORTS[AP_REDROCK], -34.0f, 0.0f, -700.0f), ap_pt(&AIRPORTS[AP_PINECREST], 24.0f, 0.0f, -110.0f),
                ap_pt(&AIRPORTS[AP_LANA], -34.0f, 0.0f, -560.0f), ap_pt(&AIRPORTS[AP_SPACE], -34.0f, 0.0f, -700.0f) };
    for (int i = 0; i < 4; i++) {
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

// A strip beside the line from A to B, off to one side by `off` meters (left of the direction of travel when positive).
static void strip_off(float ax, float az, float bx, float bz, float off, float half, unsigned int col) {
    float dx = bx - ax, dz = bz - az, len = sqrtf(dx * dx + dz * dz);
    if (len < 0.01f) return;
    float nx = -dz / len * off, nz = dx / len * off;
    ground_strip(ax + nx, az + nz, bx + nx, bz + nz, half, col);
}
// A round patch, so two road segments meeting at a bend join up instead of leaving a notch.
static void ground_disc(float x, float z, float r, unsigned int col) {
    V3 p[8];
    for (int k = 0; k < 8; k++) { float a = (k + 0.5f) * PI_F / 4.0f; p[k] = v3(x + r * 1.04f * cosf(a), 0.03f, z + r * 1.04f * sinf(a)); }
    draw_world_poly(p, 8, col);
}
// Dashes along A to B: `dash` meters of paint every `pitch` meters, only the ones within `reach` of the camera.
static void ground_dashes(float ax, float az, float bx, float bz, float half, float dash, float pitch, float reach, unsigned int col) {
    float dx = bx - ax, dz = bz - az, len = sqrtf(dx * dx + dz * dz);
    if (len < 1.0f) return;
    float ux = dx / len, uz = dz / len;
    float t0 = fmaxf(0.0f, (cam.pos.x - ax) * ux + (cam.pos.z - az) * uz - reach), t1 = fminf(len, t0 + 2.0f * reach);
    for (float t = floorf(t0 / pitch) * pitch; t < t1; t += pitch) {
        float u0 = fmaxf(t, 0.0f), u1 = fminf(t + dash, len);
        if (u1 > u0) ground_strip(ax + ux * u0, az + uz * u0, ax + ux * u1, az + uz * u1, half, col);
    }
}
static const unsigned int PAINT_W = RGBA8(228, 228, 230, 255), PAINT_Y = RGBA8(226, 200, 70, 255);

// The two long roads past the airfield: a gravel shoulder under the tarmac, white edge lines and a dashed center line on top.
static void draw_main_road_shoulders(void) {
    const unsigned int sh = RGBA8(120, 114, 98, 255);
    ground_strip(ROAD_X, cam.pos.z - 3000.0f, ROAD_X, cam.pos.z + 3000.0f, 6.2f, sh);
    ground_strip(cam.pos.x - 3000.0f, ROAD_Z, cam.pos.x + 3000.0f, ROAD_Z, 6.2f, sh);
}
static void draw_main_road_marks(void) {
    float dx = ROAD_X - cam.pos.x, dz = ROAD_Z - cam.pos.z;
    if (dx * dx > 2500.0f * 2500.0f && dz * dz > 2500.0f * 2500.0f) return;
    if (fabsf(dx) < 2500.0f) {
        float z0 = cam.pos.z - 2000.0f, z1 = cam.pos.z + 2000.0f;
        ground_strip(ROAD_X - 3.7f, z0, ROAD_X - 3.7f, z1, 0.17f, PAINT_W);
        ground_strip(ROAD_X + 3.7f, z0, ROAD_X + 3.7f, z1, 0.17f, PAINT_W);
        ground_dashes(ROAD_X, z0, ROAD_X, z1, 0.17f, 6.0f, 18.0f, 700.0f, PAINT_Y);
    }
    if (fabsf(dz) < 2500.0f) {
        float x0 = cam.pos.x - 2000.0f, x1 = cam.pos.x + 2000.0f;
        ground_strip(x0, ROAD_Z - 3.7f, x1, ROAD_Z - 3.7f, 0.17f, PAINT_W);
        ground_strip(x0, ROAD_Z + 3.7f, x1, ROAD_Z + 3.7f, 0.17f, PAINT_W);
        ground_dashes(x0, ROAD_Z, x1, ROAD_Z, 0.17f, 6.0f, 18.0f, 700.0f, PAINT_Y);
    }
    if (fabsf(dx) < 140.0f && fabsf(dz) < 140.0f) {                                 // the crossing: tarmac over the lines
        ground_strip(ROAD_X - 5.0f, ROAD_Z, ROAD_X + 5.0f, ROAD_Z, 5.0f, RGBA8(86, 86, 92, 255));
        for (int k = -4; k <= 4; k++) {                                              // stop lines
            ground_strip(ROAD_X + k * 0.8f, ROAD_Z + 7.0f, ROAD_X + k * 0.8f, ROAD_Z + 8.2f, 0.3f, PAINT_W);
            ground_strip(ROAD_X + k * 0.8f, ROAD_Z - 7.0f, ROAD_X + k * 0.8f, ROAD_Z - 8.2f, 0.3f, PAINT_W);
        }
    }
}

static void draw_highway(void) {
    const Airport *b = &AIRPORTS[AP_REDROCK];
    V3 end = ap_pt(b, 330.0f, 0.0f, -220.0f);
    float x0 = 704.0f, z0 = -300.0f, dx = end.x - x0, dz = end.z - z0, L = sqrtf(dx * dx + dz * dz);
    int n = (int)(L / 500.0f);
    const unsigned int road = RGBA8(84, 84, 90, 255), shoulder = RGBA8(122, 116, 100, 255);
    for (int pass = 0; pass < 2; pass++)
        for (int i = 0; i < n; i++) {
            float t0 = (float)i / n, t1 = (float)(i + 1) / n;
            float ax = x0 + dx * t0, az = z0 + dz * t0, bx = x0 + dx * t1, bz = z0 + dz * t1;
            float mx = (ax + bx) * 0.5f - cam.pos.x, mz = (az + bz) * 0.5f - cam.pos.z;
            float d2 = mx * mx + mz * mz;
            if (d2 > 8000.0f * 8000.0f) continue;
            if (pass == 0) { if (d2 < 4500.0f * 4500.0f) ground_strip(ax, az, bx, bz, 7.4f, shoulder); continue; }       // a wide gravel shoulder under the tarmac
            ground_strip(ax, az, bx, bz, 5.5f, road);
            if (d2 < 3000.0f * 3000.0f) {
                strip_off(ax, az, bx, bz, 4.7f, 0.2f, PAINT_W);                                                           // edge lines
                strip_off(ax, az, bx, bz, -4.7f, 0.2f, PAINT_W);
                strip_off(ax, az, bx, bz, 0.3f, 0.12f, PAINT_Y);                                                          // double yellow center, broken near
                strip_off(ax, az, bx, bz, -0.3f, 0.12f, PAINT_Y);
            }
        }
}

// Country roads (built in build_countryside): paved, or gravel.
static void draw_country_roads(void) {
    const unsigned int tar = RGBA8(86, 86, 92, 255), gravel = RGBA8(150, 124, 86, 255);
    for (int pass = 0; pass < 3; pass++)
        for (int i = 0; i + 1 < nroadp; i++) {
            if (road_new[i + 1]) continue;
            float mx = (road_x[i] + road_x[i + 1]) * 0.5f - cam.pos.x, mz = (road_z[i] + road_z[i + 1]) * 0.5f - cam.pos.z, d2 = mx * mx + mz * mz;
            if (d2 > 7000.0f * 7000.0f) continue;
            bool dirt = road_dirt[i] != 0;
            float half = dirt ? 3.0f : 4.5f, ax = road_x[i], az = road_z[i], bx = road_x[i + 1], bz = road_z[i + 1];
            if (pass == 0) {                                                       // verge: gravel round tarmac, dust round a track
                if (d2 > 3500.0f * 3500.0f) continue;
                unsigned int vc = dirt ? RGBA8(168, 146, 106, 255) : RGBA8(118, 112, 98, 255);
                ground_strip(ax, az, bx, bz, half + 1.5f, vc);
                ground_disc(ax, az, half + 1.5f, vc);
                if (i + 2 >= nroadp || road_new[i + 2]) ground_disc(bx, bz, half + 1.5f, vc);
            } else if (pass == 1) {
                unsigned int rc = dirt ? gravel : tar;
                ground_strip(ax, az, bx, bz, half, rc);
                ground_disc(ax, az, half, rc);
                if (i + 2 >= nroadp || road_new[i + 2]) ground_disc(bx, bz, half, rc);
            } else if (d2 < 1500.0f * 1500.0f) {
                if (dirt) {                                                        // two wheel ruts and grass between them
                    strip_off(ax, az, bx, bz, 1.0f, 0.4f, RGBA8(116, 92, 62, 255));
                    strip_off(ax, az, bx, bz, -1.0f, 0.4f, RGBA8(116, 92, 62, 255));
                    strip_off(ax, az, bx, bz, 0.0f, 0.35f, RGBA8(112, 128, 74, 255));
                } else {
                    strip_off(ax, az, bx, bz, 3.6f, 0.15f, PAINT_W);
                    strip_off(ax, az, bx, bz, -3.6f, 0.15f, PAINT_W);
                    ground_dashes(ax, az, bx, bz, 0.15f, 5.0f, 15.0f, 600.0f, PAINT_Y);
                }
            }
        }
}

// Knowles Works' ground: the apron, the road in from the main road, car parks, a solar field and the road to the village.
static void draw_knowles_ground(void) {
    float dx = KW_FX - cam.pos.x, dz = KW_FZ - cam.pos.z, d2 = dx * dx + dz * dz;
    if (d2 > 3500.0f * 3500.0f) return;
    const float FX = KW_FX, FZ = KW_FZ;
    const unsigned int road = RGBA8(86, 86, 92, 255), conc = RGBA8(156, 156, 160, 255), asph = RGBA8(64, 64, 70, 255), line = RGBA8(226, 200, 70, 255);
    ground_strip(FX - 350.0f, FZ - 25.0f, FX + 350.0f, FZ - 25.0f, 178.0f, conc);                  // the apron round the hall
    ground_strip(700.0f, FZ, FX - KW_HW - 80.0f, FZ, 6.0f, road);                                  // in from the north-south road
    ground_strip(700.0f, FZ, FX - KW_HW - 80.0f, FZ, 0.25f, line);
    ground_strip(FX + KW_HW + 120.0f, KW_VZ, KW_VX - 430.0f, KW_VZ, 4.5f, road);                   // out to the village
    if (d2 > 2600.0f * 2600.0f) return;
    for (int r = 0; r < 34; r++)                                                                   // solar field south of the loading dock
        ground_strip(FX - 300.0f, FZ - 240.0f - r * 10.0f, FX + 300.0f, FZ - 240.0f - r * 10.0f, 3.2f, RGBA8(36, 52, 92, 255));
    ground_strip(FX - 330.0f, FZ + 175.0f, FX + 330.0f, FZ + 175.0f, 0.0f + 28.0f, asph);          // the car park on the north side
    for (int k = 0; k <= 11; k++)
        ground_strip(FX - 330.0f + k * 60.0f, FZ + 150.0f, FX - 330.0f + k * 60.0f, FZ + 200.0f, 0.2f, RGBA8(230, 230, 232, 255));
    if (d2 > 900.0f * 900.0f) return;
    for (int r = 0; r < 4; r++)                                                                    // cars, a part-full lot
        for (int c = 0; c < 110; c++) {
            unsigned int h = hash2(c * 13 + 5, r * 29 + 7);
            if ((h & 7) > 4) continue;
            float cx = FX - 322.0f + c * 5.9f, cz = FZ + 156.0f + r * 13.0f;
            if ((r & 1) == 1) cz += 3.0f;
            float ddx = cx - cam.pos.x, ddz = cz - cam.pos.z;
            if (ddx * ddx + ddz * ddz > 700.0f * 700.0f) continue;
            static const unsigned int CAR[6] = { RGBA8(200, 200, 204, 255), RGBA8(60, 62, 70, 255), RGBA8(176, 40, 44, 255), RGBA8(46, 82, 150, 255), RGBA8(150, 152, 158, 255), RGBA8(226, 226, 230, 255) };
            ground_strip(cx, cz - 2.1f, cx, cz + 2.1f, 1.0f, CAR[(h >> 8) % 6]);
        }
}

// Downtown Knowles: the ground, streets, crossings and cars of the city. Everything follows city_d(), so the streets and blocks fade out
// raggedly instead of stopping at a straight edge. Blocks and street stretches that sit side by side are drawn as one long strip.
static bool city_street_seg(bool avenue, int line, int seg) {
    if (avenue) return line >= 0 && line <= CITY_NC && seg >= 0 && seg < CITY_NR && (city_av[line] >> seg & 1);
    return line >= 0 && line <= CITY_NR && seg >= 0 && seg < CITY_NC && (city_st[line] >> seg & 1);
}
static void draw_downtown_ground(void) {
#if CITY_DETAIL > 0
    float dx = CITY_CX - cam.pos.x, dz = CITY_CZ - cam.pos.z, d2 = dx * dx + dz * dz;
    if (d2 > 4800.0f * 4800.0f) return;
    const unsigned int asph = RGBA8(78, 78, 84, 255), pave = RGBA8(176, 176, 178, 255);
    const float S = CITY_S, X0 = CITY_X0, Z0 = CITY_Z0;
    for (int r = 0; r < CITY_NR; r++) {                                               // the ground under the blocks: lawn at the edge, paving in the middle
        float zc = Z0 + (r + 0.5f) * S;
        if (fabsf(zc - cam.pos.z) > 2600.0f) continue;
        int run = 0, c0 = 0, lv0 = 0;
        for (int c = 0; c <= CITY_NC; c++) {
            int lv = 0;
            if (c < CITY_NC) { float dd = city_d(X0 + (c + 0.5f) * S, zc); lv = dd < 0.10f ? 0 : dd < 0.30f ? 1 : dd < 0.55f ? 2 : 3; }
            if (run && lv != lv0) {
                static const unsigned int GC[4] = { 0, RGBA8(70, 150, 62, 255), RGBA8(98, 148, 88, 255), RGBA8(138, 148, 130, 255) };
                ground_strip(X0 + c0 * S, zc, X0 + c * S, zc, S * 0.5f, GC[lv0]);
                run = 0;
            }
            if (lv && !run) { run = 1; c0 = c; lv0 = lv; }
        }
    }
    {                                                                                  // the park
        float px = X0 + (CITY_PARK_C + 0.5f) * S, pz = Z0 + (CITY_PARK_R + 0.5f) * S;
        ground_strip(px - 62.0f, pz, px + 62.0f, pz, 62.0f, RGBA8(70, 150, 62, 255));
        ground_strip(px - 62.0f, pz, px + 62.0f, pz, 2.2f, RGBA8(200, 190, 160, 255));
        ground_strip(px, pz - 62.0f, px, pz + 62.0f, 2.2f, RGBA8(200, 190, 160, 255));
        ground_disc(px + 26.0f, pz - 24.0f, 14.0f, RGBA8(70, 120, 176, 255));
        ground_disc(px, pz, 5.0f, RGBA8(200, 190, 160, 255));
    }
    for (int pass = 0; pass < 3; pass++) {
        if (pass == 2 && d2 > 1500.0f * 1500.0f) break;
        for (int k = 0; k < CITY_NC + CITY_NR + 2; k++) {
            bool avenue = k <= CITY_NC;
            int line = avenue ? k : k - CITY_NC - 1, nseg = avenue ? CITY_NR : CITY_NC;
            float pos = avenue ? X0 + line * S : Z0 + line * S, base = avenue ? Z0 : X0;
            float mid = avenue ? pos - cam.pos.x : pos - cam.pos.z;
            if (fabsf(mid) > 1800.0f) continue;
            int run = 0, s0 = 0;
            for (int sg = 0; sg <= nseg; sg++) {
                bool on = sg < nseg && city_street_seg(avenue, line, sg);
                if (run && !on) {
                    float a0 = base + s0 * S - 6.0f, a1 = base + sg * S + 6.0f;
                    float amid = (a0 + a1) * 0.5f - (avenue ? cam.pos.z : cam.pos.x);
                    if (fabsf(amid) < 1800.0f + (a1 - a0) * 0.5f) {
                        #define SEG(off, half, col) do { if (avenue) ground_strip(pos + (off), a0, pos + (off), a1, (half), (col)); else ground_strip(a0, pos + (off), a1, pos + (off), (half), (col)); } while (0)
                        if (pass == 0) { SEG(-8.2f, 2.2f, pave); SEG(8.2f, 2.2f, pave); }
                        else if (pass == 1) SEG(0.0f, 6.0f, asph);
                        else if (fabsf(mid) < 600.0f) {
                            if (avenue) { ground_strip(pos - 5.0f, a0, pos - 5.0f, a1, 0.14f, PAINT_W); ground_strip(pos + 5.0f, a0, pos + 5.0f, a1, 0.14f, PAINT_W); ground_dashes(pos, a0, pos, a1, 0.14f, 4.0f, 12.0f, 260.0f, PAINT_Y); }
                            else        { ground_strip(a0, pos - 5.0f, a1, pos - 5.0f, 0.14f, PAINT_W); ground_strip(a0, pos + 5.0f, a1, pos + 5.0f, 0.14f, PAINT_W); ground_dashes(a0, pos, a1, pos, 0.14f, 4.0f, 12.0f, 260.0f, PAINT_Y); }
                        }
                        #undef SEG
                    }
                    run = 0;
                }
                if (on && !run) { run = 1; s0 = sg; }
            }
        }
    }
    if (d2 < 1200.0f * 1200.0f) {                                                      // zebra crossings at the crossings close to you, and parked cars along the avenues
        for (int c = 0; c <= CITY_NC; c++)
            for (int r = 0; r <= CITY_NR; r++) {
                float x = X0 + c * S, z = Z0 + r * S, ddx = x - cam.pos.x, ddz = z - cam.pos.z;
                if (ddx * ddx + ddz * ddz > 320.0f * 320.0f) continue;
                if (!(city_street_seg(true, c, r) || city_street_seg(true, c, r - 1)) || !(city_street_seg(false, r, c) || city_street_seg(false, r, c - 1))) continue;
                for (int s = -1; s <= 1; s += 2)
                    for (int q = -2; q <= 2; q++) {
                        ground_strip(x + s * 8.5f, z + q * 2.0f, x + s * 10.5f, z + q * 2.0f, 0.6f, PAINT_W);
                        ground_strip(x + q * 2.0f, z + s * 8.5f, x + q * 2.0f, z + s * 10.5f, 0.6f, PAINT_W);
                    }
            }
        static const unsigned int CAR[6] = { RGBA8(200, 200, 204, 255), RGBA8(60, 62, 70, 255), RGBA8(176, 40, 44, 255), RGBA8(46, 82, 150, 255), RGBA8(150, 152, 158, 255), RGBA8(226, 226, 230, 255) };
        for (int c = 0; c <= CITY_NC; c++) {
            float x = X0 + c * S;
            if (fabsf(x - cam.pos.x) > 260.0f) continue;
            for (float z = floorf((cam.pos.z - 260.0f) / 14.0f) * 14.0f; z < cam.pos.z + 260.0f; z += 14.0f) {
                int sg = (int)floorf((z - Z0) / S);
                if (!city_street_seg(true, c, sg)) continue;
                unsigned int h = hash2((int)(x * 0.1f) + 3, (int)(z * 0.5f));
                if ((h & 3) > 1 || city_d(x, z) < 0.3f) continue;
                ground_strip(x + ((h & 4) ? 3.1f : -3.1f), z - 2.0f, x + ((h & 4) ? 3.1f : -3.1f), z + 2.0f, 0.95f, CAR[(h >> 8) % 6]);
            }
        }
    }
    if (d2 < 1400.0f * 1400.0f)                                                        // yards and driveways for the houses on the edge
        for (int i = 0; i < ntb; i++) {
            if (tb_town[i] != TB_CITY) continue;
            float ddx = tb_x[i] - cam.pos.x, ddz = tb_z[i] - cam.pos.z;
            if (ddx * ddx + ddz * ddz > 700.0f * 700.0f) continue;
            float bx = tb_x[i], bz = tb_z[i], w = tb_w[i], d = tb_d[i];
            unsigned int h = hash2((int)bx + 5, (int)bz - 9);
            ground_strip(bx - w * 0.5f - 4.0f, bz, bx + w * 0.5f + 4.0f, bz, d * 0.5f + 4.0f, (h & 1) ? RGBA8(78, 156, 62, 255) : RGBA8(94, 160, 66, 255));
            float zs = Z0 + roundf((bz - Z0) / S) * S;
            ground_strip(bx, bz, bx, zs + (bz > zs ? 7.0f : -7.0f), 1.3f, pave);
        }
    // roads out of town: east along the hangar row to the field, and south to the main east-west road
    ground_strip(CITY_X1, Z0 + 5.0f * S, -140.0f, Z0 + 5.0f * S, 5.5f, asph);
    ground_strip(CITY_X1, Z0 + 5.0f * S, -140.0f, Z0 + 5.0f * S, 0.15f, PAINT_Y);
    ground_strip(X0 + 7.0f * S, Z0 - 10.0f, X0 + 7.0f * S, ROAD_Z, 5.5f, asph);
    ground_strip(X0 + 7.0f * S, Z0 - 10.0f, X0 + 7.0f * S, ROAD_Z, 0.15f, PAINT_Y);
#endif
}

// Two main streets and a little square in every town; close up, also pavements, crossings, a paved square and a yard and
// path for every building.
static void draw_town_streets(void) {
    draw_knowles_ground();
    draw_downtown_ground();
    const unsigned int st = RGBA8(88, 88, 94, 255), sq = RGBA8(172, 160, 128, 255), pave = RGBA8(168, 168, 170, 255), zebra = RGBA8(230, 230, 232, 255);
    for (int n = 0; n < ntowns; n++) {
        float dx = town_x[n] - cam.pos.x, dz = town_z[n] - cam.pos.z, d2 = dx * dx + dz * dz;
        if (d2 > 6000.0f * 6000.0f) continue;
        float x = town_x[n], z = town_z[n];
        ground_strip(x - 430.0f, z, x + 430.0f, z, 4.5f, st);
        ground_strip(x, z - 430.0f, x, z + 430.0f, 4.5f, st);
        ground_strip(x - 20.0f, z, x + 20.0f, z, 20.0f, sq);
        ground_strip(x - 430.0f, z, x + 430.0f, z, 0.25f, RGBA8(226, 200, 70, 255));
        ground_strip(x, z - 430.0f, x, z + 430.0f, 0.25f, RGBA8(226, 200, 70, 255));
        if (d2 > 1900.0f * 1900.0f) continue;
        ground_strip(x - 150.0f, z - 6.4f, x + 150.0f, z - 6.4f, 1.3f, pave);          // pavements along the main streets
        ground_strip(x - 150.0f, z + 6.4f, x + 150.0f, z + 6.4f, 1.3f, pave);
        ground_strip(x - 6.4f, z - 150.0f, x - 6.4f, z + 150.0f, 1.3f, pave);
        ground_strip(x + 6.4f, z - 150.0f, x + 6.4f, z + 150.0f, 1.3f, pave);
        for (int s = -1; s <= 1; s += 2)                                              // zebra crossings at the square
            for (int k = -3; k <= 3; k++) {
                ground_strip(x + s * 24.0f, z + k * 1.3f, x + s * 28.0f, z + k * 1.3f, 0.5f, zebra);
                ground_strip(x + k * 1.3f, z + s * 24.0f, x + k * 1.3f, z + s * 28.0f, 0.5f, zebra);
            }
        ground_strip(x - 12.0f, z, x + 12.0f, z, 12.0f, RGBA8(150, 140, 112, 255));    // the paved middle of the square
        ground_strip(x - 3.0f, z, x + 3.0f, z, 3.0f, RGBA8(84, 130, 170, 255));        // a fountain basin
        for (int i = 0; i < ntb; i++) {
            if (tb_town[i] != n) continue;
            float bx = tb_x[i], bz = tb_z[i], w = tb_w[i], d = tb_d[i];
            float ddx = bx - cam.pos.x, ddz = bz - cam.pos.z;
            if (ddx * ddx + ddz * ddz > 1300.0f * 1300.0f) continue;
            unsigned int h = hash2((int)bx + 7, (int)bz + 3);
            float lw = w * 0.5f + 5.0f, ld = d * 0.5f + 5.0f;
            unsigned int lawn = (h & 1) ? RGBA8(78, 156, 62, 255) : RGBA8(94, 160, 66, 255);
            ground_strip(bx - lw, bz, bx + lw, bz, ld, lawn);                         // the yard
            float ax = fabsf(bx - x), az = fabsf(bz - z);                              // a path from the nearer main street
            if (ax < az) ground_strip(bx, bz, x + (bx > x ? 7.0f : -7.0f), bz, 1.2f, pave);
            else         ground_strip(bx, bz, bx, z + (bz > z ? 7.0f : -7.0f), 1.2f, pave);
            if (w < 30.0f && (h & 6) == 0) ground_strip(bx + w * 0.5f + 2.0f, bz - d * 0.2f, bx + w * 0.5f + 7.0f, bz - d * 0.2f, 1.5f, RGBA8(70, 70, 76, 255));   // a parked car
        }
    }
}

// Bridges: a raised deck on piers, abutments at both ends, sloping approach ramps with earth banks, parapets and a center line.
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
        for (int r = -1; r <= 1; r += 2)                                                 // foam round the piers
            for (int c = -1; c <= 1; c += 2) {
                float a0 = r * L * 0.24f, b0 = c * (hw - 2.2f);
                ground_disc(cx + ux * a0 + px * b0, cz + uz * a0 + pz * b0, 2.8f, RGBA8(196, 222, 240, 255));
            }
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
        for (int k = 0; k < 6; k++) {                                                   // dashed center line
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
        if (dx * dx + dz * dz < 1500.0f * 1500.0f) {                                    // steel trusses along both sides, the far side first
            const unsigned int steel = RGBA8(98, 106, 118, 255), steel_l = RGBA8(136, 144, 156, 255);
            float cs = (cam.pos.x - cx) * px + (cam.pos.z - cz) * pz;
            const int NP = 8;
            for (int k = 0; k < 2; k++) {
                int s = (k == 0) ? (cs > 0.0f ? -1 : 1) : (cs > 0.0f ? 1 : -1);
                float b0 = s * (hw + 0.2f), TH = H + 5.0f;
                V3 chord[4] = { BP(-L * 0.5f, b0, TH), BP(L * 0.5f, b0, TH), BP(L * 0.5f, b0, TH + 0.7f), BP(-L * 0.5f, b0, TH + 0.7f) };
                draw_world_poly(chord, 4, steel_l);
                for (int q = 0; q < NP; q++) {
                    float a0 = -L * 0.5f + L * q / NP, a1 = -L * 0.5f + L * (q + 1) / NP;
                    float rise0 = (q & 1) ? H + 1.0f : TH, rise1 = (q & 1) ? TH : H + 1.0f;
                    V3 dg[4] = { BP(a0 - 0.3f, b0, rise0 == TH ? TH : H + 1.0f), BP(a0 + 0.3f, b0, rise0 == TH ? TH : H + 1.0f), BP(a1 + 0.3f, b0, rise1 == TH ? TH : H + 1.0f), BP(a1 - 0.3f, b0, rise1 == TH ? TH : H + 1.0f) };
                    draw_world_poly(dg, 4, steel);
                }
                for (int q = 0; q <= NP; q++) {
                    float a0 = -L * 0.5f + L * q / NP;
                    V3 post[4] = { BP(a0 - 0.3f, b0, H + 1.0f), BP(a0 + 0.3f, b0, H + 1.0f), BP(a0 + 0.3f, b0, TH), BP(a0 - 0.3f, b0, TH) };
                    draw_world_poly(post, 4, steel_l);
                }
            }
            for (int q = 0; q <= NP; q += 2) {                                          // cross beams over the deck
                float a0 = -L * 0.5f + L * q / NP;
                V3 beam[4] = { BP(a0 - 0.3f, -hw, (H + 5.0f)), BP(a0 + 0.3f, -hw, (H + 5.0f)), BP(a0 + 0.3f, hw, (H + 5.0f)), BP(a0 - 0.3f, hw, (H + 5.0f)) };
                draw_world_poly(beam, 4, steel);
            }
        }
        #undef BP
    }
}

static float river_hw(int i) { return 11.0f + 24.0f * (float)i / (float)(nriver - 2); }     // narrow at the source, wider toward the mouth
static void draw_river(void) {
    if (nriver < 2) return;
    const unsigned int verge = RGBA8(84, 138, 72, 255), sand = RGBA8(196, 184, 130, 255), shallow = RGBA8(86, 142, 196, 255), deep = RGBA8(46, 98, 164, 255);
    for (int pass = 0; pass < 5; pass++)
        for (int i = 0; i + 1 < nriver; i++) {
            float ax = river_x[i], az = river_z[i], bx = river_x[i + 1], bz = river_z[i + 1];
            float mx = (ax + bx) * 0.5f - cam.pos.x, mz = (az + bz) * 0.5f - cam.pos.z, d2 = mx * mx + mz * mz;
            if (d2 > 9000.0f * 9000.0f) continue;
            float hw = river_hw(i), hw2 = river_hw(i + 1 < nriver - 1 ? i + 1 : i);
            unsigned int col = 0; float w = 0.0f;
            if (pass == 0) { if (d2 > 5500.0f * 5500.0f) continue; col = verge; w = hw + 26.0f; }          // wet grass beside the banks
            else if (pass == 1) { col = sand; w = hw + 10.0f; }                                              // sandy banks
            else if (pass == 2) { col = shallow; w = hw; }                                                   // the water, pale at the edges
            else if (pass == 3) { if (d2 > 4000.0f * 4000.0f) continue; col = deep; w = hw * 0.62f; }       // and dark in the middle
            else {
                if (d2 > 2200.0f * 2200.0f) continue;                                                        // sandbars and streaks of foam, close up only
                unsigned int h = hash2(i * 31 + 7, 11);
                if ((h & 3) == 0) {
                    float t = 0.35f + 0.3f * hf(h, 1), px = ax + (bx - ax) * t, pz = az + (bz - az) * t;
                    float dx = bx - ax, dz = bz - az, len = sqrtf(dx * dx + dz * dz), side = (h & 4) ? 1.0f : -1.0f;
                    float off = hw * 0.58f * side;
                    ground_disc(px - dz / len * off, pz + dx / len * off, hw * 0.34f, sand);
                }
                for (int k = 0; k < 3; k++) {
                    float t0 = 0.1f + 0.3f * k + 0.1f * hf(h, 2), t1 = t0 + 0.1f;
                    float off = hw * (0.25f - 0.25f * k) * ((h >> (3 + k)) & 1 ? 1.0f : -1.0f);
                    strip_off(ax + (bx - ax) * t0, az + (bz - az) * t0, ax + (bx - ax) * t1, az + (bz - az) * t1, off, 0.5f, RGBA8(170, 208, 236, 255));
                }
                continue;
            }
            ground_strip(ax, az, bx, bz, w, col);
            ground_disc(ax, az, w, col);
            if (i + 2 == nriver) ground_disc(bx, bz, w, col);
        }
}

