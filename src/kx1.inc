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
            else if (e == 3 || e == 4 || e == 5) c = G;                   // the dark lower body (the same on both sides)
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
    for (int s = -1; s <= 1; s += 2) {                                                                            // headlights: set into the nose
        const float xa = s * 0.20f, xb = s * 0.42f;
        if (lt_head()) kx_lamp(fminf(xa, xb), fmaxf(xa, xb), 0.20f, 0.30f, 1.58f, 1.70f, 255, 246, 200);
        else           kx_box (fminf(xa, xb), fmaxf(xa, xb), 0.20f, 0.30f, 1.58f, 1.70f, 190, 192, 184);
    }
    const bool tl = lt_head() || kx_braking;                                                                       // tail lights: dim, bright on the brake
    for (int s = -1; s <= 1; s += 2) {
        const float xa = s * 0.20f, xb = s * 0.44f, x0 = fminf(xa, xb), x1 = fmaxf(xa, xb);
        if (tl) kx_lamp(x0, x1, 0.32f, 0.40f, -2.225f, -2.19f, kx_braking ? 255 : 190, 20, 20);
        else    kx_box (x0, x1, 0.32f, 0.40f, -2.225f, -2.19f, 90, 20, 20);
    }
    kx_box(-0.55f, 0.55f, 0.05f, 0.09f, 2.12f, 2.28f, KC_GRAPH);                                                  // splitter
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
    float gas;             // cold gas left in the tank, 0..100
    bool  air;             // off the ground: the free-flight model below is running
    float hiss;            // 0..1, how hard the thrusters are blowing (for the sound)
    float rx, ry, rz;      // airborne spin: pitch (nose up +), roll (right wing down +), yaw (nose right +) in rad/s
    float jet_hop, jet_roof, jet_fwd;   // 0..1 jets in use this frame
    float cp, cr, cy;      // attitude commands this frame, -1..1
    float air_t;           // seconds since leaving the ground
} KX;
static KX kx;

static int kx_npuff;                                  // (the thruster puffs: see below)
static void kx_reset(float heading) {
    memset(&kx, 0, sizeof(kx));
    kx.psi = heading; kx.batt = 1.0f; kx.gas = 100.0f;
    kx_steer_vis = 0.0f; kx_braking = false;
    kx_npuff = 0;
}

