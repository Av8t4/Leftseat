// Left Seat: the Ashley single (working name A11). A small single-engine pusher: a four-seat pod with the engine and propeller at
// the back, a swept-back low wing, twin tail booms that start as fairings on the wing's leading edge and sweep up, and an H-tail.
// Length 8.6 m, span 9.8 m, height 3.4 m.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

// z forward, y up from the ground, origin under the centre of gravity.
static const KS KS_A11[] = {                                                        // the cabin pod
    {   4.70f, 0.28f, 0.88f, 1.32f, 1.10f, 2.4f, 2.4f, 2.3f },
    {   4.30f, 0.50f, 0.80f, 1.60f, 1.20f, 2.5f, 2.5f, 2.4f },
    {   3.70f, 0.68f, 0.74f, 1.98f, 1.36f, 2.6f, 2.6f, 2.6f },
    {   2.90f, 0.76f, 0.72f, 2.12f, 1.42f, 2.7f, 2.7f, 2.8f },
    {   1.70f, 0.76f, 0.72f, 2.10f, 1.42f, 2.7f, 2.7f, 2.8f },
    {   0.50f, 0.75f, 0.74f, 2.10f, 1.42f, 2.7f, 2.7f, 2.8f },
    {  -0.20f, 0.74f, 0.82f, 2.12f, 1.47f, 2.7f, 2.7f, 2.8f },
    {  -0.55f, 0.70f, 1.00f, 2.12f, 1.56f, 2.7f, 2.7f, 2.8f },
    {  -1.00f, 0.60f, 1.50f, 2.12f, 1.81f, 2.7f, 2.7f, 2.8f },
    {  -1.45f, 0.46f, 1.92f, 2.10f, 2.01f, 2.7f, 2.7f, 2.8f },
};
static const float Z_A11[] = { 4.70f, 4.40f, 4.10f, 3.70f, 3.30f, 2.90f, 2.30f, 1.70f, 1.10f, 0.50f, 0.15f, -0.20f, -0.55f, -1.00f, -1.45f };
static const KS KS_A11_NOSE[] = {                                                   // the nose in front of you: lower than the outside, below the eye
    {   4.70f, 0.28f, 0.88f, 1.14f, 1.01f, 2.4f, 2.4f, 2.3f },
    {   4.30f, 0.50f, 0.80f, 1.36f, 1.08f, 2.5f, 2.5f, 2.4f },
    {   3.70f, 0.68f, 0.74f, 1.62f, 1.18f, 2.6f, 2.6f, 2.6f },
    {   3.30f, 0.72f, 0.73f, 1.74f, 1.24f, 2.7f, 2.7f, 2.7f },
};
static const float Z_A11_NOSE[] = { 4.70f, 4.30f, 3.70f, 3.30f };
static const KS KS_A11_ENG[] = {                                                    // the engine: a compact box on top of the fuselage (opposed pistons)
    {  0.60f, 0.26f, 2.02f, 2.36f, 2.19f, 3.2f, 3.2f, 3.2f },
    {  0.20f, 0.36f, 2.00f, 2.54f, 2.27f, 3.2f, 3.2f, 3.2f },
    { -0.50f, 0.38f, 2.00f, 2.56f, 2.28f, 3.2f, 3.2f, 3.2f },
    { -1.50f, 0.34f, 2.00f, 2.50f, 2.25f, 3.2f, 3.2f, 3.2f },
};
static const float Z_A11_ENG[] = { 0.60f, 0.35f, 0.00f, -0.50f, -1.00f, -1.50f };
#define A11_BX 2.35f                                                                // the booms stand this far out, clear of the propeller disc
static const KS KS_A11_B[] = {                                                      // a tail boom: a fairing on the wing's leading edge that sweeps up to the tail
    {  0.80f, 0.05f, 0.98f, 1.12f, 1.05f, 2.3f, 2.3f, 2.3f },
    {  0.35f, 0.15f, 0.84f, 1.28f, 1.06f, 2.3f, 2.3f, 2.3f },
    { -0.60f, 0.19f, 0.84f, 1.34f, 1.09f, 2.3f, 2.3f, 2.3f },
    { -2.50f, 0.18f, 1.28f, 1.78f, 1.53f, 2.3f, 2.3f, 2.3f },
    { -5.30f, 0.15f, 1.80f, 2.20f, 2.00f, 2.3f, 2.3f, 2.3f },
};
static const float Z_A11_B[] = { 0.80f, 0.35f, -0.60f, -2.50f, -5.30f };
static const KS KS_A11_TF[] = {                                                     // narrow fairing where each stabiliser meets its fin: swells round the join, then tapers to a point behind it
    { -4.45f, 0.10f, 3.14f, 3.34f, 3.24f, 2.3f, 2.3f, 2.3f },                       // (raised so it covers the top of the fin and the rudder's top edge)
    { -4.80f, 0.17f, 3.04f, 3.44f, 3.24f, 2.3f, 2.3f, 2.3f },
    { -5.25f, 0.19f, 3.02f, 3.46f, 3.24f, 2.3f, 2.3f, 2.3f },
    { -5.65f, 0.13f, 3.08f, 3.42f, 3.25f, 2.3f, 2.3f, 2.3f },
    { -6.00f, 0.025f, 3.19f, 3.31f, 3.25f, 2.3f, 2.3f, 2.3f },
};
static const float Z_A11_TF[] = { -4.45f, -4.65f, -4.90f, -5.15f, -5.40f, -5.65f, -5.85f, -6.00f };
static AWingS AW_A11;                                                               // swept-back wing
static void a11_init(void) {
    static bool done = false;
    if (done) return;
    done = true;
    sw_make(&AW_A11, 7, 0.00f, 4.90f, 1.30f, -0.35f, -0.55f, -1.30f, 0.150f, 0.078f, 1.00f, 0.030f, 0, 4);
    AW_A11.tipcol[0] = 20; AW_A11.tipcol[1] = 120; AW_A11.tipcol[2] = 130;
}
static const ATail AF_A11 = { 5, { 2.00f, 2.35f, 2.70f, 3.05f, 3.35f }, { -3.70f, -4.00f, -4.30f, -4.60f, -4.85f }, { 1.65f, 1.45f, 1.20f, 1.00f, 0.80f },
                              { 0.09f, 0.08f, 0.07f, 0.06f, 0.05f }, 2.50f };
