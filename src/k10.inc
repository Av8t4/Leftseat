// Left Seat: the K-10 Hauler. Built from these dimensions (length 12.67 m, span 15.88 m, height 4.5 m)
// out of smooth lofted sections, airfoil wings and shaped tail surfaces (see modelkit.inc).
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* The aircraft                                                         */
/* ------------------------------------------------------------------ */
// z forward, y up from the ground, origin under the centre of gravity. The windshield stands steeply (about 40 degrees).
static const KS KS_K10[] = {
    {  4.45f, 0.42f, 1.40f, 2.00f, 1.70f, 2.4f, 2.4f, 2.3f },
    {  3.90f, 0.52f, 1.30f, 2.12f, 1.72f, 2.5f, 2.5f, 2.4f },
    {  3.30f, 0.66f, 1.18f, 2.22f, 1.72f, 2.7f, 2.6f, 2.6f },
    {  2.75f, 0.80f, 1.05f, 2.28f, 1.70f, 2.9f, 2.8f, 2.8f },
    {  2.40f, 0.90f, 1.00f, 2.30f, 1.70f, 3.0f, 3.0f, 3.0f },
    {  2.00f, 0.95f, 0.97f, 2.58f, 1.74f, 3.2f, 3.2f, 3.4f },
    {  1.70f, 0.97f, 0.95f, 2.66f, 1.78f, 3.2f, 3.2f, 3.6f },
    {  1.20f, 0.98f, 0.95f, 2.68f, 1.80f, 3.2f, 3.2f, 3.6f },
    { -1.20f, 0.98f, 0.95f, 2.68f, 1.80f, 3.2f, 3.2f, 3.6f },
    { -2.80f, 0.97f, 0.97f, 2.64f, 1.80f, 3.2f, 3.2f, 3.5f },
    { -4.00f, 0.84f, 1.10f, 2.52f, 1.78f, 3.0f, 3.0f, 3.2f },
    { -5.20f, 0.60f, 1.36f, 2.40f, 1.76f, 2.7f, 2.7f, 2.8f },
    { -6.40f, 0.34f, 1.62f, 2.30f, 1.74f, 2.4f, 2.4f, 2.5f },
    { -7.20f, 0.17f, 1.84f, 2.22f, 1.74f, 2.3f, 2.3f, 2.3f },
    { -7.62f, 0.09f, 1.94f, 2.16f, 1.74f, 2.2f, 2.2f, 2.2f },
};
#if MODEL_DETAIL >= 2
static const float Z208[] = { 4.45f, 4.05f, 3.65f, 3.25f, 2.85f, 2.55f, 2.30f, 2.08f, 1.88f, 1.65f, 1.35f, 1.00f, 0.60f, 0.20f, -0.20f, -0.60f, -1.00f,
                              -1.40f, -1.80f, -2.20f, -2.60f, -3.00f, -3.45f, -3.95f, -4.55f, -5.25f, -6.00f, -6.75f, -7.62f };
#else
static const float Z208[] = { 4.45f, 3.85f, 3.25f, 2.65f, 2.20f, 1.90f, 1.60f, 1.00f, 0.20f, -0.60f, -1.40f, -2.20f, -3.00f, -3.95f, -5.25f, -6.75f, -7.62f };
#endif
static const KS KS_K10_POD[] = {
    {  1.30f, 0.50f, 0.62f, 1.00f, 0.86f, 2.6f, 2.6f, 2.6f },
    {  0.20f, 0.74f, 0.34f, 1.00f, 0.74f, 3.0f, 2.8f, 3.0f },
    { -3.10f, 0.74f, 0.38f, 1.00f, 0.74f, 3.0f, 2.8f, 3.0f },
    { -4.70f, 0.40f, 0.72f, 1.00f, 0.88f, 2.6f, 2.6f, 2.6f },
};
static const float Z208P[] = { 1.30f, 0.60f, -0.40f, -2.40f, -3.60f, -4.70f };
static const float Z208N[] = { 4.45f, 4.05f, 3.65f, 3.25f, 2.85f, 2.55f, 2.30f };                  // the nose, seen from the cockpit
static const KS KS_K10_NOSE[] = {                                                                       // a little lower than the outside, so you look down onto it
    { 4.45f, 0.42f, 1.34f, 1.90f, 1.62f, 2.4f, 2.4f, 2.3f },
    { 3.90f, 0.52f, 1.24f, 1.96f, 1.62f, 2.5f, 2.5f, 2.4f },
    { 3.30f, 0.66f, 1.12f, 2.00f, 1.62f, 2.7f, 2.6f, 2.6f },
    { 2.75f, 0.80f, 1.00f, 2.02f, 1.62f, 2.9f, 2.8f, 2.8f },
    { 2.30f, 0.90f, 0.96f, 2.04f, 1.62f, 3.0f, 3.0f, 3.0f },
};
static const AWing AW_K10 = { 8, { 0.00f, 1.20f, 5.00f, 5.50f, 6.90f, 7.55f, 7.85f, 7.94f }, { 1.78f, 1.78f, 1.78f, 1.78f, 1.76f, 1.58f, 1.18f, 0.74f },
                              { 0.30f, 0.30f, 0.29f, 0.27f, 0.23f, 0.18f, 0.13f, 0.08f }, { 0, 0, 0, 0, 0, 0.02f, 0.05f, 0.09f }, 2.80f, 0.024f, 0.75f,
                              { 1, 1, 1, 0, 1, 0, 0, 0 }, { 236, 126, 30 } };