// One frame of driving. Constants are SI; a mile per hour is 0.447 m/s.
#define KX_BOOST_USE 2.5f       // gas units a second from the forward jets on the ground
#define KX_WHEELBASE 2.43f
#define KX_G         9.81f
static void kx_drive(const SceCtrlData *pad) {
    const float dt = DT;
    float steer_in = shape(stick(pad->rx));                              // right stick: steering
    if (gy_mode) steer_in = clampf(steer_in + gy_ail, -1.0f, 1.0f);
    float drive_in = -stick(pad->ly);                              // left stick up: power, down: brake and then reverse (the other way: brake first, then power)
    kx.hand = (pad->buttons & SCE_CTRL_RTRIGGER) != 0;                  // R: the handbrake
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
    // The forward jets (Square, thrusters armed): a big push on top of the motor, for as long as the gas lasts.
    kx.jet_fwd = 0.0f;
    if (sw.arm && sw.batt && kx.gas > 0.0f && (pad->buttons & SCE_CTRL_SQUARE) && u > -0.5f) {
        ax = fmaxf(ax, fminf(37.0f, 700.0f / fmaxf(u, 1.0f)));
        kx.power = 1.0f; kx.jet_fwd = 1.0f;
        kx.gas = fmaxf(0.0f, kx.gas - KX_BOOST_USE * dt);
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
    kx.roll  += (clampf(-0.004f * kx.u * kx.yawrate, -0.04f, 0.04f) - kx.roll) * fminf(1.0f, 8.0f * dt);
    kx.pitch += (clampf(0.0035f * kx.ax, -0.04f, 0.035f) - kx.pitch) * fminf(1.0f, 8.0f * dt);
    kx_braking = brake > 0.1f || kx.hand;
    kx_steer_vis = kx.steer;
    kx.batt = clampf(kx.batt - fmaxf(0.0f, kx.power) * 0.00004f * dt * 60.0f + fmaxf(0.0f, -kx.power) * 0.000005f * dt * 60.0f, 0.0f, 1.0f);
    kx.odo += fabsf(kx.u) * dt;
}

/* ------------------------------------------------------------------ */
/* Cold-gas thrusters and free flight                                  */
/* ------------------------------------------------------------------ */

#define KX_HOP_ACC   17.0f      // m/s^2 along the roof-to-floor axis from the four hop jets: about 1.7 g, so a level car climbs at 0.7 g
#define KX_FWD_ACC   24.0f      // m/s^2 along the nose from the forward jets in the air
#define KX_FWD_USE   4.0f
#define KX_ROOF_ACC  10.0f      // m/s^2 pushing the car toward its floor
#define KX_RCS_ACC   4.2f       // rad/s^2 from the attitude jets
#define KX_HOP_USE   7.0f       // gas units a second (the tank holds 100: about 14 s of hop)
#define KX_ROOF_USE  8.0f
#define KX_RCS_USE   1.6f       // per axis at full stick

typedef struct { V3 p; V3 v; float age, life, size; } KXPuff;
static KXPuff kx_puffs[96];

static unsigned int kx_rng = 12345u;
static float kx_rnd(void) { kx_rng = kx_rng * 1664525u + 1013904223u; return ((kx_rng >> 9) & 0x7fff) / 16384.0f - 1.0f; }

// A jet at a point on the car (model space) blowing along `dir` (model space).
static void kx_emit(float x, float y, float z, float dx, float dy, float dz, float speed, float size) {
    if (kx_npuff >= 96) return;
    KXPuff *q = &kx_puffs[kx_npuff++];
    q->p = MW(x, y, z);
    V3 d = vadd(vadd(vscale(pl.r, dx), vscale(pl.u, dy)), vscale(pl.f, dz));
    q->v = vadd(pl.vel, vadd(vscale(d, speed), v3(kx_rnd() * 0.8f, kx_rnd() * 0.8f, kx_rnd() * 0.8f)));
    q->age = 0.0f; q->life = 0.35f + 0.15f * fabsf(kx_rnd()); q->size = size;
}

static void kx_puffs_step(void) {
    int n = 0;
    for (int i = 0; i < kx_npuff; i++) {
        KXPuff q = kx_puffs[i];
        q.age += DT;
        if (q.age >= q.life) continue;
        q.p = vadd(q.p, vscale(q.v, DT));
        q.v = vscale(q.v, 1.0f - 2.2f * DT);
        kx_puffs[n++] = q;
    }
    kx_npuff = n;
}

static void kx_draw_puffs(void) {
    for (int i = 0; i < kx_npuff; i++) {
        const KXPuff *q = &kx_puffs[i];
        float t = q->age / q->life;
        float rad = q->size * (0.5f + 1.6f * t);
        int a = (int)(150.0f * (1.0f - t));
        if (a < 4) continue;
        V3 c = q->p, pts[6];
        for (int k = 0; k < 6; k++) {
            float ang = k * (PI_F / 3.0f);
            pts[k] = vadd(c, vadd(vscale(cam.r, cosf(ang) * rad), vscale(cam.u, sinf(ang) * rad)));
        }
        draw_world_poly(pts, 6, RGBA8(235, 242, 255, a));
    }
}

// The points on the car that can touch the ground (model space, y up from the wheel bottoms).
typedef struct { float x, y, z; bool wheel; } KXPt;
static const KXPt KXP[] = {
    { -0.80f, 0.00f, -1.32f, true }, { 0.80f, 0.00f, -1.32f, true }, { -0.80f, 0.00f, 1.11f, true }, { 0.80f, 0.00f, 1.11f, true },
    {  0.00f, 0.16f,  2.18f, false }, { 0.00f, 0.26f, -2.18f, false }, { -0.88f, 0.30f, 0.00f, false }, { 0.88f, 0.30f, 0.00f, false },
    {  0.00f, 1.22f,  0.10f, false }, { 0.00f, 1.05f,  1.00f, false }, { 0.00f, 1.05f, -1.00f, false },
    { -0.85f, 0.70f,  1.40f, false }, { 0.85f, 0.70f, 1.40f, false }, { -0.85f, 0.70f, -1.40f, false }, { 0.85f, 0.70f, -1.40f, false },
};
#define KX_NPT ((int)(sizeof(KXP) / sizeof(KXP[0])))

// Leaves the ground: the speed and heading carry over as a free body.
static void kx_lift_off(void) {
    kx.air = true; kx.air_t = 0.0f;
    kx.rx = kx.ry = kx.rz = 0.0f;
    pl.pos.y = 0.03f;
}

// Touches down. Returns false (after setting the crash reason) when the landing is not survivable.
static bool kx_touch_down(float lowest_wheel, float lowest_body) {
    const float sink = -pl.vel.y;
    const float sh = sinf(kx.psi), ch = cosf(kx.psi);
    V3 hf = vnorm(v3(pl.f.x, 0.0f, pl.f.z));
    kx.psi = atan2f(hf.x, hf.z); if (kx.psi < 0.0f) kx.psi += 2.0f * PI_F;
    const float sh2 = sinf(kx.psi), ch2 = cosf(kx.psi); (void)sh; (void)ch;
    const float u = pl.vel.x * sh2 + pl.vel.z * ch2, w = pl.vel.x * ch2 - pl.vel.z * sh2;
    if (lowest_body < -0.04f && lowest_body < lowest_wheel - 0.0f) { crash_why(pl.u.y < 0.0f ? "landed upside down" : "hit the ground with the body"); return false; }
    if (pl.u.y < 0.0f)   { crash_why("landed upside down"); return false; }
    if (pl.u.y < 0.857f) { crash_why("landed on its side"); return false; }
    if (sink > 7.0f)     { crash_why("landed too hard"); return false; }
    if (fabsf(w) > 9.0f) { crash_why("landed sideways"); return false; }
    pl.pos.y -= lowest_wheel; if (pl.pos.y < 0.0f) pl.pos.y = 0.0f;
    kx.u = u; kx.w = w; kx.yawrate = kx.rz * 0.0f;
    kx.pitch = asinf(clampf(pl.f.y, -1.0f, 1.0f));
    kx.roll  = asinf(clampf(-pl.r.y / fmaxf(0.3f, cosf(kx.pitch)), -1.0f, 1.0f));
    kx.steer = 0.0f; kx.thr = 0.0f;
    kx.air = false; kx.rx = kx.ry = kx.rz = 0.0f;
    kx.ax = kx.ay = 0.0f;
    pl.pos.y = 0.0f; pl.vel.y = 0.0f;
    return true;
}

static void kx_fly(const SceCtrlData *pad) {
    const float dt = DT;
    kx.air_t += dt;
    const bool arm = sw.arm && sw.batt;
    float hop = 0.0f, roof = 0.0f, fwd = 0.0f, cp = 0.0f, cr = 0.0f, cy = 0.0f;
    if (arm && kx.gas > 0.0f) {
        if (pad->buttons & SCE_CTRL_CROSS)  hop  = 1.0f;
        if (pad->buttons & SCE_CTRL_CIRCLE) roof = 1.0f;                 // O: the roof jets
        if (pad->buttons & SCE_CTRL_SQUARE) fwd  = 1.0f;                 // Square: the forward jets
        cp = shape(stick(pad->ly)); cr = shape(stick(pad->lx)); cy = shape(stick(pad->rx));
        if (gy_mode) cr = clampf(cr + gy_ail, -1.0f, 1.0f);
    }
    float use = hop * KX_HOP_USE + roof * KX_ROOF_USE + fwd * KX_FWD_USE + (fabsf(cp) + fabsf(cr) + fabsf(cy)) * KX_RCS_USE;
    if (use * dt > kx.gas) { float k = kx.gas / (use * dt); hop *= k; roof *= k; fwd *= k; cp *= k; cr *= k; cy *= k; use = kx.gas / dt; }
    kx.gas = fmaxf(0.0f, kx.gas - use * dt);
    kx.jet_hop = hop; kx.jet_roof = roof; kx.jet_fwd = fwd; kx.cp = cp; kx.cr = cr; kx.cy = cy;
    kx.hiss += ((use > 0.01f ? clampf(0.35f + use / 18.0f, 0.0f, 1.0f) : 0.0f) - kx.hiss) * fminf(1.0f, 14.0f * dt);

    // Spin: the jets change it, nothing else does. Leave it alone and it keeps turning.
    kx.rx += cp * KX_RCS_ACC * dt; kx.ry += cr * KX_RCS_ACC * dt; kx.rz += cy * KX_RCS_ACC * dt;
    kx.rx = clampf(kx.rx, -6.0f, 6.0f); kx.ry = clampf(kx.ry, -7.0f, 7.0f); kx.rz = clampf(kx.rz, -4.0f, 4.0f);
    rot_pitch(kx.rx * dt); rot_roll(kx.ry * dt); rot_yaw(kx.rz * dt);
    orthonormalize();

    // Forces: gravity, drag, and the jets along the body's own up axis.
    V3 a = v3(0.0f, -KX_G, 0.0f);
    a = vadd(a, vscale(pl.u, hop * KX_HOP_ACC - roof * KX_ROOF_ACC));
    a = vadd(a, vscale(pl.f, fwd * KX_FWD_ACC));
    float sp = sqrtf(vdot(pl.vel, pl.vel));
    a = vsub(a, vscale(pl.vel, 0.0006f * sp));
    pl.vel = vadd(pl.vel, vscale(a, dt));
    pl.pos = vadd(pl.pos, vscale(pl.vel, dt));

    // Jets: puffs for the eye.
    if (hop > 0.0f) {
        const float jx[4] = { -0.62f, 0.62f, -0.62f, 0.62f }, jz[4] = { -1.10f, -1.10f, 0.95f, 0.95f };
        for (int i = 0; i < 4; i++) kx_emit(jx[i], 0.12f, jz[i], 0.0f, -1.0f, 0.0f, 14.0f, 0.20f);
    }
    if (fwd > 0.0f) { kx_emit(-0.30f, 0.45f, -2.15f, 0.0f, 0.0f, -1.0f, 16.0f, 0.20f); kx_emit(0.30f, 0.45f, -2.15f, 0.0f, 0.0f, -1.0f, 16.0f, 0.20f); }
    if (roof > 0.0f) kx_emit(0.0f, 1.20f, 0.10f, 0.0f, 1.0f, 0.0f, 12.0f, 0.22f);
    if (fabsf(cp) > 0.2f) {
        float sg = cp > 0.0f ? 1.0f : -1.0f;
        kx_emit(0.0f, 0.55f, 2.10f, 0.0f, -sg, 0.0f, 9.0f, 0.10f);
        kx_emit(0.0f, 0.55f, -2.10f, 0.0f,  sg, 0.0f, 9.0f, 0.10f);
    }
    if (fabsf(cr) > 0.2f) {
        float sg = cr > 0.0f ? 1.0f : -1.0f;
        kx_emit(0.92f, 0.60f, 0.0f, 0.0f,  sg, 0.0f, 9.0f, 0.10f);
        kx_emit(-0.92f, 0.60f, 0.0f, 0.0f, -sg, 0.0f, 9.0f, 0.10f);
    }
    if (fabsf(cy) > 0.2f) {
        float sg = cy > 0.0f ? 1.0f : -1.0f;
        kx_emit(0.0f, 0.55f, 2.10f, -sg, 0.0f, 0.0f, 9.0f, 0.10f);
        kx_emit(0.0f, 0.55f, -2.10f, sg, 0.0f, 0.0f, 9.0f, 0.10f);
    }

    // Contact.
    float lw = 1e9f, lb = 1e9f;
    for (int i = 0; i < KX_NPT; i++) {
        V3 P = body_pt(KXP[i].x, KXP[i].y, KXP[i].z);
        if (KXP[i].wheel) { if (P.y < lw) lw = P.y; } else if (P.y < lb) lb = P.y;
    }
    if (lw < 0.0f || lb < 0.0f) {
        if (!kx_touch_down(lw, lb)) { crash(); return; }
    }
}

static void kx_update(const SceCtrlData *pad, unsigned int pressed) {
    (void)pressed;
    eng_on = sw.batt; eng_n += ((sw.batt ? 1.0f : 0.0f) - eng_n) * 0.05f;       // (the motor: it runs when the battery is on)
    kx.jet_hop = kx.jet_roof = 0.0f; kx.cp = kx.cr = kx.cy = 0.0f;
    if (!kx.air) {
        kx.hiss += ((kx.jet_fwd > 0.0f ? 0.75f : 0.0f) - kx.hiss) * fminf(1.0f, 10.0f * DT);
        // A parked car takes on gas, slowly.
        if (fabsf(kx.u) < 1.0f && fabsf(kx.w) < 1.0f && kx.gas < 100.0f) kx.gas = fminf(100.0f, kx.gas + 4.0f * DT);
        if (sw.arm && sw.batt && kx.gas > 0.5f && (pad->buttons & SCE_CTRL_CROSS)) {
            const float sh = sinf(kx.psi), ch = cosf(kx.psi);
            pl.vel = v3(sh * kx.u + ch * kx.w, 0.0f, ch * kx.u - sh * kx.w);
            set_axes(kx.psi, kx.pitch, kx.roll);
            kx_lift_off();
        }
    }
    if (kx.air) {
        kx_fly(pad);
        kx_puffs_step();
        if (pl.crashed) return;
        if (kx.air) {
            pl.on_ground = false; pl.gs = 0.0f; pl.touched = true;
            pl.hdg = atan2f(pl.f.x, pl.f.z); pl.pitch = asinf(clampf(pl.f.y, -1.0f, 1.0f)); pl.roll = asinf(clampf(-pl.r.y, -1.0f, 1.0f));
            pl.speed = sqrtf(vdot(pl.vel, pl.vel)); pl.vs = pl.vel.y;
            pl.alpha = 0.0f; pl.beta = 0.0f; pl.throttle = kx.jet_hop; pl.rpm = 100.0f * clampf(pl.speed / 72.0f, 0.0f, 1.0f);
            pl.fuel = kx.gas / 100.0f;
            kx_steer_vis += (0.0f - kx_steer_vis) * 0.1f; kx_braking = false;
            if (pl.pos.y < 1.0f && sea_depth(pl.pos.x, pl.pos.z) > 0.0f) { crash_why("landed in the sea"); crash(); return; }
            if (pl.pos.y < 1.0f && in_lake(pl.pos.x, pl.pos.z, 1.0f)) { crash_why("landed in the lake"); crash(); return; }
            if (hit_scenery()) { crash_why("hit the scenery"); crash(); return; }
            return;
        }
    }
    kx_puffs_step();
    kx_drive(pad);
    if (kx.jet_fwd > 0.0f) { kx_emit(-0.30f, 0.45f, -2.15f, 0.0f, 0.0f, -1.0f, 16.0f, 0.20f); kx_emit(0.30f, 0.45f, -2.15f, 0.0f, 0.0f, -1.0f, 16.0f, 0.20f); }
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
    pl.fuel = kx.gas / 100.0f;
    pl.touched = true;
    if (sea_depth(pl.pos.x, pl.pos.z) > 0.0f) { crash_why("drove into the sea"); crash(); return; }
    if (in_lake(pl.pos.x, pl.pos.z, 1.0f)) { crash_why("drove into the lake"); crash(); return; }
    if (hit_scenery()) { crash_why("hit the scenery"); crash(); return; }
}
