// Left Seat: Strake Proving Ground, the KX-1's test site. It is always part of the world, so aircraft can fly over it and see it,
// day and night. The KX-1 only comes here for its challenges (it is never driven in the open world).
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.
//
// The layout follows the plan: x east, z north, about 2 km by 1.2 km around (SPG_X, SPG_Z). "Map" coordinates (px, py) are the
// plan drawing's, 2 m to a unit, with up being north.

#define SPG_SX(px) (SPG_X + 2.0f * ((px) - 500.0f))
#define SPG_SZ(py) (SPG_Z - 2.0f * ((py) - 310.0f))

// The circuit: a rounded rectangle, 1040 m by 600 m outside, with 280 m corners (about 2.4 km round).
#define SPC_CX   SPG_SX(520.0f)
#define SPC_CZ   SPG_SZ(300.0f)
#define SPC_HX   520.0f
#define SPC_HZ   300.0f
#define SPC_R    280.0f
#define SPC_HALF 9.0f                    // half the road width
#define SPC_N    52                      // points round the circuit

static float spc_x[SPC_N], spc_z[SPC_N];
static bool  spc_ready;
static void spc_build(void) {
    if (spc_ready) return;
    spc_ready = true;
    const int per = SPC_N / 4;           // points in each corner arc
    int n = 0;
    for (int c = 0; c < 4; c++) {        // corners, counter-clockwise from the north-east
        const float sx = (c == 0 || c == 3) ? 1.0f : -1.0f, sz = (c < 2) ? 1.0f : -1.0f;
        const float cx = SPC_CX + sx * (SPC_HX - SPC_R), cz = SPC_CZ + sz * (SPC_HZ - SPC_R);
        const float a0 = (c == 0) ? 0.0f : (c == 1) ? PI_F * 0.5f : (c == 2) ? PI_F : PI_F * 1.5f;
        for (int k = 0; k < per; k++) {
            const float a = a0 + (PI_F * 0.5f) * k / (per - 1);
            spc_x[n] = cx + SPC_R * cosf(a); spc_z[n] = cz + SPC_R * sinf(a); n++;
        }
    }
}

// The five gates of the speed run, where the plan has its yellow marks: {map x, map y, 1 if the gap runs across x (a road running north-south), 0 if across z (a road running east-west)}.
static const float SPG_GATES[5][3] = { { 522.0f, 150.0f, 0.0f }, { 402.0f, 450.0f, 0.0f }, { 660.0f, 450.0f, 0.0f }, { 780.0f, 318.0f, 1.0f }, { 260.0f, 318.0f, 1.0f } };
#define SPG_GATE_HW 13.0f                // half the gap between the posts

