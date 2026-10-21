// Left Seat: the Jet 1 Sprint. Built from these dimensions (length 5.03 m, span 5.49 m, height 1.55 m): a tiny
// single-seat jet with a bubble canopy, the engine on top of the rear fuselage and a V-tail with the exhaust between the two fins.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

static const KS KS_J1[] = {
    {  2.00f, 0.015f, 0.78f, 0.86f, 0.82f, 2.4f, 2.4f, 2.3f },             // the tip of the nose: a point
    {  1.84f, 0.13f,  0.62f, 0.97f, 0.79f, 2.4f, 2.4f, 2.3f },
    {  1.55f, 0.27f,  0.47f, 1.05f, 0.77f, 2.5f, 2.5f, 2.4f },
    {  1.20f, 0.37f,  0.39f, 1.15f, 0.76f, 2.7f, 2.6f, 2.6f },
    {  0.70f, 0.40f, 0.36f, 1.29f, 0.78f, 2.9f, 2.7f, 2.8f },
    {  0.20f, 0.40f, 0.36f, 1.45f, 0.82f, 2.6f, 2.7f, 2.9f },
    { -0.30f, 0.38f, 0.38f, 1.50f, 0.86f, 2.5f, 2.7f, 2.8f },
    { -0.90f, 0.34f, 0.42f, 1.40f, 0.90f, 2.5f, 2.6f, 2.6f },
    { -1.50f, 0.30f, 0.48f, 1.34f, 0.92f, 2.4f, 2.5f, 2.5f },
    { -2.10f, 0.24f, 0.56f, 1.28f, 0.93f, 2.3f, 2.4f, 2.4f },
    { -2.60f, 0.17f, 0.66f, 1.22f, 0.94f, 2.3f, 2.3f, 2.3f },
    { -2.90f, 0.11f, 0.74f, 1.16f, 0.95f, 2.2f, 2.2f, 2.2f },
};
static const float Z_J1[] = { 2.00f, 1.93f, 1.84f, 1.72f, 1.55f, 1.35f, 1.15f, 0.95f, 0.75f, 0.50f, 0.25f, 0.00f, -0.30f, -0.65f, -1.00f, -1.40f, -1.90f, -2.40f, -2.90f };
static const float Z_J1_NOSE[] = { 2.00f, 1.93f, 1.84f, 1.72f, 1.55f, 1.35f, 1.15f, 0.95f, 0.85f };

static const KS KS_J1_ENG[] = {                                          // the engine, mounted on top of the rear fuselage
    { -0.60f, 0.11f, 1.30f, 1.55f, 1.42f, 2.2f, 2.2f, 2.2f },
    { -0.95f, 0.17f, 1.24f, 1.63f, 1.43f, 2.1f, 2.1f, 2.1f },
    { -1.60f, 0.19f, 1.22f, 1.64f, 1.43f, 2.0f, 2.0f, 2.0f },
    { -2.30f, 0.18f, 1.22f, 1.60f, 1.41f, 2.0f, 2.0f, 2.0f },
    { -2.95f, 0.14f, 1.24f, 1.54f, 1.39f, 2.1f, 2.1f, 2.1f },
};
static const float Z_J1_ENG[] = { -0.60f, -0.95f, -1.60f, -2.30f, -2.95f };
static const KS KS_J1_NOZ[] = {
    { -2.95f, 0.13f, 1.24f, 1.54f, 1.39f, 2.1f, 2.1f, 2.1f },
    { -3.12f, 0.09f, 1.30f, 1.49f, 1.39f, 2.1f, 2.1f, 2.1f },
};
static const float Z_J1_NOZ[] = { -2.95f, -3.12f };

static const AWing AW_J1 = { 8, { 0.00f, 0.35f, 1.00f, 1.50f, 2.10f, 2.55f, 2.70f, 2.745f }, { 1.12f, 1.10f, 1.04f, 1.02f, 1.00f, 0.93f, 0.78f, 0.52f },
                              { 0.17f, 0.16f, 0.145f, 0.135f, 0.12f, 0.10f, 0.07f, 0.04f }, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0.80f, 0.035f, 0.36f,
                              { 1, 1, 0, 1, 1, 0, 0, 0 }, { 200, 30, 40 } };
static const Paint P_J1 = { {246,247,250}, {200,30,40}, {46,50,60}, {226,230,238}, {26,46,76}, {26,26,30}, {96,102,112}, {200,30,40} };

