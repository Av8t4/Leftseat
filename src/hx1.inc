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

typedef struct {
    float psi;                     // the heading it stands facing (radians)
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
        pf_poly(6, h1, &c0, HX_SHIELD[0], HX_SHIELD[1], HX_SHIELD[2], 0, 255);
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
    for (int i = 0; i < 4; i++) {                                                  // four landing legs: flat struts that splay out from the lower hull
        const float a = i * (PI_F / 2.0f) + PI_F / 4.0f, sa = sinf(a), ca = cosf(a), ta = -ca, tb = sa;      // outward (sa, ca) and the tangent
        V3 q[4] = { MW(sa * HX_R * 0.98f + ta * 0.35f, ca * HX_R * 0.98f + tb * 0.35f, -9.0f), MW(sa * HX_R * 0.98f - ta * 0.35f, ca * HX_R * 0.98f - tb * 0.35f, -9.0f),
                    MW(sa * 7.4f - ta * 0.35f, ca * 7.4f - tb * 0.35f, -25.0f), MW(sa * 7.4f + ta * 0.35f, ca * 7.4f + tb * 0.35f, -25.0f) };
        V3 fc = MW(sa * 7.4f, ca * 7.4f, -25.0f);
        pf_poly(4, q, NULL, HX_DARK[0], HX_DARK[1], HX_DARK[2], PF_TWO, 255);
        V3 pad[4] = { MW(sa * 7.4f + 0.9f * sa - 0.9f * ta, ca * 7.4f + 0.9f * ca - 0.9f * tb, -25.0f), MW(sa * 7.4f + 0.9f * sa + 0.9f * ta, ca * 7.4f + 0.9f * ca + 0.9f * tb, -25.0f),
                      MW(sa * 7.4f - 0.9f * sa + 0.9f * ta, ca * 7.4f - 0.9f * ca + 0.9f * tb, -25.0f), MW(sa * 7.4f - 0.9f * sa - 0.9f * ta, ca * 7.4f - 0.9f * ca - 0.9f * tb, -25.0f) };
        (void)fc;
        pf_poly(4, pad, NULL, 50, 52, 60, PF_TWO, 255);
    }
    for (int f = 0; f < 4; f++) {                                                  // the four flaps: hinged on the sides, low on the belly side
        const float s = (f & 1) ? 1.0f : -1.0f;                                    // (the belly is the black side; the flaps are on the sides next to it)
        const bool fore = f < 2;
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
    set_axes(hx.psi, HX_UP, 0.0f);
    chase_dir = v3(sinf(hx.psi), 0.0f, cosf(hx.psi));
    look_ang = 0.0f; view_mode = 1;
}
static void hx_restart(void) { hx_start(); }

// Where the chase camera looks from: level with the horizon, from behind the way the ship faces.
static V3 hx_chase_target(void) { return v3(sinf(hx.psi), 0.12f, cosf(hx.psi)); }

static void hx_update(const SceCtrlData *pad, unsigned int pressed) {
    (void)pressed;
    view_mode = 1;
    const float p = -stick(pad->ly), r = stick(pad->lx);            // the left stick: up opens the flaps; left / right works one side more than the other
    for (int i = 0; i < 4; i++) {
        const float s = (i & 1) ? 1.0f : -1.0f;
        const float want = clampf(p + s * r * 0.6f, 0.0f, 1.0f);
        hx.flap[i] += (want - hx.flap[i]) * fminf(1.0f, 3.0f * DT);
    }
    set_axes(hx.psi, HX_UP, 0.0f);
    pl.on_ground = true; pl.gs = 0.0f; pl.vel = v3(0, 0, 0);
    pl.hdg = pl.hdg_g = hx.psi; pl.pitch = HX_UP; pl.roll = 0.0f;
    pl.speed = 0.0f; pl.vs = 0.0f; pl.alpha = 0.0f; pl.beta = 0.0f; pl.throttle = 0.0f; pl.rpm = 0.0f;
    pl.touched = true;
}

// The ship's height above the sea (feet): the engines, not the middle.
static float hx_alt_ft(void) { return (pl.pos.y - HX_HALF * fabsf(pl.f.y)) * 3.28084f; }

static void draw_hx_panel(vita2d_pgf *pgf) {
    const unsigned int white = RGBA8(240, 242, 248, 255), dim = RGBA8(150, 156, 172, 255), gold = RGBA8(255, 206, 130, 255);
    vita2d_draw_rectangle(0, VIEW_H, SCREEN_W, SCREEN_H - VIEW_H, RGBA8(30, 32, 40, 255));
    vita2d_draw_rectangle(0, VIEW_H, SCREEN_W, 3, RGBA8(120, 124, 136, 255));
    vita2d_pgf_draw_text(pgf, 24, VIEW_H + 34, gold, 1.1f, "STRAKE HX-1");
    vita2d_pgf_draw_text(pgf, 24, VIEW_H + 58, dim, 0.7f, "HOP TEST: PAD VIEW");
    vita2d_pgf_draw_textf(pgf, 24, VIEW_H + 96, white, 1.3f, "%d FT", (int)(hx_alt_ft() + 0.5f));
    vita2d_pgf_draw_text(pgf, 24, VIEW_H + 114, dim, 0.62f, "ABOVE SEA LEVEL");
    vita2d_pgf_draw_text(pgf, 250, VIEW_H + 34, white, 0.8f, "FLAPS");
    static const char *const FN[4] = { "FWD L", "FWD R", "AFT L", "AFT R" };
    for (int i = 0; i < 4; i++) {
        const float x = 250.0f + i * 92.0f, y = VIEW_H + 46.0f;
        vita2d_draw_rectangle(x, y, 74, 14, RGBA8(52, 56, 68, 255));
        vita2d_draw_rectangle(x, y, 74.0f * hx.flap[i], 14, gold);
        vita2d_pgf_draw_text(pgf, x, y + 32, dim, 0.6f, FN[i]);
    }
    vita2d_pgf_draw_text(pgf, 250, VIEW_H + 114, dim, 0.62f, "Left stick: flaps    L / R: look around    Triangle: start over");
    vita2d_pgf_draw_text(pgf, 250, VIEW_H + 132, dim, 0.62f, "The engines and the flight controls come in the next update.");
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
