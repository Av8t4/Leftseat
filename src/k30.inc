// Left Seat: the K-30 Courier. A small twin-engine business jet: engines on the rear fuselage, a T-tail,
// a swept low wing, canards on the nose with moving trailing edges, and retractable landing gear.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

typedef struct { int n; float x[8], ch[8], th[8], droop[8], zle[8]; float y0, dih; int hinge[8]; float tipcol[3]; } AWingS;   // a swept wing: the leading edge moves back along the span
static void a_wing_sw(const AWingS *W, int sx, const float *dy, const Paint *p, bool light) {
    V3 sec[8][8], cc[8];
    for (int k = 0; k < W->n; k++) {
        float x = sx * W->x[k], c = W->ch[k], t = W->th[k] / 0.12f, yc = W->y0 + W->dih * W->x[k] - W->droop[k], z0 = W->zle[k];
        V3 *s = sec[k];
        s[0] = MW(x, yc, z0);
        s[1] = MW(x, yc + AF_YU[1] * c * t, z0 - AF_X[1] * c);
        s[2] = MW(x, yc + AF_YU[2] * c * t, z0 - AF_X[2] * c);
        s[3] = MW(x, yc + AF_YU[3] * c * t + 0.3f * dy[k], z0 - AF_X[3] * c);
        s[4] = MW(x, yc + AF_YU[4] * c * t + dy[k], z0 - c);
        s[5] = MW(x, yc + AF_YL[2] * c * t + 0.3f * dy[k], z0 - AF_X[3] * c);
        s[6] = MW(x, yc + AF_YL[1] * c * t, z0 - AF_X[2] * c);
        s[7] = MW(x, yc + AF_YL[0] * c * t, z0 - AF_X[1] * c);
    }
    for (int k = 0; k < W->n - 1; k++) { V3 a = avgv(sec[k], 8), b = avgv(sec[k + 1], 8); cc[k] = vscale(vadd(a, b), 0.5f); }
    static const float M[8] = { 1.0f, 1.0f, 1.0f, 0.93f, 0.84f, 0.84f, 0.88f, 0.92f };
    for (int k = 0; k < W->n - 1; k++)
        for (int e = 0; e < 8; e++) {
            V3 A = sec[k][e], B = sec[k][(e + 1) % 8], C = sec[k + 1][(e + 1) % 8], D = sec[k + 1][e];
            V3 q[4] = { A, B, C, D };
            bool tip = (k >= W->n - 3);
            float r = p->body[0] * M[e], g = p->body[1] * M[e], b = p->body[2] * M[e];
            if (tip && (e == 0 || e == 1 || e == 7)) { r = W->tipcol[0] * M[e]; g = W->tipcol[1] * M[e]; b = W->tipcol[2] * M[e]; }
            pf_poly(4, q, &cc[k], r, g, b, PF_TWO, 255);       // every face is drawn from both sides: with a big control-surface deflection on a thin panel, the "which way does it face" test can flip
            if (e == 2 && W->hinge[k]) decal_quad(A, B, C, D, 0.0f, 0.04f, 0.0f, 1.0f, 112, 118, 128);
            if (e == 7 && k == 3 && sx < 0 && light) decal_quad(A, B, C, D, 0.15f, 0.85f, 0.15f, 0.60f, 255, 250, 215);   // landing light
        }
    int n1 = W->n - 1;
    pf_poly(8, sec[n1], &cc[n1 - 1], W->tipcol[0], W->tipcol[1], W->tipcol[2], 0, 255);
    V3 tc = avgv(sec[n1], 8);
    V3 nl[4] = { vadd(tc, vscale(pl.f, 0.05f)), vadd(tc, vscale(pl.u, 0.05f)), vadd(tc, vscale(pl.f, -0.05f)), vadd(tc, vscale(pl.u, -0.05f)) };
    pf_poly(4, nl, NULL, sx < 0 ? 255 : 40, sx < 0 ? 40 : 255, 40, PF_DECAL | PF_EMIT, 255);                    // navigation light
    bool flash = (((frame_count + (sx < 0 ? 0 : 7)) / 3) % 17) == 0;
    V3 te = sec[n1][4];
    V3 sl[4] = { vadd(te, vscale(pl.f, 0.07f)), vadd(te, vscale(pl.u, 0.07f)), vadd(te, vscale(pl.f, -0.07f)), vadd(te, vscale(pl.u, -0.07f)) };
    pf_poly(4, sl, NULL, flash ? 255 : 80, flash ? 255 : 80, flash ? 255 : 84, PF_DECAL | PF_EMIT, 255);      // strobe
}

