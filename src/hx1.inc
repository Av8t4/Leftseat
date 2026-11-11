// Left Seat: the Strake HX-1, a stainless steel hop-test ship, on the launch mount at Gulf Point Spaceport.
// Phase 1: the ship model, the hangar entry and a pad session (look around it, work the flaps). It cannot fly yet.
// It is only used at the spaceport, never in free flight. Part of Left Seat's single translation unit: this file is #included by main.c, in order.

#define HX_R      4.5f             // the hull radius (m), as the scenery rocket at the spaceport
#define HX_HALF   25.0f            // half the length (m): the body origin is the middle of the ship, tail at -25, nose tip at +25
#define HX_UP     1.5700f          // standing upright: pitch just short of 90 degrees (the heading stays defined)
#define HX_MOUNT_LX 1160.0f        // the second launch mount (the empty one)
#define HX_MOUNT_LZ 130.0f
#define HX_DECK     16.0f          // the top of the mount (m)
#define HX_PAD_LX   1190.0f        // the landing pad, south of the mounts
#define HX_PAD_LZ   505.0f
#define HX_PAD_R    34.0f

#define HX_G      9.81f
#define HX_TWR    2.2f             // thrust over weight at full throttle with full tanks
#define HX_BURN   70.0f            // seconds of fuel at full throttle
#define HX_MINTHR 0.30f            // the engines cannot throttle below this
#define HX_PITCH_SIGN -1.0f        // the fore flaps push the nose up when they open more, so they open when the nose should come toward the belly less
#define HX_ROLL_SIGN  1.0f         // and the right flaps the roll

typedef struct {
    float psi;                     // the heading it stands facing (radians)
    bool  eng, ground;             // the engines are lit; the legs are on something
    float thr, fuel;               // the throttle setting (0.3 to 1) and the fuel (0 to 1)
    float wx, wy, wz;              // turn rates about the ship's own axes: x = right (nose toward the belly), y = back (nose to the right), z = long axis (radians per second)
    V3    fh, uh;                  // the attitude the computer is holding: where the nose points and which way the back faces
    bool  dep;                     // the flaps are asked to open
    float leg; bool legc;          // the landing legs: how far out (0 = folded against the hull, 1 = out) and whether they are asked out
    bool  assist, flip;            // the computer helps (it does the flip and the landing burn); the flip has been asked for
    float agl, manual;             // how far the legs are above what is under them (m); seconds the pilot has had a hand on the throttle
    float dp, dr;                  // the computer's flap trims for pitch and roll (-1 to 1)
    float maxalt, td, t;           // highest point (m, engines), last touchdown speed (m/s), a clock for the flame
    V3    pad;                     // the landing pad's center
    float flap[4];                 // fore left, fore right, aft left, aft right: 0 = stowed, 1 = fully out
} HX;
static HX hx;

static const float HX_STEEL[3] = { 204, 208, 216 }, HX_SHIELD[3] = { 40, 42, 50 }, HX_DARK[3] = { 62, 66, 76 };

// Rings of the hull: z along the ship, radius.
typedef struct { float z, r; } HXS;
static const HXS HXA[] = {
    { -25.0f, 4.5f }, { -16.0f, 4.5f }, { -7.0f, 4.5f }, { 2.0f, 4.5f }, { 11.0f, 4.5f },        // the cylinder
    { 15.0f, 4.25f }, { 19.0f, 3.5f }, { 22.0f, 2.3f }, { 24.2f, 1.1f }, { 25.0f, 0.0f } };      // the nose
#define HXN ((int)(sizeof(HXA) / sizeof(HXA[0])))
#define HXE 12                      // sides

static void hx_ring(V3 *R, const HXS *s) {
    for (int e = 0; e < HXE; e++) {
        const float a = e * (2.0f * PI_F / HXE) + PI_F / HXE;
        R[e] = MW(s->r * sinf(a), s->r * cosf(a), s->z);          // e = 0 near the top (the back), half way round is the belly
    }
}

