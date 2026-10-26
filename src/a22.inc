// Left Seat: the Ashley A22. A twin-engine turboprop with twin tail booms: a short glazed crew pod in the middle, a forward-swept high
// wing, a turboprop on each side with a boom running back from it, an inverted-V tail joining the booms, retractable gear.
// Length 10.2 m, span 13.2 m, height 3.9 m.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

// z forward, y up from the ground, origin under the centre of gravity.
static const KS KS_A22[] = {                                                        // the crew pod
    {  3.80f, 0.30f, 0.92f, 1.52f, 1.22f, 2.4f, 2.4f, 2.3f },
    {  3.30f, 0.58f, 0.84f, 1.80f, 1.30f, 2.5f, 2.5f, 2.4f },
    {  2.70f, 0.78f, 0.80f, 2.12f, 1.45f, 2.6f, 2.6f, 2.6f },
    {  2.00f, 0.86f, 0.80f, 2.38f, 1.55f, 2.7f, 2.7f, 2.8f },
    {  1.20f, 0.90f, 0.80f, 2.46f, 1.60f, 2.8f, 2.8f, 3.0f },
    {  0.00f, 0.90f, 0.80f, 2.42f, 1.60f, 2.8f, 2.8f, 3.0f },
    { -1.40f, 0.84f, 0.84f, 2.24f, 1.56f, 2.7f, 2.7f, 2.8f },
    { -2.60f, 0.62f, 0.96f, 1.96f, 1.50f, 2.5f, 2.5f, 2.6f },
    { -3.60f, 0.28f, 1.10f, 1.62f, 1.36f, 2.3f, 2.3f, 2.3f },
};
static const float Z_A22[] = { 3.80f, 3.45f, 3.10f, 2.70f, 2.30f, 1.90f, 1.45f, 1.00f, 0.55f, 0.10f, -0.35f, -0.80f, -1.25f, -1.70f, -2.15f, -2.60f, -3.05f, -3.60f };
static const KS KS_A22_BOOM[] = {                                                   // engine nacelle and tail boom, in one piece
    {  2.40f, 0.34f, 1.46f, 2.30f, 1.90f, 2.4f, 2.4f, 2.4f },
    {  2.00f, 0.46f, 1.34f, 2.40f, 1.90f, 2.6f, 2.6f, 2.6f },
    {  1.00f, 0.54f, 1.30f, 2.44f, 1.90f, 2.8f, 2.8f, 2.8f },
    { -0.40f, 0.54f, 1.32f, 2.40f, 1.90f, 2.8f, 2.8f, 2.8f },
    { -1.60f, 0.42f, 1.48f, 2.32f, 1.92f, 2.6f, 2.6f, 2.6f },
    { -2.80f, 0.28f, 1.72f, 2.30f, 2.00f, 2.4f, 2.4f, 2.4f },
    { -4.50f, 0.22f, 1.90f, 2.36f, 2.12f, 2.3f, 2.3f, 2.3f },
    { -6.40f, 0.16f, 2.02f, 2.40f, 2.21f, 2.3f, 2.3f, 2.3f },
};
static const float Z_A22_BOOM[] = { 2.40f, 2.10f, 1.70f, 1.20f, 0.60f, 0.00f, -0.60f, -1.20f, -1.80f, -2.40f, -3.00f, -3.60f, -4.20f, -4.80f, -5.40f, -6.00f, -6.40f };
static const KS KS_A22_NOSE[] = {                                                   // the nose in front of you: it falls away below the glareshield, so only a hint shows
    {  3.80f, 0.30f, 0.92f, 1.66f, 1.30f, 2.4f, 2.4f, 2.3f },
    {  3.30f, 0.58f, 0.84f, 1.78f, 1.32f, 2.5f, 2.5f, 2.4f },
    {  2.70f, 0.78f, 0.80f, 1.92f, 1.42f, 2.6f, 2.6f, 2.6f },
    {  2.30f, 0.84f, 0.80f, 2.02f, 1.48f, 2.7f, 2.7f, 2.8f },
};
static const float Z_A22_NOSE[] = { 3.80f, 3.45f, 3.10f, 2.70f, 2.30f };
#define A22_BX 4.05f                                                                // pusher layout: the booms sit outboard of the propeller discs
static const KS KS_A22_PN[] = {                                                     // short pusher nacelle
    {  2.40f, 0.30f, 1.46f, 2.26f, 1.90f, 2.4f, 2.4f, 2.4f },
    {  1.80f, 0.46f, 1.34f, 2.40f, 1.90f, 2.6f, 2.6f, 2.6f },
    {  0.60f, 0.54f, 1.30f, 2.44f, 1.90f, 2.8f, 2.8f, 2.8f },
    { -0.80f, 0.50f, 1.34f, 2.40f, 1.90f, 2.8f, 2.8f, 2.8f },
    { -1.60f, 0.22f, 1.60f, 2.26f, 1.92f, 2.5f, 2.5f, 2.5f },
};
static const float Z_A22_PN[] = { 2.40f, 1.80f, 0.60f, -0.80f, -1.60f };
static const KS KS_A22_PB[] = {                                                     // slim tail boom: starts as a fairing on the wing's leading edge
    {  2.05f, 0.05f, 2.10f, 2.22f, 2.16f, 2.3f, 2.3f, 2.3f },
    {  1.65f, 0.15f, 1.96f, 2.36f, 2.16f, 2.3f, 2.3f, 2.3f },
    {  1.00f, 0.20f, 1.86f, 2.42f, 2.15f, 2.3f, 2.3f, 2.3f },
    {  0.00f, 0.20f, 1.82f, 2.40f, 2.12f, 2.3f, 2.3f, 2.3f },
    { -3.00f, 0.19f, 1.86f, 2.34f, 2.10f, 2.3f, 2.3f, 2.3f },
    { -6.40f, 0.16f, 2.02f, 2.40f, 2.21f, 2.3f, 2.3f, 2.3f },
};
static const float Z_A22_PB[] = { 2.05f, 1.65f, 1.00f, 0.00f, -3.00f, -6.40f };
static const KS KS_A22_TF[] = {                                                     // fairing where each ruddervator meets its boom: swells round the join, then tapers to a point behind it
    { -4.10f, 0.11f, 2.10f, 2.30f, 2.20f, 2.3f, 2.3f, 2.3f },
    { -4.70f, 0.23f, 2.00f, 2.54f, 2.26f, 2.3f, 2.3f, 2.3f },
    { -5.40f, 0.27f, 1.97f, 2.60f, 2.28f, 2.3f, 2.3f, 2.3f },
    { -6.10f, 0.24f, 1.99f, 2.54f, 2.26f, 2.3f, 2.3f, 2.3f },
    { -6.60f, 0.15f, 2.04f, 2.44f, 2.24f, 2.3f, 2.3f, 2.3f },
    { -7.05f, 0.025f, 2.15f, 2.30f, 2.22f, 2.3f, 2.3f, 2.3f },
};
static const float Z_A22_TF[] = { -4.10f, -4.40f, -4.80f, -5.20f, -5.60f, -6.00f, -6.40f, -6.80f, -7.05f };
static AWingS AW_A22;                                                               // forward-swept wing: the tips are ahead of the roots
static void a22_init(void) {
    static bool done = false;
    if (done) return;
    done = true;
    sw_make(&AW_A22, 7, 0.00f, 6.60f, 0.90f, 2.60f, -1.00f, 1.60f, 0.150f, 0.075f, 2.10f, 0.015f, 0, 4);
    AW_A22.tipcol[0] = 122; AW_A22.tipcol[1] = 64; AW_A22.tipcol[2] = 160;
}
typedef struct { int n; float rx, ry, rzl, rch, tx, ty, tzl, tch, th; } AVTail;      // one half of the inverted-V tail: root at the centre line, tip on a boom
static const AVTail AV_A22 = { 5, 0.00f, 3.95f, -5.20f, 1.30f, 4.05f, 2.30f, -4.90f, 1.70f, 0.075f };
static const Paint P_A22 = { {246,247,250}, {122,64,160}, {250,200,60}, {226,230,238}, {58,104,156}, {26,26,30}, {96,102,112}, {122,64,160} };
#define A22_X 2.45f                                                                  // the engines and booms stand this far out