static const AStab AS_A11 = { 5, { 0.00f, 0.60f, 1.20f, 1.80f, 2.35f }, { -4.75f, -4.75f, -4.75f, -4.75f, -4.75f }, { 0.95f, 0.92f, 0.90f, 0.88f, 0.85f },
                              { 0.090f, 0.085f, 0.080f, 0.075f, 0.070f }, 3.05f };
static const Paint P_A11 = { {246,247,250}, {20,120,130}, {250,200,60}, {226,230,238}, {58,104,156}, {26,26,30}, {96,102,112}, {20,120,130} };

static void a11_wings(const Paint *p) {
    a11_init();
    float flap = -sinf(pl.flaps * 10.0f * PI_F / 180.0f) * 0.75f, ail = pl.ail * 0.30f;
    float dR[8] = { flap, flap, 0, ail, ail, ail * 0.5f, 0, 0 }, dL[8] = { flap, flap, 0, -ail, -ail, -ail * 0.5f, 0, 0 };
    a_wing_sw(&AW_A11, +1, dR, p, false);
    a_wing_sw(&AW_A11, -1, dL, p, true);
}

// Fixed tricycle gear: each main leg comes down from the pod's belly, and the nose leg from under the nose.
static void build_a11_gear(const Paint *p) {
    prism(0.0f, 1.00f, 4.00f, 0.0f, 0.24f, 3.95f, 0.04f, 0.07f, p->metal[0], p->metal[1], p->metal[2]);
    a_tyre30(0.0f, 0.24f, 3.95f, 0.24f, 0.08f, p);
    for (int sx = -1; sx <= 1; sx += 2) {
        prism(sx * 1.00f, 0.95f, -0.55f, sx * 1.15f, 0.30f, -0.55f, 0.05f, 0.09f, p->metal[0], p->metal[1], p->metal[2]);
        a_tyre30(sx * 1.15f, 0.30f, -0.55f, 0.30f, 0.10f, p);
    }
}