static void build_hx1(void) {
    npf = 0; pf_last = -1;
    V3 ring[HXN][HXE], cen[HXN];
    for (int i = 0; i < HXN; i++) { hx_ring(ring[i], &HXA[i]); cen[i] = MW(0.0f, 0.0f, HXA[i].z); }
    for (int i = 0; i + 1 < HXN; i++) {
        const V3 cc = vscale(vadd(cen[i], cen[i + 1]), 0.5f);
        for (int e = 0; e < HXE; e++) {
            const int j = (e + 1) % HXE;
            const float *c = HX_STEEL;
            if (e >= 4 && e <= 7 && HXA[i].z < 11.0f) c = HX_SHIELD;               // the belly: the black heat-shield tiles
            else if (e == 3 || e == 8) c = (HXA[i].z < 11.0f) ? HX_DARK : HX_STEEL;   // the edge of the tiles
            if (i + 2 == HXN) { V3 q[3] = { ring[i][e], ring[i][j], ring[i + 1][0] }; pf_poly(3, q, &cc, c[0], c[1], c[2], 0, 255); }
            else { V3 q[4] = { ring[i][e], ring[i][j], ring[i + 1][j], ring[i + 1][e] }; pf_poly(4, q, &cc, c[0], c[1], c[2], 0, 255); }
        }
    }
    {                                                                              // the tail: a dark cap and three engine bells
        V3 cap[HXE]; for (int e = 0; e < HXE; e++) cap[e] = ring[0][HXE - 1 - e];
        V3 c0 = cen[0];
        V3 h1[6] = { cap[0], cap[2], cap[4], cap[6], cap[8], cap[10] };
        pf_poly(6, h1, NULL, HX_SHIELD[0], HX_SHIELD[1], HX_SHIELD[2], PF_TWO, 255);       // (two-sided: its middle is in its own plane, so there is no 'outward' to pick)
        for (int b = 0; b < 3; b++) {
            const float a = b * (2.0f * PI_F / 3.0f) + 0.5f, bx = 1.7f * sinf(a), by = 1.7f * cosf(a);
            V3 r0[6], r1[6], cc = MW(bx, by, -26.0f);
            for (int k = 0; k < 6; k++) {
                const float t = k * (PI_F / 3.0f);
                r0[k] = MW(bx + 0.75f * sinf(t), by + 0.75f * cosf(t), -25.0f);
                r1[k] = MW(bx + 1.15f * sinf(t), by + 1.15f * cosf(t), -26.6f);
            }
            for (int k = 0; k < 6; k++) { const int m = (k + 1) % 6; V3 q[4] = { r0[k], r0[m], r1[m], r1[k] }; pf_poly(4, q, &cc, 34, 34, 40, PF_TWO, 255); }
        }
    }
    {                                                                              // four landing legs: flat struts that fold against the hull and swing out to land
        const float rf = 4.7f + 2.7f * hx.leg;
        for (int i = 0; i < 4; i++) {
            const float a = i * (PI_F / 2.0f) + PI_F / 4.0f, sa = sinf(a), ca = cosf(a), ta = -ca, tb = sa;      // outward (sa, ca) and the tangent
            V3 q[4] = { MW(sa * HX_R * 0.98f + ta * 0.35f, ca * HX_R * 0.98f + tb * 0.35f, -9.0f), MW(sa * HX_R * 0.98f - ta * 0.35f, ca * HX_R * 0.98f - tb * 0.35f, -9.0f),
                        MW(sa * rf - ta * 0.35f, ca * rf - tb * 0.35f, -25.0f), MW(sa * rf + ta * 0.35f, ca * rf + tb * 0.35f, -25.0f) };
            pf_poly(4, q, NULL, HX_DARK[0], HX_DARK[1], HX_DARK[2], PF_TWO, 255);
            if (hx.leg > 0.3f) {
                V3 pad[4] = { MW(sa * rf + 0.9f * sa - 0.9f * ta, ca * rf + 0.9f * ca - 0.9f * tb, -25.0f), MW(sa * rf + 0.9f * sa + 0.9f * ta, ca * rf + 0.9f * ca + 0.9f * tb, -25.0f),
                              MW(sa * rf - 0.9f * sa + 0.9f * ta, ca * rf - 0.9f * ca + 0.9f * tb, -25.0f), MW(sa * rf - 0.9f * sa - 0.9f * ta, ca * rf - 0.9f * ca - 0.9f * tb, -25.0f) };
                pf_poly(4, pad, NULL, 50, 52, 60, PF_TWO, 255);
            }
        }
    }
    if (hx.eng && !pl.crashed) {                                                   // the flame: three cones of fire
        const float L = (4.0f + 14.0f * hx.thr) * (0.85f + 0.15f * sinf(hx.t * 53.0f) * cosf(hx.t * 31.0f));
        for (int b = 0; b < 3; b++) {
            const float a = b * (2.0f * PI_F / 3.0f) + 0.5f, bx = 1.7f * sinf(a), by = 1.7f * cosf(a);
            const V3 apex = MW(bx, by, -26.6f - L), ap2 = MW(bx, by, -26.6f - L * 0.55f);
            V3 r0[6], r1[6];
            for (int k = 0; k < 6; k++) {
                const float t = k * (PI_F / 3.0f);
                r0[k] = MW(bx + 1.15f * sinf(t), by + 1.15f * cosf(t), -26.6f);
                r1[k] = MW(bx + 0.62f * sinf(t), by + 0.62f * cosf(t), -26.7f);
            }
            for (int k = 0; k < 6; k++) { const int m = (k + 1) % 6; V3 q[3] = { r0[k], r0[m], apex }; pf_poly(3, q, NULL, 255, 150, 50, PF_TWO | PF_EMIT, 255); }
            for (int k = 0; k < 6; k++) { const int m = (k + 1) % 6; V3 q[3] = { r1[k], r1[m], ap2 }; pf_poly(3, q, NULL, 255, 235, 170, PF_TWO | PF_EMIT, 255); }
        }
    }
    for (int f = 0; f < 4; f++) {                                                  // the four flaps: hinged on the sides, low on the belly side
        const float s = (f & 1) ? 1.0f : -1.0f;                                    // (the belly is the black side; the flaps are on the sides next to it)
        const bool fore = f < 2;
        if (hx.flap[f] < 0.12f) continue;                                          // tucked away: they lie inside the hull, and would show through it
        const float z0 = fore ? 6.0f : -20.0f, z1 = fore ? 11.5f : -8.0f, span = fore ? 3.4f : 5.2f;
        const float th = -0.55f + hx.flap[f] * 2.3f;                               // 0 = stowed, tucked up the side toward the back; 1 = out and a little toward the belly
        const float dx = s * sinf(th), dy = cosf(th);
        const float hx0 = s * HX_R * 0.9f, hy0 = -HX_R * 0.4f;
        V3 q[4] = { MW(hx0, hy0, z0), MW(hx0, hy0, z1), MW(hx0 + dx * span, hy0 + dy * span, fore ? z1 - 1.0f : z1 - 1.5f), MW(hx0 + dx * span, hy0 + dy * span, fore ? z0 + 1.0f : z0 + 0.5f) };
        pf_poly(4, q, NULL, HX_SHIELD[0] + 20, HX_SHIELD[1] + 20, HX_SHIELD[2] + 22, PF_TWO, 255);
    }
}

