// Left Seat: the landing challenge (start, scoring, best-score file).
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* Landing challenge: start on final approach and get scored on the     */
/* touchdown (sink rate, centre line, touchdown point, attitude).       */
/* ------------------------------------------------------------------ */

#ifndef SCORE_PATH
#define SCORE_PATH "ux0:data/skyhawk_scores.txt"      /* (the old file name is kept so saved scores carry over) */
#endif
enum { MODE_FREE = 0, MODE_LANDING = 1 };
enum { CH_IDLE = 0, CH_FLYING, CH_TOUCHED, CH_DONE };

typedef struct {
    float sink, x, z, hdg_err, bank_deg;
    int   bounces;
    bool  nose_first, crashed, new_best;
    int   p_sink, p_line, p_point, p_att, penalty, total;
    float trip_s, fuel_used;              // cross-country: time in the air and fuel burned
    int   rings_hit, p_rings, p_time;     // lake rings: rings flown through, and the points for them and for speed
    float par_s;                          // lake rings: the time that earns the full speed bonus
    char  note[56];
} ChResult;

static int      game_mode;                  // MODE_FREE or MODE_LANDING
static int      ch_state;
static float    ch_t, ch_td_t;
static bool     ch_air_after;
static int      ch_land_ap;                 // which airport you touched down at
static bool     ch_xc_up;                   // cross-country: the aircraft has been well off the ground
static float    ch_fuel0;
enum { CK_LANDING = 0, CK_ENGINE, CK_XWIND, CK_XCOUNTRY, CK_BUSH, CK_RINGS, CK_COUNT };
static const char *const CK_NAMES[CK_COUNT] = { "LANDING", "ENGINE OUT", "CROSSWIND", "CROSS-COUNTRY", "BACKCOUNTRY", "LAKE RINGS" };
static const char *const CK_KEYS[CK_COUNT]  = { "landing", "engine", "xwind", "xcountry", "bush", "rings" };
static int      ch_kind;                    // which challenge is running (or was run last)
// The trip challenges start on the home runway and finish on a runway somewhere else.
static bool ch_trip(void)    { return ch_kind == CK_XCOUNTRY || ch_kind == CK_BUSH; }
static bool ch_rings(void)   { return ch_kind == CK_RINGS; }
static int  ch_dest_ap(void) { return ch_kind == CK_BUSH ? AP_PINECREST : AP_REDROCK; }
static int      ch_best_a[AC_COUNT][CK_COUNT], ch_att_a[AC_COUNT][CK_COUNT];     // best score and attempts, for each aircraft and challenge
#define ch_best     ch_best_a[ac_flying][ch_kind]
#define ch_attempts ch_att_a[ac_flying][ch_kind]
static ChResult chr;

static void scores_load(void) {
    FILE *f = fopen(SCORE_PATH, "r");
    if (!f) return;
    char key[40];
    int v;
    while (fscanf(f, " %39[^=]=%d", key, &v) == 2) {
        if (!strcmp(key, "attempts")) { ch_att_a[AC_K4][CK_LANDING] = v; continue; }      // first version of the file
        if (!strcmp(key, "found_gps")) { ap_found = v != 0; continue; }                        // the hidden spaceport has been flown to
        if (!strcmp(key, "gyro")) { gy_mode = v != 0; continue; }
        if (!strcmp(key, "gyro_roll_rev")) { gy_rinv = v != 0; continue; }
        if (!strcmp(key, "gyro_pitch_rev")) { gy_pinv = v != 0; continue; }
        if (!strcmp(key, "gyro_sens")) { if (v >= 0 && v < 3) gy_sens = v; continue; }
        if (!strcmp(key, "sound")) { if (v >= 0 && v < 4) snd_level = v; continue; }            // the volume setting
        for (int a = 0; a < AC_COUNT; a++)
            for (int k = 0; k < CK_COUNT; k++) {
                char b[72];                                           // the K-4's keys have no prefix, as in older files
                snprintf(b, sizeof(b), "%s%s%s_best", a ? SPECS[a].key : "", a ? "_" : "", CK_KEYS[k]);     if (!strcmp(key, b)) ch_best_a[a][k] = v;
                snprintf(b, sizeof(b), "%s%s%s_attempts", a ? SPECS[a].key : "", a ? "_" : "", CK_KEYS[k]); if (!strcmp(key, b)) ch_att_a[a][k] = v;
            }
    }
    fclose(f);
}
static void scores_save(void) {
    FILE *f = fopen(SCORE_PATH, "w");
    if (!f) return;
    fprintf(f, "sound=%d\n", snd_level);
    fprintf(f, "gyro=%d\ngyro_roll_rev=%d\ngyro_pitch_rev=%d\ngyro_sens=%d\n", gy_mode, gy_rinv, gy_pinv, gy_sens);
    fprintf(f, "found_gps=%d\n", ap_found);
    for (int a = 0; a < AC_COUNT; a++)
        for (int k = 0; k < CK_COUNT; k++)
            fprintf(f, "%s%s%s_best=%d\n%s%s%s_attempts=%d\n", a ? SPECS[a].key : "", a ? "_" : "", CK_KEYS[k], ch_best_a[a][k],
                    a ? SPECS[a].key : "", a ? "_" : "", CK_KEYS[k], ch_att_a[a][k]);
    fclose(f);
}

