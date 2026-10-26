// Left Seat: the east coast and the hidden spaceport (Gulf Point): a long way from home, off the chart and the airport list until you
// have flown there. Two catch towers (one with a rocket ready on its mount), a tank farm, the build site with its bays, a rocket
// garden and a runway. The sea runs along the whole east side of the map.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* The sea                                                              */
/* ------------------------------------------------------------------ */

#define SEA_X 36300.0f                                  // where the coast runs (east of home), before the wiggle
#define SP_Z  (AIRPORTS[AP_SPACE].cz)

// The coast: nearly straight, with a gentle wiggle that fades out near the spaceport so the sites stand well back from the water.
static float sea_coast_x(float z) {
    float f = clampf((fabsf(z - SP_Z) - 2500.0f) / 4000.0f, 0.0f, 1.0f);
    return SEA_X + f * (380.0f * sinf(z * 0.00041f + 0.7f) + 150.0f * sinf(z * 0.00065f + 2.0f));
}
static float sea_depth(float x, float z) { return x - sea_coast_x(z); }          // metres into the sea (negative: on land)

static unsigned int sea_col(float d) {                                           // shallows are pale, the deep sea dark
    float k = clampf(d / 2600.0f, 0.0f, 1.0f);
    k = k * (2.0f - k);
    return RGBA8((int)(104 + (34 - 104) * k), (int)(178 + (90 - 178) * k), (int)(188 + (150 - 188) * k), 255);
}

static void sea_band(float z0, float z1, float e0, float e1) {
    V3 p[4] = { v3(sea_coast_x(z0) + e0, 0.03f, z0), v3(sea_coast_x(z0) + e1, 0.03f, z0), v3(sea_coast_x(z1) + e1, 0.03f, z1), v3(sea_coast_x(z1) + e0, 0.03f, z1) };
    draw_world_poly(p, 4, sea_col(sqrtf(fmaxf(e0, 0.0f) * fmaxf(e1, 1.0f))));
}
static void sea_strip(float z0, float z1, float e0, float e1, unsigned int col) {
    V3 p[4] = { v3(sea_coast_x(z0) + e0, 0.04f, z0), v3(sea_coast_x(z0) + e1, 0.04f, z0), v3(sea_coast_x(z1) + e1, 0.04f, z1), v3(sea_coast_x(z1) + e0, 0.04f, z1) };
    draw_world_poly(p, 4, col);
}

static void draw_sea(void) {
    if (cam.pos.x < SEA_X - 60000.0f) return;                                    // too far away to see
    static const float E8[9] = { 0.0f, 260.0f, 800.0f, 1800.0f, 3600.0f, 7000.0f, 14000.0f, 30000.0f, 80000.0f };
    static const float E4[5] = { 0.0f, 1800.0f, 8000.0f, 30000.0f, 80000.0f };
    float zc = floorf(cam.pos.z / 1600.0f) * 1600.0f;
    float rows[40];
    int nr = 0;
    rows[nr++] = zc - 64000.0f;
    for (float z = zc - 16000.0f; z < zc - 4800.0f - 1.0f; z += 3200.0f) rows[nr++] = z;
    for (float z = zc - 4800.0f; z <= zc + 6400.0f + 1.0f; z += 1600.0f) rows[nr++] = z;
    for (float z = zc + 6400.0f + 3200.0f; z <= zc + 16000.0f + 1.0f; z += 3200.0f) rows[nr++] = z;
    rows[nr++] = zc + 64000.0f;
    for (int r = 0; r + 1 < nr; r++) {
        float z0 = rows[r], z1 = rows[r + 1], dz = fabsf(0.5f * (z0 + z1) - cam.pos.z);
        if (vdot(vsub(v3(SEA_X, 0, 0.5f * (z0 + z1)), cam.pos), cam.f) < -(z1 - z0) - 9000.0f && dz > 30000.0f) continue;
        if (dz < 5600.0f) {
            for (int b = 0; b < 8; b++) sea_band(z0, z1, E8[b], E8[b + 1]);
            if (dz < 4000.0f) sea_strip(z0, z1, 0.0f, 12.0f, RGBA8(240, 244, 244, 255));   // surf
        } else
            for (int b = 0; b < 4; b++) sea_band(z0, z1, E4[b], E4[b + 1]);
        if (dz < 17000.0f) sea_strip(z0, z1, -150.0f, 0.0f, RGBA8(218, 202, 154, 255));       // the beach
    }
}

/* ------------------------------------------------------------------ */
/* Finding the spaceport                                                */
/* ------------------------------------------------------------------ */