static void sw_make(AWingS *W, int n, float xr, float xt, float lr, float lt, float tr, float tt, float thr, float tht, float y0, float dih, int h0, int h1) {
    W->n = n;
    for (int k = 0; k < n; k++) {
        float u = (n > 1) ? (float)k / (n - 1) : 0.0f;
        W->x[k] = xr + (xt - xr) * u; W->zle[k] = lr + (lt - lr) * u;
        W->ch[k] = W->zle[k] - (tr + (tt - tr) * u); W->th[k] = thr + (tht - thr) * u; W->droop[k] = 0.0f;
        W->hinge[k] = (k >= h0 && k <= h1) ? 1 : 0;
    }
    W->y0 = y0; W->dih = dih; W->tipcol[0] = 200; W->tipcol[1] = 30; W->tipcol[2] = 40;
}
static AWingS AW_K30, AW_K30_CAN;
static void k30_init(void) {
    static bool done = false;
    if (done) return;
    done = true;
    sw_make(&AW_K30,     7, 0.95f, 7.20f, 1.13f, -1.60f, -2.89f, -2.60f, 0.120f, 0.070f, 1.45f, 0.035f, 3, 4);   // main wing (flaps are the inner two hinge lines)
    sw_make(&AW_K30_CAN, 5, 0.95f, 3.45f, 5.62f,  4.70f,  4.10f,  3.95f, 0.095f, 0.070f, 1.82f, 0.05f, 0, 4);   // canard
}

