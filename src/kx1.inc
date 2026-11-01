// Left Seat: the Strake KX-1, an electric coupe. It is driven, not flown, so it has its own update path: update() hands
// each frame to kx_update(), which does the driving (this file) and, in the same place, the cold-gas thrusters.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

static float sea_depth(float x, float z);                  // (spaceport.inc)

/* ------------------------------------------------------------------ */
/* The model                                                           */
/* ------------------------------------------------------------------ */

// Model space as for the aircraft: x right, y up from the ground, z forward, origin in the middle of the car.
// 4.4 m long, 1.95 m wide, about 1.1 m tall. The wheels are at z = -1.32 (rear) and +1.11 (front).
#define KC_GRAPH 46, 52, 62
#define KC_YEL   246, 200, 44
static const float KX_WZ_REAR = -1.32f, KX_WZ_FRONT = 1.11f, KX_WX = 0.80f;

static void kx_box(float x0, float x1, float y0, float y1, float z0, float z1, float r, float g, float b) {
    V3 c[8]; int k = 0;
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) for (int l = 0; l < 2; l++) c[k++] = MW(i ? x1 : x0, j ? y1 : y0, l ? z1 : z0);
    V3 cc = MW((x0 + x1) * 0.5f, (y0 + y1) * 0.5f, (z0 + z1) * 0.5f);
    static const int F[6][4] = { {0,1,3,2}, {4,6,7,5}, {0,4,5,1}, {2,3,7,6}, {0,2,6,4}, {1,5,7,3} };
    for (int f = 0; f < 6; f++) { V3 q[4] = { c[F[f][0]], c[F[f][1]], c[F[f][2]], c[F[f][3]] }; pf_poly(4, q, &cc, r, g, b, 0, 255); }
}
// The same, lit from within (lamps).
static void kx_lamp(float x0, float x1, float y0, float y1, float z0, float z1, float r, float g, float b) {
    V3 c[8]; int k = 0;
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) for (int l = 0; l < 2; l++) c[k++] = MW(i ? x1 : x0, j ? y1 : y0, l ? z1 : z0);
    V3 cc = MW((x0 + x1) * 0.5f, (y0 + y1) * 0.5f, (z0 + z1) * 0.5f);
    static const int F[6][4] = { {0,1,3,2}, {4,6,7,5}, {0,4,5,1}, {2,3,7,6}, {0,2,6,4}, {1,5,7,3} };
    for (int f = 0; f < 6; f++) { V3 q[4] = { c[F[f][0]], c[F[f][1]], c[F[f][2]], c[F[f][3]] }; pf_poly(4, q, &cc, r, g, b, PF_EMIT, 255); }
}

typedef struct { float z, w, wt, yb, yt; } KXS;          // a section of the body: half width, half width of the top, bottom, top
static const KXS KXA[] = {
    { 2.20f, 0.12f, 0.08f, 0.14f, 0.20f }, { 2.00f, 0.30f, 0.22f, 0.12f, 0.28f }, { 1.62f, 0.47f, 0.36f, 0.10f, 0.40f },
    { 1.23f, 0.61f, 0.48f, 0.09f, 0.46f }, { 1.05f, 0.68f, 0.50f, 0.09f, 0.54f }, { 0.55f, 0.78f, 0.52f, 0.09f, 0.90f },
    { 0.15f, 0.84f, 0.56f, 0.09f, 0.98f }, {-0.35f, 0.88f, 0.56f, 0.09f, 0.96f }, {-0.75f, 0.91f, 0.56f, 0.09f, 0.86f },
    {-1.05f, 0.93f, 0.66f, 0.09f, 0.74f }, {-1.50f, 0.92f, 0.74f, 0.10f, 0.67f }, {-1.87f, 0.86f, 0.70f, 0.12f, 0.62f },
    {-2.12f, 0.73f, 0.60f, 0.16f, 0.58f }, {-2.20f, 0.49f, 0.40f, 0.20f, 0.54f } };
