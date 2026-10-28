// Left Seat: the 3D K-4 model, cockpit nose and the aircraft shadow.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* rendering                                                           */
/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/* K-4 model: rebuilt in world space every frame so the control */
/* surfaces, flaps and propeller move with the aircraft.               */
/* ------------------------------------------------------------------ */

#define MAX_PFACES 1500
#define PF_TWO     1      // visible from both sides (blades, antenna)
#define PF_EMIT    2      // unshaded, glows (lights)
#define PF_DECAL   4      // sits on the previous solid face (stripes, windows)

typedef struct {
    V3 p[8];
    int n;
    unsigned int color;
    V3 nrm, c;
    bool twosided, isdecal, emit;
    int ndecals;
} PFace;

static PFace pfaces[MAX_PFACES];
static int   npf, pf_last;
static float prop_phase;
static const V3 SUN = { 0.50f, 0.78f, 0.37f };

// Colors (r, g, b) used by the model.
#define C_WHITE  246, 247, 250
#define C_LGRAY  218, 224, 234
#define C_GRAY   150, 156, 168
#define C_RED    204, 36, 40
#define C_NAVY   26, 52, 120
#define C_GLASS  34, 62, 104
#define C_BLACK  30, 30, 34
#define C_STEEL  96, 102, 112
#define C_YELLOW 246, 200, 44

// Model space: x right, y up from the ground, z forward, origin under the center of gravity.
// The aircraft pivots about a point 1.15 m above the wheels.
static V3 MW(float x, float y, float z) {
    V3 o = v3(pl.pos.x, pl.pos.y + spec->piv_y, pl.pos.z);
    o = vadd(o, vscale(pl.r, x));
    o = vadd(o, vscale(pl.u, y - spec->piv_y));
    o = vadd(o, vscale(pl.f, z));
    return o;
}

static void pf_poly(int n, const V3 *pts, const V3 *center, float r, float g, float b, int flags, int alpha) {
    if (npf >= MAX_PFACES || n < 3 || n > 8) return;
    PFace *f = &pfaces[npf];
    f->n = n;
    f->isdecal  = (flags & PF_DECAL) != 0;
    f->twosided = (flags & PF_TWO) != 0;
    f->emit     = (flags & PF_EMIT) != 0;
    f->ndecals  = 0;
    V3 c = v3(0, 0, 0);
    for (int i = 0; i < n; i++) { f->p[i] = pts[i]; c = vadd(c, pts[i]); }
    f->c = vscale(c, 1.0f / n);

    V3 nr = vcross(vsub(pts[1], pts[0]), vsub(pts[2], pts[0]));
    float len = sqrtf(vdot(nr, nr));
    nr = len > 1e-7f ? vscale(nr, 1.0f / len) : v3(0, 1, 0);
    if (center && !f->isdecal && vdot(nr, vsub(f->c, *center)) < 0.0f) {    // make the normal point outward
        for (int i = 0; i < n; i++) f->p[i] = pts[n - 1 - i];
        nr = vscale(nr, -1.0f);
    }
    if (f->isdecal && pf_last >= 0) { nr = pfaces[pf_last].nrm; pfaces[pf_last].ndecals++; }
    else if (!f->isdecal)           pf_last = npf;
    f->nrm = nr;

    float shade = 1.0f;
    if (!(flags & PF_EMIT)) {
        float dl = vdot(nr, SUN);
        if (f->twosided) dl = fabsf(dl);
        shade = 0.46f + 0.54f * fmaxf(0.0f, dl) + 0.10f * fmaxf(0.0f, nr.y) + 0.12f * fmaxf(0.0f, -nr.y);
        if (shade > 1.08f) shade = 1.08f;
    }
    f->color = RGBA8((int)clampf(r * shade, 0, 255), (int)clampf(g * shade, 0, 255),
                     (int)clampf(b * shade, 0, 255), alpha);
    npf++;
}

static V3 avgv(const V3 *r, int n) {
    V3 c = v3(0, 0, 0);
    for (int i = 0; i < n; i++) c = vadd(c, r[i]);
    return vscale(c, 1.0f / n);
}