// A steady 3 degree final approach, about 4.5 km out, a little off line and height, trimmed for 75 kt.
// In the crosswind challenge the nose starts turned into the wind, so the aircraft tracks along the runway.
static void challenge_begin_approach(bool crosswind) {
    const float kt = spec->appr_kt, V = kt / 1.944f, gam = -0.045f, dist = 4500.0f;
    float cl = G_ACC * cosf(gam) / (KL * V * V);
    float alpha = (cl - FLAP_CL[spec->appr_flaps]) / CLA + ALPHA0;
    pl.pos = v3(frand(-120.0f, 120.0f), dist * 0.0524f + frand(-40.0f, 40.0f), -1000.0f - dist);
    V3 w = wx_wind_at(pl.pos);
    float psi = crosswind ? asinf(clampf(-w.x / V, -0.7f, 0.7f)) : 0.0f;
    set_axes(psi, gam + alpha, 0.0f);
    pl.vel = vadd(v3(sinf(psi) * cosf(gam) * V, sinf(gam) * V, cosf(psi) * cosf(gam) * V), w);
    pl.flaps = spec->appr_flaps;
    pl.trim = clampf((alpha - ALPHA_TRIM) * PITCH_MA / PITCH_MD, -0.5f, 0.5f);
    float drag = (KD0 + FLAP_DRAG[spec->appr_flaps] + KD_WINDMILL * 0.7f) * V * V + KDI * V * V * cl * cl;
    pl.throttle = throttle_for_thrust(drag + G_ACC * sinf(gam), V);
    pl.on_ground = false; pl.touched = false; pl.rpm = 2000.0f; pl.gamma_prev = V;
}


/* ------------------------------------------------------------------ */
/* Lake rings: fly through a course of rings over and around Lana Lake  */
/* ------------------------------------------------------------------ */

// The course in Lana Lake Regional's frame: across the runway (left is negative), height above the ground, along the
// runway. It starts on the north side, bends round the lake and runs on to the south. Every leg turns less than 25 degrees.
// (world.inc keeps mountains and hills off this course, using a copy of these points: change both together.)
#define NRINGS 7
static const float RING_PT[NRINGS + 1][3] = {
    { -250.0f, 130.0f, -3100.0f },                                  // where you start (not a ring)
    { -420.0f, 110.0f, -2200.0f }, { -720.0f,  90.0f, -1250.0f }, { -1050.0f, 75.0f, -350.0f }, { -1300.0f, 65.0f,  550.0f },
    { -1250.0f, 70.0f, 1500.0f }, { -900.0f,  90.0f,  2400.0f }, {  -450.0f, 110.0f, 3200.0f } };
static int     ch_ring_i;                   // the ring you are flying to
static int     ch_ring_hit;                 // rings flown through so far
static V3      ch_ring_prev;                // where the aircraft was last frame
static bool    ch_ring_prev_ok;
static float   ch_ring_r;                   // hole radius (bigger for the faster aircraft)
static float   ch_ring_par;                 // par time for the whole course
static V3 ring_wpos(int k) { return ap_pt(&AIRPORTS[AP_LANA], RING_PT[k][0], RING_PT[k][1], RING_PT[k][2]); }   // k = 0 is the start, 1.. the rings
static V3 ring_pos(int i)  { return ring_wpos(i + 1); }
static V3 ring_nrm(int i) {                                         // the way the course runs through ring i
    V3 a = ring_wpos(i), b = ring_wpos(i + 2 > NRINGS ? NRINGS : i + 2);
    V3 d = vsub(b, a);
    d.y = 0.0f;
    float l = sqrtf(vdot(d, d));
    return l > 1.0f ? vscale(d, 1.0f / l) : v3(0.0f, 0.0f, 1.0f);
}