static const KS KS_K30[] = {
    {  7.20f, 0.03f, 1.78f, 2.02f, 1.90f, 2.4f, 2.4f, 2.3f },
    {  6.40f, 0.52f, 1.30f, 2.55f, 1.85f, 2.4f, 2.5f, 2.4f },
    {  5.60f, 0.86f, 1.06f, 2.92f, 1.95f, 2.5f, 2.6f, 2.5f },
    {  4.50f, 1.00f, 0.96f, 3.12f, 2.00f, 2.6f, 2.6f, 2.6f },
    {  2.00f, 1.05f, 0.95f, 3.20f, 2.05f, 2.6f, 2.6f, 2.6f },
    { -1.50f, 1.05f, 1.00f, 3.20f, 2.05f, 2.6f, 2.6f, 2.6f },
    { -3.50f, 0.98f, 1.25f, 3.12f, 2.10f, 2.5f, 2.5f, 2.5f },
    { -5.20f, 0.78f, 1.70f, 3.05f, 2.30f, 2.4f, 2.4f, 2.4f },
    { -6.50f, 0.52f, 2.15f, 3.02f, 2.50f, 2.3f, 2.3f, 2.3f },
    { -7.30f, 0.26f, 2.65f, 3.05f, 2.80f, 2.2f, 2.2f, 2.2f },
};
static const float Z_K30[] = { 7.20f, 6.90f, 6.40f, 6.00f, 5.60f, 5.20f, 4.80f, 4.30f, 3.80f, 3.30f, 2.90f, 2.40f, 2.00f, 1.50f, 1.10f, 0.60f, 0.20f, -0.30f, -0.70f, -1.20f, -1.80f, -2.50f, -3.20f, -4.00f, -4.80f, -5.60f, -6.30f, -6.90f, -7.30f };
static const float Z_K30_NOSE[] = { 7.20f, 6.90f, 6.40f, 6.00f, 5.80f };
static const KS KS_K30_ENG[] = {                                                   // an engine pod on each side of the rear fuselage
    { -2.00f, 0.40f, 2.48f, 3.52f, 3.00f, 2.2f, 2.2f, 2.2f },
    { -2.60f, 0.52f, 2.42f, 3.58f, 3.00f, 2.2f, 2.2f, 2.2f },
    { -3.40f, 0.56f, 2.40f, 3.60f, 3.00f, 2.2f, 2.2f, 2.2f },
    { -4.40f, 0.52f, 2.44f, 3.56f, 3.00f, 2.2f, 2.2f, 2.2f },
    { -5.30f, 0.42f, 2.52f, 3.48f, 3.00f, 2.1f, 2.1f, 2.1f },
    { -5.70f, 0.36f, 2.58f, 3.42f, 3.00f, 2.1f, 2.1f, 2.1f },
};
static const float Z_K30_ENG[] = { -2.00f, -2.60f, -3.40f, -4.40f, -5.30f, -5.70f };
static const KS KS_K30_NOZ[] = {
    { -5.70f, 0.34f, 2.60f, 3.40f, 3.00f, 2.1f, 2.1f, 2.1f },
    { -5.98f, 0.27f, 2.68f, 3.32f, 3.00f, 2.1f, 2.1f, 2.1f },
};
static const float Z_K30_NOZ[] = { -5.70f, -5.98f };
static const Paint P_K30 = { {246,247,250}, {200,30,40}, {26,52,120}, {226,230,238}, {26,46,76}, {26,26,30}, {96,102,112}, {200,30,40} };
static const ATail AT_K30 = { 5, { 3.00f, 3.90f, 4.80f, 5.40f, 6.00f }, { -3.17f, -3.96f, -4.75f, -5.28f, -5.80f }, { 4.23f, 3.44f, 2.65f, 2.12f, 1.60f },
                              { 0.17f, 0.15f, 0.12f, 0.10f, 0.08f }, 3.20f };
static const AStab AH_K30 = { 3, { 0.00f, 1.40f, 2.60f }, { -5.40f, -6.15f, -6.90f }, { 2.00f, 1.50f, 1.00f }, { 0.13f, 0.10f, 0.08f }, 5.93f };

// A solid tyre: tread, and a full sidewall and hub on BOTH sides, every face drawn from both sides (no see-through rim from any angle).
static void a_tyre30(float cx, float cy, float cz, float rad, float hw, const Paint *p) {
    enum { T = 14 };
    V3 r0[T], r1[T], i0[T], i1[T];
    for (int i = 0; i < T; i++) {
        float a = i * 2.0f * PI_F / T, ca = cosf(a), sa = sinf(a);
        r0[i] = MW(cx - hw, cy + rad * ca, cz + rad * sa);          r1[i] = MW(cx + hw, cy + rad * ca, cz + rad * sa);
        i0[i] = MW(cx - hw * 1.04f, cy + rad * 0.55f * ca, cz + rad * 0.55f * sa); i1[i] = MW(cx + hw * 1.04f, cy + rad * 0.55f * ca, cz + rad * 0.55f * sa);
    }
    V3 cc = MW(cx, cy, cz);
    for (int i = 0; i < T; i++) {
        int j = (i + 1) % T;
        V3 q[4] = { r0[i], r0[j], r1[j], r1[i] };
        pf_poly(4, q, &cc, p->tire[0], p->tire[1], p->tire[2], PF_TWO, 255);
        V3 a[4] = { r0[i], r0[j], i0[j], i0[i] }, b[4] = { r1[i], r1[j], i1[j], i1[i] };
        pf_poly(4, a, &cc, 46, 46, 52, PF_TWO, 255);
        pf_poly(4, b, &cc, 46, 46, 52, PF_TWO, 255);
    }
    for (int s = 0; s < 2; s++) {
        V3 *h = s ? i1 : i0, half1[8], half2[8];
        for (int k = 0; k < 8; k++) half1[k] = h[k];
        for (int k = 0; k < 7; k++) half2[k] = h[7 + k];
        half2[7] = h[0];
        pf_poly(8, half1, &cc, 120, 124, 134, PF_TWO, 255);
        pf_poly(8, half2, &cc, 120, 124, 134, PF_TWO, 255);
    }
}