#define KXN ((int)(sizeof(KXA) / sizeof(KXA[0])))
static void kx_ring(V3 *R, const KXS *s) {
    const float belt = 0.52f;
    float ysh = s->yt < belt ? s->yt * 0.88f : belt, ymid = s->yb + (ysh - s->yb) * 0.45f;
    R[0] = MW(-s->wt, s->yt, s->z); R[1] = MW(s->wt, s->yt, s->z);
    R[2] = MW(s->w * 0.92f, ysh, s->z); R[3] = MW(s->w, ymid, s->z); R[4] = MW(s->w * 0.88f, s->yb, s->z);
    R[5] = MW(-s->w * 0.88f, s->yb, s->z); R[6] = MW(-s->w, ymid, s->z); R[7] = MW(-s->w * 0.92f, ysh, s->z);
}

// A front wheel, turned by the steering (a tire is an octagonal cylinder along x, as in models.inc).
static void kx_wheel(float cx, float cy, float cz, float rad, float hw, int hub_side, float steer) {
    float cs = cosf(steer), sn = sinf(steer);
    V3 r0[8], r1[8];
    for (int i = 0; i < 8; i++) {
        float a = i * PI_F / 4.0f + PI_F / 8.0f, y = rad * cosf(a), z = rad * sinf(a);
        r0[i] = MW(cx + (-hw) * cs + z * sn, cy + y, cz + z * cs - (-hw) * sn);
        r1[i] = MW(cx + ( hw) * cs + z * sn, cy + y, cz + z * cs - ( hw) * sn);
    }
    V3 cc = MW(cx, cy, cz);
    for (int i = 0; i < 8; i++) { int j = (i + 1) & 7; V3 q[4] = { r0[i], r0[j], r1[j], r1[i] }; pf_poly(4, q, &cc, 26, 26, 30, 0, 255); }
    pf_poly(8, r0, &cc, 34, 34, 38, 0, 255);
    pf_poly(8, r1, &cc, 34, 34, 38, 0, 255);
    V3 h[8];
    for (int i = 0; i < 8; i++) {
        float a = i * PI_F / 4.0f + PI_F / 8.0f, y = rad * 0.5f * cosf(a), z = rad * 0.5f * sinf(a), xo = hub_side * (hw + 0.004f);
        h[i] = MW(cx + xo * cs + z * sn, cy + y, cz + z * cs - xo * sn);
    }
    pf_poly(8, h, NULL, 196, 202, 212, PF_DECAL, 255);
}

static float kx_steer_vis;                                  // the steering angle on the front wheels (radians)
static bool  kx_braking;                                    // the brake lights