static void build_proving_ground(void) {
    // Gates: two posts and a beam over the road.
    for (int g = 0; g < 5; g++) {
        const float x = SPG_SX(SPG_GATES[g][0]), z = SPG_SZ(SPG_GATES[g][1]);
        const bool acx = SPG_GATES[g][2] > 0.5f;
        for (int s = -1; s <= 1; s += 2) {
            const float px = acx ? x + s * SPG_GATE_HW : x, pz = acx ? z : z + s * SPG_GATE_HW;
            add_box_faces(px, pz, 1.2f, 1.2f, 0.0f, 7.5f, 236, 238, 242);
            town_box(px, pz, 1.2f, 1.2f, 7.5f);
            if (ntown_lights < 630) town_lights[ntown_lights++] = v3(px, 8.2f, pz);
        }
        add_box_faces(x, z, acx ? 2.0f * SPG_GATE_HW + 1.2f : 1.4f, acx ? 1.4f : 2.0f * SPG_GATE_HW + 1.2f, 6.3f, 7.8f, 246, 200, 44);
    }
    // The pit: a garage with a yellow Strake stripe, and the gas depot (three tanks) beside it.
    {
        const float gx = SPG_SX(150.0f), gz = SPG_SZ(520.0f);
        add_building(gx, gz, 84.0f, 28.0f, 9.0f, 224, 227, 234);
        add_box_faces(gx, gz, 86.0f, 30.0f, 9.0f, 9.8f, 58, 64, 76);                       // roof edge
        quad_face(v3(gx - 42.0f, 4.5f, gz - 14.25f), v3(gx + 42.0f, 4.5f, gz - 14.25f), v3(gx + 42.0f, 6.5f, gz - 14.25f), v3(gx - 42.0f, 6.5f, gz - 14.25f), 246, 200, 44);
        for (int k = 0; k < 4; k++) {                                                      // the garage doors, on the side facing the track
            const float xx = gx - 31.5f + k * 21.0f, zz = gz - 14.35f;
            quad_face(v3(xx - 7.0f, 0.0f, zz), v3(xx + 7.0f, 0.0f, zz), v3(xx + 7.0f, 4.0f, zz), v3(xx - 7.0f, 4.0f, zz), 44, 50, 62);
        }
        static const float TR[3] = { 8.5f, 8.5f, 0.0f }, TH[3] = { 0.0f, 13.0f, 13.4f };
        for (int k = 0; k < 3; k++) {
            const float tx = gx + 76.0f + k * 24.0f, tz = gz + 4.0f;
            add_lathe(tx, tz, TR, TH, 3, 8, 1, 176, 214, 238, 226, 230, 236);
            town_box(tx, tz, 16.0f, 16.0f, 13.4f);
        }
        if (ntown_lights < 630) { town_lights[ntown_lights++] = v3(gx - 42.0f, 8.0f, gz - 15.0f); town_lights[ntown_lights++] = v3(gx + 42.0f, 8.0f, gz - 15.0f); }
    }
    // The beacon tower: a pale shaft with a yellow cab, so the site shows from far away.
    {
        const float bx = SPG_SX(910.0f), bz = SPG_SZ(190.0f);
        static const float BR[5] = { 5.5f, 3.0f, 3.0f, 6.5f, 0.0f }, BH[5] = { 0.0f, 70.0f, 70.0f, 78.0f, 86.0f };
        add_lathe(bx, bz, BR, BH, 5, 8, 2, 226, 228, 234, 246, 200, 44);
        town_box(bx, bz, 11.0f, 11.0f, 86.0f);
    }
    // Floodlight poles round the pit and the big pad.
    {
        static const float FP[4][2] = { { 120.0f, 500.0f }, { 240.0f, 500.0f }, { 420.0f, 236.0f }, { 570.0f, 236.0f } };
        for (int k = 0; k < 4; k++) {
            const float fx = SPG_SX(FP[k][0]), fz = SPG_SZ(FP[k][1]);
            add_box_faces(fx, fz, 1.0f, 1.0f, 0.0f, 22.0f, 150, 156, 166);
            town_box(fx, fz, 1.0f, 1.0f, 22.0f);
        }
    }
}

// Painted ground, drawn every frame while the site is near.
static void spg_rect(float x0, float z0, float x1, float z1, unsigned int col) {          // a flat rectangle (corners in any order)
    V3 q[4] = { v3(x0, 0.03f, z0), v3(x1, 0.03f, z0), v3(x1, 0.03f, z1), v3(x0, 0.03f, z1) };
    draw_world_poly(q, 4, col);
}
static void spg_frame(float x0, float z0, float x1, float z1, float w, unsigned int col) {    // the same as a painted border
    spg_rect(x0, z0, x1, z0 + w, col); spg_rect(x0, z1 - w, x1, z1, col);
    spg_rect(x0, z0 + w, x0 + w, z1 - w, col); spg_rect(x1 - w, z0 + w, x1, z1 - w, col);
}
// A cubic bezier road from A to D with controls B and C, as short strips.
static void spg_curve(float ax, float az, float bx, float bz, float cx, float cz, float dx, float dz, float half, unsigned int col) {
    float px = ax, pz = az;
    for (int i = 1; i <= 10; i++) {
        const float t = i / 10.0f, u = 1.0f - t;
        const float x = u * u * u * ax + 3 * u * u * t * bx + 3 * u * t * t * cx + t * t * t * dx;
        const float z = u * u * u * az + 3 * u * u * t * bz + 3 * u * t * t * cz + t * t * t * dz;
        ground_strip(px, pz, x, z, half, col);
        ground_disc(x, z, half, col);
        px = x; pz = z;
    }
}