// A decal is drawn together with the solid face stored just before it. The belly doors are made after the tail, so on their own
// they were drawn with the tail and showed through the fuselage as you banked. This adds a door right after the fuselage face nearest to it.
static void pf_door(const V3 *q, V3 near_pt) {
    int best = -1; float bd = 1e30f;
    for (int i = 0; i < npf; i++) {
        if (pfaces[i].isdecal) continue;
        V3 d = vsub(pfaces[i].c, near_pt);
        float dd = vdot(d, d);
        if (dd < bd) { bd = dd; best = i; }
    }
    if (best < 0 || npf >= MAX_PFACES) return;
    pf_last = best;
    pf_poly(4, q, NULL, 150, 156, 166, PF_DECAL, 255);                          // appended at the end, with the nearest face's normal
    PFace door = pfaces[npf - 1];
    int at = best + 1 + (pfaces[best].ndecals - 1);                              // after the face's existing decals
    for (int i = npf - 1; i > at; i--) pfaces[i] = pfaces[i - 1];
    pfaces[at] = door;
    pf_last = -1;
}

static void build_k30_gear(const Paint *p) {
    float g = clampf(pl.gear, 0.0f, 1.0f);
    if (g < 0.10f) {                                                            // gear up: just the closed doors on the belly
        V3 n[4] = { MW(-0.26f, 1.04f, 5.60f), MW(0.26f, 1.04f, 5.60f), MW(0.26f, 0.956f, 4.40f), MW(-0.26f, 0.956f, 4.40f) };
        pf_door(n, MW(0.0f, 1.0f, 5.0f));
        for (int sx = -1; sx <= 1; sx += 2) {
            V3 m[4] = { MW(sx * 0.16f, 0.953f, 0.15f), MW(sx * 0.58f, 0.953f, 0.15f), MW(sx * 0.58f, 0.953f, -0.95f), MW(sx * 0.16f, 0.953f, -0.95f) };
            pf_door(m, MW(sx * 0.37f, 0.953f, -0.40f));
        }
        return;
    }
    {                                                                           // nose gear: swings back and up into the nose
        float wy = 1.18f + (0.33f - 1.18f) * g, wz = 3.90f + (4.95f - 3.90f) * g;
        prism(0.0f, 1.05f, 5.00f, 0.0f, wy, wz, 0.045f, 0.075f, p->metal[0], p->metal[1], p->metal[2]);
        a_tyre30(0.0f, wy, wz, 0.30f, 0.09f, p);
    }
    for (int sx = -1; sx <= 1; sx += 2) {                                       // main gear: folds in towards the centreline
        float wx = 0.45f + (1.50f - 0.45f) * g, wy = 1.22f + (0.33f - 1.22f) * g;
        prism(sx * 1.00f, 1.42f, -0.40f, sx * wx, wy, -0.40f, 0.060f, 0.10f, p->metal[0], p->metal[1], p->metal[2]);
        a_tyre30(sx * wx, wy, -0.40f, 0.33f, 0.11f, p);
    }
}