static void build_kx1(void) {
    npf = 0; pf_last = -1;
    V3 ring[KXN][8], cen[KXN];
    for (int i = 0; i < KXN; i++) { kx_ring(ring[i], &KXA[i]); cen[i] = avgv(ring[i], 8); }
    static const float W[3] = { 238, 241, 246 }, G[3] = { 46, 52, 62 }, GL[3] = { 30, 56, 96 };
    for (int i = 0; i + 1 < KXN; i++) {
        const float zm = 0.5f * (KXA[i].z + KXA[i + 1].z);
        const V3 cc = vscale(vadd(cen[i], cen[i + 1]), 0.5f);
        const bool wind = zm < 1.10f && zm > 0.20f, rearw = zm < -0.78f && zm > -1.25f, cab = zm > -0.78f && zm < 1.10f;
        for (int e = 0; e < 8; e++) {
            const int j = (e + 1) & 7;
            V3 q[4] = { ring[i][e], ring[i][j], ring[i + 1][j], ring[i + 1][e] };
            const float *c = W;
            if (e == 0) c = (wind || rearw) ? GL : W;                    // windshield and rear window
            else if (e == 1 || e == 7) c = cab ? GL : W;                 // side windows
            else if (e == 3 || e == 6 || e == 4 || e == 5) c = G;         // the dark lower body
            pf_poly(4, q, &cc, c[0], c[1], c[2], 0, 255);
        }
    }
    pf_poly(8, ring[0], &cen[0], KC_GRAPH, 0, 255);                     // nose and tail
    pf_poly(8, ring[KXN - 1], &cen[KXN - 1], KC_GRAPH, 0, 255);
    for (int s = -1; s <= 1; s += 2) {
        const float xo = s * 0.935f;
        V3 d[4] = { MW(xo, 0.12f, 0.95f), MW(xo, 0.46f, 0.95f), MW(xo, 0.46f, -0.70f), MW(xo, 0.12f, -0.70f) };       // the door
        if (s < 0) { V3 t = d[0]; d[0] = d[3]; d[3] = t; t = d[1]; d[1] = d[2]; d[2] = t; }
        pf_poly(4, d, NULL, 150, 156, 166, PF_DECAL, 255);
        V3 in[4] = { MW(s * 0.955f, 0.14f, -0.86f), MW(s * 0.955f, 0.46f, -0.86f), MW(s * 0.955f, 0.46f, -1.28f), MW(s * 0.955f, 0.14f, -1.28f) };   // side intake
        if (s < 0) { V3 t = in[0]; in[0] = in[3]; in[3] = t; t = in[1]; in[1] = in[2]; in[2] = t; }
        pf_poly(4, in, NULL, 16, 18, 22, PF_DECAL, 255);
        kx_box(s * 0.935f - 0.015f, s * 0.935f + 0.015f, 0.09f, 0.15f, -1.00f, 1.55f, KC_YEL);                  // sill strake
        kx_box(s * 0.12f - 0.012f, s * 0.12f + 0.012f, 0.66f, 0.96f, -1.55f, -0.85f, KC_YEL);                    // rear deck strakes
        kx_box(s * 0.12f - 0.012f, s * 0.12f + 0.012f, 0.62f, 0.76f, -2.05f, -1.55f, KC_YEL);
        kx_box(s * 0.30f - 0.02f, s * 0.30f + 0.02f, 0.455f, 0.485f, 1.20f, 1.42f, KC_YEL);                       // nose strakes along the hood
        kx_box(s * 0.30f - 0.02f, s * 0.30f + 0.02f, 0.395f, 0.425f, 1.42f, 1.68f, KC_YEL);
        kx_box(s * 0.28f - 0.02f, s * 0.28f + 0.02f, 0.310f, 0.340f, 1.68f, 1.92f, KC_YEL);
        kx_box(s * 0.84f - 0.04f, s * 0.84f + 0.04f, 0.50f, 0.56f, 0.92f, 1.06f, KC_GRAPH);                       // mirror
        kx_box(s * 0.84f - 0.05f, s * 0.84f + 0.05f, 0.56f, 0.66f, 0.96f, 1.02f, 230, 235, 242);
        kx_box(s * 0.78f - 0.015f, s * 0.78f + 0.015f, 0.58f, 0.98f, -2.18f, -2.10f, KC_GRAPH);                   // wing uprights
        kx_box(s * 0.90f - 0.02f, s * 0.90f + 0.02f, 0.58f, 0.92f, -2.26f, -1.86f, KC_GRAPH);                     // end plates
    }
    kx_box(-0.92f, 0.92f, 0.90f, 0.94f, -2.26f, -1.84f, KC_GRAPH);                                                // rear wing
    if (lt_landing()) { kx_lamp(-0.55f, -0.18f, 0.20f, 0.28f, 2.17f, 2.22f, 255, 246, 200); kx_lamp(0.18f, 0.55f, 0.20f, 0.28f, 2.17f, 2.22f, 255, 246, 200); }
    else              { kx_box(-0.55f, -0.18f, 0.20f, 0.28f, 2.17f, 2.22f, 190, 192, 184);   kx_box(0.18f, 0.55f, 0.20f, 0.28f, 2.17f, 2.22f, 190, 192, 184); }
    const bool tl = lt_nav() || kx_braking;                                                                       // tail lights: dim, bright on the brake
    for (int s = -1; s <= 1; s += 2) {
        if (tl) kx_lamp(s * 0.30f, s * 0.72f > 0 ? s * 0.72f : s * 0.30f, 0.30f, 0.38f, -2.215f, -2.185f, kx_braking ? 255 : 190, 20, 20);
        else    kx_box(s * 0.30f, s * 0.72f > 0 ? s * 0.72f : s * 0.30f, 0.30f, 0.38f, -2.215f, -2.185f, 90, 20, 20);
    }
    kx_box(-0.55f, 0.55f, 0.05f, 0.09f, 2.12f, 2.28f, KC_GRAPH);                                                  // splitter
    kx_box(-0.55f, 0.55f, 0.09f, 0.10f, 2.26f, 2.28f, KC_YEL);
    for (int s = -1; s <= 1; s += 2) {
        wheel(s * KX_WX, 0.30f, KX_WZ_REAR, 0.32f, 0.15f, s);
        kx_wheel(s * KX_WX, 0.30f, KX_WZ_FRONT, 0.30f, 0.12f, s, kx_steer_vis);
    }
}