static bool ap_shown(int k) { return k != AP_SPACE || ap_found; }
static int  ap_next(int k, int dir) {                                            // the next airport you know about
    for (int i = 0; i < AP_COUNT; i++) { k = (k + dir + AP_COUNT) % AP_COUNT; if (ap_shown(k)) return k; }
    return k;
}
static int  ap_nshown(void) { int n = 0; for (int k = 0; k < AP_COUNT; k++) n += ap_shown(k); return n; }
static void space_check_found(void) {
    if (ap_found) return;
    if (sat_dist(pl.pos.x, pl.pos.z, AIRPORTS[AP_SPACE].cx + 600.0f, AIRPORTS[AP_SPACE].cz) < 4200.0f) {
        ap_found = 1;
        found_msg_t = 330;
        scores_save();
    }
}

/* ------------------------------------------------------------------ */
/* Building blocks: tubes, cones and elevated boxes in world coordinates */
/* ------------------------------------------------------------------ */

static void sp_cyl(float x, float z, float r, float y0, float y1, float cr, float cg, float cb, int dark) {   // 8-sided tube with a flat top
    V3 b[8], t[8];
    for (int i = 0; i < 8; i++) {
        float a = (i + 0.5f) * PI_F / 4.0f;
        b[i] = v3(x + r * cosf(a), y0, z + r * sinf(a));
        t[i] = v3(x + r * cosf(a), y1, z + r * sinf(a));
    }
    for (int i = 0; i < 8; i++) {
        int j = (i + 1) & 7;
        V3 q[4] = { b[i], b[j], t[j], t[i] };
        float k = (dark && i >= 2 && i <= 5) ? 0.18f : 1.0f;                       // a dark side: the heat-shield tiles
        add_face(4, q, cr * k, cg * k, cb * k);
    }
    V3 c1[4] = { t[0], t[1], t[2], t[3] }, c2[4] = { t[0], t[3], t[4], t[5] }, c3[4] = { t[0], t[5], t[6], t[7] };
    add_face(4, c1, cr * 1.1f, cg * 1.1f, cb * 1.1f); add_face(4, c2, cr * 1.1f, cg * 1.1f, cb * 1.1f); add_face(4, c3, cr * 1.1f, cg * 1.1f, cb * 1.1f);
}
static void sp_cone(float x, float z, float r, float y0, float y1, float cr, float cg, float cb) {
    V3 b[8], ap = v3(x, y1, z);
    for (int i = 0; i < 8; i++) { float a = (i + 0.5f) * PI_F / 4.0f; b[i] = v3(x + r * cosf(a), y0, z + r * sinf(a)); }
    for (int i = 0; i < 8; i++) { V3 q[3] = { b[i], b[(i + 1) & 7], ap }; add_face(3, q, cr, cg, cb); }
}
static void sp_col(const Airport *a, float lx, float lz, float w, float d, float h) {   // a collision box (the faces are drawn elsewhere)
    V3 c0 = ap_pt(a, lx, 0.0f, lz);
    if (nboxes < MAX_BOXES) boxes[nboxes++] = (Box){ c0.x, c0.z, fabsf(w * a->c) + fabsf(d * a->s), fabsf(w * a->s) + fabsf(d * a->c), h };
}
static void sp_box(const Airport *a, float lx, float lz, float w, float d, float y0, float y1, float r, float g, float b) {
    V3 c0 = ap_pt(a, lx, 0.0f, lz);
    add_box_rot(c0.x, c0.z, w, d, y0, y1, a, r, g, b);
}