// Point inside a quad A,B (section i) / D,C (section i+1) at fractions s (along A-B) and t (along A-D).
static V3 bilerp(V3 A, V3 B, V3 C, V3 D, float s, float t) {
    V3 p0 = vadd(vscale(A, 1 - s), vscale(B, s));
    V3 p1 = vadd(vscale(D, 1 - s), vscale(C, s));
    return vadd(vscale(p0, 1 - t), vscale(p1, t));
}
static void decal_quad(V3 A, V3 B, V3 C, V3 D, float s0, float s1, float t0, float t1, float r, float g, float b) {
    V3 q[4] = { bilerp(A, B, C, D, s0, t0), bilerp(A, B, C, D, s1, t0),
                bilerp(A, B, C, D, s1, t1), bilerp(A, B, C, D, s0, t1) };
    pf_poly(4, q, NULL, r, g, b, PF_DECAL, 255);
}

// Fuselage-style octagonal cross-sections lofted along z.
typedef struct { float z, w, yb, yt; } FusSec;
static const FusSec FUS[10] = {
    { 3.05f, 0.42f, 0.78f, 1.40f }, { 2.55f, 0.55f, 0.68f, 1.52f }, { 2.05f, 0.62f, 0.64f, 1.58f },
    { 1.45f, 0.62f, 0.64f, 1.62f }, { 0.60f, 0.62f, 0.66f, 1.98f }, {-0.60f, 0.60f, 0.68f, 2.00f },
    {-1.30f, 0.50f, 0.74f, 1.82f }, {-2.60f, 0.32f, 0.86f, 1.64f }, {-4.00f, 0.13f, 1.04f, 1.46f },
    {-4.65f, 0.06f, 1.12f, 1.36f }
};
static const FusSec PANT[4] = {
    { 0.33f, 0.04f, 0.20f, 0.46f }, {-0.02f, 0.11f, 0.06f, 0.62f }, {-0.40f, 0.09f, 0.08f, 0.58f },
    {-0.72f, 0.02f, 0.26f, 0.42f }
};
static const unsigned char EC_BODY[8][3] = {
    { 246, 247, 250 }, { 246, 247, 250 }, { 240, 242, 247 }, { 218, 224, 234 },
    { 150, 156, 168 }, { 218, 224, 234 }, { 240, 242, 247 }, { 246, 247, 250 } };
static const unsigned char EC_PANT[8][3] = {
    { 246, 247, 250 }, { 240, 242, 247 }, { 232, 235, 242 }, { 214, 220, 232 },
    { 190, 196, 208 }, { 214, 220, 232 }, { 232, 235, 242 }, { 240, 242, 247 } };

static void oct_ring(V3 *out, const FusSec *s, float xoff) {
    float yc = 0.5f * (s->yb + s->yt), hh = 0.5f * (s->yt - s->yb), w = s->w;
    float xs[8] = { -0.6f * w, 0.6f * w, w, w, 0.6f * w, -0.6f * w, -w, -w };
    float ys[8] = { s->yt, s->yt, yc + 0.45f * hh, yc - 0.45f * hh, s->yb, s->yb, yc - 0.45f * hh, yc + 0.45f * hh };
    for (int i = 0; i < 8; i++) out[i] = MW(xoff + xs[i], ys[i], s->z);
}

static const unsigned char LIV[2][2][3] = { { { 204, 36, 40 }, { 26, 52, 120 } },       // K-4: red and navy
                                            { { 236, 126, 30 }, { 26, 52, 120 } } };     // K-10: orange and navy