static void challenge_begin_rings(void) {
    const float V = spec->appr_kt * 1.35f / 1.944f;
    V3 p0 = ring_wpos(0), p1 = ring_pos(0);
    float psi = atan2f(p1.x - p0.x, p1.z - p0.z);
    float cl = G_ACC / (KL * V * V);
    float alpha = (cl - FLAP_CL[0]) / CLA + ALPHA0;
    pl.pos = p0;
    set_axes(psi, alpha, 0.0f);
    pl.vel = vadd(v3(sinf(psi) * V, 0.0f, cosf(psi) * V), wx_wind_at(pl.pos));
    pl.flaps = 0;
    pl.trim = clampf((alpha - ALPHA_TRIM) * PITCH_MA / PITCH_MD, -0.5f, 0.5f);
    float drag = (KD0 + KD_WINDMILL * 0.7f) * V * V + KDI * V * V * cl * cl;
    pl.throttle = throttle_for_thrust(drag, V);
    pl.on_ground = false; pl.touched = false; pl.rpm = 2000.0f; pl.gamma_prev = V;
    ch_ring_i = 0; ch_ring_hit = 0; ch_ring_prev_ok = false;
    ch_ring_r = clampf(V * 1.1f, 45.0f, 90.0f);
    float len = 0.0f;
    for (int k = 0; k < NRINGS; k++) { V3 a = ring_wpos(k), b = ring_wpos(k + 1); len += sqrtf((b.x - a.x) * (b.x - a.x) + (b.z - a.z) * (b.z - a.z)); }
    ch_ring_par = len / V * 1.3f;
}

// Engine failure at about 1,500 ft, 3 to 3.7 km from the runway and off to one side, gliding at about 70 kt.
// Flat out the aircraft glides about 9.5 : 1 (7 : 1 with full flaps), so it arrives a little high and flaps or S-turns are needed.
static void challenge_begin_engine(void) {
    const float V = spec->glide_ms, gam = spec->glide_gamma;
    float th = frand(-0.95f, 0.95f), D = frand(3000.0f, 3700.0f);
    pl.pos = v3(D * sinf(th), frand(spec->eng_h0, spec->eng_h1), -1000.0f - D * cosf(th));
    float psi = atan2f(-pl.pos.x, -1800.0f - pl.pos.z);          // facing a point on the extended centre line, short of the runway
    float cl = G_ACC * cosf(gam) / (KL * V * V);
    float alpha = (cl - FLAP_CL[0]) / CLA + ALPHA0;
    set_axes(psi, gam + alpha, 0.0f);
    pl.vel = vadd(v3(sinf(psi) * cosf(gam) * V, sinf(gam) * V, cosf(psi) * cosf(gam) * V), wx_wind_at(pl.pos));
    pl.flaps = 0;
    pl.trim = clampf((alpha - ALPHA_TRIM) * PITCH_MA / PITCH_MD, -0.5f, 0.5f);
    pl.throttle = 0.0f;
    engine_failed = true;
    pl.on_ground = false; pl.touched = false; pl.rpm = 1100.0f; pl.gamma_prev = V;
}

static void challenge_begin(void) {
    reset_plane();                                                     // (cross-country starts on the runway at home)
    if (ch_kind == CK_ENGINE)            challenge_begin_engine();
    else if (ch_rings())                 challenge_begin_rings();
    else if (!ch_trip())                 challenge_begin_approach(ch_kind == CK_XWIND);
    ch_state = CH_FLYING; ch_t = 0.0f; ch_td_t = 0.0f; ch_air_after = false;
    ch_xc_up = false; ch_fuel0 = pl.fuel;
    pl.n1 = pl.throttle;                                                // (a jet starts with its engine already at that power)
    memset(&chr, 0, sizeof(chr));
    game_mode = MODE_LANDING;
}