static void build_cockpit_nose_hx1(void) { npf = 0; pf_last = -1; }            // (there is no cockpit view: the camera stays outside)
static void draw_plane_shadow_hx1(void) {}

/* ------------------------------------------------------------------ */
/* The pad session                                                      */
/* ------------------------------------------------------------------ */

typedef struct { V3 p, v; float age, life, size; bool dust; } HXPuff;
#define HX_NPUFF 90
static HXPuff hx_puffs[HX_NPUFF];
static int    hx_np;

static void hx_start(void) {
    const Airport *a = &AIRPORTS[AP_SPACE];
    game_mode = MODE_HX; ch_state = CH_IDLE;
    ac_flying = AC_HX1; spec = &SPECS[AC_HX1];
    memset(&pl, 0, sizeof(pl));
    pl.pos = ap_pt(a, HX_MOUNT_LX, HX_DECK + HX_HALF, HX_MOUNT_LZ);
    pl.hdg = pl.hdg_g = a->hdg;
    pl.on_ground = true; pl.fuel = 1.0f; pl.gear = 1.0f; pl.gear_cmd = true; pl.touched = true;
    sw_cold();
    engine_failed = false; eng_on = false; eng_n = 0.0f;
    ntrk = 0; trk_i = 0; trk_t = 0;
    hx.psi = a->hdg;
    for (int i = 0; i < 4; i++) hx.flap[i] = 0.0f;
    hx_np = 0;
    hx.eng = false; hx.ground = true; hx.thr = 0.6f; hx.fuel = 1.0f;
    hx.leg = 0.0f; hx.legc = false; hx.assist = true; hx.flip = false; hx.agl = 0.0f; hx.manual = 0.0f;       // the legs are folded: the ship is held on its mount
    hx.wx = hx.wy = hx.wz = 0.0f; hx.dep = false; hx.dp = hx.dr = 0.0f; hx.maxalt = 0.0f; hx.td = 0.0f; hx.t = 0.0f;
    hx.pad = ap_pt(a, HX_PAD_LX, 0.0f, HX_PAD_LZ);
    set_axes(hx.psi, HX_UP, 0.0f);
    hx.fh = pl.f; hx.uh = pl.u;
    chase_dir = v3(sinf(hx.psi), 0.0f, cosf(hx.psi));
    look_ang = 0.0f; view_mode = 1;
}
static void hx_restart(void) { hx_start(); }

// The way the chase camera looks: from behind when the ship stands up, from its right side when it lies flat (so the belly flop shows).
static V3 hx_chase_target(void) {
    V3 fh = v3(pl.f.x, 0.0f, pl.f.z);
    const float l = sqrtf(fh.x * fh.x + fh.z * fh.z);
    fh = l > 0.05f ? vscale(fh, 1.0f / l) : v3(sinf(hx.psi), 0.0f, cosf(hx.psi));
    const V3 beh = v3(sinf(hx.psi), 0.0f, cosf(hx.psi)), side = v3(-fh.z, 0.0f, fh.x);
    const float w = clampf((fabsf(pl.f.y) - 0.35f) / 0.5f, 0.0f, 1.0f);
    V3 t = vadd(vscale(beh, w), vscale(side, 1.0f - w));
    t.y = 0.12f;
    return t;
}

// What the legs can stand on at (x, z): the ground, the landing pad, and anything low enough to land on (the launch mount, a roof).
static float hx_support(float x, float z) {
    float h = 0.0f;
    for (int i = 0; i < nboxes; i++) {
        const Box *b = &boxes[i];
        if (b->h <= 20.0f && b->h > h && fabsf(x - b->x) < b->w * 0.5f && fabsf(z - b->z) < b->d * 0.5f) h = b->h;
    }
    const float dx = x - hx.pad.x, dz = z - hx.pad.z, r = sqrtf(dx * dx + dz * dz) / HX_PAD_R;
    const float ph = r < 0.14f ? 0.66f : r < 0.42f ? 0.54f : r < 0.74f ? 0.42f : r < 1.0f ? 0.30f : 0.0f;
    return fmaxf(h, ph);
}
static float hx_tilt(void) { return acosf(clampf(pl.f.y, -1.0f, 1.0f)); }      // how far the nose is from straight up (radians)

// The height of the engines above the sea (feet) as the pilot reads it.
static float hx_alt_ft(void) { return (pl.pos.y - HX_HALF * fabsf(pl.f.y)) * 3.28084f; }

// ---- The air: the hull and the flaps are flat plates and a drum in the wind. -------------------------------------------------------
#define HX_M0   120000.0f          // mass with full tanks (kg)
#define HX_ICG  -2.0f              // the center of gravity along the ship (m): about where the drum's pressure acts, so the hull alone is neutral
#define HX_CD   0.8f               // the drum, side on
#define HX_I_T  200.0f             // turning inertia about the cross axes, per kilogram
#define HX_I_L  15.0f              // about the long axis

