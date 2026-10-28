// Left Seat: a small kit for building smooth aircraft models: lofted fuselage sections, airfoil wings, tail surfaces, tires.
// Used by the K-10 and the Jet 1.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* A small kit for smooth fuselage-style shapes                         */
/* ------------------------------------------------------------------ */

#ifndef MODEL_DETAIL
#define MODEL_DETAIL 2          // 2 = full detail (default), 1 = lighter K-10: fewer sections, about 30% fewer polygons
#endif

typedef struct { float z, w, yb, yt, yw, nt, nb, nx; } KS;           // section: half-width, bottom, top, height of the widest point, roundness
static float cr4(float a, float b, float c, float d, float t) {      // smooth (Catmull-Rom) curve through four values
    return 0.5f * ((2 * b) + (-a + c) * t + (2 * a - 5 * b + 4 * c - d) * t * t + (-a + 3 * b - 3 * c + d) * t * t * t);
}
static void ks_at(const KS *K, int nk, float z, KS *o) {             // section at any z, smoothly between the key sections
    int i = 0;
    while (i < nk - 2 && z < K[i + 1].z) i++;
    float t = clampf((K[i].z - z) / (K[i].z - K[i + 1].z), 0.0f, 1.0f);
    const KS *p0 = &K[i > 0 ? i - 1 : 0], *p1 = &K[i], *p2 = &K[i + 1], *p3 = &K[i + 2 < nk ? i + 2 : nk - 1];
    o->z = z;
    o->w  = cr4(p0->w,  p1->w,  p2->w,  p3->w,  t);
    o->yb = cr4(p0->yb, p1->yb, p2->yb, p3->yb, t);
    o->yt = cr4(p0->yt, p1->yt, p2->yt, p3->yt, t);
    o->yw = cr4(p0->yw, p1->yw, p2->yw, p3->yw, t);
    o->nt = cr4(p0->nt, p1->nt, p2->nt, p3->nt, t);
    o->nb = cr4(p0->nb, p1->nb, p2->nb, p3->nb, t);
    o->nx = cr4(p0->nx, p1->nx, p2->nx, p3->nx, t);
}
static float spw(float v, float p) { float a = powf(fabsf(v), p); return v < 0.0f ? -a : a; }
#define RN 12
static void ks_ring(V3 *out, const KS *s, float xoff) {
    for (int k = 0; k < RN; k++) {
        float th = 2.0f * PI_F * k / RN, sn = sinf(th), cs = cosf(th);
        float x = s->w * spw(sn, 2.0f / s->nx);
        float y = cs >= 0.0f ? s->yw + (s->yt - s->yw) * spw(cs, 2.0f / s->nt) : s->yw - (s->yw - s->yb) * spw(-cs, 2.0f / s->nb);
        out[k] = MW(xoff + x, y, s->z);
    }
}

typedef struct { unsigned char body[3], a1[3], a2[3], lower[3], glass[3], tire[3], metal[3], tip[3]; } Paint;

static void paint_face(const Paint *p, int e, float *r, float *g, float *b) {   // color by where the face sits round the section
    int s = e <= 5 ? e : 11 - e;                                             // 0 top ... 5 belly, mirrored on the left
    const unsigned char *c = p->body; float k = 1.0f;
    if (s == 0) k = 1.0f; else if (s == 1) k = 0.97f; else if (s == 2) k = 0.95f;
    else { c = p->lower; k = (s == 3) ? 0.93f : (s == 4 ? 0.90f : 0.82f); }
    *r = c[0] * k; *g = c[1] * k; *b = c[2] * k;
}
static bool in_any(const float (*R)[2], int n, float z) { for (int i = 0; i < n; i++) if (z < R[i][0] && z > R[i][1]) return true; return false; }