// A catch tower: a tall steel tower with a band every 14 m, and the two arms that catch a booster.
static void sp_tower(const Airport *a, float lx, float lz) {
    const float H = 146.0f;
    sp_box(a, lx, lz, 6.0f, 6.0f, 0.0f, H, 168, 172, 182);
    for (int i = 0; i < 4; i++)
        sp_box(a, lx + ((i & 1) ? 4.6f : -4.6f), lz + ((i & 2) ? 4.6f : -4.6f), 1.6f, 1.6f, 0.0f, H, 92, 96, 106);
    for (float y = 8.0f; y < H - 4.0f; y += 14.0f) sp_box(a, lx, lz, 11.6f, 11.6f, y, y + 0.9f, 128, 132, 144);
    sp_box(a, lx, lz, 9.0f, 9.0f, H, H + 4.0f, 150, 154, 164);
    sp_box(a, lx, lz, 0.7f, 0.7f, H + 4.0f, H + 18.0f, 200, 60, 50);                                       // mast
    sp_box(a, lx, lz, 7.0f, 25.0f, 76.0f, 81.0f, 150, 154, 166);                                           // the carriage
    for (int s = -1; s <= 1; s += 2) {
        sp_box(a, lx - 16.0f, lz + s * 10.0f, 22.0f, 3.2f, 77.0f, 80.4f, 190, 194, 204);                  // the arms, open
        sp_box(a, lx - 28.5f, lz + s * 8.8f, 3.0f, 3.4f, 76.5f, 80.9f, 120, 124, 136);                    // their tips
    }
    sp_col(a, lx, lz, 12.0f, 12.0f, H);
}
// The launch mount: a deck on four legs over a flame pit.
static void sp_mount(const Airport *a, float lx, float lz) {
    sp_box(a, lx, lz, 26.0f, 26.0f, 12.0f, 16.0f, 150, 150, 146);
    for (int i = 0; i < 4; i++) sp_box(a, lx + ((i & 1) ? 10.0f : -10.0f), lz + ((i & 2) ? 10.0f : -10.0f), 3.6f, 3.6f, 0.0f, 12.0f, 120, 122, 126);
    sp_col(a, lx, lz, 26.0f, 26.0f, 16.0f);
}
// A stainless steel rocket on the mount: booster, ship with its dark side, nose cone, and the fins.
static void sp_rocket(const Airport *a, float lx, float lz) {
    V3 c = ap_pt(a, lx, 0.0f, lz);
    sp_cyl(c.x, c.z, 4.7f, 16.0f, 21.0f, 40, 42, 48, 0);                                                   // engine skirt
    sp_cyl(c.x, c.z, 4.5f, 21.0f, 80.0f, 198, 202, 212, 0);                                                // booster
    sp_cyl(c.x, c.z, 4.55f, 80.0f, 86.0f, 44, 46, 54, 0);                                                  // interstage
    sp_cyl(c.x, c.z, 4.5f, 86.0f, 122.0f, 204, 208, 216, 1);                                               // ship
    sp_cone(c.x, c.z, 4.5f, 122.0f, 136.0f, 192, 196, 206);                                                // nose
    for (int i = 0; i < 4; i++) {
        float ox = (i & 1) ? (i < 2 ? 6.4f : -6.4f) : 0.0f, oz = (i & 1) ? 0.0f : (i < 2 ? 6.4f : -6.4f);
        sp_box(a, lx + ox, lz + oz, (i & 1) ? 4.0f : 0.5f, (i & 1) ? 0.5f : 4.0f, 76.0f, 79.5f, 60, 64, 72);       // grid fins
        sp_box(a, lx + ox * 0.85f, lz + oz * 0.85f, (i & 1) ? 4.5f : 0.5f, (i & 1) ? 0.5f : 4.5f, 96.0f, 104.0f, 60, 64, 72);   // ship flaps
    }
    sp_col(a, lx, lz, 9.5f, 9.5f, 136.0f);
}

/* ------------------------------------------------------------------ */
/* The spaceport                                                        */
/* ------------------------------------------------------------------ */