static float hx_rho(float h) { return 1.225f * expf(-fmaxf(h, 0.0f) / 8500.0f); }

// A flap's geometry in the ship's own axes: hinge, direction out from the hinge, length along the ship, area, and how far out it is (0 to 1).
typedef struct { float hx, hy, z0, z1, dx, dy, area, out; } HXFL;
static void hx_flap_geo(int f, HXFL *g) {
    const float s = (f & 1) ? 1.0f : -1.0f;
    const bool fore = f < 2;
    const float span = fore ? 3.4f : 5.2f;
    const float th = -0.55f + hx.flap[f] * 2.3f;
    g->hx = s * HX_R * 0.9f; g->hy = -HX_R * 0.4f;
    g->z0 = fore ? 6.0f : -20.0f; g->z1 = fore ? 11.5f : -8.0f;
    g->dx = s * sinf(th); g->dy = cosf(th);
    g->area = span * (g->z1 - g->z0);
    g->out = clampf(hx.flap[f] * 1.3f - 0.15f, 0.0f, 1.0f);              // tucked in the hull they do nothing
}

// The forces and turning moments of the air, in the ship's axes, per kilogram. acc: m/s^2 (x right, y back, z nose); alp: radians/s^2.
static void hx_air(V3 *acc, V3 *alp) {
    *acc = v3(0, 0, 0); *alp = v3(0, 0, 0);
    const float rho = hx_rho(pl.pos.y);
    const float m = HX_M0 * (0.55f + 0.45f * hx.fuel);
    const V3 vb = v3(vdot(pl.vel, pl.r), vdot(pl.vel, pl.u), vdot(pl.vel, pl.f)), w = v3(hx.wx, hx.wy, hx.wz);
    V3 F = v3(0, 0, 0), T = v3(0, 0, 0);
    // The drum: five stations along the ship, each pushed by the wind across it (this also damps the turning).
    static const float SZ[5] = { -20.0f, -10.0f, 0.0f, 10.0f, 20.0f }, SD[5] = { 9.0f, 9.0f, 9.0f, 9.0f, 5.0f };
    for (int k = 0; k < 5; k++) {
        const V3 p = v3(0.0f, 0.0f, SZ[k] - HX_ICG);
        const V3 vl = v3(vb.x + (w.y * p.z - w.z * p.y), vb.y + (w.z * p.x - w.x * p.z), 0.0f);
        const float sl = sqrtf(vl.x * vl.x + vl.y * vl.y);
        const float c = -0.5f * rho * HX_CD * SD[k] * 10.0f * sl;
        const V3 fk = v3(c * vl.x, c * vl.y, 0.0f);
        F = vadd(F, fk);
        T = vadd(T, vcross(p, fk));
    }
    F.z += -0.5f * rho * 0.5f * 63.6f * vb.z * fabsf(vb.z);                       // along the ship: the nose or the engines first
    for (int f = 0; f < 4; f++) {                                                  // the flaps: a plate in the wind, pushed along its own normal
        HXFL g; hx_flap_geo(f, &g);
        if (g.out <= 0.0f) continue;
        const float sp = (f < 2) ? 3.4f : 5.2f;
        const V3 p = v3(g.hx + g.dx * sp * 0.5f, g.hy + g.dy * sp * 0.5f, 0.5f * (g.z0 + g.z1) - HX_ICG);
        const V3 vl = v3(vb.x + (w.y * p.z - w.z * p.y), vb.y + (w.z * p.x - w.x * p.z), vb.z + (w.x * p.y - w.y * p.x));
        const V3 n = v3(g.dy, -g.dx, 0.0f);
        const float vn = vl.x * n.x + vl.y * n.y;
        const float c = -0.5f * rho * 1.4f * g.area * g.out * fabsf(vn);
        const V3 ff = v3(c * vn * n.x, c * vn * n.y, 0.0f);
        F = vadd(F, ff);
        T = vadd(T, vcross(p, ff));
    }
    *acc = vscale(F, 1.0f / m);
    *alp = v3(T.x / (m * HX_I_T), T.y / (m * HX_I_T), T.z / (m * HX_I_L));
}

static void hx_rotate(V3 wbody, float dt) {                                         // turn the ship's axes by a turn rate given in its own axes
    const V3 wd = vadd(vadd(vscale(pl.r, wbody.x), vscale(pl.u, wbody.y)), vscale(pl.f, wbody.z));
    const float ang = sqrtf(vdot(wd, wd)) * dt;
    if (ang < 1e-7f) return;
    const V3 ax = vnorm(wd);
    const float c = cosf(ang), sn = sinf(ang);
    V3 *v[3] = { &pl.f, &pl.r, &pl.u };
    for (int i = 0; i < 3; i++) {                                                  // Rodrigues
        const V3 x = *v[i];
        *v[i] = vadd(vadd(vscale(x, c), vscale(vcross(ax, x), sn)), vscale(ax, vdot(ax, x) * (1.0f - c)));
    }
    orthonormalize();
}

// The pad camera stands on the ground a little beyond the landing pad, off to one side.
static V3 hx_pad_cam(void) { return v3(hx.pad.x + 80.0f, 0.0f, hx.pad.z + 45.0f); }