static void loft_body(const FusSec *S, int ns, float xoff, const unsigned char ec[8][3], int decor, int cap) {
    V3 ring[10][8], cen[10];
    for (int i = 0; i < ns; i++) { oct_ring(ring[i], &S[i], xoff); cen[i] = avgv(ring[i], 8); }
    for (int i = 0; i < ns - 1; i++) {
        V3 cc = vscale(vadd(cen[i], cen[i + 1]), 0.5f);
        for (int e = 0; e < 8; e++) {
            V3 A = ring[i][e], B = ring[i][(e + 1) & 7], C = ring[i + 1][(e + 1) & 7], D = ring[i + 1][e];
            V3 q[4] = { A, B, C, D };
            pf_poly(4, q, &cc, ec[e][0], ec[e][1], ec[e][2], 0, 255);
            if (decor == 2) {                                                  // cockpit nose: black anti-glare panel
                if (e == 0) decal_quad(A, B, C, D, 0.10f, 0.90f, 0.0f, 1.0f, 36, 38, 44);
                continue;
            }
            if (decor == 3) {                                                  // K-10: stripe, side windows and windshield
                if (e == 2 || e == 6) {
                    decal_quad(A, B, C, D, 0.62f, 0.72f, 0.0f, 1.0f, LIV[1][0][0], LIV[1][0][1], LIV[1][0][2]);
                    decal_quad(A, B, C, D, 0.72f, 0.82f, 0.0f, 1.0f, LIV[1][1][0], LIV[1][1][1], LIV[1][1][2]);
                    if (i >= 3 && i <= 6) decal_quad(A, B, C, D, 0.06f, 0.52f, 0.12f, 0.88f, C_GLASS);
                }
                if (i == 3 && e == 0) {
                    decal_quad(A, B, C, D, 0.08f, 0.92f, 0.08f, 0.92f, C_GLASS);
                    decal_quad(A, B, C, D, 0.12f, 0.40f, 0.12f, 0.88f, 118, 160, 210);
                }
                continue;
            }
            if (!decor) continue;
            if (e == 2 || e == 6) {                                           // cheat line
                decal_quad(A, B, C, D, 0.42f, 0.54f, 0.0f, 1.0f, LIV[spec->model][0][0], LIV[spec->model][0][1], LIV[spec->model][0][2]);
                decal_quad(A, B, C, D, 0.54f, 0.67f, 0.0f, 1.0f, LIV[spec->model][1][0], LIV[spec->model][1][1], LIV[spec->model][1][2]);
            }
            if (i >= 3 && i <= 5) {                                           // cabin glass
                if (e == 1 || e == 7) decal_quad(A, B, C, D, 0.06f, 0.94f, 0.05f, 0.95f, C_GLASS);
                if ((e == 2 || e == 6) && i >= 4) decal_quad(A, B, C, D, 0.0f, 0.30f, 0.05f, 0.95f, C_GLASS);
            }
            if (i == 3 && e == 0) {
                decal_quad(A, B, C, D, 0.08f, 0.92f, 0.06f, 0.92f, C_GLASS);                      // windshield
                decal_quad(A, B, C, D, 0.12f, 0.40f, 0.12f, 0.88f, 118, 160, 210);                // sky reflection
            }
            if (i == 5 && e == 0) decal_quad(A, B, C, D, 0.20f, 0.80f, 0.10f, 0.65f, C_GLASS);   // rear window
        }
    }
    if (cap) pf_poly(8, ring[0], &cen[1], 52, 56, 64, 0, 255);               // cowling intake
}

typedef struct { float xs[4], ch[4], th[4], y0, dih, z0; float tip[3]; } WingSpec;
static const WingSpec WINGS[2] = {
    { { 0.00f, 2.70f, 2.95f, 5.50f }, { 1.62f, 1.62f, 1.35f, 1.12f }, { 0.24f, 0.22f, 0.17f, 0.10f }, 2.06f, 0.026f, 0.88f, { 204, 36, 40 } },
    { { 0.00f, 5.00f, 5.25f, 7.94f }, { 1.78f, 1.78f, 1.76f, 1.62f }, { 0.30f, 0.28f, 0.26f, 0.18f }, 2.78f, 0.026f, 0.75f, { 236, 126, 30 } },
};
typedef struct { float xs[2], z0[2], ch[2], th[2], yc; } HTailSpec;
static const HTailSpec HTAILS[2] = {
    { { 0.12f, 1.72f }, { -3.55f, -3.88f }, { 1.05f, 0.74f }, { 0.09f, 0.05f }, 1.28f },
    { { 0.15f, 2.80f }, { -6.35f, -6.80f }, { 1.50f, 0.95f }, { 0.15f, 0.08f }, 2.28f },
};
typedef struct { float y[3], zl[3], zt[3], t[3], ramp0, ramph; } FinSpec;
static const FinSpec FINS[2] = {
    { { 1.30f, 2.12f, 2.72f }, { -2.58f, -3.36f, -3.98f }, { -4.52f, -4.64f, -4.78f }, { 0.05f, 0.04f, 0.03f }, 1.45f, 1.27f },
    { { 2.35f, 3.50f, 4.55f }, { -4.60f, -5.70f, -6.55f }, { -7.50f, -7.62f, -7.75f }, { 0.07f, 0.06f, 0.04f }, 2.45f, 2.10f },
};

