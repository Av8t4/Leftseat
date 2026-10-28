// Left Seat: landing gear springs, tire friction and ground strikes.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* Landing gear: three spring-and-damper legs, so touchdown is gradual  */
/* ------------------------------------------------------------------ */

// Wheel-bottom positions in model space (x right, y up from the compressed tire, z forward).
// The negative heights are the extra leg length that compresses under weight, so the aircraft
// sits at about pos.y = 0 when parked.
// (The legs, springs and inertias belong to the aircraft: see aircraft.inc.)
static float gear_pen[3];                        // how far each leg is compressed (m)

static char last_crash[48];
static void crash_why(const char *why) {
    snprintf(last_crash, sizeof(last_crash), "%s", why);
    if (logfile) { fprintf(logfile, "crash: %s\n", why); fflush(logfile); }
}

static V3 body_pt(float x, float y, float z) {          // model space -> world (same as the 3D model)
    V3 o = v3(pl.pos.x, pl.pos.y + spec->piv_y, pl.pos.z);
    o = vadd(o, vscale(pl.r, x));
    o = vadd(o, vscale(pl.u, y - spec->piv_y));
    return vadd(o, vscale(pl.f, z));
}

// Adds the gear's push, pitch/roll moments and tire friction. Returns the deepest compression.
static float gear_forces(bool braking, float *dq, float *dp, float *dr, V3 *acc) {
    V3 piv = v3(pl.pos.x, pl.pos.y + spec->piv_y, pl.pos.z);
    V3 om = vadd(vadd(vscale(pl.r, -pl.pitchrate), vscale(pl.f, -pl.rollrate)), vscale(pl.u, pl.yawrate));
    float nmain = 0.0f, maxpen = 0.0f;
    for (int i = 0; i < 3; i++) {
        const GearLeg *g = &GEAR[i];
        if (spec->retract && pl.gear < 0.95f) { gear_pen[i] = 0.0f; continue; }          // gear not down and locked: no wheels on the ground
        V3 P = body_pt(g->x, g->y, g->z);
        float pen = -P.y;
        gear_pen[i] = pen > 0.0f ? pen : 0.0f;
        if (pen <= 0.0f) continue;
        if (pen > maxpen) maxpen = pen;
        V3 rl = vsub(P, piv);
        V3 pv = vadd(pl.vel, vcross(om, rl));                       // velocity of the wheel
        float F = g->ks * pen * (1.0f + 12.0f * pen * pen) + g->cs * (-pv.y);   // stiffens as it bottoms out
        F = clampf(F, 0.0f, 8.0f * G_ACC);
        acc->y += F;
        if (i < 2) nmain += F;
        V3 tau = vcross(rl, v3(0.0f, F, 0.0f));
        *dq += -vdot(tau, pl.r) / IYY_M;                            // nose-up positive
        *dp += -vdot(tau, pl.f) / IXX_M;                            // roll-right positive
        *dr +=  vdot(tau, pl.u) / IZZ_M;                            // nose-right positive
    }
    if (nmain > 0.0f) {                                             // tires: rolling drag, brakes, side grip
        V3 hd = v3(pl.f.x, 0.0f, pl.f.z);
        float hl = sqrtf(vdot(hd, hd));
        hd = hl > 1e-3f ? vscale(hd, 1.0f / hl) : v3(0.0f, 0.0f, 1.0f);
        V3 sd = v3(hd.z, 0.0f, -hd.x);
        float vl = vdot(pl.vel, hd), vsd = vdot(pl.vel, sd);
        float dec = (0.03f + (braking ? 0.40f : 0.0f)) * nmain;
        float lim = fabsf(vl) / DT;
        if (dec > lim) dec = lim;
        *acc = vsub(*acc, vscale(hd, vl >= 0.0f ? dec : -dec));
        *acc = vsub(*acc, vscale(sd, clampf(vsd * 15.0f, -0.8f * nmain, 0.8f * nmain)));
        *dr -= 3.0f * pl.yawrate * fminf(nmain / G_ACC, 1.0f);
    }
    return maxpen;
}

// Wingtip, tail or propeller touching the ground.
static const char *ground_strike(void) {
    V3 lt = body_pt(-spec->tip_x, spec->tip_y, 0.0f), rt = body_pt(spec->tip_x, spec->tip_y, 0.0f);
    if (lt.y < 0.05f || rt.y < 0.05f) return "wingtip strike";
    if (body_pt(0.0f, spec->tail_y, spec->tail_z).y < -0.03f) return "tail strike";
    if (body_pt(0.0f, spec->prop_y, spec->prop_z).y < -0.02f) return "propeller strike";
    return NULL;
}

