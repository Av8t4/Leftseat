// Left Seat: the per-frame flight update (inputs, aerodynamics, ground roll).
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

static int  ff_level = 0;                           // fast forward: 0 = normal, 1 = x2, 2 = x5
static bool ff_sub   = false;                       // true while running the extra steps of a fast-forwarded frame
static const int FF_MULT[3] = { 1, 2, 5 };

static int         gear_msg_t = 0;                  // a short message about the gear (top right)
static const char *gear_msg   = "";
static void gear_say(const char *m) { gear_msg = m; gear_msg_t = 150; }

static void update(const SceCtrlData *pad, unsigned int pressed) {
    if (gear_msg_t > 0 && !ff_sub) gear_msg_t--;
    if (!ff_sub) {                                                  // Select: a tap changes the camera, holding it shows the chart
        static int  sel_hold;
        static bool sel_used;
        bool sel = (pad->buttons & SCE_CTRL_SELECT) != 0;
        if (pressed & SCE_CTRL_SELECT) {
            sel_hold = 0; sel_used = false;
            if ((pad->buttons & SCE_CTRL_LTRIGGER) && (pad->buttons & SCE_CTRL_RTRIGGER)) {
                invert_pitch = !invert_pitch;                  // L + R + Select
                sel_used = true;
            }
        }
        if (sel) {
            sel_hold++;
            chart_open = (sel_hold >= 18 && !sel_used);
        } else {
            if (sel_hold > 0 && sel_hold < 18 && !sel_used) {
                view_mode = (view_mode + 1) % (ENABLE_TOWER_VIEW ? 3 : 2);   // cockpit -> chase (-> tower)
                view_banner = 150;
                chase_dir = pl.f;
            }
            sel_hold = 0;
            chart_open = false;
        }
    }
    if (pressed & SCE_CTRL_TRIANGLE) { if (game_mode == MODE_LANDING) challenge_begin(); else reset_free(); return; }
    wx_update();                                       // the weather keeps changing, even after a crash
    if (!pl.crashed && ++trk_t >= 90) {                // a breadcrumb every second and a half, for the chart
        trk_t = 0;
        trk_x[trk_i % 300] = pl.pos.x; trk_z[trk_i % 300] = pl.pos.z;
        trk_i++;
        if (ntrk < 300) ntrk++;
    }
    {                                                  // L / R: look left / right, both together: look behind
        static float look_side = 1.0f;
        bool lb = (pad->buttons & SCE_CTRL_LTRIGGER) != 0, rb = (pad->buttons & SCE_CTRL_RTRIGGER) != 0;
        float lt = 0.0f;
        if (lb && rb)  lt = (view_mode == 1) ? PI_F * look_side : 0.0f;       // chase view: swing round to the front; cockpit: stay ahead
        else if (lb)   { lt = -0.5f * PI_F; look_side = -1.0f; }
        else if (rb)   { lt =  0.5f * PI_F; look_side =  1.0f; }
        look_ang += (lt - look_ang) * 0.22f;
    }
    if (pl.crashed) return;

    if (spec->retract) {                                // Square: landing gear up / down
        if (pressed & SCE_CTRL_SQUARE) {
            if (pl.on_ground)                         gear_say("GEAR LOCKED ON THE GROUND");
            else if (pl.speed > spec->gear_vmax)      gear_say("TOO FAST FOR THE GEAR");
            else                                      pl.gear_cmd = !pl.gear_cmd;
        }
        float gt = DT / spec->gear_time;
        if (pl.gear_cmd) pl.gear = fminf(1.0f, pl.gear + gt);
        else             pl.gear = fmaxf(0.0f, pl.gear - gt);
    }
    if (pressed & SCE_CTRL_CIRCLE) { if (pl.flaps < 3) pl.flaps++; }
    if (pressed & SCE_CTRL_CROSS)  { if (pl.flaps > 0) pl.flaps--; }
    if (pad->buttons & SCE_CTRL_LEFT)  pl.trim -= 0.25f * DT;
    if (pad->buttons & SCE_CTRL_RIGHT) pl.trim += 0.25f * DT;
    pl.trim = clampf(pl.trim, -0.5f, 0.5f);
    float ry = stick(pad->ry);                              // right stick up/down: throttle
    if (fabsf(ry) > 0.2f) pl.throttle -= ry * 0.6f * DT;
    pl.throttle = clampf(pl.throttle, 0.0f, 1.0f);

    // Pilot inputs: response curve, then control surfaces move smoothly.
    float ail_in = shape(stick(pad->lx));
    float ele_in = shape(stick(pad->ly));          // stick back (down) = nose up
    if (invert_pitch) ele_in = -ele_in;
    float rud_in = clampf(shape(stick(pad->rx)), -1.0f, 1.0f);        // right stick left/right: rudder pedals
    pl.ail += (ail_in - pl.ail) * fminf(1.0f, 8.0f * DT);
    pl.ele += (ele_in - pl.ele) * fminf(1.0f, 8.0f * DT);
    pl.rud += (rud_in - pl.rud) * fminf(1.0f, 4.0f * DT);
    float elev = clampf(pl.ele + pl.trim, -1.0f, 1.0f);

    // Fuel: the engine quits when the tanks run dry; a lighter plane flies a little better.
    float mf  = WEIGHT_FULL / (WEIGHT_FULL - spec->lb_per_gal * (FUEL_FULL - pl.fuel));
    float thr = (pl.fuel > 0.0f && !engine_failed) ? pl.throttle : 0.0f;
    if (spec->spool_up > 0.0f) {                       // a jet engine answers the throttle with a delay
        float tau = thr > pl.n1 ? spec->spool_up : spec->spool_down;
        pl.n1 += (thr - pl.n1) * (1.0f - expf(-DT / tau));
        thr = pl.n1;
    }
    pl.fuel = fmaxf(0.0f, pl.fuel - (spec->burn_idle + spec->burn_full * thr) / 3600.0f * DT);

    V3 wv = wx_wind_at(pl.pos);                        // moving air mass

    if (!pl.on_ground) {
        // ---------------- airborne: forces + body rotation ----------------
        V3 v = vsub(pl.vel, wv);                        // velocity through the air
        float V = sqrtf(vdot(v, v));
        float vF = vdot(v, pl.f), vR = vdot(v, pl.r), vU = vdot(v, pl.u);
        float alpha = 0.0f, beta = 0.0f;
        if (V > 3.0f) {
            alpha = atan2f(-vU, vF);                       // nose above the airflow
            beta  = asinf(clampf(vR / V, -1.0f, 1.0f));    // airflow from the left/right
        }
        pl.alpha = alpha; pl.beta = beta;
        float eff = clampf(V * V / (VREF * VREF), 0.0f, 1.6f);   // control authority ~ airspeed^2

        // Angular accelerations: control moment, static stability, damping, coupling.
        // Phugoid damper: lean the trimmed angle of attack with the airspeed trend, so a
        // speed gained is traded for height smoothly and slow speed/pitch swings die out.
        if (pl.gamma_prev <= 0.0f) pl.gamma_prev = V;
        float vrate = (V - pl.gamma_prev) / DT;
        pl.gamma_prev = V;
        pl.gamma_rate += (vrate - pl.gamma_rate) * 0.05f;             // filtered m/s^2
        float alpha_trim = ALPHA_TRIM + PHUGOID_DAMP * clampf(pl.gamma_rate, -4.0f, 4.0f);
        float stalled = clampf((alpha - ALPHA_S) / 0.05f, 0.0f, 1.0f);   // 0 flying .. 1 fully stalled
        float qd = PITCH_MD * elev * eff - PITCH_MA * (alpha - alpha_trim) * eff - PITCH_MQ * pl.pitchrate;
        if (alpha > ALPHA_S) qd -= 10.0f * (alpha - ALPHA_S);            // gentle nose-drop at the stall
        float pd = spec->roll_pow * pl.ail * eff - spec->roll_damp * pl.rollrate
                   - 4.0f * beta * eff * (1.0f - stalled)                // dihedral effect
                   + 4.0f * beta * stalled                               // wing drop if stalled while slipping
                   + 0.6f * pl.yawrate                                   // yaw rolls the plane
                   - 0.08f * sinf(pl.roll) * eff;                        // gentle wing-levelling (stable spiral)
        float rd = spec->yaw_pow * pl.rud * eff - spec->yaw_damp * pl.yawrate
                   + spec->weathercock * beta * eff                                   // weathercock stability
                   - 0.3f * pl.ail * eff                                 // adverse yaw
                   - spec->pfactor * thr * clampf(1.0f - eff, 0.0f, 1.0f);       // engine torque / P-factor
        float tbe = fminf(eff, 1.0f);                   // turbulence shakes the aircraft
        qd += wx.turb * 0.9f * wx_noise(3.1f * wx_time + 1.0f) * tbe;
        pd += wx.turb * 1.2f * wx_noise(2.3f * wx_time + 5.0f) * tbe;
        rd += wx.turb * 0.5f * wx_noise(1.9f * wx_time + 9.0f) * tbe;
        V3 gacc = v3(0.0f, 0.0f, 0.0f);
        float maxpen = gear_forces((pad->buttons & SCE_CTRL_DOWN) != 0, &qd, &pd, &rd, &gacc);
        bool contact = (gear_pen[0] > 0.0f || gear_pen[1] > 0.0f || gear_pen[2] > 0.0f);
        if (contact && !pl.touched) {                                // first touch of this landing
            pl.touched = true;
            challenge_touchdown();
            if (pl.vel.y < -5.0f)                      { crash_why("too hard a landing"); crash(); return; }
            if (fabsf(vdot(pl.vel, pl.r)) > 9.0f)      { crash_why("sideways landing"); crash(); return; }
        }
        if (!contact && pl.pos.y > 1.0f) pl.touched = false;
        if (maxpen > 0.45f)                            { crash_why("landing gear collapsed"); crash(); return; }
        pl.pitchrate = clampf(pl.pitchrate + qd * DT, -2.0f, 2.0f);
        pl.rollrate  = clampf(pl.rollrate  + pd * DT, -3.0f, 3.0f);
        pl.yawrate   = clampf(pl.yawrate   + rd * DT, -1.5f, 1.5f);

        rot_pitch(pl.pitchrate * DT);
        rot_roll(pl.rollrate * DT);
        rot_yaw(pl.yawrate * DT);
        orthonormalize();

        // Forces (per unit mass).
        float cl = lift_coeff(alpha, pl.flaps);
        float ge = pl.pos.y < 10.0f ? 1.0f + 0.12f * (1.0f - pl.pos.y / 10.0f) : 1.0f;   // ground effect
        float lift = clampf(KL * mf * V * V * cl * ge, -1.5f * G_ACC, 3.8f * G_ACC);     // +3.8 / -1.5 g limits
        float drag = mf * ((KD0 + FLAP_DRAG[pl.flaps] + spec->kd_gear * pl.gear + KD_WINDMILL * (1.0f - thr)) * V * V
                           + KDI * V * V * cl * cl);
        V3 a = v3(0.0f, -G_ACC, 0.0f);
        a = vadd(a, vscale(pl.f, mf * thrust_accel(thr, V)));
        // Lift acts perpendicular to the airflow (induced drag is accounted for separately).
        V3 vhat = (V > 0.5f) ? vscale(v, 1.0f / V) : pl.f;
        V3 ldir = vsub(pl.u, vscale(vhat, vdot(pl.u, vhat)));
        float llen = sqrtf(vdot(ldir, ldir));
        ldir = (llen > 0.2f) ? vscale(ldir, 1.0f / llen) : pl.u;
        a = vadd(a, vscale(ldir, lift));
        a = vadd(a, vscale(pl.r, -KSIDE * mf * V * vR));
        if (V > 0.5f) a = vsub(a, vscale(v, drag / V));
        a = vadd(a, gacc);
        pl.vel = vadd(pl.vel, vscale(a, DT));
        pl.pos = vadd(pl.pos, vscale(pl.vel, DT));

        if (spec->retract && pl.gear < 0.95f && pl.pos.y < 4.0f) {          // belly landing: the gear is not down and locked
            if (body_pt(0.0f, 0.95f, 5.0f).y < 0.0f || body_pt(0.0f, 0.95f, 0.0f).y < 0.0f || body_pt(0.0f, 1.25f, -3.5f).y < 0.0f) {
                crash_why("landed with the gear up"); crash(); return;
            }
        }
        if (pl.pos.y < 4.0f) {
            const char *why = ground_strike();
            if (why) { crash_why(why); crash(); return; }
        }
        // Once all three wheels are down and the aircraft has settled, hand over to the ground-roll model.
        float bank = atan2f(-pl.r.y, pl.u.y);
        if (gear_pen[0] > 0.03f && gear_pen[1] > 0.03f && gear_pen[2] > 0.03f &&
            fabsf(pl.vel.y) < 0.5f && fabsf(bank) < 0.06f) {
            V3 hd = v3(pl.f.x, 0.0f, pl.f.z);
            float hl = sqrtf(vdot(hd, hd));
            hd = hl > 1e-3f ? vscale(hd, 1.0f / hl) : v3(0.0f, 0.0f, 1.0f);
            pl.on_ground = true;
            pl.hdg_g  = atan2f(pl.f.x, pl.f.z);
            pl.gpitch = clampf(asinf(clampf(pl.f.y, -1.0f, 1.0f)), 0.0f, spec->gpitch_max);
            pl.gs     = fmaxf(0.0f, vdot(pl.vel, hd));
        }
    } else {
        // ---------------- on the wheels ----------------
        float s = pl.gs;
        float hs0 = sinf(pl.hdg_g), hc0 = cosf(pl.hdg_g);
        float windlong  = wv.x * hs0 + wv.z * hc0;     // tailwind is positive
        float windcross = wv.x * hc0 - wv.z * hs0;     // wind blowing toward the right is positive
        float sa = fmaxf(0.0f, s - windlong);          // airspeed along the runway
        float eff = clampf(sa * sa / (VREF * VREF), 0.0f, 1.6f);

        float steer = clampf(pl.rud + pl.ail * 0.5f, -1.0f, 1.0f);
        pl.hdg_g += steer * 0.45f * DT * fminf(s / 6.0f, 1.0f) / (1.0f + (s / 12.0f) * (s / 12.0f));
        pl.hdg_g -= spec->gpull * thr * clampf(1.0f - eff, 0.0f, 1.0f) * fminf(s / 8.0f, 1.0f) * DT;  // pulls left under power
        if (s > 3.0f) pl.hdg_g -= 0.006f * windcross * clampf(sa / 20.0f, 0.0f, 1.0f) * DT;   // weathercocks into a crosswind
        if (pl.hdg_g >= 2 * PI_F) pl.hdg_g -= 2 * PI_F;
        if (pl.hdg_g <  0.0f)     pl.hdg_g += 2 * PI_F;

        // Rotation: nose wheel comes up with back-pressure once there is airflow.
        if (sa > spec->rot_speed && elev > 0.05f) pl.gpitch += elev * 0.5f * (0.4f + eff) * DT;
        else                           pl.gpitch -= pl.gpitch * 0.8f * DT;
        pl.gpitch = clampf(pl.gpitch, 0.0f, spec->gpitch_max);      // tail-strike limit

        float lift_g = KL * mf * sa * sa * lift_coeff(pl.gpitch, pl.flaps);
        float accel = mf * thrust_accel(thr, sa) - mf * KD0 * sa * sa;
        if (s > 0.0f) accel -= 0.03f * fmaxf(G_ACC - lift_g, 0.0f);         // rolling resistance
        if ((pad->buttons & SCE_CTRL_DOWN) && s > 0.0f)                   // toe brakes
            accel -= spec->brake * fmaxf(1.0f - lift_g / G_ACC, 0.2f);
        pl.gs = fmaxf(0.0f, s + accel * DT);

        float sh = sinf(pl.hdg_g), ch = cosf(pl.hdg_g);
        pl.pos.x += sh * pl.gs * DT;
        pl.pos.z += ch * pl.gs * DT;
        pl.pos.y -= pl.pos.y * 0.2f;                             // settle onto the tyres smoothly
        if (fabsf(pl.pos.y) < 0.002f) pl.pos.y = 0.0f;
        pl.vel = v3(sh * pl.gs, 0.0f, ch * pl.gs);
        set_axes(pl.hdg_g, pl.gpitch, 0.0f);
        pl.alpha = pl.gpitch; pl.beta = 0.0f;

        // Lift-off once the wing can carry the weight.
        float sa2 = fmaxf(0.0f, pl.gs - windlong);
        float lift = KL * mf * sa2 * sa2 * lift_coeff(pl.gpitch, pl.flaps);
        if (pl.gpitch > 0.03f && lift * cosf(pl.gpitch) > G_ACC * 1.15f) {
            pl.on_ground = false;
            pl.pitchrate = pl.rollrate = pl.yawrate = 0.0f;
            pl.vel.y = 0.5f;                         // clean break from the runway
            pl.pos.y = 0.12f;                        // the legs have extended: wheels just clear of the ground
            pl.touched = true;
            pl.gamma_prev = pl.gs; pl.gamma_rate = 0.0f;
        }
    }

    // Values used by the camera, panel and warnings.
    pl.hdg   = atan2f(pl.f.x, pl.f.z);
    if (pl.hdg < 0.0f) pl.hdg += 2 * PI_F;
    pl.pitch = asinf(clampf(pl.f.y, -1.0f, 1.0f));
    pl.roll  = atan2f(-pl.r.y, pl.u.y);
    if (pl.on_ground) {                                // the airspeed indicator reads speed through the air
        pl.speed = fmaxf(0.0f, pl.gs - (wv.x * sinf(pl.hdg_g) + wv.z * cosf(pl.hdg_g)));
    } else {
        V3 va = vsub(pl.vel, wv);
        pl.speed = sqrtf(vdot(va, va));
    }
    pl.vs    = pl.vel.y;
    {                                                  // engine speed follows throttle and airspeed
        float target;
        if (pl.fuel > 0.0f && !engine_failed) target = spec->rpm_idle + spec->rpm_thr * thr + spec->rpm_air * pl.speed * (0.3f + 0.7f * thr);
        else                target = pl.on_ground ? 0.0f : fminf(spec->windmill_k * pl.speed, spec->windmill_cap);   // windmilling
        pl.rpm += (clampf(target, 0.0f, spec->rpm_cap) - pl.rpm) * 0.08f;
    }

    if (hit_scenery()) crash();
}