// ---- Exhaust smoke and the dust the engines blow up near the ground. ----------------------------------------------------------------
static void hx_puff_add(V3 p, V3 v, float life, float size, bool dust) {
    if (hx_np >= HX_NPUFF) return;
    HXPuff *q = &hx_puffs[hx_np++];
    q->p = p; q->v = vadd(v, v3(kx_rnd() * 1.5f, kx_rnd() * 0.8f, kx_rnd() * 1.5f)); q->age = 0.0f; q->life = life * (0.8f + 0.2f * fabsf(kx_rnd())); q->size = size; q->dust = dust;
}
static void hx_puffs_step(void) {
    int n = 0;
    for (int i = 0; i < hx_np; i++) {
        HXPuff q = hx_puffs[i];
        q.age += DT;
        if (q.age >= q.life) continue;
        q.p = vadd(q.p, vscale(q.v, DT));
        q.v = vscale(q.v, 1.0f - (q.dust ? 1.4f : 1.0f) * DT);
        if (q.dust) q.v.y += 1.5f * DT;
        hx_puffs[n++] = q;
    }
    hx_np = n;
    static int tick;
    tick++;
    if (!hx.eng || pl.crashed) return;
    if ((tick & 1) == 0) {                                                         // smoke from the bells
        const V3 nz = MW(0.0f, 0.0f, -28.0f);
        hx_puff_add(nz, vadd(pl.vel, vscale(pl.f, -(6.0f + 18.0f * hx.thr))), 1.4f, 0.9f + 0.7f * hx.thr, false);
    }
    if (pl.f.y > 0.4f && hx.agl < 70.0f) {                                         // dust and spray blown out from under the engines
        const float k = (1.0f - hx.agl / 70.0f) * hx.thr;
        const V3 nz = MW(0.0f, 0.0f, -26.0f);
        const float gy = hx_support(nz.x, nz.z);
        for (int i = 0; i < 2; i++) {
            const float a = (kx_rnd() + 1.0f) * PI_F;
            hx_puff_add(v3(nz.x + 2.5f * sinf(a), gy + 0.8f, nz.z + 2.5f * cosf(a)), v3(sinf(a) * 28.0f * k, 1.5f, cosf(a) * 28.0f * k), 1.8f, 1.8f + 1.8f * k, true);
        }
    }
}
static void hx_draw_puffs(void) {
    for (int i = 0; i < hx_np; i++) {
        const HXPuff *q = &hx_puffs[i];
        const float t = q->age / q->life;
        const float rad = q->size * (0.5f + 1.2f * t);
        const int a = (int)((q->dust ? 120.0f : 80.0f) * (1.0f - t));
        if (a < 4) continue;
        V3 pts[6];
        for (int k = 0; k < 6; k++) {
            const float ang = k * (PI_F / 3.0f);
            pts[k] = vadd(q->p, vadd(vscale(cam.r, cosf(ang) * rad), vscale(cam.u, sinf(ang) * rad)));
        }
        draw_world_poly(pts, 6, q->dust ? RGBA8(196, 176, 140, a) : RGBA8(225, 226, 232, a));
    }
}