static void build_k30_surfaces(const Paint *p, bool all) {
    k30_init();
    float elev = clampf(pl.ele + pl.trim, -1.0f, 1.0f);
    float flap = -sinf(pl.flaps * 10.0f * PI_F / 180.0f) * 0.85f, ail = pl.ail * 0.32f;
    float dR[8] = { flap, flap, 0, ail, ail, ail * 0.5f, 0, 0 }, dL[8] = { flap, flap, 0, -ail, -ail, -ail * 0.5f, 0, 0 };
    a_wing_sw(&AW_K30, +1, dR, p, false);
    a_wing_sw(&AW_K30, -1, dL, p, true);
    float dc = -elev * 0.20f;                                                   // canard: only the trailing edge moves, like an elevator (down = nose up)
    float dcv[8] = { dc, dc, dc, dc, dc, 0, 0, 0 };
    a_wing_sw(&AW_K30_CAN, +1, dcv, p, false);
    a_wing_sw(&AW_K30_CAN, -1, dcv, p, false);
    (void)all;
}

static void build_k30(void) {
    npf = 0; pf_last = -1;
    const Paint *p = &P_K30;
    k30_init();
    loft_car(KS_K30, 10, Z_K30, (int)(sizeof(Z_K30) / sizeof(Z_K30[0])), p, 5, CAP_FRONT_BODY | CAP_BACK_BODY);
    Paint eng = *p; eng.body[0] = 214; eng.body[1] = 218; eng.body[2] = 226;
    Paint noz = *p; noz.body[0] = 70; noz.body[1] = 66; noz.body[2] = 66;
    for (int sx = -1; sx <= 1; sx += 2) {
        loft_xoff = sx * 1.35f;
        loft_car(KS_K30_ENG, 6, Z_K30_ENG, 6, &eng, 2, CAP_FRONT_DARK);
        loft_car(KS_K30_NOZ, 2, Z_K30_NOZ, 2, &noz, 2, CAP_BACK_DARK);
    }
    loft_xoff = 0.0f;
    build_k30_surfaces(p, true);
    float elev = clampf(pl.ele + pl.trim, -1.0f, 1.0f);
    a_fin(&AT_K30, pl.rud, p);
    a_stab(&AH_K30, +1, elev * 0.30f, p);
    a_stab(&AH_K30, -1, elev * 0.30f, p);
    build_k30_gear(p);
    float g = clampf((pl.n1 - 0.25f) / 0.75f, 0.0f, 1.0f);                      // the exhausts glow when the engines work hard
    if (g > 0.02f && pl.fuel > 0.0f && !engine_failed) {
        for (int sx = -1; sx <= 1; sx += 2) {
            V3 d[8], e[8];
            for (int i = 0; i < 8; i++) { float a = i * PI_F / 4.0f; d[i] = MW(sx * 1.35f + 0.20f * cosf(a), 3.0f + 0.20f * sinf(a), -6.02f); }
            pf_poly(8, d, NULL, 255, 150, 50, PF_TWO | PF_EMIT, (int)(230.0f * g));
            for (int i = 0; i < 8; i++) { float a = i * PI_F / 4.0f; e[i] = MW(sx * 1.35f + 0.10f * cosf(a), 3.0f + 0.10f * sinf(a), -6.04f); }
            pf_poly(8, e, NULL, 255, 235, 170, PF_TWO | PF_EMIT, (int)(240.0f * g));
        }
    }
}

// From the seat you can see the wings (and canards) when you look out to the side.
static void build_cockpit_parts_k30(void) { build_k30_surfaces(&P_K30, false); }
static const KS KS_K30_NOSE[] = {                                                  // the nose in front of you, with a black anti-glare panel on top
    { 7.20f, 0.03f, 2.30f, 2.62f, 2.46f, 2.4f, 2.4f, 2.3f },
    { 6.40f, 0.85f, 1.90f, 2.86f, 2.38f, 2.4f, 2.5f, 2.4f },
    { 5.80f, 1.00f, 1.40f, 2.92f, 2.20f, 2.5f, 2.6f, 2.5f },
    { 5.60f, 1.04f, 1.30f, 2.92f, 2.10f, 2.5f, 2.6f, 2.5f },
};
static void build_cockpit_nose_k30(void) {
    loft_car(KS_K30_NOSE, 4, Z_K30_NOSE, (int)(sizeof(Z_K30_NOSE) / sizeof(Z_K30_NOSE[0])), &P_K30, 3, CAP_FRONT_BODY);
}

