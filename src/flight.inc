// Left Seat: aircraft state, controls shaping, scenery collisions and the crash handler.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* flight model                                                        */
/* ------------------------------------------------------------------ */

// Build body axes from heading / pitch / roll (used on the ground and at reset).
static void set_axes(float hdg, float pitch, float roll) {
    float sh = sinf(hdg),   ch = cosf(hdg);
    float sp = sinf(pitch), cp = cosf(pitch);
    float sr = sinf(roll),  cr = cosf(roll);
    V3 f  = v3(sh * cp, sp, ch * cp);
    V3 r0 = v3(ch, 0.0f, -sh);
    V3 u0 = v3(-sh * sp, cp, -ch * sp);
    pl.f = f;
    pl.r = v3(r0.x * cr - u0.x * sr, r0.y * cr - u0.y * sr, r0.z * cr - u0.z * sr);
    pl.u = v3(u0.x * cr + r0.x * sr, u0.y * cr + r0.y * sr, u0.z * cr + r0.z * sr);
}

// Small rotations about the body axes (nose up / right wing down / nose right).
static void rot_pitch(float a) {
    float c = cosf(a), sn = sinf(a); V3 f = pl.f, u = pl.u;
    pl.f = vadd(vscale(f, c), vscale(u, sn));
    pl.u = vsub(vscale(u, c), vscale(f, sn));
}
static void rot_roll(float a) {
    float c = cosf(a), sn = sinf(a); V3 r = pl.r, u = pl.u;
    pl.r = vsub(vscale(r, c), vscale(u, sn));
    pl.u = vadd(vscale(u, c), vscale(r, sn));
}
static void rot_yaw(float a) {
    float c = cosf(a), sn = sinf(a); V3 f = pl.f, r = pl.r;
    pl.f = vadd(vscale(f, c), vscale(r, sn));
    pl.r = vsub(vscale(r, c), vscale(f, sn));
}
static void orthonormalize(void) {
    pl.f = vnorm(pl.f);
    pl.r = vnorm(vsub(pl.r, vscale(pl.f, vdot(pl.r, pl.f))));
    pl.u = vcross(pl.f, pl.r);
}

// Fixed-pitch propeller: thrust is highest standing still and fades with airspeed.
static float thrust_accel(float throttle, float V) {
    float f = clampf(1.0f - V / THRUST_V0, 0.0f, 1.0f);
    if (spec->thr_exp == 1.0f && spec->thr_idle == 0.0f) return TSTAT * throttle * f;       // propeller aircraft
    bool running = (pl.fuel > 0.0f && !engine_failed);                                       // a jet: nonlinear, with some thrust at idle
    float t = spec->thr_idle * (running ? 1.0f : 0.0f) + (1.0f - spec->thr_idle) * powf(throttle, spec->thr_exp);
    return TSTAT * t * f;
}

// The throttle setting that gives a wanted thrust acceleration at an airspeed (used to start a challenge in trim).
static float throttle_for_thrust(float acc, float V) {
    if (spec->thr_exp == 1.0f && spec->thr_idle == 0.0f) return clampf(acc / (TSTAT * (1.0f - V / THRUST_V0)), 0.05f, 1.0f);
    float f = clampf(1.0f - V / THRUST_V0, 0.05f, 1.0f);
    float t = clampf((acc / (TSTAT * f) - spec->thr_idle) / (1.0f - spec->thr_idle), 0.0f, 1.0f);
    return clampf(powf(t, 1.0f / spec->thr_exp), 0.05f, 1.0f);
}

static float lift_coeff(float alpha, int flaps) {
    float fl = FLAP_CL[flaps < 0 ? 0 : (flaps > 3 ? 3 : flaps)];
    if (alpha > ALPHA_S) {                       // past the stall the lift falls away gradually
        float cl = CL_MAX + fl - (alpha - ALPHA_S) * 4.0f;
        return cl < 0.45f ? 0.45f : cl;
    }
    float cl = CLA * (alpha - ALPHA0) + fl;
    return cl < -0.9f ? -0.9f : cl;
}

static int start_ap = AP_HOME;   // open-world start airport (also where Triangle resets to)

static void reset_plane_at(int ap) {
    const Airport *a = &AIRPORTS[ap];
    float back = a->hl - (ap == AP_HOME ? 100.0f : 60.0f);       // a little in from the threshold
    memset(&pl, 0, sizeof(pl));
    pl.pos = ap_pt(a, 0.0f, 0.0f, -back);     // threshold of the runway, facing along it
    pl.hdg = pl.hdg_g = a->hdg;                 // (hdg_g is the heading the ground roll steers from)
    pl.on_ground = true;
    pl.fuel = FUEL_FULL;
    pl.gear = 1.0f; pl.gear_cmd = true;               // (always starts with the gear down)
    engine_failed = false;
    ntrk = 0; trk_i = 0; trk_t = 0;
    pl.rpm = spec->rpm_idle;
    pl.touched = true;
    set_axes(a->hdg, 0, 0);
    chase_dir = pl.f;
    look_ang = 0.0f;
}
static void reset_plane(void)  { reset_plane_at(AP_HOME); }       // challenges always start at home
static void reset_free(void)   { reset_plane_at(start_ap); }      // open world: the airport picked on the menu

static float stick(unsigned char v) {
    float f = ((int)v - 128) / 128.0f, a = fabsf(f);
    if (a < 0.12f) return 0.0f;                          // dead zone, with the response starting from zero at its edge
    a = (a - 0.12f) / 0.88f;                             // (so there is no sudden step as the stick leaves the center)
    return f < 0.0f ? -a : a;
}

// Response curve: gentle near center for fine control, full range at the ends.
static float shape(float x) { return 0.4f * x + 0.6f * x * fabsf(x); }

static bool hit_scenery(void) {
    for (int i = 0; i < nboxes; i++) {
        const Box *b = &boxes[i];
        if (fabsf(pl.pos.x - b->x) < b->w * 0.5f &&
            fabsf(pl.pos.z - b->z) < b->d * 0.5f &&
            pl.pos.y < b->h) return true;
    }
    for (int i = 0; i < nmounts; i++) {                                      // (a surface of zero is just flat ground: the aircraft's own ground contact deals with that)
        float h = mountain_surface(&mounts[i], pl.pos.x, pl.pos.z);
        if (h > 0.0f && pl.pos.y < h) return true;
    }
    for (int i = 0; i < nmesas; i++) {
        float h = mesa_surface(&mesas[i], pl.pos.x, pl.pos.z);
        if (h > 0.0f && pl.pos.y < h) return true;
    }
    return false;
}

static void crash(void) {
    pl.crashed = true;
    pl.rpm = 0.0f;
    pl.speed = 0;
    pl.vs = 0;
    pl.vel = v3(0, 0, 0);
}