// Called the moment a wheel first touches the runway.
static void challenge_touchdown(void) {
    if (game_mode != MODE_LANDING || ch_state != CH_FLYING || ch_rings()) return;
    if (ch_trip() && !ch_xc_up) return;                                // still taking off at home
    ch_state = CH_TOUCHED; ch_td_t = 0.0f; ch_air_after = false;
    chr.sink = fmaxf(0.0f, -pl.vel.y);
    // Work in the frame of the runway you are landing on (for cross-country, the one nearest to where you touched down).
    const Airport *a = &AIRPORTS[AP_HOME];
    if (ch_trip()) {
        const Airport *D = &AIRPORTS[ch_dest_ap()];
        float d0 = sat_dist(pl.pos.x, pl.pos.z, AIRPORTS[AP_HOME].cx, AIRPORTS[AP_HOME].cz);
        float d1 = sat_dist(pl.pos.x, pl.pos.z, D->cx, D->cz);
        a = d1 < d0 ? D : &AIRPORTS[AP_HOME];
    }
    float lx, lz;
    ap_local(a, pl.pos.x, pl.pos.z, &lx, &lz);
    float hd = pl.hdg - a->hdg;
    while (hd > PI_F)   hd -= 2.0f * PI_F;
    while (hd <= -PI_F) hd += 2.0f * PI_F;
    if (cosf(hd) < 0.0f) {                                             // landing the other way: use the other end of the runway
        lx = -lx; lz = -lz; hd += (hd > 0.0f) ? -PI_F : PI_F;
    }
    chr.x = lx; chr.z = lz;
    chr.hdg_err = hd * 57.2958f;
    ch_land_ap = (int)(a - AIRPORTS);
    chr.bank_deg = pl.roll * 57.2958f;
    chr.nose_first = (gear_pen[2] > 0.0f && gear_pen[0] <= 0.0f && gear_pen[1] <= 0.0f);
    chr.bounces = 0;
}

static void challenge_score(void) {
    const Airport *L = &AIRPORTS[ch_land_ap];
    float sc = clampf(L->hl / 1000.0f, 0.25f, 1.0f);                    // the target area shrinks on a short runway
    float d = chr.z + L->hl;                                            // metres past the threshold
    float lo = 150.0f * sc, hi = 450.0f * sc, tail = 550.0f * sc;       // the engine-out window is wider: there is no second chance
    if (ch_kind == CK_ENGINE) { lo = 60.0f * sc; hi = 650.0f * sc; tail = 700.0f * sc; }
    float pt;
    if (d < 0.0f)        pt = 0.0f;
    else if (d < lo)     pt = 25.0f * d / lo;
    else if (d <= hi)    pt = 25.0f;
    else                 pt = 25.0f * clampf(1.0f - (d - hi) / tail, 0.0f, 1.0f);
    chr.p_sink  = (int)(40.0f * clampf(1.0f - (chr.sink - 0.5f) / 3.0f, 0.0f, 1.0f) + 0.5f);
    chr.p_line  = (int)(25.0f * clampf(1.0f - fabsf(chr.x) / L->hw, 0.0f, 1.0f) + 0.5f);
    chr.p_point = (int)(pt + 0.5f);
    chr.p_att   = (int)(10.0f * clampf(1.0f - (fabsf(chr.hdg_err) + fabsf(chr.bank_deg)) / 20.0f, 0.0f, 1.0f) + 0.5f);
    chr.penalty = chr.bounces * 10 + (chr.nose_first ? 10 : 0);
    int t = chr.p_sink + chr.p_line + chr.p_point + chr.p_att - chr.penalty;
    chr.total = t < 0 ? 0 : (t > 100 ? 100 : t);
}

// Lake rings: 80 points for the rings you flew through and 20 for being quick, in proportion to the rings you got.
static void challenge_score_rings(void) {
    chr.rings_hit = ch_ring_hit;
    chr.par_s = ch_ring_par;
    chr.p_rings = (int)(80.0f * ch_ring_hit / NRINGS + 0.5f);
    chr.p_time  = (int)(20.0f * clampf(1.0f - (ch_t - ch_ring_par) / ch_ring_par, 0.0f, 1.0f) * ch_ring_hit / NRINGS + 0.5f);
    chr.total = chr.p_rings + chr.p_time;
}