// One half of the V-tail: a panel leaning out at 40 degrees from the vertical, with a movable rear part (ruddervator).
static void a_vtail(int sx, float defl, const Paint *p) {
    const float phi = 0.70f, sphi = sinf(phi), cphi = cosf(phi), x0 = 0.12f, y0 = 1.10f, L = 1.12f;
    V3 sec[4][6];
    for (int k = 0; k < 4; k++) {
        float t = k / 3.0f, s = L * t;
        float px = sx * (x0 + sphi * s), py = y0 + cphi * s;
        float c = 0.90f - 0.44f * t, zl = -2.16f - 0.36f * t, th = 0.045f - 0.016f * t;
        float nx = sx * cphi, ny = -sphi;                                       // across the thickness of the panel
        float d = (k == 3) ? 0.5f * defl : defl;
        #define VP(off, zz) MW(px + nx * (off), py + ny * (off), (zz))
        sec[k][0] = VP(0.0f, zl);
        sec[k][1] = VP(th * 0.9f, zl - 0.18f * c);
        sec[k][2] = VP(th * 0.9f + 0.4f * d, zl - 0.60f * c);
        sec[k][3] = VP(d, zl - c);
        sec[k][4] = VP(-th * 0.9f + 0.4f * d, zl - 0.60f * c);
        sec[k][5] = VP(-th * 0.9f, zl - 0.18f * c);
        #undef VP
    }
    for (int k = 0; k < 3; k++) {
        V3 cc = vscale(vadd(avgv(sec[k], 6), avgv(sec[k + 1], 6)), 0.5f);
        for (int e = 0; e < 6; e++) {
            V3 A = sec[k][e], B = sec[k][(e + 1) % 6], C = sec[k + 1][(e + 1) % 6], D = sec[k + 1][e];
            V3 q[4] = { A, B, C, D };
            float m = (e == 4 || e == 5) ? 0.85f : 1.0f;
            pf_poly(4, q, &cc, p->a1[0] * m, p->a1[1] * m, p->a1[2] * m, 0, 255);
            if (e == 2 || e == 3) decal_quad(A, B, C, D, 0.0f, 0.04f, 0.0f, 1.0f, 120, 110, 114);        // hinge line
        }
    }
    V3 cc = avgv(sec[2], 6);
    pf_poly(6, sec[3], &cc, p->a2[0], p->a2[1], p->a2[2], 0, 255);
}

// The fuselage, engine and nozzle: every end is closed, so none of it can be seen through.
static void build_jet1_body(const Paint *p) {
    loft_car(KS_J1, 12, Z_J1, (int)(sizeof(Z_J1) / sizeof(Z_J1[0])), p, 4, CAP_FRONT_BODY | CAP_BACK_BODY);
    Paint eng = *p; eng.body[0] = 210; eng.body[1] = 214; eng.body[2] = 222;
    loft_car(KS_J1_ENG, 5, Z_J1_ENG, 5, &eng, 2, CAP_FRONT_DARK);
    Paint noz = *p; noz.body[0] = 70; noz.body[1] = 66; noz.body[2] = 66;
    loft_car(KS_J1_NOZ, 2, Z_J1_NOZ, 2, &noz, 2, CAP_BACK_DARK);
}

static void build_jet1(void) {
    npf = 0; pf_last = -1;
    const Paint *p = &P_J1;
    float elev = clampf(pl.ele + pl.trim, -1.0f, 1.0f);
    float flap = -sinf(pl.flaps * 10.0f * PI_F / 180.0f) * 0.30f, ail = pl.ail * 0.16f;
    build_jet1_body(p);
    float dR[8] = { flap, flap, flap, ail, ail, ail * 0.5f, 0, 0 }, dL[8] = { flap, flap, flap, -ail, -ail, -ail * 0.5f, 0, 0 };
    a_wing(&AW_J1, +1, dR, p, false);
    a_wing(&AW_J1, -1, dL, p, true);
    float dv = elev * 0.16f, rv = pl.rud * 0.16f;                              // each half of the V-tail moves for pitch and for yaw
    a_vtail(+1, dv + rv, p);
    a_vtail(-1, dv - rv, p);
    for (int sx = -1; sx <= 1; sx += 2) {
        prism(sx * 0.26f, 0.50f, -0.20f, sx * 0.76f, 0.19f, -0.25f, 0.030f, 0.060f, p->metal[0], p->metal[1], p->metal[2]);   // main gear leg
        a_tyre(sx * 0.80f, 0.15f, -0.25f, 0.15f, 0.05f, sx, p);
    }
    prism(0, 0.52f, 1.46f, 0, 0.16f, 1.50f, 0.024f, 0.040f, p->metal[0], p->metal[1], p->metal[2]);           // nose gear
    a_tyre(0, 0.13f, 1.50f, 0.13f, 0.04f, 0, p);
    float g = clampf((pl.n1 - 0.25f) / 0.75f, 0.0f, 1.0f);                    // the exhaust glows when the engine is working hard
    if (g > 0.02f && pl.fuel > 0.0f && !engine_failed) {
        V3 d[8];
        for (int i = 0; i < 8; i++) { float a = i * PI_F / 4.0f; d[i] = MW(0.085f * cosf(a), 1.39f + 0.085f * sinf(a), -3.14f); }
        pf_poly(8, d, NULL, 255, 150, 50, PF_TWO | PF_EMIT, (int)(230.0f * g));
        V3 e[8];
        for (int i = 0; i < 8; i++) { float a = i * PI_F / 4.0f; e[i] = MW(0.045f * cosf(a), 1.39f + 0.045f * sinf(a), -3.15f); }
        pf_poly(8, e, NULL, 255, 235, 170, PF_TWO | PF_EMIT, (int)(240.0f * g));
    }
    prism(0, 1.50f, -0.20f, 0, 1.78f, -0.42f, 0.005f, 0.010f, 40, 40, 48);                                    // antenna
}