static void build_wing(int sx, float ail_dy, float flap_dy) {
    const WingSpec *W = &WINGS[spec->model];
    const float *XS = W->xs, *CH = W->ch, *TH = W->th;
    float dy[4] = { flap_dy, flap_dy, ail_dy, ail_dy };
    static const unsigned char col[5][3] = { { 246, 247, 250 }, { 246, 247, 250 }, { 205, 212, 224 },
                                             { 214, 220, 232 }, { 222, 228, 238 } };
    V3 sec[4][5], cc[3];
    for (int k = 0; k < 4; k++) {
        float x = sx * XS[k], yc = W->y0 + W->dih * XS[k], c = CH[k], t = TH[k], z0 = W->z0;
        sec[k][0] = MW(x, yc, z0);                                    // leading edge
        sec[k][1] = MW(x, yc + 0.55f * t, z0 - 0.25f * c);            // top, forward
        sec[k][2] = MW(x, yc + 0.05f * t + dy[k], z0 - c);            // trailing edge (top)
        sec[k][3] = MW(x, yc - 0.05f * t + dy[k], z0 - c);            // trailing edge (bottom)
        sec[k][4] = MW(x, yc - 0.45f * t, z0 - 0.35f * c);            // bottom
    }
    for (int k = 0; k < 3; k++) {
        V3 s = v3(0, 0, 0);
        for (int j = 0; j < 5; j++) s = vadd(vadd(s, sec[k][j]), sec[k + 1][j]);
        cc[k] = vscale(s, 0.1f);
    }
    for (int k = 0; k < 3; k++)
        for (int e = 0; e < 5; e++) {
            V3 A = sec[k][e], B = sec[k][(e + 1) % 5], C = sec[k + 1][(e + 1) % 5], D = sec[k + 1][e];
            V3 q[4] = { A, B, C, D };
            pf_poly(4, q, &cc[k], col[e][0], col[e][1], col[e][2], 0, 255);
            if (e == 0 && k == 2 && sx < 0)                                         // landing light
                decal_quad(A, B, C, D, 0.15f, 0.85f, 0.08f, 0.16f, 255, 250, 215);
            if (e == 1 && (k == 0 || k == 2))                                      // flap / aileron hinge line
                decal_quad(A, B, C, D, 0.58f, 0.61f, 0.0f, 1.0f, 120, 126, 136);
        }
    pf_poly(5, sec[3], &cc[2], W->tip[0], W->tip[1], W->tip[2], 0, 255);           // painted tip
    V3 tc = avgv(sec[3], 5);
    V3 nl[4] = { vadd(tc, vscale(pl.f, 0.05f)), vadd(tc, vscale(pl.u, 0.05f)),
                 vadd(tc, vscale(pl.f, -0.05f)), vadd(tc, vscale(pl.u, -0.05f)) };
    pf_poly(4, nl, NULL, sx < 0 ? 255 : 40, sx < 0 ? 40 : 255, 40, PF_DECAL | PF_EMIT, 255);   // nav light
    bool flash = (((frame_count + (sx < 0 ? 0 : 7)) / 3) % 17) == 0;                         // wingtip strobe
    V3 te = sec[3][2];
    V3 sl[4] = { vadd(te, vscale(pl.f, 0.07f)), vadd(te, vscale(pl.u, 0.07f)),
                 vadd(te, vscale(pl.f, -0.07f)), vadd(te, vscale(pl.u, -0.07f)) };
    pf_poly(4, sl, NULL, flash ? 255 : 80, flash ? 255 : 80, flash ? 255 : 84, PF_DECAL | PF_EMIT, 255);
}