static void build_spaceport(void) {
    const Airport *a = &AIRPORTS[AP_SPACE];
    // The airfield: a hangar, a small terminal and the tower, on the east side of the runway.
    ap_building(a, 150.0f, -200.0f, 62.0f, 56.0f, 15.0f, 205, 208, 214);
    ap_building(a, 120.0f, 30.0f, 40.0f, 18.0f, 7.0f, 196, 196, 190);
    ap_building(a, 100.0f, -60.0f, 7.0f, 7.0f, 18.0f, 214, 208, 198);
    {
        V3 c0 = ap_pt(a, 100.0f, 0.0f, -60.0f);
        add_box_rot(c0.x, c0.z, 12.0f, 12.0f, 18.0f, 23.0f, a, 80, 120, 172);                               // tower cab
    }
    // The launch site, on the beach side: two mounts with their towers; the rocket is on the first.
    sp_mount(a, 1160.0f, -120.0f); sp_tower(a, 1196.0f, -120.0f); sp_rocket(a, 1160.0f, -120.0f);
    sp_mount(a, 1160.0f, 130.0f);  sp_tower(a, 1196.0f, 130.0f);
    for (int i = 0; i < 8; i++) {                                                                           // the tank farm
        float lx = 1100.0f + (i & 1) * 18.0f, lz = -340.0f + (i >> 1) * 22.0f;
        V3 c = ap_pt(a, lx, 0.0f, lz);
        sp_cyl(c.x, c.z, 4.2f, 0.0f, 30.0f, 228, 230, 236, 0);
        sp_cone(c.x, c.z, 4.2f, 30.0f, 34.0f, 214, 216, 224);
    }
    sp_col(a, 1109.0f, -307.0f, 34.0f, 90.0f, 34.0f);
    for (int i = 0; i < 3; i++) sp_box(a, 1100.0f, 300.0f + 24.0f * i, 26.0f, 7.0f, 1.0f, 8.0f, 224, 226, 232);   // horizontal tanks
    sp_col(a, 1100.0f, 324.0f, 26.0f, 72.0f, 8.0f);
    ap_building(a, 1020.0f, -10.0f, 56.0f, 16.0f, 12.0f, 176, 186, 200);                                     // control building
    for (int i = 0; i < 6; i++) sp_box(a, 1060.0f + 11.0f * (i % 3), 90.0f + 9.0f * (i / 3), 8.0f, 3.0f, 0.0f, 3.0f, 150 + 20 * (i % 3), 160, 170);   // containers
    // The build site, inland along the road: the Mega Bay and the High Bay with their cranes, the Midbay, the long Starfactory
    // and the rocket garden.
    ap_building(a, 690.0f, 1450.0f, 76.0f, 112.0f, 58.0f, 226, 228, 234);
    sp_box(a, 690.0f, 1450.0f, 4.0f, 4.0f, 58.0f, 80.0f, 196, 198, 206);
    sp_box(a, 670.0f, 1450.0f, 40.0f, 2.4f, 78.0f, 80.4f, 196, 198, 206);
    ap_building(a, 790.0f, 1380.0f, 44.0f, 44.0f, 86.0f, 218, 222, 230);
    sp_box(a, 790.0f, 1380.0f, 5.0f, 5.0f, 86.0f, 112.0f, 196, 198, 206);
    sp_box(a, 770.0f, 1380.0f, 50.0f, 3.0f, 110.0f, 113.0f, 196, 198, 206);
    ap_building(a, 790.0f, 1480.0f, 38.0f, 50.0f, 46.0f, 214, 218, 226);
    ap_building(a, 700.0f, 1760.0f, 70.0f, 210.0f, 40.0f, 210, 214, 222);
    sp_box(a, 664.5f, 1760.0f, 1.0f, 210.0f, 16.0f, 22.0f, 40, 120, 130);                                    // a stripe along the side
    static const float GH[6] = { 38.0f, 30.0f, 34.0f, 26.0f, 38.0f, 30.0f };
    for (int i = 0; i < 6; i++) {                                                                           // the rocket garden
        V3 c = ap_pt(a, 770.0f + 17.0f * i, 0.0f, 1730.0f);
        sp_cyl(c.x, c.z, 4.5f, 0.0f, GH[i], 196, 200, 210, 0);
        if (i & 1) sp_cone(c.x, c.z, 4.5f, GH[i], GH[i] + 12.0f, 188, 192, 202);
    }
    sp_col(a, 820.0f, 1730.0f, 100.0f, 12.0f, 40.0f);
    for (int i = 0; i < 3; i++) sp_box(a, 622.0f, 1250.0f + 20.0f * i, 28.0f, 14.0f, 0.0f, 7.0f, 238, 238, 240);   // tents
    sp_box(a, 596.0f, 1230.0f, 38.0f, 1.6f, 4.0f, 11.0f, 244, 246, 248);                                     // the sign
    sp_box(a, 596.0f, 1229.0f, 38.0f, 0.3f, 7.0f, 8.6f, 40, 120, 130);

    // ---- more detail ----------------------------------------------------------------------------------------------------------------------
    // Cryogenic tank farm east of the towers: big white tanks with a dark top ring, joined to the mounts by a pipe rack.
    for (int i = 0; i < 4; i++) {
        float lz = (i < 2) ? -330.0f + i * 34.0f : 268.0f + (i - 2) * 34.0f;
        V3 c = ap_pt(a, 1238.0f, 0.0f, lz);
        sp_cyl(c.x, c.z, 8.0f, 0.0f, 22.0f, 232, 234, 240, 0);
        sp_cyl(c.x, c.z, 8.2f, 22.0f, 24.0f, 70, 78, 96, 0);
        sp_cone(c.x, c.z, 8.0f, 24.0f, 29.0f, 214, 218, 228);
    }
    sp_col(a, 1238.0f, -313.0f, 18.0f, 52.0f, 29.0f); sp_col(a, 1238.0f, 285.0f, 18.0f, 52.0f, 29.0f);
    sp_box(a, 1222.0f, -80.0f, 2.2f, 460.0f, 4.0f, 5.6f, 82, 118, 170);                                        // the trunk pipe rack
    sp_box(a, 1198.0f, -105.0f, 50.0f, 2.0f, 4.0f, 5.4f, 82, 118, 170);                                        // spurs to the mounts
    sp_box(a, 1198.0f, 140.0f, 50.0f, 2.0f, 4.0f, 5.4f, 82, 118, 170);
    sp_box(a, 1230.0f, -313.0f, 18.0f, 2.0f, 4.0f, 5.4f, 82, 118, 170);
    sp_box(a, 1230.0f, 285.0f, 18.0f, 2.0f, 4.0f, 5.4f, 82, 118, 170);
    for (int i = 0; i < 12; i++) sp_box(a, 1222.0f, -290.0f + i * 42.0f, 1.4f, 1.4f, 0.0f, 4.0f, 110, 112, 118);   // its supports
    // The small tanks get a low bund wall round them.
    sp_box(a, 1109.0f, -262.0f, 44.0f, 1.0f, 0.0f, 1.3f, 176, 174, 168); sp_box(a, 1109.0f, -352.0f, 44.0f, 1.0f, 0.0f, 1.3f, 176, 174, 168);
    sp_box(a, 1087.0f, -307.0f, 1.0f, 91.0f, 0.0f, 1.3f, 176, 174, 168); sp_box(a, 1131.0f, -307.0f, 1.0f, 91.0f, 0.0f, 1.3f, 176, 174, 168);
    // Lightning masts round the pad, the water tower for the deluge system, floodlight poles and the fence.
    for (int i = 0; i < 4; i++) {
        float lx = (i & 1) ? 1262.0f : 1084.0f, lz = (i & 2) ? 250.0f : -250.0f;
        sp_box(a, lx, lz, 0.9f, 0.9f, 0.0f, 90.0f, 150, 154, 162);
        sp_box(a, lx, lz, 2.4f, 2.4f, 90.0f, 92.0f, 200, 60, 50);
        sp_col(a, lx, lz, 1.2f, 1.2f, 92.0f);
    }
    {
        V3 c = ap_pt(a, 1252.0f, 0.0f, -200.0f);
        sp_cyl(c.x, c.z, 7.0f, 34.0f, 46.0f, 226, 228, 234, 0);
        sp_cone(c.x, c.z, 7.0f, 46.0f, 51.0f, 190, 194, 204);
        for (int i = 0; i < 4; i++) sp_box(a, 1252.0f + ((i & 1) ? 5.0f : -5.0f), -200.0f + ((i & 2) ? 5.0f : -5.0f), 1.2f, 1.2f, 0.0f, 34.0f, 120, 124, 136);
        sp_box(a, 1252.0f, -200.0f, 10.0f, 10.0f, 18.0f, 18.8f, 130, 134, 144);
        sp_col(a, 1252.0f, -200.0f, 14.0f, 14.0f, 51.0f);
    }
    for (int i = 0; i < 6; i++) {
        float lz = -360.0f + i * 144.0f;
        sp_box(a, 1090.0f, lz, 0.7f, 0.7f, 0.0f, 22.0f, 120, 124, 132);
        sp_box(a, 1090.0f, lz, 4.0f, 1.0f, 22.0f, 23.2f, 235, 235, 220);
    }
    sp_box(a, 1056.0f, 0.0f, 0.3f, 830.0f, 0.0f, 2.4f, 150, 152, 158);                                         // the fence
    sp_box(a, 1166.0f, 422.0f, 220.0f, 0.3f, 0.0f, 2.4f, 150, 152, 158);
    sp_box(a, 1166.0f, -422.0f, 220.0f, 0.3f, 0.0f, 2.4f, 150, 152, 158);
    // Trucks and vans round the pad and the control building.
    static const float TRK[8][2] = { { 1030.0f, 20.0f }, { 1038.0f, 22.0f }, { 1046.0f, -30.0f }, { 1070.0f, 10.0f }, { 1140.0f, -80.0f }, { 1150.0f, 170.0f }, { 1100.0f, 250.0f }, { 1050.0f, 60.0f } };
    for (int i = 0; i < 8; i++) sp_box(a, TRK[i][0], TRK[i][1], 5.0f, 2.2f, 0.0f, 2.0f, i & 1 ? 235 : 210, i & 1 ? 235 : 60, i & 1 ? 235 : 50);
    // Airfield: a second hangar, a fuel farm, a guard house, and a car park.
    ap_building(a, 160.0f, 150.0f, 52.0f, 40.0f, 12.0f, 200, 204, 212);
    sp_cyl(ap_pt(a, 92.0f, 0.0f, 130.0f).x, ap_pt(a, 92.0f, 0.0f, 130.0f).z, 3.0f, 0.0f, 6.0f, 230, 230, 236, 0);
    sp_cyl(ap_pt(a, 92.0f, 0.0f, 140.0f).x, ap_pt(a, 92.0f, 0.0f, 140.0f).z, 3.0f, 0.0f, 6.0f, 230, 230, 236, 0);
    ap_building(a, 62.0f, 52.0f, 5.0f, 5.0f, 3.2f, 214, 206, 188);
    for (int i = 0; i < 6; i++) sp_box(a, 66.0f + 6.0f * (i % 3), 20.0f + 12.0f * (i / 3), 4.4f, 2.0f, 0.0f, 1.6f, 60 + 40 * (i % 4), 70 + 30 * (i % 3), 90 + 20 * (i % 2));
    // The build site: an office, warehouses, a car park, fuel tanks, and a solar array down the road.
    ap_building(a, 640.0f, 1100.0f, 34.0f, 22.0f, 10.0f, 204, 208, 214);
    ap_building(a, 880.0f, 1240.0f, 44.0f, 22.0f, 9.0f, 214, 214, 208);
    ap_building(a, 880.0f, 1290.0f, 44.0f, 22.0f, 9.0f, 196, 204, 214);
    for (int i = 0; i < 3; i++) { V3 c = ap_pt(a, 900.0f + i * 11.0f, 0.0f, 1700.0f); sp_cyl(c.x, c.z, 4.5f, 0.0f, 14.0f, 226, 228, 234, 0); }
    sp_col(a, 911.0f, 1700.0f, 32.0f, 10.0f, 14.0f);
    for (int i = 0; i < 8; i++) sp_box(a, 606.0f + 7.0f * (i % 4), 1150.0f + 12.0f * (i / 4), 4.4f, 2.0f, 0.0f, 1.6f, 70 + 25 * (i % 5), 80 + 20 * (i % 3), 100 + 15 * (i % 2));
    for (int i = 0; i < 6; i++) sp_box(a, 630.0f, 760.0f + i * 14.0f, 56.0f, 5.0f, 0.8f, 2.0f, 30, 52, 112);
    for (int i = 0; i < 3; i++) sp_box(a, 880.0f + 12.0f * i, 1190.0f, 8.0f, 6.0f, 0.0f, 4.0f, 160, 164, 172);   // air-conditioning units
    // A jetty on the beach: a deck on posts, running out into the sea from the end of the beach road.
    {
        float cl = sea_coast_x(a->cz) - a->cx;
        sp_box(a, cl + 40.0f, 0.0f, 230.0f, 5.0f, 1.8f, 2.4f, 150, 124, 90);
        for (int i = 0; i < 10; i++) { sp_box(a, cl - 60.0f + i * 25.0f, 2.8f, 1.0f, 1.0f, 0.0f, 1.8f, 100, 84, 62); sp_box(a, cl - 60.0f + i * 25.0f, -2.8f, 1.0f, 1.0f, 0.0f, 1.8f, 100, 84, 62); }
        sp_box(a, cl + 150.0f, 0.0f, 14.0f, 9.0f, 2.4f, 5.5f, 220, 220, 224);                                  // a little shed at the end
    }
}