static void draw_proving_ground(void) {
    const float dx = SPG_X - cam.pos.x, dz = SPG_Z - cam.pos.z;
    const bool near_site = dx * dx + dz * dz < 6500.0f * 6500.0f;
    spc_build();
    const unsigned int asph = RGBA8(64, 66, 74, 255), pad = RGBA8(86, 88, 96, 255), yel = RGBA8(246, 200, 44, 255), wht = PAINT_W;
    if (near_site) {
        // The circuit: tarmac, white edge lines, and the start line at the first gate.
        for (int i = 0; i < SPC_N; i++) {
            const int j = (i + 1) % SPC_N;
            const float mx = 0.5f * (spc_x[i] + spc_x[j]) - cam.pos.x, mz = 0.5f * (spc_z[i] + spc_z[j]) - cam.pos.z;
            if (mx * mx + mz * mz > 4500.0f * 4500.0f) continue;
            ground_strip(spc_x[i], spc_z[i], spc_x[j], spc_z[j], SPC_HALF, asph);
            ground_disc(spc_x[j], spc_z[j], SPC_HALF, asph);
            strip_off(spc_x[i], spc_z[i], spc_x[j], spc_z[j],  SPC_HALF - 0.6f, 0.18f, wht);
            strip_off(spc_x[i], spc_z[i], spc_x[j], spc_z[j], -(SPC_HALF - 0.6f), 0.18f, wht);
        }
        // The long straights (the launch strip is the third): a dashed yellow center line on the circuit's straights.
        ground_dashes(SPC_CX - (SPC_HX - SPC_R), SPC_CZ + SPC_HZ, SPC_CX + (SPC_HX - SPC_R), SPC_CZ + SPC_HZ, 0.18f, 6.0f, 16.0f, 900.0f, yel);
        ground_dashes(SPC_CX - (SPC_HX - SPC_R), SPC_CZ - SPC_HZ, SPC_CX + (SPC_HX - SPC_R), SPC_CZ - SPC_HZ, 0.18f, 6.0f, 16.0f, 900.0f, yel);
        // The road in from the east-west road, past the pit, to the circuit.
        {
            const float ax = SPG_SX(120.0f), az = SPG_SZ(-25.0f), bx = SPG_SX(120.0f), bz = SPG_SZ(200.0f);
            ground_strip(ax, az, bx, bz, 6.5f, asph);
            ground_disc(bx, bz, 6.5f, asph);
            spg_curve(bx, bz, SPG_SX(190.0f), SPG_SZ(200.0f), SPG_SX(210.0f), SPG_SZ(300.0f), SPG_SX(262.0f), SPG_SZ(300.0f), 6.5f, asph);
            spg_curve(SPG_SX(170.0f), SPG_SZ(300.0f), SPG_SX(170.0f), SPG_SZ(380.0f), SPG_SX(150.0f), SPG_SZ(440.0f), SPG_SX(175.0f), SPG_SZ(500.0f), 6.0f, asph);
        }
        // The pit: a big pad with a yellow border.
        {
            const float x0 = SPG_SX(110.0f), x1 = SPG_SX(230.0f), z0 = SPG_SZ(575.0f), z1 = SPG_SZ(505.0f);
            spg_rect(x0, z0, x1, z1, pad); spg_frame(x0, z0, x1, z1, 1.2f, yel);
        }
        // The launch strip along the north edge, with the landing pad at its end.
        {
            const float z = SPG_SZ(72.0f), x0 = SPG_SX(300.0f), x1 = SPG_SX(770.0f);
            ground_strip(x0, z, x1, z, 8.0f, asph);
            strip_off(x0, z, x1, z,  7.2f, 0.2f, wht); strip_off(x0, z, x1, z, -7.2f, 0.2f, wht);
            ground_dashes(x0, z, x1, z, 0.2f, 10.0f, 28.0f, 800.0f, wht);
            const float lx0 = SPG_SX(880.0f), lx1 = SPG_SX(960.0f), lz0 = SPG_SZ(88.0f), lz1 = SPG_SZ(56.0f);
            spg_rect(lx0, lz0, lx1, lz1, pad); spg_frame(lx0, lz0, lx1, lz1, 1.2f, yel);
            ground_strip(SPG_SX(770.0f), z, lx0, z, 8.0f, RGBA8(110, 112, 120, 255));
        }
        // The trick pad: a big pad with three Strake bars, a sign from the air.
        {
            const float x0 = SPG_SX(420.0f), x1 = SPG_SX(570.0f), z0 = SPG_SZ(350.0f), z1 = SPG_SZ(240.0f);
            spg_rect(x0, z0, x1, z1, pad); spg_frame(x0, z0, x1, z1, 1.6f, yel);
            for (int k = 0; k < 3; k++) {
                const float cx = 0.5f * (x0 + x1) + (k - 1) * 70.0f, cz = 0.5f * (z0 + z1);
                V3 q[4] = { v3(cx - 7.0f, 0.04f, cz - 50.0f), v3(cx + 7.0f, 0.04f, cz - 50.0f), v3(cx + 33.0f, 0.04f, cz + 50.0f), v3(cx + 19.0f, 0.04f, cz + 50.0f) };
                draw_world_poly(q, 4, yel);
            }
        }
        // The landing platform by the pond: a pad with a target.
        {
            const float x0 = SPG_SX(790.0f), x1 = SPG_SX(870.0f), z0 = SPG_SZ(520.0f), z1 = SPG_SZ(460.0f);
            spg_rect(x0, z0, x1, z1, pad); spg_frame(x0, z0, x1, z1, 1.2f, yel);
            const float cx = SPG_SX(830.0f), cz = SPG_SZ(490.0f);
            ground_disc(cx, cz, 44.0f * 0.5f, wht); ground_disc(cx, cz, 17.0f, RGBA8(226, 60, 44, 255)); ground_disc(cx, cz, 10.0f, wht); ground_disc(cx, cz, 4.0f, RGBA8(226, 60, 44, 255));
        }
    }

    // At night: the beacon, the floodlights and the lamps along the circuit.
    if (tod != TOD_DAY && dx * dx + dz * dz < 12000.0f * 12000.0f) {
        const unsigned int white = RGBA8(255, 252, 240, 255), red = RGBA8(255, 62, 44, 255), blue = RGBA8(190, 215, 255, 255);
        const float bx = SPG_SX(910.0f), bz = SPG_SZ(190.0f);
        const bool flash = (frame_count % 90) < 14;
        light_far = 45000.0f;
        draw_light(bx, 88.0f, bz, flash ? 3.4f : 2.4f, flash ? white : RGBA8(255, 170, 60, 255));              // the beacon: a short white flash, otherwise amber
        light_far = 9000.0f;
        for (int k = 1; k <= 3; k++) draw_light(bx + 5.0f, 22.0f * k, bz, 1.8f, ((frame_count / 30) & 1) ? red : RGBA8(120, 24, 20, 255));
        static const float FP[4][2] = { { 120.0f, 500.0f }, { 240.0f, 500.0f }, { 420.0f, 236.0f }, { 570.0f, 236.0f } };
        for (int k = 0; k < 4; k++) draw_light(SPG_SX(FP[k][0]), 22.5f, SPG_SZ(FP[k][1]), 3.0f, white);
        for (int i = 0; i < SPC_N; i += 2) {
            const int j = (i + 1) % SPC_N, h = (i + SPC_N - 1) % SPC_N;
            float tx = spc_x[j] - spc_x[h], tz = spc_z[j] - spc_z[h]; const float l = sqrtf(tx * tx + tz * tz) + 0.001f;
            tx /= l; tz /= l;
            const float ox = spc_x[i] - cam.pos.x, oz = spc_z[i] - cam.pos.z;
            if (ox * ox + oz * oz > 5000.0f * 5000.0f) continue;
            draw_light(spc_x[i] - tz * (SPC_HALF + 1.2f), 0.5f, spc_z[i] + tx * (SPC_HALF + 1.2f), 1.5f, blue);
            draw_light(spc_x[i] + tz * (SPC_HALF + 1.2f), 0.5f, spc_z[i] - tx * (SPC_HALF + 1.2f), 1.5f, blue);
        }
    }
}