static const ATail AF_K10 = { 5, { 2.30f, 3.00f, 3.65f, 4.15f, 4.55f }, { -4.46f, -5.04f, -5.66f, -6.20f, -6.62f }, { 3.04f, 2.58f, 2.10f, 1.66f, 1.12f },
                              { 0.085f, 0.075f, 0.065f, 0.052f, 0.040f }, 2.45f };
static const AStab AS_K10 = { 5, { 0.16f, 1.00f, 1.90f, 2.60f, 2.84f }, { -6.34f, -6.50f, -6.68f, -6.84f, -6.92f }, { 1.52f, 1.40f, 1.14f, 0.82f, 0.46f },
                              { 0.15f, 0.13f, 0.105f, 0.075f, 0.05f }, 2.30f };
static const PS PS_K10_SCOOP[] = { { 4.05f, 0.18f, 1.22f, 1.40f }, { 3.55f, 0.30f, 1.06f, 1.46f }, { 3.00f, 0.32f, 1.10f, 1.40f }, { 2.55f, 0.20f, 1.16f, 1.34f } };
static const Paint P_K10 = { {246,247,250}, {236,126,30}, {26,52,120}, {246,247,250}, {30,52,86}, {26,26,30}, {96,102,112}, {236,126,30} };

static void build_k10(void) {
    npf = 0; pf_last = -1;
    const Paint *p = &P_K10;
    float elev = clampf(pl.ele + pl.trim, -1.0f, 1.0f);
    float flap = -sinf(pl.flaps * 10.0f * PI_F / 180.0f) * 0.55f, ail = pl.ail * 0.22f;
    loft_car(KS_K10, 15, Z208, (int)(sizeof(Z208) / sizeof(Z208[0])), p, 1, CAP_FRONT_DARK | CAP_BACK_BODY);
    Paint pod = *p; pod.body[0] = 214; pod.body[1] = 220; pod.body[2] = 232;
    loft_car(KS_K10_POD, 4, Z208P, 6, &pod, 2, CAP_FRONT_BODY | CAP_BACK_BODY);
    float dR[8] = { flap, flap, flap, 0, ail, ail, ail * 0.5f, 0 }, dL[8] = { flap, flap, flap, 0, -ail, -ail, -ail * 0.5f, 0 };
    a_wing(&AW_K10, +1, dR, p, false);
    a_wing(&AW_K10, -1, dL, p, true);
    float ed = elev * 0.20f;
    a_stab(&AS_K10, +1, ed, p);
    a_stab(&AS_K10, -1, ed, p);
    a_fin(&AF_K10, pl.rud, p);
    a_pant(PS_K10_SCOOP, 4, 0.0f, p);                                                                          // air scoop under the nose
    for (int sx = -1; sx <= 1; sx += 2) {
        prism(sx * 1.02f, 1.10f, 0.22f, sx * 4.60f, 2.68f, 0.14f, 0.028f, 0.095f, 238, 240, 244);              // wing strut
        prism(sx * 2.90f, 1.94f, 0.18f, sx * 3.30f, 2.72f, 0.14f, 0.015f, 0.055f, 238, 240, 244);              // jury strut
        prism(sx * 0.96f, 1.32f, -0.45f, sx * 1.74f, 0.60f, -0.55f, 0.050f, 0.095f, p->metal[0], p->metal[1], p->metal[2]);   // main gear leg
        prism(sx * 0.96f, 0.98f, -0.22f, sx * 1.72f, 0.54f, -0.50f, 0.026f, 0.050f, p->metal[0], p->metal[1], p->metal[2]);   // brace
        a_tyre(sx * 1.82f, 0.37f, -0.55f, 0.37f, 0.125f, sx, p);
        prism(sx * 0.58f, 1.70f, 3.40f, sx * 0.80f, 2.06f, 2.90f, 0.045f, 0.055f, 62, 62, 68);                  // exhaust stack
        prism(sx * 0.40f, 1.88f, -6.92f, sx * 1.55f, 2.26f, -6.96f, 0.020f, 0.045f, 238, 240, 244);              // tail strut
    }
    prism(0, 1.34f, 2.55f, 0, 0.34f, 2.75f, 0.042f, 0.065f, p->metal[0], p->metal[1], p->metal[2]);           // nose gear
    prism(0.0f, 0.52f, 2.84f, 0.0f, 0.46f, 2.66f, 0.014f, 0.024f, 20, 20, 24);
    a_tyre(0, 0.30f, 2.75f, 0.30f, 0.075f, 0, p);
    prism(0, 2.70f, -1.40f, 0, 3.12f, -1.78f, 0.006f, 0.013f, 40, 40, 48);                                    // antenna
    build_prop(0);
}

