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

typedef struct {
    float psi;                     // the heading it stands facing (radians)
    bool  eng, ground;             // the engines are lit; the legs are on something
    float thr, fuel;               // the throttle setting (0.3 to 1) and the fuel (0 to 1)
    float wp, wy, wr;              // turn rates: pitch, yaw, roll (radians per second)
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
    hx.eng = false; hx.ground = true; hx.thr = 0.6f; hx.fuel = 1.0f;
    hx.wp = hx.wy = hx.wr = 0.0f; hx.maxalt = 0.0f; hx.td = 0.0f; hx.t = 0.0f;
    hx.pad = ap_pt(a, HX_PAD_LX, 0.0f, HX_PAD_LZ);
    set_axes(hx.psi, HX_UP, 0.0f);
    chase_dir = v3(sinf(hx.psi), 0.0f, cosf(hx.psi));
    look_ang = 0.0f; view_mode = 1;
}
static void hx_restart(void) { hx_start(); }

// Where the chase camera looks from: level with the horizon, from behind the way the ship faces.
static V3 hx_chase_target(void) { return v3(sinf(hx.psi), 0.12f, cosf(hx.psi)); }

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

static void hx_update(const SceCtrlData *pad, unsigned int pressed) {
    const float dt = DT;
    view_mode = 1;
    hx.t += dt;
    // The flaps stay tucked for the hop (hold O to see them open; they come into their own in the belly flop).
    for (int i = 0; i < 4; i++) hx.flap[i] += (((pad->buttons & SCE_CTRL_CIRCLE) ? 1.0f : 0.0f) - hx.flap[i]) * fminf(1.0f, 3.0f * dt);

    // Engines: X lights or shuts them down, the D-pad up and down sets the throttle.
    if ((pressed & SCE_CTRL_CROSS) && !hx.eng && hx.fuel > 0.0f) hx.eng = true;
    else if (pressed & SCE_CTRL_CROSS) hx.eng = false;
    if (pad->buttons & SCE_CTRL_UP)   hx.thr += 0.5f * dt;
    if (pad->buttons & SCE_CTRL_DOWN) hx.thr -= 0.5f * dt;
    hx.thr = clampf(hx.thr, HX_MINTHR, 1.0f);
    if (hx.fuel <= 0.0f) hx.eng = false;
    const float thrust = hx.eng ? hx.thr : 0.0f;
    hx.fuel = fmaxf(0.0f, hx.fuel - thrust * dt / HX_BURN);

    // Attitude: the left stick tilts the nose (up: toward the belly, which is the way the ship faces; sideways: toward that side),
    // the right stick turns it about its long axis. Let go and the turn stops (the computer holds the rates at zero). Not on the ground.
    const float k = fminf(1.0f, 4.0f * dt);
    const float cp = hx.ground ? 0.0f : shape(stick(pad->ly)) * 0.5f, cy = hx.ground ? 0.0f : shape(stick(pad->lx)) * 0.5f, cr = hx.ground ? 0.0f : shape(stick(pad->rx)) * 0.5f;
    hx.wp += (cp - hx.wp) * k; hx.wy += (cy - hx.wy) * k; hx.wr += (cr - hx.wr) * k;
    rot_pitch(hx.wp * dt); rot_yaw(hx.wy * dt); rot_roll(hx.wr * dt);
    orthonormalize();

    // Forces: gravity, thrust along the ship's long axis (lighter as the fuel burns), and a little drag.
    const float mf = 1.0f / (0.55f + 0.45f * hx.fuel);
    V3 acc = v3(0.0f, -HX_G, 0.0f);
    acc = vadd(acc, vscale(pl.f, thrust * HX_TWR * HX_G * mf));
    const float sp = sqrtf(vdot(pl.vel, pl.vel));
    acc = vsub(acc, vscale(pl.vel, 0.0004f * sp));
    pl.vel = vadd(pl.vel, vscale(acc, dt));
    pl.pos = vadd(pl.pos, vscale(pl.vel, dt));

    // The legs: find the lowest foot above whatever is under it.
    float gap = 1e9f;
    for (int i = 0; i < 4; i++) {
        const float a = i * (PI_F / 2.0f) + PI_F / 4.0f;
        const V3 f = MW(7.4f * sinf(a), 7.4f * cosf(a), -25.0f);
        const float g = f.y - hx_support(f.x, f.z);
        if (g < gap) gap = g;
        if (f.y < 1.0f && (sea_depth(f.x, f.z) > 0.0f || in_lake(f.x, f.z, 1.0f))) { crash_why("landed in the water"); crash(); return; }
    }
    if (gap < -0.001f || (gap < 0.05f && pl.vel.y <= 0.0f)) {                         // on something (or coming down onto it)
        const float vv = -pl.vel.y, hs = sqrtf(pl.vel.x * pl.vel.x + pl.vel.z * pl.vel.z), tilt = hx_tilt();
        if (!hx.ground) {                                                              // touching down
            if (gap < -4.0f) { crash_why("hit the mount"); crash(); return; }
            if (vv > 4.0f)   { crash_why("came down too fast"); crash(); return; }
            if (hs > 3.0f)   { crash_why("came down sideways"); crash(); return; }
            if (tilt > 0.20f) { crash_why("came down tilted"); crash(); return; }
            hx.td = vv;
        }
        pl.pos.y -= gap;
        pl.vel = v3(0.0f, 0.0f, 0.0f);
        hx.ground = true;
    } else hx.ground = false;
    if (hx.ground) {                                                                   // standing: settle upright
        const V3 up = v3(0.0f, 1.0f, 0.0f);
        pl.f = vnorm(vadd(pl.f, vscale(vsub(up, pl.f), fminf(1.0f, 3.0f * dt))));
        pl.r = vnorm(vsub(pl.r, vscale(pl.f, vdot(pl.r, pl.f))));
        pl.u = vcross(pl.f, pl.r);
        if (thrust * HX_TWR * mf <= 1.0f) pl.vel = v3(0.0f, 0.0f, 0.0f);
    }
    {                                                                                  // the tower and other tall things
        const float zs[4] = { -25.0f, -8.0f, 10.0f, 25.0f };
        for (int j = 0; j < 4; j++) {
            const V3 q = MW(0.0f, 0.0f, zs[j]);
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
    vita2d_pgf_draw_textf(pgf, 410, VIEW_H + 104, hx_tilt() > 0.20f ? gold : white, 0.8f, "TILT %d", (int)(hx_tilt() * 57.2958f + 0.5f));
    // The state of the ship.
    const char *st = hx.ground ? "ON THE GROUND" : "FLYING";
    vita2d_pgf_draw_text(pgf, 560, VIEW_H + 34, hx.ground ? green : white, 0.8f, st);
    if (hx.ground && hx.td > 0.0f) vita2d_pgf_draw_textf(pgf, 560, VIEW_H + 58, dim, 0.62f, "TOUCHDOWN %.1f M/S", hx.td);
    vita2d_pgf_draw_text(pgf, 560, VIEW_H + 80, dim, 0.62f, "SAFE LANDING: UNDER 4 M/S, UPRIGHT");
    vita2d_pgf_draw_text(pgf, 250, VIEW_H + 124, dim, 0.62f, "X: engines   D-pad up/down: throttle   L stick: tilt   R stick: spin   Triangle: start over");
    vita2d_pgf_draw_text(pgf, 250, VIEW_H + 142, dim, 0.62f, "Full throttle lifts it; about half holds it still.");
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