static void build_htail(int sx, float dy) {
    const HTailSpec *H = &HTAILS[spec->model];
    const float *XS = H->xs, *Z0 = H->z0, *CH = H->ch, *TH = H->th;
    static const unsigned char col[4][3] = { { 246, 247, 250 }, { 246, 247, 250 }, { 205, 212, 224 }, { 214, 220, 232 } };
    V3 sec[2][4];
    for (int k = 0; k < 2; k++) {
        float x = sx * XS[k], yc = H->yc, c = CH[k], t = TH[k], z0 = Z0[k];
        sec[k][0] = MW(x, yc, z0);
        sec[k][1] = MW(x, yc + 0.5f * t, z0 - 0.3f * c);
        sec[k][2] = MW(x, yc + dy, z0 - c);
        sec[k][3] = MW(x, yc - 0.5f * t, z0 - 0.3f * c);
    }
    V3 cc = vscale(vadd(avgv(sec[0], 4), avgv(sec[1], 4)), 0.5f);
    for (int e = 0; e < 4; e++) {
        V3 A = sec[0][e], B = sec[0][(e + 1) & 3], C = sec[1][(e + 1) & 3], D = sec[1][e];
        V3 q[4] = { A, B, C, D };
        pf_poly(4, q, &cc, col[e][0], col[e][1], col[e][2], 0, 255);
        if (e == 1) decal_quad(A, B, C, D, 0.50f, 0.53f, 0.0f, 1.0f, 120, 126, 136);       // elevator hinge
    }
    pf_poly(4, sec[1], &cc, C_NAVY, 0, 255);
}

static void build_fin(float rud) {
    const FinSpec *F = &FINS[spec->model];
    const float *Y = F->y, *ZL = F->zl, *ZT = F->zt, *T = F->t;
    V3 sec[3][4];
    for (int k = 0; k < 3; k++) {
        float c = ZL[k] - ZT[k], ramp = clampf((Y[k] - F->ramp0) / F->ramph, 0.0f, 1.0f), dx = rud * 0.13f * ramp;
        sec[k][0] = MW(0, Y[k], ZL[k]);
        sec[k][1] = MW(T[k], Y[k], ZL[k] - 0.28f * c);
        sec[k][2] = MW(dx, Y[k], ZT[k]);
        sec[k][3] = MW(-T[k], Y[k], ZL[k] - 0.28f * c);
    }
    for (int k = 0; k < 2; k++) {
        V3 cc = vscale(vadd(avgv(sec[k], 4), avgv(sec[k + 1], 4)), 0.5f);
        for (int e = 0; e < 4; e++) {
            V3 A = sec[k][e], B = sec[k][(e + 1) & 3], C = sec[k + 1][(e + 1) & 3], D = sec[k + 1][e];
            V3 q[4] = { A, B, C, D };
            pf_poly(4, q, &cc, C_WHITE, 0, 255);
            if (k == 1 && (e == 1 || e == 2)) {                                           // fin stripes
                decal_quad(A, B, C, D, 0.0f, 1.0f, 0.50f, 0.68f, LIV[spec->model][0][0], LIV[spec->model][0][1], LIV[spec->model][0][2]);
                decal_quad(A, B, C, D, 0.0f, 1.0f, 0.68f, 0.86f, LIV[spec->model][1][0], LIV[spec->model][1][1], LIV[spec->model][1][2]);
            }
        }
    }
    V3 cc = avgv(sec[1], 4);
    pf_poly(4, sec[2], &cc, C_NAVY, 0, 255);
    bool strobe = ((frame_count / 6) % 10) == 0;                                          // blinking beacon / strobe
    V3 tc = avgv(sec[2], 4);
    V3 lq[4] = { vadd(tc, vscale(pl.r, 0.05f)), vadd(tc, vscale(pl.f, -0.08f)),
                 vadd(tc, vscale(pl.r, -0.05f)), vadd(tc, vscale(pl.f, 0.04f)) };
    pf_poly(4, lq, NULL, strobe ? 255 : 150, strobe ? 255 : 40, strobe ? 255 : 40, PF_DECAL | PF_EMIT, 255);
}