// kind 1: the K-10's body (windows, stripes), 2: plain (the cargo pod), 3: the cowling seen from the cockpit,
// 4: the Jet 1's body. caps: which ends to close, so the shape is solid from every side.
enum { CAP_FRONT_DARK = 1, CAP_FRONT_BODY = 2, CAP_BACK_BODY = 4, CAP_BACK_DARK = 8 };
static float loft_xoff = 0.0f;                      // sideways offset for a lofted shape (engine nacelles)
static void loft_car(const KS *K, int nk, const float *Z, int nz, const Paint *p, int kind, int caps) {
    V3 ring[40][RN], cen[40];
    KS s;
    static const float WIN[5][2] = { { 2.00f, 1.05f }, { 0.50f, -0.65f }, { -1.00f, -1.80f }, { -2.20f, -3.00f }, { -3.20f, -3.95f } };
    // The section shapes never change, only where the aircraft is: work them out once (model space) and keep them.
    // (This used to redo the curve fitting and the powf calls for every vertex of every frame.)
    typedef struct { const KS *K; const float *Z; int nk, nz; float xoff; V3 p[40][RN]; } LoftCache;
    static LoftCache cache[24]; static int ncache = 0;
    LoftCache *lc = NULL;
    for (int c = 0; c < ncache; c++)
        if (cache[c].K == K && cache[c].Z == Z && cache[c].nk == nk && cache[c].nz == nz && cache[c].xoff == loft_xoff) { lc = &cache[c]; break; }
    if (!lc && ncache < 24 && nz <= 40) {
        lc = &cache[ncache++];
        lc->K = K; lc->Z = Z; lc->nk = nk; lc->nz = nz; lc->xoff = loft_xoff;
        for (int i = 0; i < nz; i++) {
            ks_at(K, nk, Z[i], &s);
            for (int k = 0; k < RN; k++) {
                float th = 2.0f * PI_F * k / RN, sn = sinf(th), cs = cosf(th);
                float x = s.w * spw(sn, 2.0f / s.nx);
                float y = cs >= 0.0f ? s.yw + (s.yt - s.yw) * spw(cs, 2.0f / s.nt) : s.yw - (s.yw - s.yb) * spw(-cs, 2.0f / s.nb);
                lc->p[i][k] = v3(loft_xoff + x, y, s.z);
            }
        }
    }
    for (int i = 0; i < nz; i++) {
        if (lc) { for (int k = 0; k < RN; k++) ring[i][k] = MW(lc->p[i][k].x, lc->p[i][k].y, lc->p[i][k].z); }
        else    { ks_at(K, nk, Z[i], &s); ks_ring(ring[i], &s, loft_xoff); }
        cen[i] = avgv(ring[i], RN);
    }
    for (int i = 0; i < nz - 1; i++) {
        V3 cc = vscale(vadd(cen[i], cen[i + 1]), 0.5f);
        float zm = 0.5f * (Z[i] + Z[i + 1]);
        for (int e = 0; e < RN; e++) {
            V3 A = ring[i][e], B = ring[i][(e + 1) % RN], C = ring[i + 1][(e + 1) % RN], D = ring[i + 1][e];
            V3 q[4] = { A, B, C, D };
            float r, g, b; paint_face(p, e, &r, &g, &b);
            int sd = e <= 5 ? e : 11 - e;
            bool glass = false;
            if (kind == 1) {
                if (sd == 0 && zm > 1.70f && zm < 2.40f) glass = true;                  // windshield
                if (sd == 2 && in_any(WIN, 5, zm)) glass = true;                        // side windows
            } else if (kind == 4) {
                if (sd <= 2 && zm > -0.35f && zm < 0.85f) glass = true;                 // bubble canopy
            } else if (kind == 5) {
                static const float W30[7][2] = { { 6.05f, 5.15f }, { 3.80f, 3.30f }, { 2.90f, 2.40f }, { 2.00f, 1.50f }, { 1.10f, 0.60f }, { 0.20f, -0.30f }, { -0.70f, -1.20f } };
                if (sd <= 1 && zm > 5.35f && zm < 6.55f) glass = true;                  // windscreen
                if (sd == 2 && in_any(W30, 7, zm)) glass = true;                        // cockpit and cabin windows
            } else if (kind == 8) {
                if (sd <= 1 && zm > 3.35f && zm < 4.25f) glass = true;                  // windscreen (the small single)
                if (sd == 2 && zm > 3.35f && zm < 4.25f) glass = true;
            } else if (kind == 7) {
                if (sd <= 1 && zm > 2.05f && zm < 3.15f) glass = true;                  // windscreen
                if (sd == 2 && zm > 2.05f && zm < 3.15f) glass = true;                  // cockpit side windows, level with the windscreen
                        } else if (kind == 6) {
                static const float W22[6][2] = { { 2.10f, 1.45f }, { 0.95f, 0.35f }, { -0.05f, -0.65f }, { -1.05f, -1.65f }, { -2.05f, -2.65f }, { -3.05f, -3.55f } };
                if (sd <= 1 && zm > 1.95f && zm < 2.95f) glass = true;                  // windscreen
                if (sd == 2 && in_any(W22, 6, zm)) glass = true;                        // cockpit and cabin windows
            } else if (kind == 2) { r = p->body[0] * 0.92f; g = p->body[1] * 0.92f; b = p->body[2] * 0.92f; }
            else if (kind == 3 && sd == 0) { r = 40.0f; g = 42.0f; b = 48.0f; }         // black anti-glare panel
            if (glass) { r = p->glass[0]; g = p->glass[1]; b = p->glass[2]; }
            pf_poly(4, q, &cc, r, g, b, kind == 3 ? PF_TWO : 0, 255);             // (the cockpit nose is drawn solid from every side)
            if ((kind == 7 || kind == 8) && sd == 2) {                                   // passenger windows: one small window on every other panel
                static const float PW[4] = { 1.225f, -1.475f, -1.925f, -2.375f }, PW8[4] = { 2.35f, 1.35f, 9.0f, 9.0f };
                const float *PWP = kind == 8 ? PW8 : PW;
                for (int w = 0; w < 4; w++)
                    if (fabsf(zm - PWP[w]) < 0.1f) {                                                                       // a round window: an octagon drawn on the panel
                        V3 o[8];
                        for (int q = 0; q < 8; q++) { float a = q * PI_F / 4.0f + PI_F / 8.0f; o[q] = bilerp(A, B, C, D, 0.5f + 0.40f * cosf(a), 0.5f + 0.42f * sinf(a)); }
                        pf_poly(8, o, NULL, 34, 62, 104, PF_DECAL, 255);
                    }
            }
            if (kind == 1 || kind == 4 || kind == 5 || kind == 6 || kind == 7 || kind == 8) {
                if (sd == 3) {                                                           // belt stripes
                    decal_quad(A, B, C, D, 0.10f, 0.32f, 0.0f, 1.0f, p->a1[0], p->a1[1], p->a1[2]);
                    decal_quad(A, B, C, D, 0.32f, 0.50f, 0.0f, 1.0f, p->a2[0], p->a2[1], p->a2[2]);
                }
                if (kind != 5 && kind != 6 && kind != 7 && kind != 8 && zm > 3.55f && sd < 3) decal_quad(A, B, C, D, 0.0f, 1.0f, 0.0f, 1.0f, p->a1[0], p->a1[1], p->a1[2]);   // cowl band
            }
        }
    }
    int pfl = (kind == 3) ? PF_TWO : 0;
    if (caps & (CAP_FRONT_DARK | CAP_FRONT_BODY)) {                              // close the front end: the whole ring, in two halves
        float r = 46, g = 48, b = 56;
        if (caps & CAP_FRONT_BODY) { r = p->body[0] * 0.95f; g = p->body[1] * 0.95f; b = p->body[2] * 0.95f; }
        V3 h1[7], h2[7];
        for (int k = 0; k < 7; k++) h1[k] = ring[0][k];
        for (int k = 0; k < 6; k++) h2[k] = ring[0][6 + k];
        h2[6] = ring[0][0];
        pf_poly(7, h1, &cen[1], r, g, b, pfl, 255);
        pf_poly(7, h2, &cen[1], r, g, b, pfl, 255);
    }
    if (caps & (CAP_BACK_BODY | CAP_BACK_DARK)) {                                // close the back end
        const int L = nz - 1;
        float r = p->body[0] * 0.90f, g = p->body[1] * 0.90f, b = p->body[2] * 0.90f;
        if (caps & CAP_BACK_DARK) { r = 36; g = 32; b = 32; }
        V3 h1[7], h2[7];
        for (int k = 0; k < 7; k++) h1[k] = ring[L][k];
        for (int k = 0; k < 6; k++) h2[k] = ring[L][6 + k];
        h2[6] = ring[L][0];
        pf_poly(7, h1, &cen[L - 1], r, g, b, pfl, 255);
        pf_poly(7, h2, &cen[L - 1], r, g, b, pfl, 255);
    }
}