static void a22_wings(const Paint *p) {
    a22_init();
    float flap = -sinf(pl.flaps * 10.0f * PI_F / 180.0f) * 0.85f, ail = pl.ail * 0.32f;
    float dR[8] = { flap, flap, 0, ail, ail, ail * 0.5f, 0, 0 }, dL[8] = { flap, flap, 0, -ail, -ail, -ail * 0.5f, 0, 0 };
    a_wing_sw(&AW_A22, +1, dR, p, false);
    a_wing_sw(&AW_A22, -1, dL, p, true);
}

// One half of the inverted-V tail: a panel running from a peak on the centre line down and out to the top of a boom. Each
// panel is a "ruddervator": the trailing edge moves for pitch and, the other way round on the two sides, for yaw.
static void a_vtail22(const AVTail *V, int sx, float dn, const Paint *p) {
    V3 sec[5][6];
    float dx = V->tx - V->rx, dy = V->ty - V->ry, L = sqrtf(dx * dx + dy * dy);
    float nx = sx * (-dy / L), ny = dx / L;                                         // across the panel: up and outwards
    for (int k = 0; k < V->n; k++) {
        float u = (float)k / (V->n - 1);
        float cx = sx * (V->rx + dx * u), cy = V->ry + dy * u, zl = V->rzl + (V->tzl - V->rzl) * u, c = V->rch + (V->tch - V->rch) * u, t = V->th;
        float d = (k == V->n - 1) ? 0.4f * dn : dn * fminf(1.0f, u / 0.5f);          // nothing moves at the centre line, so the two halves can never touch
        sec[k][0] = MW(cx, cy, zl);
        sec[k][1] = MW(cx + nx * t * 0.9f, cy + ny * t * 0.9f, zl - 0.18f * c);
        sec[k][2] = MW(cx + nx * (t * 0.9f + d * 0.4f), cy + ny * (t * 0.9f + d * 0.4f), zl - 0.60f * c);
        sec[k][3] = MW(cx + nx * d, cy + ny * d, zl - c);
        sec[k][4] = MW(cx + nx * (-t * 0.9f + d * 0.4f), cy + ny * (-t * 0.9f + d * 0.4f), zl - 0.60f * c);
        sec[k][5] = MW(cx - nx * t * 0.9f, cy - ny * t * 0.9f, zl - 0.18f * c);
    }
    for (int k = 0; k < V->n - 1; k++) {
        V3 cc = vscale(vadd(avgv(sec[k], 6), avgv(sec[k + 1], 6)), 0.5f);
        for (int e = 0; e < 6; e++) {
            V3 A = sec[k][e], B = sec[k][(e + 1) % 6], C = sec[k + 1][(e + 1) % 6], D = sec[k + 1][e];
            V3 q[4] = { A, B, C, D };
            float m = (e == 4 || e == 5) ? 0.86f : 1.0f;
            pf_poly(4, q, &cc, p->body[0] * m, p->body[1] * m, p->body[2] * m, 0, 255);
            if (k >= 1 && (e == 1 || e == 4)) {
                decal_quad(A, B, C, D, 0.0f, 1.0f, 0.34f, 0.52f, p->a1[0], p->a1[1], p->a1[2]);
                decal_quad(A, B, C, D, 0.0f, 1.0f, 0.52f, 0.70f, p->a2[0], p->a2[1], p->a2[2]);
            }
            if (e == 2 || e == 3) decal_quad(A, B, C, D, 0.0f, 0.04f, 0.0f, 1.0f, 112, 118, 128);       // hinge line
        }
    }
    V3 cc = avgv(sec[V->n - 2], 6);
    pf_poly(6, sec[V->n - 1], &cc, p->tip[0], p->tip[1], p->tip[2], 0, 255);
}