// Straight bar from a to b (model space) with a small rectangular section.
static void prism(float ax, float ay, float az, float bx, float by, float bz, float tq, float te,
                  float r, float g, float b) {
    float vx = bx - ax, vy = by - ay;
    float qx = vy, qy = -vx, ql = sqrtf(qx * qx + qy * qy);
    if (ql < 1e-5f) return;
    qx /= ql; qy /= ql;
    static const float sq[4] = { -1, 1, 1, -1 }, se[4] = { -1, -1, 1, 1 };
    V3 A[4], B[4];
    for (int i = 0; i < 4; i++) {
        A[i] = MW(ax + qx * sq[i] * tq, ay + qy * sq[i] * tq, az + se[i] * te);
        B[i] = MW(bx + qx * sq[i] * tq, by + qy * sq[i] * tq, bz + se[i] * te);
    }
    V3 cc = vscale(vadd(avgv(A, 4), avgv(B, 4)), 0.5f);
    for (int i = 0; i < 4; i++) {
        int j = (i + 1) & 3;
        V3 q[4] = { A[i], A[j], B[j], B[i] };
        pf_poly(4, q, &cc, r, g, b, 0, 255);
    }
}

// Tire: octagonal cylinder along the x axis.
static void wheel(float cx, float cy, float cz, float rad, float hw, int hub_side) {
    V3 r0[8], r1[8];
    for (int i = 0; i < 8; i++) {
        float a = i * PI_F / 4.0f + PI_F / 8.0f, y = cy + rad * cosf(a), z = cz + rad * sinf(a);
        r0[i] = MW(cx - hw, y, z);
        r1[i] = MW(cx + hw, y, z);
    }
    V3 cc = MW(cx, cy, cz);
    for (int i = 0; i < 8; i++) {
        int j = (i + 1) & 7;
        V3 q[4] = { r0[i], r0[j], r1[j], r1[i] };
        pf_poly(4, q, &cc, 26, 26, 30, 0, 255);
    }
    pf_poly(8, r0, &cc, 34, 34, 38, 0, 255);
    if (hub_side <= 0) {
        V3 h[8];
        for (int i = 0; i < 8; i++) {
            float a = i * PI_F / 4.0f + PI_F / 8.0f;
            h[i] = MW(cx - hw - 0.004f, cy + rad * 0.5f * cosf(a), cz + rad * 0.5f * sinf(a));
        }
        pf_poly(8, h, NULL, 150, 154, 166, PF_DECAL, 255);
    }
    pf_poly(8, r1, &cc, 34, 34, 38, 0, 255);
    if (hub_side >= 0) {
        V3 h[8];
        for (int i = 0; i < 8; i++) {
            float a = i * PI_F / 4.0f + PI_F / 8.0f;
            h[i] = MW(cx + hw + 0.004f, cy + rad * 0.5f * cosf(a), cz + rad * 0.5f * sinf(a));
        }
        pf_poly(8, h, NULL, 150, 154, 166, PF_DECAL, 255);
    }
}