// From the driver's seat: the hood ahead and the dashboard, with the pillars of the windshield.
static void build_cockpit_nose_kx1(void) {
    npf = 0; pf_last = -1;
    kx_box(-0.70f, 0.70f, 0.44f, 0.50f, 0.45f, 0.92f, KC_GRAPH);                    // the dashboard
    kx_box(-0.70f, 0.70f, 0.30f, 0.44f, 0.88f, 0.92f, 20, 22, 26);
    // the hood, with the yellow strakes
    static const float HZ[4] = { 0.92f, 1.23f, 1.62f, 2.00f }, HY[4] = { 0.50f, 0.46f, 0.40f, 0.28f }, HW[4] = { 0.50f, 0.48f, 0.36f, 0.22f };
    for (int i = 0; i < 3; i++) {
        V3 q[4] = { MW(-HW[i], HY[i], HZ[i]), MW(HW[i], HY[i], HZ[i]), MW(HW[i + 1], HY[i + 1], HZ[i + 1]), MW(-HW[i + 1], HY[i + 1], HZ[i + 1]) };
        V3 cc = MW(0, HY[i] - 0.3f, HZ[i]);
        pf_poly(4, q, &cc, 238, 241, 246, 0, 255);
    }
}

static void draw_plane_shadow_kx1(void) {
    static const float X[10] = { -0.2f, -0.6f, -0.9f, -0.95f, -0.8f, 0.8f, 0.95f, 0.9f, 0.6f, 0.2f };
    static const float Z[10] = { 2.25f, 1.6f, 0.8f, -1.2f, -2.25f, -2.25f, -1.2f, 0.8f, 1.6f, 2.25f };
    V3 w[10];
    for (int i = 0; i < 10; i++) w[i] = shadow_pt(MW(X[i], 0.5f, Z[i]));
    draw_world_poly(w, 10, RGBA8(0, 0, 0, 85));
}

/* ------------------------------------------------------------------ */
/* Driving                                                             */
/* ------------------------------------------------------------------ */

typedef struct {
    float u, w;            // speed along the car and sideways (m/s); w > 0 is to the right
    float psi;             // heading (radians, 0 = north along +z, turning right is positive)
    float yawrate;
    float steer;           // front wheel angle (radians)
    float roll, pitch;     // body lean from cornering, braking and acceleration (radians)
    float ax, ay;          // accelerations along and across the car, smoothed (m/s^2)
    float thr;             // what the right stick is asking for: -1 (brake / reverse) .. 1 (full power)
    float power;           // the motor, -1 (regeneration) .. 1
    float batt;            // battery charge, 0..1
    float reverse_t;       // seconds the car has been held still while the stick asked for reverse
    bool  hand;            // the handbrake
    float odo;             // meters driven since the start
} KX;
static KX kx;