static void draw_plane_shadow_k30(void) {
    const unsigned int col = RGBA8(0, 0, 0, 85);
    static const float FX[12] = { 0.0f, 0.5f, 1.0f, 1.05f, 1.0f, 0.5f, -0.5f, -1.0f, -1.05f, -1.0f, -0.5f, 0.0f };
    static const float FZ[12] = { 7.2f, 6.4f, 5.0f, 1.0f, -3.5f, -6.5f, -6.5f, -3.5f, 1.0f, 5.0f, 6.4f, 7.2f };
    V3 w[8];
    for (int i = 0; i < 6; i++) w[i] = shadow_pt(MW(FX[i], 2.0f, FZ[i]));
    for (int i = 0; i < 6; i++) { V3 q = shadow_pt(MW(FX[6 + i], 2.0f, FZ[6 + i])); (void)q; }
    V3 R[8] = { shadow_pt(MW(0.0f, 2.0f, 7.2f)), shadow_pt(MW(0.5f, 2.0f, 6.4f)), shadow_pt(MW(1.0f, 2.0f, 5.0f)), shadow_pt(MW(1.05f, 2.0f, 1.0f)),
                shadow_pt(MW(1.0f, 2.0f, -3.5f)), shadow_pt(MW(0.5f, 2.0f, -6.5f)), shadow_pt(MW(0.25f, 2.0f, -7.3f)), shadow_pt(MW(0.0f, 2.0f, -7.3f)) };
    V3 L[8] = { R[7], shadow_pt(MW(-0.25f, 2.0f, -7.3f)), shadow_pt(MW(-0.5f, 2.0f, -6.5f)), shadow_pt(MW(-1.0f, 2.0f, -3.5f)),
                shadow_pt(MW(-1.05f, 2.0f, 1.0f)), shadow_pt(MW(-1.0f, 2.0f, 5.0f)), shadow_pt(MW(-0.5f, 2.0f, 6.4f)), R[0] };
    draw_world_poly(R, 8, col);
    draw_world_poly(L, 8, col);
    (void)w; (void)FX; (void)FZ;
    for (int sx = -1; sx <= 1; sx += 2) {
        V3 wg[4] = { shadow_pt(MW(sx * 0.95f, 1.5f, 1.13f)), shadow_pt(MW(sx * 7.2f, 1.7f, -1.60f)), shadow_pt(MW(sx * 7.2f, 1.7f, -2.60f)), shadow_pt(MW(sx * 0.95f, 1.5f, -2.89f)) };
        draw_world_poly(wg, 4, col);
        V3 cn[4] = { shadow_pt(MW(sx * 0.95f, 1.8f, 5.62f)), shadow_pt(MW(sx * 3.45f, 2.0f, 4.70f)), shadow_pt(MW(sx * 3.45f, 2.0f, 3.95f)), shadow_pt(MW(sx * 0.95f, 1.8f, 4.10f)) };
        draw_world_poly(cn, 4, col);
        V3 tl[4] = { shadow_pt(MW(sx * 0.15f, 6.0f, -5.4f)), shadow_pt(MW(sx * 2.6f, 6.0f, -6.9f)), shadow_pt(MW(sx * 2.6f, 6.0f, -7.9f)), shadow_pt(MW(sx * 0.15f, 6.0f, -7.4f)) };
        draw_world_poly(tl, 4, col);
        V3 en[4] = { shadow_pt(MW(sx * 0.85f, 3.0f, -2.0f)), shadow_pt(MW(sx * 1.85f, 3.0f, -2.0f)), shadow_pt(MW(sx * 1.85f, 3.0f, -6.0f)), shadow_pt(MW(sx * 0.85f, 3.0f, -6.0f)) };
        draw_world_poly(en, 4, col);
    }
}