// Landing gear: the nose wheel folds back into the nose, each main wheel folds back up into its engine nacelle.
static void build_a22_gear(const Paint *p) {
    float g = clampf(pl.gear, 0.0f, 1.0f);
    if (g < 0.10f) return;
    {
        float wy = 0.95f + (0.30f - 0.95f) * g, wz = 2.80f + (3.20f - 2.80f) * g;
        prism(0.0f, 1.00f, 3.10f, 0.0f, wy, wz, 0.045f, 0.075f, p->metal[0], p->metal[1], p->metal[2]);
        a_tyre30(0.0f, wy, wz, 0.27f, 0.085f, p);
    }
    for (int sx = -1; sx <= 1; sx += 2) {
        float wy = 1.62f + (0.38f - 1.62f) * g, wz = -1.00f + (-0.30f + 1.00f) * g;
        prism(sx * A22_X, 1.40f, -0.25f, sx * A22_X, wy, wz, 0.06f, 0.10f, p->metal[0], p->metal[1], p->metal[2]);
        a_tyre30(sx * A22_X, wy, wz, 0.38f, 0.12f, p);
    }
}

static void build_a22(void) {
    npf = 0; pf_last = -1;
    const Paint *p = &P_A22;
    float elev = clampf(pl.ele + pl.trim, -1.0f, 1.0f);
    loft_car(KS_A22, 9, Z_A22, (int)(sizeof(Z_A22) / sizeof(Z_A22[0])), p, 7, CAP_FRONT_BODY | CAP_BACK_BODY);
    Paint nac = *p; nac.body[0] = 238; nac.body[1] = 240; nac.body[2] = 246;
    for (int sx = -1; sx <= 1; sx += 2) {
        loft_xoff = sx * A22_X;
        loft_car(KS_A22_PN, 5, Z_A22_PN, 5, &nac, 2, CAP_FRONT_DARK | CAP_BACK_BODY);
        loft_xoff = sx * A22_BX;
        loft_car(KS_A22_PB, 6, Z_A22_PB, 6, &nac, 2, CAP_FRONT_BODY | CAP_BACK_BODY);
        loft_car(KS_A22_TF, 6, Z_A22_TF, 9, &nac, 2, CAP_BACK_BODY);
    }
    loft_xoff = 0.0f;
    a22_wings(p);
    {
        float vd = elev * 0.30f;                                                    // both ruddervators for pitch, differently for yaw
        a_vtail22(&AV_A22, +1, vd - pl.rud * 0.20f, p);
        a_vtail22(&AV_A22, -1, vd + pl.rud * 0.20f, p);
    }
    for (int sx = -1; sx <= 1; sx += 2)
        prism(sx * (A22_X + 0.50f), 2.10f, 0.60f, sx * (A22_X + 0.66f), 2.22f, 0.30f, 0.04f, 0.07f, 62, 62, 68);   // exhaust stub
    build_a22_gear(p);
    prism(0, 2.42f, -1.20f, 0, 2.82f, -1.45f, 0.006f, 0.013f, 40, 40, 48);                                  // antenna
    prop_pusher = true; prop_pusher_z = -1.75f;
    build_prop_x(-A22_X, 0, true);
    build_prop_x( A22_X, 0, false);
    prop_pusher = false;
}