static bool prop_pusher = false; static float prop_pusher_z = 0.0f;               // pusher: the prop sits behind its nacelle, spinner pointing back
static void build_prop_x(float xo, int cockpit, bool adv) {
    const float cy = spec->prop_cy, pz = prop_pusher ? prop_pusher_z : spec->prop_pz, R = spec->prop_r, k = spec->prop_r / 0.95f, sr = spec->spin_r;
    const int nb = spec->nblades;
    // spinner
    V3 base[8], tip = MW(xo + 0, cy, prop_pusher ? pz - 0.50f : spec->spin_tip), cs = MW(xo + 0, cy, prop_pusher ? pz + 0.35f : spec->spin_back);
    for (int i = 0; i < 8; i++) {
        float a = i * PI_F / 4.0f;
        base[i] = MW(xo + sr * cosf(a), cy + sr * sinf(a), pz);
    }
    for (int i = 0; i < 8; i++) {
        V3 q[3] = { base[i], base[(i + 1) & 7], tip };
        pf_poly(3, q, &cs, C_RED, 0, 255);
    }
    pf_poly(8, base, &tip, 180, 30, 34, 0, 255);                      // back plate: what you see from the cockpit
    // blades
    float rate = pl.rpm * 0.10472f;                              // rad/s from rpm
    if (adv) { prop_phase += rate * DT; if (prop_phase > 2 * PI_F) prop_phase -= 2 * PI_F; }
    for (int bl = 0; bl < nb && (!cockpit || pl.rpm < spec->blade_vis_rpm); bl++) {     // in the cockpit the blades blur away at speed
        float th = prop_phase + bl * (2.0f * PI_F / nb), ux = cosf(th), uy = sinf(th), px = -uy, py = ux;
        const float r0 = 0.16f * k, r1 = R, w0 = 0.075f * k, w1 = 0.05f * k, rt = 0.78f * k;
        float wt = w0 + (w1 - w0) * (rt - r0) / (r1 - r0);
        V3 q[4] = { MW(xo + r0 * ux + px * w0, cy + r0 * uy + py * w0, pz + 0.02f),
                    MW(xo + rt * ux + px * wt, cy + rt * uy + py * wt, pz + 0.02f),
                    MW(xo + rt * ux - px * wt, cy + rt * uy - py * wt, pz + 0.02f),
                    MW(xo + r0 * ux - px * w0, cy + r0 * uy - py * w0, pz + 0.02f) };
        pf_poly(4, q, NULL, C_BLACK, PF_TWO, 255);
        V3 t[4] = { q[1], MW(xo + r1 * ux + px * w1, cy + r1 * uy + py * w1, pz + 0.02f),
                    MW(xo + r1 * ux - px * w1, cy + r1 * uy - py * w1, pz + 0.02f), q[2] };
        pf_poly(4, t, NULL, C_YELLOW, PF_TWO, 255);
    }
    // blurred disc at speed
    int alpha = (int)(clampf((rate - 60.0f) / 160.0f, 0.0f, 1.0f) * (cockpit ? 30.0f : 80.0f));
    if (alpha > 4) {
        unsigned int dcol = tod_apply(RGBA8(225, 228, 236, 255)), ycol = tod_apply(RGBA8(244, 208, 70, 255));   // dimmer after dark
        if (cockpit)                                                       // faint yellow ring where the blade tips sweep
            for (int i = 0; i < 8; i++) {
                float a0 = i * PI_F / 4.0f, a1 = (i + 1) * PI_F / 4.0f;
                V3 rq[4] = { MW(xo + 0.76f * k * cosf(a0), cy + 0.76f * k * sinf(a0), pz + 0.03f), MW(xo + R * cosf(a0), cy + R * sinf(a0), pz + 0.03f),
                             MW(xo + R * cosf(a1), cy + R * sinf(a1), pz + 0.03f), MW(xo + 0.76f * k * cosf(a1), cy + 0.76f * k * sinf(a1), pz + 0.03f) };
                pf_poly(4, rq, NULL, (float)(ycol & 255u), (float)((ycol >> 8) & 255u), (float)((ycol >> 16) & 255u), PF_TWO | PF_EMIT, alpha / 2);
            }
        V3 d[8];
        for (int i = 0; i < 8; i++) {
            float a = i * PI_F / 4.0f;
            d[i] = MW(xo + R * cosf(a), cy + R * sinf(a), pz + 0.03f);
        }
        pf_poly(8, d, NULL, (float)(dcol & 255u), (float)((dcol >> 8) & 255u), (float)((dcol >> 16) & 255u), PF_TWO | PF_EMIT, alpha);
    }
}
static void build_prop(int cockpit) { build_prop_x(0.0f, cockpit, true); }