// From the seat you can see the wings and struts when you look out to the side.
static void build_cockpit_parts_k10(void) {
    const Paint *p = &P_K10;
    float flap = -sinf(pl.flaps * 10.0f * PI_F / 180.0f) * 0.55f, ail = pl.ail * 0.22f;
    float dR[8] = { flap, flap, flap, 0, ail, ail, ail * 0.5f, 0 }, dL[8] = { flap, flap, flap, 0, -ail, -ail, -ail * 0.5f, 0 };
    a_wing(&AW_K10, +1, dR, p, false);
    a_wing(&AW_K10, -1, dL, p, true);
    for (int sx = -1; sx <= 1; sx += 2) {
        prism(sx * 1.02f, 1.10f, 0.22f, sx * 4.60f, 2.68f, 0.14f, 0.028f, 0.095f, 238, 240, 244);
        prism(sx * 2.90f, 1.94f, 0.18f, sx * 3.30f, 2.72f, 0.14f, 0.015f, 0.055f, 238, 240, 244);
    }
}
static void build_cockpit_nose_k10(void) {
    loft_car(KS_K10_NOSE, 5, Z208N, (int)(sizeof(Z208N) / sizeof(Z208N[0])), &P_K10, 3, 0);
    build_prop(1);
}

static void draw_plane_shadow_k10(void) {
    V3 w[8];
    const unsigned int col = RGBA8(0, 0, 0, 85);
    static const float FX[8] = { -0.50f, -0.95f, -1.00f, -0.15f, 0.15f, 1.00f, 0.95f, 0.50f };
    static const float FZ[8] = { 4.40f, 3.00f, -2.80f, -7.60f, -7.60f, -2.80f, 3.00f, 4.40f };
    for (int i = 0; i < 8; i++) w[i] = shadow_pt(MW(FX[i], 1.9f, FZ[i]));
    draw_world_poly(w, 8, col);
    for (int sx = -1; sx <= 1; sx += 2) {
        V3 wg[4] = { shadow_pt(MW(sx * 1.0f, 2.8f, 0.75f)), shadow_pt(MW(sx * 7.94f, 3.0f, 0.75f)),
                     shadow_pt(MW(sx * 7.94f, 3.0f, -0.87f)), shadow_pt(MW(sx * 1.0f, 2.8f, -1.03f)) };
        draw_world_poly(wg, 4, col);
        V3 tl[4] = { shadow_pt(MW(sx * 0.30f, 2.28f, -6.40f)), shadow_pt(MW(sx * 2.80f, 2.28f, -6.80f)),
                     shadow_pt(MW(sx * 2.80f, 2.28f, -7.75f)), shadow_pt(MW(sx * 0.30f, 2.28f, -7.85f)) };
        draw_world_poly(tl, 4, col);
    }
}