// From the seat you can see the wings below you when you look out to the side.
static void build_cockpit_parts_jet1(void) {
    const Paint *p = &P_J1;
    float flap = -sinf(pl.flaps * 10.0f * PI_F / 180.0f) * 0.30f, ail = pl.ail * 0.16f;
    float dR[8] = { flap, flap, flap, ail, ail, ail * 0.5f, 0, 0 }, dL[8] = { flap, flap, flap, -ail, -ail, -ail * 0.5f, 0, 0 };
    a_wing(&AW_J1, +1, dR, p, false);
    a_wing(&AW_J1, -1, dL, p, true);
}
static const KS KS_J1_NOSE[] = {                                                                     // the deck and pointed nose in front of you, sloping down away from the instrument panel
    { 2.00f, 0.015f, 0.76f, 0.82f, 0.79f, 2.4f, 2.4f, 2.3f },
    { 1.84f, 0.13f,  0.60f, 0.93f, 0.77f, 2.4f, 2.4f, 2.3f },
    { 1.55f, 0.27f,  0.45f, 1.01f, 0.75f, 2.5f, 2.5f, 2.4f },
    { 1.25f, 0.37f,  0.36f, 1.07f, 0.72f, 2.7f, 2.6f, 2.6f },
    { 0.85f, 0.40f,  0.34f, 1.12f, 0.73f, 2.9f, 2.7f, 2.8f },
};
static void build_cockpit_nose_jet1(void) {
    loft_car(KS_J1_NOSE, 5, Z_J1_NOSE, (int)(sizeof(Z_J1_NOSE) / sizeof(Z_J1_NOSE[0])), &P_J1, 3, CAP_FRONT_BODY);
}

static void draw_plane_shadow_jet1(void) {
    V3 w[8];
    const unsigned int col = RGBA8(0, 0, 0, 85);
    static const float FX[8] = { -0.18f, -0.38f, -0.40f, -0.12f, 0.12f, 0.40f, 0.38f, 0.18f };
    static const float FZ[8] = { 2.00f, 1.20f, -1.80f, -3.05f, -3.05f, -1.80f, 1.20f, 2.00f };
    for (int i = 0; i < 8; i++) w[i] = shadow_pt(MW(FX[i], 1.0f, FZ[i]));
    draw_world_poly(w, 8, col);
    for (int sx = -1; sx <= 1; sx += 2) {
        V3 wg[4] = { shadow_pt(MW(sx * 0.35f, 0.85f, 0.36f)), shadow_pt(MW(sx * 2.745f, 0.95f, 0.36f)),
                     shadow_pt(MW(sx * 2.745f, 0.95f, -0.16f)), shadow_pt(MW(sx * 0.35f, 0.85f, -0.76f)) };
        draw_world_poly(wg, 4, col);
        V3 tl[4] = { shadow_pt(MW(sx * 0.15f, 1.2f, -2.20f)), shadow_pt(MW(sx * 0.95f, 1.7f, -2.45f)),
                     shadow_pt(MW(sx * 0.95f, 1.7f, -3.00f)), shadow_pt(MW(sx * 0.15f, 1.2f, -3.05f)) };
        draw_world_poly(tl, 4, col);
    }
}