static void kx_reset(float heading) {
    memset(&kx, 0, sizeof(kx));
    kx.psi = heading; kx.batt = 1.0f;
    kx_steer_vis = 0.0f; kx_braking = false;
}

// One frame of driving. Constants are SI; a mile per hour is 0.447 m/s.
#define KX_WHEELBASE 2.43f
#define KX_G         9.81f
static void kx_drive(const SceCtrlData *pad) {
    const float dt = DT;
    float steer_in = shape(stick(pad->lx));
    if (gy_mode) steer_in = clampf(steer_in + gy_ail, -1.0f, 1.0f);
    float drive_in = -stick(pad->ry);                              // right stick up: power, down: brake and then reverse
    kx.hand = (pad->buttons & SCE_CTRL_CIRCLE) != 0;
    if (!sw.batt) drive_in = fminf(drive_in, 0.0f);                // no battery, no power (the brakes still work)
    kx.thr += (drive_in - kx.thr) * fminf(1.0f, 10.0f * dt);

    const float u = kx.u;
    float throttle = fmaxf(0.0f, kx.thr), brake = 0.0f, rev = 0.0f;
    if (kx.thr < -0.05f) {
        if (u > 0.5f) brake = -kx.thr;                              // moving forward: the brake
        else {                                                      // (almost) stopped: after a moment, reverse
            kx.reverse_t += dt;
            if (kx.reverse_t > 0.35f) rev = -kx.thr;
        }
    } else kx.reverse_t = 0.0f;
    if (kx.thr > 0.05f && u < -0.5f) brake = kx.thr * 0.7f;        // power while rolling backwards: brake first

    // Motor and brakes. About 240 W/kg: traction limited up to about 58 mph, then power limited, with a top speed near 160 mph.
    float ax = 0.0f;
    kx.power = 0.0f;
    if (throttle > 0.0f && u > -0.5f) {
        float amax = fminf(9.3f, 240.0f / fmaxf(u, 1.0f));
        ax += throttle * amax; kx.power = throttle;
    }
    if (rev > 0.0f) { ax -= rev * 4.5f * clampf(1.0f - (-u) / 11.0f, 0.0f, 1.0f); kx.power = -rev * 0.3f; }
    if (brake > 0.0f) { ax -= (u > 0.0f ? 1.0f : -1.0f) * brake * 11.0f; kx.power = -brake; }
    if (kx.hand) ax -= (u > 0.0f ? 1.0f : -1.0f) * 6.0f * fminf(1.0f, fabsf(u) / 2.0f);
    if (kx.power < 0.0f && u > 3.0f && brake < 0.01f) kx.power = 0.0f;
    ax -= (u > 0.0f ? 1.0f : -1.0f) * (0.12f * fminf(1.0f, fabsf(u) / 1.5f) + 0.0006f * u * u);        // rolling resistance and drag
    if (throttle < 0.02f && brake < 0.01f && u > 1.0f) ax -= 0.5f * clampf(u / 20.0f, 0.0f, 1.0f);     // lifting off: a little regeneration

    // Steering: a lot of lock when slow, a little when fast; the front tires can only give so much sideways grip.
    const float dmax = 0.52f / (1.0f + (u / 16.0f) * (u / 16.0f)) + 0.035f;
    kx.steer += (steer_in * dmax - kx.steer) * fminf(1.0f, 6.0f * dt);
    float r_want = u / KX_WHEELBASE * tanf(kx.steer);
    const float grip = 1.12f * KX_G;
    float r_lim = grip / fmaxf(fabsf(u), 1.5f);
    r_want = clampf(r_want, -r_lim, r_lim);
    kx.yawrate += (r_want - kx.yawrate) * fminf(1.0f, 7.0f * dt);

    // Rear grip: it gives way under the handbrake, and when a lot of power meets a low speed.
    float mu_r = 1.10f;
    if (kx.hand) mu_r = 0.38f;
    mu_r -= 0.35f * throttle * clampf(1.0f - fabsf(u) / 25.0f, 0.0f, 1.0f);
    float ay = -clampf(9.0f * kx.w, -mu_r * KX_G, mu_r * KX_G);
    const float du = ax + kx.w * kx.yawrate, dw = ay - u * kx.yawrate;
    kx.u += du * dt;
    kx.w += dw * dt;
    if (fabsf(kx.u) < 0.04f && throttle < 0.02f && rev < 0.02f && brake > 0.0f) kx.u = 0.0f;    // brought to a stop: it stays stopped
    kx.psi += kx.yawrate * dt;
    if (kx.psi >= 2.0f * PI_F) kx.psi -= 2.0f * PI_F;
    if (kx.psi < 0.0f)         kx.psi += 2.0f * PI_F;

    kx.ax += (ax - kx.ax) * fminf(1.0f, 6.0f * dt);
    kx.ay += ((ay + kx.u * kx.yawrate) - kx.ay) * fminf(1.0f, 6.0f * dt);
    kx.roll  += (clampf(-0.015f * kx.u * kx.yawrate, -0.16f, 0.16f) - kx.roll) * fminf(1.0f, 8.0f * dt);
    kx.pitch += (clampf(0.0055f * kx.ax, -0.07f, 0.06f) - kx.pitch) * fminf(1.0f, 8.0f * dt);
    kx_braking = brake > 0.1f || kx.hand;
    kx_steer_vis = kx.steer;
    kx.batt = clampf(kx.batt - fmaxf(0.0f, kx.power) * 0.00004f * dt * 60.0f + fmaxf(0.0f, -kx.power) * 0.000005f * dt * 60.0f, 0.0f, 1.0f);
    kx.odo += fabsf(kx.u) * dt;
}