// From the seat you can see the wing and the propellers when you look out to the side.
static void build_cockpit_parts_a22(void) {
    a22_wings(&P_A22);
    Paint nac = P_A22; nac.body[0] = 238; nac.body[1] = 240; nac.body[2] = 246;
    for (int sx = -1; sx <= 1; sx += 2) {                                           // the engine nacelles are just outside the pod: you see them when you look out
        loft_xoff = sx * A22_X;
        loft_car(KS_A22_PN, 5, Z_A22_PN, 5, &nac, 2, CAP_FRONT_DARK | CAP_BACK_BODY);
        loft_xoff = sx * A22_BX;
        loft_car(KS_A22_PB, 6, Z_A22_PB, 6, &nac, 2, CAP_FRONT_BODY | CAP_BACK_BODY);
    }
    loft_xoff = 0.0f;
    prop_pusher = true; prop_pusher_z = -1.75f;
    build_prop_x(-A22_X, 1, false);
    build_prop_x( A22_X, 1, false);
    prop_pusher = false;
}
static void build_cockpit_nose_a22(void) {
    loft_car(KS_A22_NOSE, 4, Z_A22_NOSE, (int)(sizeof(Z_A22_NOSE) / sizeof(Z_A22_NOSE[0])), &P_A22, 3, 0);
}

static void draw_plane_shadow_a22(void) {
    V3 w[8];
    const unsigned int col = RGBA8(0, 0, 0, 85);
    static const float FX[8] = { -0.30f, -0.88f, -0.90f, -0.25f, 0.25f, 0.90f, 0.88f, 0.30f };
    static const float FZ[8] = { 3.75f, 2.30f, -1.60f, -3.55f, -3.55f, -1.60f, 2.30f, 3.75f };
    for (int i = 0; i < 8; i++) w[i] = shadow_pt(MW(FX[i], 1.8f, FZ[i]));
    draw_world_poly(w, 8, col);
    for (int sx = -1; sx <= 1; sx += 2) {
        V3 wg[4] = { shadow_pt(MW(sx * 0.0f, 2.1f, 0.90f)), shadow_pt(MW(sx * 6.6f, 2.2f, 2.60f)),
                     shadow_pt(MW(sx * 6.6f, 2.2f, 1.60f)), shadow_pt(MW(sx * 0.0f, 2.1f, -1.00f)) };
        draw_world_poly(wg, 4, col);
        V3 bm[4] = { shadow_pt(MW(sx * (A22_X - 0.45f), 1.9f, 2.4f)), shadow_pt(MW(sx * (A22_X + 0.45f), 1.9f, 2.4f)),
                     shadow_pt(MW(sx * (A22_X + 0.30f), 2.2f, -1.6f)), shadow_pt(MW(sx * (A22_X - 0.30f), 2.2f, -1.6f)) };
        draw_world_poly(bm, 4, col);
        V3 bb[4] = { shadow_pt(MW(sx * (A22_BX - 0.18f), 2.1f, 2.0f)), shadow_pt(MW(sx * (A22_BX + 0.18f), 2.1f, 2.0f)),
                     shadow_pt(MW(sx * (A22_BX + 0.16f), 2.2f, -6.4f)), shadow_pt(MW(sx * (A22_BX - 0.16f), 2.2f, -6.4f)) };
        draw_world_poly(bb, 4, col);
    }
    for (int sx = -1; sx <= 1; sx += 2) {
        V3 tp[4] = { shadow_pt(MW(0.0f, 3.95f, -5.20f)), shadow_pt(MW(sx * A22_BX, 2.30f, -4.90f)), shadow_pt(MW(sx * A22_BX, 2.30f, -6.60f)), shadow_pt(MW(0.0f, 3.95f, -6.50f)) };
        draw_world_poly(tp, 4, col);
    }
}