static void draw_spaceport_all(void) {
    const Airport *a = &AIRPORTS[AP_SPACE];
    const float hl = a->hl, hw = a->hw;
    const unsigned int wht = RGBA8(235, 235, 235, 255), tw = RGBA8(92, 88, 88, 255), apron = RGBA8(150, 146, 138, 255), conc = RGBA8(172, 170, 164, 255);
    float dc = sat_dist(cam.pos.x, cam.pos.z, a->cx + 600.0f, a->cz);
    if (dc > 16000.0f) return;
    ap_quad(a, -hw, -hl, hw, hl, RGBA8(72, 72, 78, 255));                    // runway
    ap_quad(a, -hw, -hl, -hw + 1.5f, hl, wht);
    ap_quad(a, hw - 1.5f, -hl, hw, hl, wht);
    for (float z = -hl + 60.0f; z < hl - 60.0f; z += 60.0f) ap_quad(a, -1.0f, z, 1.0f, z + 30.0f, wht);
    for (int e = 0; e < 2; e++) {
        float z0 = e ? hl - 40.0f : -hl + 10.0f;
        for (float x = -12.0f; x <= 12.0f; x += 6.0f) ap_quad(a, x - 1.0f, z0, x + 1.0f, z0 + 30.0f, wht);
    }
    if (dc < 5200.0f) {
        ap_quad(a, 36.0f, -hl + 20.0f, 48.0f, hl - 20.0f, tw);              // taxiway, connectors and the apron
        ap_quad(a, 15.0f, -hl + 8.0f, 36.0f, -hl + 26.0f, tw);
        ap_quad(a, 15.0f, hl - 26.0f, 36.0f, hl - 8.0f, tw);
        ap_quad(a, 15.0f, -6.0f, 36.0f, 6.0f, tw);
        ap_quad(a, 48.0f, -260.0f, 190.0f, 70.0f, apron);
        ap_quad(a, 42.0f, 36.0f, 1090.0f, 48.0f, tw);                        // the long road to the launch site
        ap_quad(a, 1075.0f, -400.0f, 1270.0f, 400.0f, conc);                 // pad apron
        ap_quad(a, 1128.0f, -152.0f, 1192.0f, -88.0f, RGBA8(88, 84, 80, 255));     // scorched concrete round the mounts
        ap_quad(a, 1128.0f, 98.0f, 1192.0f, 162.0f, RGBA8(98, 94, 90, 255));
        ap_quad(a, 1062.0f, -414.0f, 1074.0f, 414.0f, tw);                       // ring road round the pad
        ap_quad(a, 574.0f, 400.0f, 1275.0f, 414.0f, tw);                          // (the north side of it is also the road down from the build site)
        ap_quad(a, 1062.0f, -414.0f, 1275.0f, -400.0f, tw);
        ap_quad(a, 1270.0f, -6.0f, 1640.0f, 6.0f, tw);                           // the road to the beach
        ap_quad(a, 1240.0f, -230.0f, 1264.0f, -170.0f, conc);                    // water tower pad
        ap_quad(a, 1062.0f, 36.0f, 1090.0f, 48.0f, tw);
        ap_quad(a, 1148.0f, -132.0f, 1172.0f, -108.0f, RGBA8(54, 54, 58, 255));   // flame pits
        ap_quad(a, 1148.0f, 118.0f, 1172.0f, 142.0f, RGBA8(54, 54, 58, 255));
        ap_quad(a, 560.0f, 48.0f, 574.0f, 2260.0f, tw);                      // the road up to the build site
        ap_quad(a, 574.0f, 1180.0f, 920.0f, 1330.0f, conc);                  // build-site aprons
        ap_quad(a, 650.0f, 1620.0f, 900.0f, 1960.0f, conc);
        ap_quad(a, 574.0f, 1400.0f, 650.0f, 1420.0f, tw);
        ap_quad(a, 574.0f, 1740.0f, 760.0f, 1756.0f, tw);
        for (float x = 60.0f; x < 1050.0f; x += 24.0f) ap_quad(a, x, 41.7f, x + 10.0f, 42.3f, RGBA8(235, 205, 70, 255));      // centre line
        for (float z = 70.0f; z < 2200.0f; z += 24.0f) ap_quad(a, 566.7f, z, 567.3f, z + 10.0f, RGBA8(235, 205, 70, 255));
        for (float x = 600.0f; x < 1050.0f; x += 24.0f) ap_quad(a, x, 406.7f, x + 10.0f, 407.3f, RGBA8(235, 205, 70, 255));
        ap_quad(a, 922.0f, 1180.0f, 936.0f, 1980.0f, tw);                        // service roads on the build site
        ap_quad(a, 574.0f, 1330.0f, 936.0f, 1344.0f, tw);
        ap_quad(a, 574.0f, 1090.0f, 650.0f, 1100.0f, tw);
        ap_quad(a, 596.0f, 1090.0f, 660.0f, 1176.0f, conc);                      // car parks (with their bay lines)
        for (int i = 0; i < 6; i++) ap_quad(a, 603.0f + i * 10.0f, 1142.0f, 603.6f + i * 10.0f, 1166.0f, wht);
        ap_quad(a, 56.0f, 4.0f, 104.0f, 66.0f, conc);
        for (int i = 0; i < 6; i++) ap_quad(a, 62.0f + i * 7.0f, 14.0f, 62.5f + i * 7.0f, 34.0f, wht);
        ap_quad(a, 48.0f, 56.0f, 56.0f, 64.0f, tw);
        ap_quad(a, 104.0f, 120.0f, 190.0f, 175.0f, apron);                       // the second hangar's apron
        ap_quad(a, 190.0f, 116.0f, 206.0f, 124.0f, tw);
        ap_quad(a, 590.0f, 740.0f, 690.0f, 850.0f, RGBA8(138, 130, 112, 255));   // gravel under the solar array
        ap_quad(a, 1640.0f - 20.0f, -9.0f, 1700.0f, 9.0f, RGBA8(214, 196, 150, 255));   // the end of the beach road
        static const int NUM[2][2] = { { 3, 6 }, { 1, 8 } };                   // runway 36 at the north end of the frame, 18 at the other
        for (int north = 0; north < 2; north++) {
            ap_digit(a, NUM[north][0], north, -9.0f, wht);
            ap_digit(a, NUM[north][1], north,  2.0f, wht);
            for (int s = -1; s <= 1; s += 2) ap_rwy_rect(a, north, s * 4.0f, s * 6.0f, 280.0f, 302.0f, wht);
        }
    }
    if (dc < 4500.0f) draw_ap_papi(a);
    if (tod != TOD_DAY && dc < 9000.0f) {
        unsigned int lw = RGBA8(255, 246, 214, 255), red = RGBA8(255, 62, 44, 255), grn = RGBA8(80, 245, 120, 255);
        unsigned int blu = RGBA8(80, 140, 255, 255), win = RGBA8(255, 224, 140, 255);
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
        for (float z = -hl + 20.0f; z <= hl - 20.0f; z += 80.0f) { V3 t = ap_pt(a, 49.0f, 0.4f, z); draw_light(t.x, t.y, t.z, 1.4f, blu); }
        for (int t = 0; t < 2; t++) {                                                                       // red beacons up the towers, floodlit pads
            float lz = t ? 130.0f : -120.0f;
            bool on = (((frame_count / 30) + t) & 1) != 0;
            for (int k = 0; k < 4; k++) { V3 q = ap_pt(a, 1196.0f, 36.0f * (k + 1), lz); draw_light(q.x, q.y, q.z, 2.0f, on ? red : RGBA8(120, 24, 20, 255)); }
            V3 top = ap_pt(a, 1196.0f, 166.0f, lz); light_far = 45000.0f; draw_light(top.x, top.y, top.z, 2.8f, on ? red : RGBA8(120, 24, 20, 255)); light_far = 9000.0f;
            for (int k = 0; k < 5; k++) { V3 f = ap_pt(a, 1140.0f, 18.0f, lz - 20.0f + k * 10.0f); draw_light(f.x, f.y, f.z, 2.6f, RGBA8(255, 252, 240, 255)); }
        }
        for (int i = 0; i < 5; i++) { V3 w = ap_pt(a, 1019.0f + i * 3.0f, 8.0f, -1.9f); draw_light(w.x, w.y, w.z, 1.8f, win); }
        for (int i = 0; i < 6; i++) {                                                                       // floodlight poles, mast tops, the water tower
            V3 f = ap_pt(a, 1090.0f, 22.5f, -360.0f + i * 144.0f); draw_light(f.x, f.y, f.z, 3.0f, RGBA8(255, 252, 240, 255));
        }
        for (int i = 0; i < 4; i++) {
            float lx = (i & 1) ? 1262.0f : 1084.0f, lz = (i & 2) ? 250.0f : -250.0f;
            V3 m = ap_pt(a, lx, 93.0f, lz); draw_light(m.x, m.y, m.z, 2.2f, ((frame_count / 30) & 1) ? red : RGBA8(120, 24, 20, 255));
        }
        { V3 w = ap_pt(a, 1252.0f, 52.5f, -200.0f); draw_light(w.x, w.y, w.z, 2.2f, red); }
        for (int i = 0; i < 6; i++) { V3 w = ap_pt(a, 62.0f + i * 8.0f, 4.0f, 52.0f); draw_light(w.x, w.y, w.z, 1.4f, lw); }
        for (int i = 0; i < 6; i++) { V3 w = ap_pt(a, 1040.0f + i * 14.0f, 14.0f, 1450.0f); draw_light(w.x, w.y, w.z, 1.6f, lw); }
    }
    if (tod != TOD_DAY && dc < 12000.0f) {                                                                  // the build site, lit
        unsigned int lw = RGBA8(255, 246, 214, 255);
        for (float z = 1300.0f; z <= 1900.0f; z += 60.0f) { V3 t = ap_pt(a, 640.0f, 6.0f, z); draw_light(t.x, t.y, t.z, 1.8f, lw); }
        V3 t2 = ap_pt(a, 790.0f, 116.0f, 1380.0f); light_far = 20000.0f; draw_light(t2.x, t2.y, t2.z, 2.6f, RGBA8(255, 62, 44, 255)); light_far = 9000.0f;
    }
    for (float z = -hl + 90.0f; z <= hl - 90.0f; z += 90.0f)                // edge lights by day
        for (int s = -1; s <= 1; s += 2) {
            V3 w = ap_pt(a, s * 24.5f, 0.0f, z);
            float dx = w.x - cam.pos.x, dz = w.z - cam.pos.z;
            if (dx * dx + dz * dz < 2200.0f * 2200.0f)
                ap_quad(a, s * 24.5f - 0.7f, z - 0.7f, s * 24.5f + 0.7f, z + 0.7f, RGBA8(255, 246, 190, 255));
        }
    if (tod != TOD_DAY) draw_beacon_at(a, 100.0f, -60.0f, 26.0f);
}