static void hx_update(const SceCtrlData *pad, unsigned int pressed) {
    const float dt = DT;
    view_mode = 1;
    hx.t += dt;
    hx_puffs_step();

    // Engines: X lights or shuts them down, the D-pad up and down sets the throttle. Square opens or tucks the flaps.
    if ((pressed & SCE_CTRL_CROSS) && !hx.eng && hx.fuel > 0.0f) hx.eng = true;
    else if (pressed & SCE_CTRL_CROSS) hx.eng = false;
    if (pressed & SCE_CTRL_SQUARE) hx.dep = !hx.dep;
    if (pressed & SCE_CTRL_CIRCLE) hx.legc = !hx.legc;
    if (pressed & SCE_CTRL_LEFT)   { hx.assist = !hx.assist; if (!hx.assist) hx.flip = false; }
    if (pressed & SCE_CTRL_RIGHT && hx.assist && !hx.ground) hx.flip = !hx.flip;
    if (pad->buttons & (SCE_CTRL_UP | SCE_CTRL_DOWN)) hx.manual = 1.5f;
    else hx.manual = fmaxf(0.0f, hx.manual - dt);
    if (pad->buttons & SCE_CTRL_UP)   hx.thr += 0.5f * dt;
    if (pad->buttons & SCE_CTRL_DOWN) hx.thr -= 0.5f * dt;

    // The computer's landing guidance (the flip, the burn, the legs): when the flip has been asked for it points the engines so the ship
    // comes to the pad, lights them when it must to stop in time, holds the descent to what it can stop, and puts the legs out.
    const float mf0 = 1.0f / (0.55f + 0.45f * hx.fuel);
    const bool stirred = fabsf(stick(pad->ly)) > 0.35f || fabsf(stick(pad->lx)) > 0.35f;
    if (hx.flip && (hx.ground || stirred)) hx.flip = false;                       // touched down, or the pilot takes over
    const bool guide = hx.assist && hx.flip && !hx.ground;
    if (guide) {
        const float aeff = fmaxf(1.0f, (HX_TWR * 0.9f * mf0 * 0.92f - 1.0f) * HX_G * 0.8f);       // the braking it can count on (tilted a little, throttle in hand)
        const float vy = pl.vel.y, h = fmaxf(0.0f, hx.agl);
        // Aim: where the engines must push to bring the ship over the pad and then straight down.
        const float ex = hx.pad.x - pl.pos.x, ez = hx.pad.z - pl.pos.z, d = sqrtf(ex * ex + ez * ez);
        const float vmax = clampf(h * 0.12f, 2.0f, 30.0f);
        const float k = d > 1.0f ? fminf(0.12f * d, vmax) / d : 0.0f;
        float ahx = 0.8f * (ex * k - pl.vel.x), ahz = 0.8f * (ez * k - pl.vel.z);
        if (h < 40.0f) { ahx = -1.0f * pl.vel.x; ahz = -1.0f * pl.vel.z; }
        const float lim = (h > 100.0f ? 0.45f : 0.15f) * HX_G;
        const float am = sqrtf(ahx * ahx + ahz * ahz);
        if (am > lim) { ahx *= lim / am; ahz *= lim / am; }
        hx.fh = vnorm(v3(ahx, HX_G, ahz));
        if (pl.f.y > 0.6f) hx.dep = false;                                         // flaps away once the nose is coming up
        if (h < 120.0f) hx.legc = true;
        // The burn: light the engines when the ship is upright and could just stop in time.
        if (!hx.eng && hx.fuel > 0.0f && pl.f.y > 0.85f && vy < -5.0f && h < 1.25f * vy * vy / (2.0f * aeff) + 50.0f) hx.eng = true;
        // The throttle: hold the sink rate to what the engines can stop (a little less near the ground).
        if (hx.eng && hx.manual <= 0.0f) {
            const float vt = -fminf(sqrtf(2.0f * aeff * fmaxf(h - 4.0f, 0.0f)) * 0.95f, 150.0f) - 1.0f;
            const float ff = h > 4.0f ? aeff * 0.9f : 0.0f;                         // the braking the profile itself calls for
            const float ac = clampf(ff + 3.0f * (vt - vy), -0.9f * HX_G, 1.5f * HX_G);
            hx.thr = (HX_G + ac) / (HX_TWR * HX_G * mf0 * fmaxf(pl.f.y, 0.5f));
        }
    }
    hx.thr = clampf(hx.thr, HX_MINTHR, 1.0f);
    hx.leg += clampf((hx.legc ? 1.0f : 0.0f) - hx.leg, -0.5f * dt, 0.5f * dt);
    if (hx.fuel <= 0.0f) hx.eng = false;
    const float thrust = hx.eng ? hx.thr : 0.0f;
    hx.fuel = fmaxf(0.0f, hx.fuel - thrust * dt / HX_BURN);

    // The pilot asks for turn rates: the left stick tilts the nose (up: toward the belly), sideways: toward that side;
    // the right stick banks it (rolls it about its long axis). The computer works to hold the rates the sticks ask for.
    // The thrusters and the engine's swivel do the fast work; the flaps, trimmed by the computer, carry the steady load in the air.
    float cx = hx.ground ? 0.0f : -shape(stick(pad->ly)) * 0.6f, cy = hx.ground ? 0.0f : shape(stick(pad->lx)) * 0.5f, cz = hx.ground ? 0.0f : -shape(stick(pad->rx)) * 0.8f;
    if (hx.ground || fabsf(cx) > 0.005f || fabsf(cy) > 0.005f) hx.fh = pl.f;      // hands off: the computer holds the attitude it had
    else { const float lm = guide ? 0.6f : 0.4f; cx = clampf(-1.5f * vdot(hx.fh, pl.u), -lm, lm); cy = clampf(1.5f * vdot(hx.fh, pl.r), -lm, lm); }
    if (hx.ground || fabsf(cz) > 0.005f) hx.uh = pl.u;
    else cz = clampf(-1.5f * vdot(hx.uh, pl.r), -0.5f, 0.5f);
    V3 aair, alair;
    hx_air(&aair, &alair);
    if (hx.ground) { aair = v3(0, 0, 0); alair = v3(0, 0, 0); }
    const float amax = 0.30f + (thrust > 0.0f ? 0.40f * thrust : 0.0f);           // turning authority of the thrusters and the swivel (rad/s^2)
    const float azmax = 0.40f;
    const float ax = clampf(4.0f * (cx - hx.wx), -amax, amax), ay = clampf(4.0f * (cy - hx.wy), -amax, amax), az = clampf(4.0f * (cz - hx.wz), -azmax, azmax);
    hx.wx += (alair.x + ax) * dt; hx.wy += (alair.y + ay) * dt; hx.wz += (alair.z + az) * dt;
    if (hx.ground) hx.wx = hx.wy = hx.wz = 0.0f;
    hx_rotate(v3(hx.wx, hx.wy, hx.wz), dt);

    // The flaps: tucked, or out and trimmed. The trims creep until the turn rate is the one asked for.
    if (hx.dep && !hx.ground) {
        hx.dp = clampf(hx.dp + 0.5f * (cx - hx.wx) * dt, -1.0f, 1.0f);
        hx.dr = clampf(hx.dr + 0.5f * (cz - hx.wz) * dt, -1.0f, 1.0f);
    } else { hx.dp *= 1.0f - fminf(1.0f, 1.0f * dt); hx.dr *= 1.0f - fminf(1.0f, 1.0f * dt); }
    for (int f = 0; f < 4; f++) {
        const float sd = (f & 1) ? 1.0f : -1.0f;                                    // +1: right flaps
        const float pf = (f < 2) ? HX_PITCH_SIGN : -HX_PITCH_SIGN;                  // fore and aft work against each other
        const float want = hx.dep ? clampf(0.70f + 0.30f * (pf * hx.dp + sd * HX_ROLL_SIGN * hx.dr), 0.0f, 1.0f) : 0.0f;
        hx.flap[f] += (want - hx.flap[f]) * fminf(1.0f, 4.0f * dt);
    }

    // Forces: gravity, thrust along the ship's long axis (lighter as the fuel burns), and the air.
    const float mf = 1.0f / (0.55f + 0.45f * hx.fuel);
    V3 acc = v3(0.0f, -HX_G, 0.0f);
    acc = vadd(acc, vscale(pl.f, thrust * HX_TWR * HX_G * mf));
    acc = vadd(acc, vadd(vadd(vscale(pl.r, aair.x), vscale(pl.u, aair.y)), vscale(pl.f, aair.z)));
    pl.vel = vadd(pl.vel, vscale(acc, dt));
    pl.pos = vadd(pl.pos, vscale(pl.vel, dt));

    // The legs: find the lowest foot above whatever is under it.
    float gap = 1e9f;
    for (int i = 0; i < 4; i++) {
        const float a = i * (PI_F / 2.0f) + PI_F / 4.0f;
        const float rf = 4.7f + 2.7f * hx.leg;
        const V3 f = MW(rf * sinf(a), rf * cosf(a), -25.0f);
        const float g = f.y - hx_support(f.x, f.z);
        if (g < gap) gap = g;
        if (f.y < 1.0f && (sea_depth(f.x, f.z) > 0.0f || in_lake(f.x, f.z, 1.0f))) { crash_why("landed in the water"); crash(); return; }
    }
    if (gap < -0.001f || (gap < 0.05f && pl.vel.y <= 0.0f)) {                         // on something (or coming down onto it)
        const float vv = -pl.vel.y, hs = sqrtf(pl.vel.x * pl.vel.x + pl.vel.z * pl.vel.z), tilt = hx_tilt();
        if (!hx.ground) {                                                              // touching down
            if (gap < -4.0f) { crash_why("hit the mount"); crash(); return; }
            if (hx.leg < 0.9f) { crash_why("landed without its legs"); crash(); return; }
            if (vv > 4.0f)   { crash_why("came down too fast"); crash(); return; }
            if (hs > 3.0f)   { crash_why("came down sideways"); crash(); return; }
            if (tilt > 0.20f) { crash_why("came down tilted"); crash(); return; }
            hx.td = vv;
            if (hx.assist) hx.eng = false;                                            // the computer shuts the engines down on touchdown
        }
        pl.pos.y -= gap;
        pl.vel = v3(0.0f, 0.0f, 0.0f);
        hx.ground = true;
    } else hx.ground = false;
    hx.agl = gap;
    if (hx.ground) {                                                                   // standing: settle upright
        const V3 up = v3(0.0f, 1.0f, 0.0f);
        pl.f = vnorm(vadd(pl.f, vscale(vsub(up, pl.f), fminf(1.0f, 3.0f * dt))));
        pl.r = vnorm(vsub(pl.r, vscale(pl.f, vdot(pl.r, pl.f))));
        pl.u = vcross(pl.f, pl.r);
        if (thrust * HX_TWR * mf <= 1.0f) pl.vel = v3(0.0f, 0.0f, 0.0f);
    }
    {                                                                                  // the hull on the ground (tipped over), the tower and other tall things
        const float zs[4] = { -25.0f, -8.0f, 10.0f, 25.0f };
        const float side = sqrtf(fmaxf(0.0f, 1.0f - pl.f.y * pl.f.y)) * HX_R;
        for (int j = 0; j < 4; j++) {
            const V3 q = MW(0.0f, 0.0f, zs[j]);
            if (hx_tilt() > 0.35f && q.y - side < hx_support(q.x, q.z) - 0.2f) { crash_why("hit the ground"); crash(); return; }
            for (int i = 0; i < nboxes; i++) {
                const Box *b = &boxes[i];
                if (b->h > 20.0f && fabsf(q.x - b->x) < b->w * 0.5f + HX_R && fabsf(q.z - b->z) < b->d * 0.5f + HX_R && q.y < b->h) { crash_why("hit the tower"); crash(); return; }
            }
        }
    }
    if (hx_alt_ft() * 0.3048f > hx.maxalt) hx.maxalt = hx_alt_ft() * 0.3048f;

    pl.on_ground = hx.ground; pl.gs = 0.0f; pl.touched = true;
    pl.hdg = pl.hdg_g = hx.psi; pl.pitch = asinf(clampf(pl.f.y, -1.0f, 1.0f)); pl.roll = 0.0f;
    pl.speed = sqrtf(vdot(pl.vel, pl.vel)); pl.vs = pl.vel.y; pl.alpha = 0.0f; pl.beta = 0.0f;
    pl.throttle = thrust; pl.rpm = 0.0f; pl.fuel = hx.fuel;
}