static void kx_update(const SceCtrlData *pad, unsigned int pressed) {
    (void)pressed;
    eng_on = sw.batt; eng_n += ((sw.batt ? 1.0f : 0.0f) - eng_n) * 0.05f; sw.pump = sw.ign = true;       // (the motor: it runs when the battery is on)
    kx_drive(pad);
    const float sh = sinf(kx.psi), ch = cosf(kx.psi);
    pl.vel = v3(sh * kx.u + ch * kx.w, 0.0f, ch * kx.u - sh * kx.w);
    pl.pos.x += pl.vel.x * DT; pl.pos.z += pl.vel.z * DT; pl.pos.y = 0.0f;
    pl.on_ground = true; pl.gs = kx.u; pl.hdg_g = kx.psi; pl.gpitch = 0.0f;
    set_axes(kx.psi, kx.pitch, kx.roll);
    pl.hdg = kx.psi; pl.pitch = kx.pitch; pl.roll = kx.roll;
    pl.speed = sqrtf(kx.u * kx.u + kx.w * kx.w); pl.vs = 0.0f;
    pl.alpha = 0.0f; pl.beta = clampf(atan2f(kx.w, fmaxf(fabsf(kx.u), 3.0f)), -0.5f, 0.5f);
    pl.throttle = fmaxf(0.0f, kx.thr);
    pl.rpm = 100.0f * clampf(pl.speed / 72.0f, 0.0f, 1.0f);
    pl.fuel = 1.0f;
    pl.touched = true;
    if (sea_depth(pl.pos.x, pl.pos.z) > 0.0f) { crash_why("drove into the sea"); crash(); return; }
    if (in_lake(pl.pos.x, pl.pos.z, 1.0f)) { crash_why("drove into the lake"); crash(); return; }
    if (hit_scenery()) { crash_why("hit the scenery"); crash(); return; }
}