static void build_a11(void) {
    npf = 0; pf_last = -1;
    const Paint *p = &P_A11;
    float elev = clampf(pl.ele + pl.trim, -1.0f, 1.0f);
    loft_car(KS_A11, 10, Z_A11, (int)(sizeof(Z_A11) / sizeof(Z_A11[0])), p, 8, CAP_FRONT_BODY | CAP_BACK_BODY);
    Paint eg = *p; eg.body[0] = 232; eg.body[1] = 235; eg.body[2] = 242;
    loft_car(KS_A11_ENG, 4, Z_A11_ENG, (int)(sizeof(Z_A11_ENG) / sizeof(Z_A11_ENG[0])), &eg, 2, CAP_FRONT_DARK | CAP_BACK_BODY);
    Paint bm = *p; bm.body[0] = 238; bm.body[1] = 240; bm.body[2] = 246;
    for (int sx = -1; sx <= 1; sx += 2) {
        loft_xoff = sx * A11_BX;
        loft_car(KS_A11_B, 5, Z_A11_B, 5, &bm, 2, CAP_FRONT_BODY | CAP_BACK_BODY);
        loft_car(KS_A11_TF, 5, Z_A11_TF, 8, &bm, 2, CAP_BACK_BODY);
        fin_xoff = sx * A11_BX;
        a_fin(&AF_A11, pl.rud, p);
    }
    loft_xoff = 0.0f; fin_xoff = 0.0f;
    a11_wings(p);
    a_stab(&AS_A11, +1, elev * 0.28f, p);
    a_stab(&AS_A11, -1, elev * 0.28f, p);
    build_a11_gear(p);
    prism(0, 2.12f, 1.60f, 0, 2.50f, 1.35f, 0.006f, 0.013f, 40, 40, 48);                                  // antenna
    prop_pusher = true; prop_pusher_z = -1.65f;
    build_prop_x(0.0f, 0, true);
    prop_pusher = false;
}

// From the seat you can see the wing when you look out to the side.
static void build_cockpit_parts_a11(void) { a11_wings(&P_A11); }
static void build_cockpit_nose_a11(void) {
    loft_car(KS_A11_NOSE, 4, Z_A11_NOSE, (int)(sizeof(Z_A11_NOSE) / sizeof(Z_A11_NOSE[0])), &P_A11, 3, 0);
}

static void draw_plane_shadow_a11(void) {
    V3 w[8];
    const unsigned int col = RGBA8(0, 0, 0, 85);
    static const float FX[8] = { -0.25f, -0.76f, -0.76f, -0.70f, 0.70f, 0.76f, 0.76f, 0.25f };
    static const float FZ[8] = { 4.70f, 3.70f, 0.15f, -1.45f, -1.45f, 0.15f, 3.70f, 4.70f };
    for (int i = 0; i < 8; i++) w[i] = shadow_pt(MW(FX[i], 1.6f, FZ[i]));
    draw_world_poly(w, 8, col);
    { V3 eb[4] = { shadow_pt(MW(-0.3f, 2.3f, 0.6f)), shadow_pt(MW(0.3f, 2.3f, 0.6f)), shadow_pt(MW(0.36f, 2.3f, -1.5f)), shadow_pt(MW(-0.36f, 2.3f, -1.5f)) }; draw_world_poly(eb, 4, col); }
    for (int sx = -1; sx <= 1; sx += 2) {
        V3 wg[4] = { shadow_pt(MW(sx * 0.0f, 1.0f, 1.30f)), shadow_pt(MW(sx * 4.9f, 1.1f, -0.35f)),
                     shadow_pt(MW(sx * 4.9f, 1.1f, -1.30f)), shadow_pt(MW(sx * 0.0f, 1.0f, -0.55f)) };
        draw_world_poly(wg, 4, col);
        V3 bb[4] = { shadow_pt(MW(sx * (A11_BX - 0.18f), 2.0f, 0.8f)), shadow_pt(MW(sx * (A11_BX + 0.18f), 2.0f, 0.8f)),
                     shadow_pt(MW(sx * (A11_BX + 0.15f), 2.0f, -5.3f)), shadow_pt(MW(sx * (A11_BX - 0.15f), 2.0f, -5.3f)) };
        draw_world_poly(bb, 4, col);
        V3 tp[4] = { shadow_pt(MW(0.0f, 3.0f, -4.75f)), shadow_pt(MW(sx * A11_BX, 3.0f, -4.75f)), shadow_pt(MW(sx * A11_BX, 3.0f, -5.65f)), shadow_pt(MW(0.0f, 3.0f, -5.65f)) };
        draw_world_poly(tp, 4, col);
    }
}