static void draw_hx_panel(vita2d_pgf *pgf) {
    const unsigned int white = RGBA8(240, 242, 248, 255), dim = RGBA8(150, 156, 172, 255), gold = RGBA8(255, 206, 130, 255), red = RGBA8(255, 110, 100, 255), green = RGBA8(120, 220, 140, 255);
    vita2d_draw_rectangle(0, VIEW_H, SCREEN_W, SCREEN_H - VIEW_H, RGBA8(30, 32, 40, 255));
    vita2d_draw_rectangle(0, VIEW_H, SCREEN_W, 3, RGBA8(120, 124, 136, 255));
    vita2d_pgf_draw_text(pgf, 24, VIEW_H + 34, gold, 1.1f, "STRAKE HX-1");
    vita2d_pgf_draw_text(pgf, 24, VIEW_H + 58, dim, 0.7f, "HOP TEST");
    vita2d_pgf_draw_textf(pgf, 24, VIEW_H + 96, white, 1.3f, "%d FT", (int)(hx_alt_ft() + 0.5f));
    vita2d_pgf_draw_text(pgf, 24, VIEW_H + 114, dim, 0.62f, "ABOVE SEA LEVEL");
    vita2d_pgf_draw_textf(pgf, 24, VIEW_H + 134, dim, 0.62f, "HIGHEST %d FT", (int)(hx.maxalt * 3.28084f + 0.5f));
    // Engine, throttle and fuel.
    vita2d_pgf_draw_text(pgf, 250, VIEW_H + 34, hx.eng ? green : dim, 0.8f, hx.eng ? "ENGINES ON" : "ENGINES OFF");
    vita2d_pgf_draw_text(pgf, 250, VIEW_H + 58, dim, 0.62f, "THROTTLE");
    vita2d_draw_rectangle(340, VIEW_H + 46, 120, 14, RGBA8(52, 56, 68, 255));
    vita2d_draw_rectangle(340, VIEW_H + 46, 120.0f * hx.thr, 14, hx.eng ? gold : dim);
    vita2d_pgf_draw_text(pgf, 250, VIEW_H + 80, dim, 0.62f, "FUEL");
    vita2d_draw_rectangle(340, VIEW_H + 68, 120, 14, RGBA8(52, 56, 68, 255));
    vita2d_draw_rectangle(340, VIEW_H + 68, 120.0f * hx.fuel, 14, hx.fuel > 0.2f ? white : red);
    vita2d_pgf_draw_textf(pgf, 250, VIEW_H + 104, white, 0.8f, "%+d FPM", (int)(pl.vs * 196.85f));
    vita2d_pgf_draw_textf(pgf, 410, VIEW_H + 104, white, 0.8f, "PITCH %d", (int)(asinf(clampf(pl.f.y, -1.0f, 1.0f)) * 57.2958f + 0.5f));
    // The state of the ship.
    const char *st = hx.ground ? "ON THE GROUND" : "FLYING";
    vita2d_pgf_draw_text(pgf, 560, VIEW_H + 34, hx.ground ? green : white, 0.8f, st);
    vita2d_pgf_draw_text(pgf, 700, VIEW_H + 34, hx.assist ? green : dim, 0.8f, hx.assist ? (hx.flip ? "ASSIST: FLIPPING" : "ASSIST ON") : "ASSIST OFF");
    if (hx.ground && hx.td > 0.0f) vita2d_pgf_draw_textf(pgf, 560, VIEW_H + 58, dim, 0.62f, "TOUCHDOWN %.1f M/S", hx.td);
    vita2d_pgf_draw_textf(pgf, 560, VIEW_H + 82, white, 0.8f, "%d KT", (int)(pl.speed * 1.94384f + 0.5f));
    vita2d_pgf_draw_text(pgf, 700, VIEW_H + 82, hx.dep ? gold : dim, 0.8f, hx.dep ? "FLAPS OUT" : "FLAPS TUCKED");
    vita2d_pgf_draw_text(pgf, 560, VIEW_H + 104, hx.leg > 0.9f ? green : (hx.leg > 0.05f ? gold : dim), 0.8f, hx.leg > 0.9f ? "LEGS OUT" : (hx.leg > 0.05f ? "LEGS MOVING" : "LEGS FOLDED"));
    {                                                                              // the pad: how far, and which way (a compass bearing, like the other challenges)
        const float dx = hx.pad.x - pl.pos.x, dz = hx.pad.z - pl.pos.z, b = atan2f(dx, dz) * 57.2958f;
        vita2d_pgf_draw_textf(pgf, 700, VIEW_H + 104, white, 0.8f, "PAD %03d  %d FT", (int)(b < 0.0f ? b + 360.0f : b + 0.5f) % 360, (int)(sqrtf(dx * dx + dz * dz) * 3.28084f + 0.5f));
    }
    vita2d_pgf_draw_text(pgf, 250, VIEW_H + 124, dim, 0.62f, "X: engines  Square: flaps  O: legs  D-pad up/down: throttle  left: assist  right: FLIP  Triangle: restart   SELECT: camera");
    vita2d_pgf_draw_text(pgf, 250, VIEW_H + 142, dim, 0.62f, "L stick: nose   R stick: bank.  Safe landing: legs out, upright, under 4 m/s.");
}

/* ------------------------------------------------------------------ */
/* The landing pad, south of the launch mounts                          */
/* ------------------------------------------------------------------ */

static void hx_build_pad(const Airport *a) {
    V3 c = ap_pt(a, HX_PAD_LX, 0.0f, HX_PAD_LZ);
    sp_cyl(c.x, c.z, HX_PAD_R, 0.0f, 0.3f, 150, 150, 146, 0);               // the concrete
    sp_cyl(c.x, c.z, HX_PAD_R * 0.74f, 0.3f, 0.42f, 92, 94, 100, 0);        // the dark landing ring
    sp_cyl(c.x, c.z, HX_PAD_R * 0.42f, 0.42f, 0.54f, 150, 150, 146, 0);
    sp_cyl(c.x, c.z, HX_PAD_R * 0.14f, 0.54f, 0.66f, 226, 176, 40, 0);      // the bullseye
}