/* ---------- wings and tail surfaces with real airfoil sections ---------- */
static const float AF_X[5]  = { 0.0f, 0.13f, 0.42f, 0.78f, 1.0f };
static const float AF_YU[5] = { 0.0f, 0.062f, 0.076f, 0.040f, 0.004f };      // upper surface (fraction of chord)
static const float AF_YL[3] = { -0.036f, -0.042f, -0.014f };                  // lower surface at the three stations
typedef struct { int n; float x[8], ch[8], th[8], droop[8]; float y0, dih, z0; int hinge[8]; float tipcol[3]; } AWing;
// Eight points round each section: leading edge, three on top, trailing edge, three underneath.
static void a_wing(const AWing *W, int sx, const float *dy, const Paint *p, bool light) {
    V3 sec[8][8], cc[8];
    for (int k = 0; k < W->n; k++) {
        float x = sx * W->x[k], c = W->ch[k], t = W->th[k] / 0.12f, yc = W->y0 + W->dih * W->x[k] - W->droop[k], z0 = W->z0;
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
            pf_poly(4, q, &cc[k], r, g, b, (e == 3 || e == 4) ? PF_TWO : 0, 255);       // the two rear faces are drawn from both sides: a tilted aileron must never vanish from some angles
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

typedef struct { int n; float y[5], zl[5], ch[5], th[5]; float yc; } ATail;           // the fin
static float fin_xoff = 0.0f;                     // sideways offset for a fin (twin-boom tails)
static void a_fin(const ATail *F, float rud, const Paint *p) {
    V3 sec[6][6];
    for (int k = 0; k < F->n; k++) {
        float c = F->ch[k], t = F->th[k], dx = rud * 0.13f * clampf((F->y[k] - F->yc) / 1.3f, 0.0f, 1.0f);
        float zl = F->zl[k], y = F->y[k];
        sec[k][0] = MW(fin_xoff + 0, y, zl);
        sec[k][1] = MW(fin_xoff + t * 0.9f, y, zl - 0.18f * c);
        sec[k][2] = MW(fin_xoff + t * 0.9f + dx * 0.4f, y, zl - 0.60f * c);
        sec[k][3] = MW(fin_xoff + dx, y, zl - c);
        sec[k][4] = MW(fin_xoff + -t * 0.9f + dx * 0.4f, y, zl - 0.60f * c);
        sec[k][5] = MW(fin_xoff + -t * 0.9f, y, zl - 0.18f * c);
    }
    for (int k = 0; k < F->n - 1; k++) {
        V3 cc = vscale(vadd(avgv(sec[k], 6), avgv(sec[k + 1], 6)), 0.5f);
        for (int e = 0; e < 6; e++) {
            V3 A = sec[k][e], B = sec[k][(e + 1) % 6], C = sec[k + 1][(e + 1) % 6], D = sec[k + 1][e];
            V3 q[4] = { A, B, C, D };
            pf_poly(4, q, &cc, p->body[0], p->body[1], p->body[2], 0, 255);
            if (k >= 1 && (e == 1 || e == 2 || e == 4 || e == 5)) {
                decal_quad(A, B, C, D, 0.0f, 1.0f, 0.34f, 0.52f, p->a1[0], p->a1[1], p->a1[2]);
                decal_quad(A, B, C, D, 0.0f, 1.0f, 0.52f, 0.70f, p->a2[0], p->a2[1], p->a2[2]);
            }
            if (e == 2 || e == 3) decal_quad(A, B, C, D, 0.0f, 0.04f, 0.0f, 1.0f, 112, 118, 128);      // rudder hinge
        }
    }
    V3 cc = avgv(sec[F->n - 2], 6);
    pf_poly(6, sec[F->n - 1], &cc, p->tip[0], p->tip[1], p->tip[2], 0, 255);
    bool strobe = ((frame_count / 6) % 10) == 0;
    V3 tc = avgv(sec[F->n - 1], 6);
    V3 lq[4] = { vadd(tc, vscale(pl.r, 0.05f)), vadd(tc, vscale(pl.f, -0.08f)), vadd(tc, vscale(pl.r, -0.05f)), vadd(tc, vscale(pl.f, 0.04f)) };
    pf_poly(4, lq, NULL, strobe ? 255 : 150, strobe ? 255 : 40, strobe ? 255 : 40, PF_DECAL | PF_EMIT, 255);
}
typedef struct { int n; float x[5], zl[5], ch[5], th[5]; float yc; } AStab;           // the horizontal stabiliser
static void a_stab(const AStab *H, int sx, float dy, const Paint *p) {
    V3 sec[5][6];
    for (int k = 0; k < H->n; k++) {
        float x = sx * H->x[k], c = H->ch[k], t = H->th[k], zl = H->zl[k], y = H->yc, d = (k == H->n - 1) ? 0.4f * dy : dy;
        sec[k][0] = MW(x, y, zl);
        sec[k][1] = MW(x, y + t * 0.9f, zl - 0.18f * c);
        sec[k][2] = MW(x, y + t * 0.9f + d * 0.4f, zl - 0.60f * c);
        sec[k][3] = MW(x, y + d, zl - c);
        sec[k][4] = MW(x, y - t * 0.9f + d * 0.4f, zl - 0.60f * c);
        sec[k][5] = MW(x, y - t * 0.9f, zl - 0.18f * c);
    }
    for (int k = 0; k < H->n - 1; k++) {
        V3 cc = vscale(vadd(avgv(sec[k], 6), avgv(sec[k + 1], 6)), 0.5f);
        for (int e = 0; e < 6; e++) {
            V3 A = sec[k][e], B = sec[k][(e + 1) % 6], C = sec[k + 1][(e + 1) % 6], D = sec[k + 1][e];
            V3 q[4] = { A, B, C, D };
            float m = (e == 4 || e == 5) ? 0.86f : 1.0f;
            pf_poly(4, q, &cc, p->body[0] * m, p->body[1] * m, p->body[2] * m, 0, 255);
            if (e == 2 || e == 3) decal_quad(A, B, C, D, 0.0f, 0.04f, 0.0f, 1.0f, 112, 118, 128);
        }
    }
    V3 cc = avgv(sec[H->n - 2], 6);
    pf_poly(6, sec[H->n - 1], &cc, p->tip[0], p->tip[1], p->tip[2], 0, 255);
}

// Tire with a hub and brake disc.
static void a_tire(float cx, float cy, float cz, float rad, float hw, int side, const Paint *p) {
    enum { T = (MODEL_DETAIL >= 2) ? 14 : 10 };
    V3 r0[T], r1[T];
    for (int i = 0; i < T; i++) { float a = i * 2.0f * PI_F / T, y = cy + rad * cosf(a), z = cz + rad * sinf(a); r0[i] = MW(cx - hw, y, z); r1[i] = MW(cx + hw, y, z); }
    V3 cc = MW(cx, cy, cz);
    for (int i = 0; i < T; i++) { int j = (i + 1) % T; V3 q[4] = { r0[i], r0[j], r1[j], r1[i] }; pf_poly(4, q, &cc, p->tire[0], p->tire[1], p->tire[2], 0, 255); }
    for (int s = 0; s < 2; s++) {
        if ((s == 0 && side > 0) || (s == 1 && side < 0)) continue;
        float xo = s == 0 ? -hw : hw;
        V3 o[8], hub[8];
        for (int i = 0; i < 8; i++) { float a = i * PI_F / 4.0f; o[i] = MW(cx + xo, cy + rad * 0.86f * cosf(a), cz + rad * 0.86f * sinf(a)); hub[i] = MW(cx + xo * 1.06f, cy + rad * 0.52f * cosf(a), cz + rad * 0.52f * sinf(a)); }
        pf_poly(8, o, &cc, 46, 46, 52, 0, 255);
        pf_poly(8, hub, NULL, 176, 180, 190, PF_DECAL, 255);
    }
}
typedef struct { float z, w, yb, yt; } PS;                                  // a short smooth fairing (the air scoop under the nose)
static void a_pant(const PS *S, int ns, float xoff, const Paint *p) {
    enum { PN = 8 };
    V3 ring[8][PN], cen[8];
    for (int i = 0; i < ns; i++) {
        float yc = 0.5f * (S[i].yb + S[i].yt), hh = 0.5f * (S[i].yt - S[i].yb);
        for (int k = 0; k < PN; k++) { float th = 2.0f * PI_F * k / PN; ring[i][k] = MW(xoff + S[i].w * spw(sinf(th), 0.8f), yc + hh * spw(cosf(th), 0.8f), S[i].z); }
        cen[i] = avgv(ring[i], PN);
    }
    for (int i = 0; i < ns - 1; i++) {
        V3 cc = vscale(vadd(cen[i], cen[i + 1]), 0.5f);
        for (int e = 0; e < PN; e++) {
            V3 q[4] = { ring[i][e], ring[i][(e + 1) % PN], ring[i + 1][(e + 1) % PN], ring[i + 1][e] };
            float m = (e >= 3 && e <= 5) ? 0.82f : 1.0f;
            pf_poly(4, q, &cc, p->body[0] * m, p->body[1] * m, p->body[2] * m, 0, 255);
        }
    }
}