static void build_k4(void) {
    npf = 0; pf_last = -1;
    float elev = clampf(pl.ele + pl.trim, -1.0f, 1.0f);
    float flap = -sinf(pl.flaps * 10.0f * PI_F / 180.0f) * 0.45f;

    loft_body(FUS, 10, 0.0f, EC_BODY, 1, 1);
    build_wing(+1, +pl.ail * 0.17f, flap);
    build_wing(-1, -pl.ail * 0.17f, flap);
    build_htail(+1, elev * 0.16f);
    build_htail(-1, elev * 0.16f);
    build_fin(pl.rud);

    for (int sx = -1; sx <= 1; sx += 2) {
        prism(sx * 0.50f, 0.85f, 0.30f, sx * 2.70f, 2.04f, 0.12f, 0.022f, 0.075f, 232, 234, 240);   // wing strut
        prism(sx * 0.30f, 0.82f, -0.12f, sx * 1.18f, 0.42f, -0.14f, 0.030f, 0.070f, C_STEEL);        // main gear leg
        loft_body(PANT, 4, sx * 1.25f, EC_PANT, 0, 0);                                                // wheel fairing
        wheel(sx * 1.25f, 0.30f, -0.14f, 0.30f, 0.07f, sx);                                           // tire
    }
    prism(0, 0.68f, 1.55f, 0, 0.30f, 1.62f, 0.030f, 0.045f, C_STEEL);                                 // nose gear
    wheel(0, 0.24f, 1.62f, 0.24f, 0.05f, 0);

    V3 ant[4] = { MW(0, 2.00f, -0.20f), MW(0, 2.00f, -0.45f), MW(0, 2.28f, -0.44f), MW(0, 2.28f, -0.38f) };
    pf_poly(4, ant, NULL, 40, 40, 48, PF_TWO, 255);
    build_prop(0);
}

// From the seat you can see the wings and struts when you look out to the side.
static void build_cockpit_parts_k4(void) {
    float flap = -sinf(pl.flaps * 10.0f * PI_F / 180.0f) * 0.45f;
    build_wing(+1, +pl.ail * 0.17f, flap);
    build_wing(-1, -pl.ail * 0.17f, flap);
    for (int sx = -1; sx <= 1; sx += 2)
        prism(sx * 0.50f, 0.85f, 0.30f, sx * 2.70f, 2.04f, 0.12f, 0.022f, 0.075f, 232, 234, 240);
}

// Seen over the glareshield: the engine cowling, spinner and the blur of the propeller.
static void build_cockpit_nose_k4(void) {
    static const FusSec NOSE[5] = {
        { 3.05f, 0.42f, 0.78f, 1.12f }, { 2.55f, 0.52f, 0.70f, 1.19f }, { 2.05f, 0.58f, 0.66f, 1.24f },
        { 1.50f, 0.60f, 0.66f, 1.27f }, { 0.95f, 0.60f, 0.66f, 1.30f } };
    loft_body(NOSE, 5, 0.0f, EC_BODY, 2, 0);
    build_prop(1);
}

// Soft shadow on the ground: helps judge height and the landing flare.
static V3 shadow_pt(V3 p) {
    float k = p.y / SUN.y;
    return v3(p.x - SUN.x * k, 0.04f, p.z - SUN.z * k);
}
static void draw_plane_shadow_k4(void) {
    V3 w[8];
    const unsigned int col = RGBA8(0, 0, 0, 85);
    static const float FX[8] = { -0.45f, -0.62f, -0.60f, -0.12f, 0.12f, 0.60f, 0.62f, 0.45f };
    static const float FZ[8] = { 3.00f, 1.80f, -0.60f, -4.50f, -4.50f, -0.60f, 1.80f, 3.00f };
    for (int i = 0; i < 8; i++) w[i] = shadow_pt(MW(FX[i], 1.3f, FZ[i]));
    draw_world_poly(w, 8, col);
    for (int sx = -1; sx <= 1; sx += 2) {
        V3 wg[4] = { shadow_pt(MW(sx * 0.62f, 2.1f, 0.88f)), shadow_pt(MW(sx * 5.50f, 2.2f, 0.88f)),
                     shadow_pt(MW(sx * 5.50f, 2.2f, -0.24f)), shadow_pt(MW(sx * 0.62f, 2.1f, -0.74f)) };
        draw_world_poly(wg, 4, col);
        V3 tl[4] = { shadow_pt(MW(sx * 0.30f, 1.28f, -3.58f)), shadow_pt(MW(sx * 1.70f, 1.28f, -3.88f)),
                     shadow_pt(MW(sx * 1.70f, 1.28f, -4.62f)), shadow_pt(MW(sx * 0.30f, 1.28f, -4.60f)) };
        draw_world_poly(tl, 4, col);
    }
}