static void challenge_finish(bool crashed, const char *why) {
    ch_state = CH_DONE;
    chr.trip_s = ch_t;
    chr.fuel_used = fmaxf(0.0f, ch_fuel0 - pl.fuel);
    if (!crashed && ch_trip()) {                                       // the landing has to be on the runway you were sent to
        static char why_buf[56];
        const Airport *D = &AIRPORTS[ch_dest_ap()];
        bool on_rwy = (ch_land_ap == ch_dest_ap() && fabsf(chr.x) < D->hw + 8.0f && chr.z > -D->hl - 50.0f && chr.z < D->hl + 50.0f);
        if (!on_rwy) {
            crashed = true;
            if (ch_land_ap == AP_HOME) why = "That's the airport you left!";
            else { snprintf(why_buf, sizeof(why_buf), "Not on the %s runway", D->place); why = why_buf; }
        }
    }
    chr.crashed = crashed;
    if (crashed) { chr.total = 0; snprintf(chr.note, sizeof(chr.note), "%s", why); }
    else if (ch_rings()) challenge_score_rings();
    else         challenge_score();
    if (crashed) { if (!pl.crashed) snd_event(EV_CHIME_BAD, 1.0f); }          // (a crash has its own sound)
    else         snd_event(chr.total >= 80 ? EV_CHIME_GOOD : (chr.total >= 50 ? EV_CHIME_OK : EV_CHIME_BAD), 1.0f);
    ch_attempts++;
    if (chr.total > ch_best) { ch_best = chr.total; chr.new_best = true; }
    scores_save();
}

// Bearing (degrees from north) and distance (metres) from the aircraft to an airport.
static void ap_bearing(const Airport *a, float *brg_deg, float *dist_m) {
    float dx = a->cx - pl.pos.x, dz = a->cz - pl.pos.z;
    *dist_m = sqrtf(dx * dx + dz * dz);
    float b = atan2f(dx, dz) * 57.2958f;
    *brg_deg = b < 0.0f ? b + 360.0f : b;
}

// Lake rings: look for the aircraft crossing the plane of the ring it is flying to.
static void rings_update(void) {
    V3 C = ring_pos(ch_ring_i), N = ring_nrm(ch_ring_i);
    float s = vdot(vsub(pl.pos, C), N);
    if (ch_ring_prev_ok) {
        float sp = vdot(vsub(ch_ring_prev, C), N);
        if (sp < 0.0f && s >= 0.0f) {
            float t = sp / (sp - s);
            V3 cp = vadd(ch_ring_prev, vscale(vsub(pl.pos, ch_ring_prev), t));
            V3 d = vsub(cp, C);
            bool hit = sqrtf(vdot(d, d)) < ch_ring_r;
            if (hit) ch_ring_hit++;
            snd_event(hit ? EV_CHIME_GOOD : EV_CHIME_BAD, 1.0f);
            ch_ring_i++;
            ch_ring_prev_ok = false;
            if (ch_ring_i >= NRINGS) challenge_finish(false, "");
            return;
        }
    }
    ch_ring_prev = pl.pos; ch_ring_prev_ok = true;
}

// Called once per flying frame while a landing challenge is running.
static void challenge_update(void) {
    if (game_mode != MODE_LANDING) return;
    if (ch_state == CH_FLYING) {
        ch_t += DT;
        if (ch_rings() && !pl.crashed) { rings_update(); if (ch_state != CH_FLYING) return; }
        if (ch_rings() && !pl.crashed && ch_t > 300.0f) challenge_finish(false, "");      // time up: you keep the rings you got
        else if (pl.crashed)                    challenge_finish(true, last_crash[0] ? last_crash : "Crashed");
        else if (ch_t > (ch_trip() ? 2700.0f : 420.0f))          challenge_finish(true, "Out of time");
        else if (!ch_trip() && !ch_rings() && pl.pos.z > 1100.0f)               challenge_finish(true, "Flew past the runway");
        else if (pl.pos.y > (ch_trip() ? 6000.0f : 2500.0f))     challenge_finish(true, "Left the area");
        if (pl.pos.y > 120.0f) ch_xc_up = true;
    } else if (ch_state == CH_TOUCHED) {
        ch_td_t += DT;
        bool contact = (gear_pen[0] > 0.0f || gear_pen[1] > 0.0f || gear_pen[2] > 0.0f);
        if (!contact && pl.pos.y > 0.35f) ch_air_after = true;
        if (contact && ch_air_after) { chr.bounces++; ch_air_after = false; }
        if (pl.crashed)                                     challenge_finish(true, last_crash[0] ? last_crash : "Crashed");
        else if ((pl.on_ground && pl.gs < 12.0f) || ch_td_t > 25.0f) challenge_finish(false, "");
    }
}

