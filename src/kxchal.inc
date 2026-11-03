// Left Seat: the KX-1 challenges at Strake Proving Ground: free roam and the five timed challenges (the speed run is the first built).
// The KX-1 is only used here, never in the open world. Part of Left Seat's single translation unit: this file is #included by main.c, in order.

enum { KC_FREE = 0, KC_SPEED, KC_HOOP, KC_TRICK, KC_PRECISE, KC_FLIP, KC_COUNT };
static const char *const KC_NAMES[KC_COUNT] = { "FREE ROAM", "SPEED RUN", "HOOP JUMP", "TRICK RUN", "PRECISION LANDING", "UPSIDE-DOWN FLIP" };
static const char *const KC_DESC[KC_COUNT] = {
    "Drive and hop around the proving ground. No timer.",
    "One lap of the circuit, through all five gates in order.",
    "Launch off the strip ramp, fly through the lit hoop, land on the far pad.",
    "String flips and rolls over the trick pad and land on the wheels.",
    "Hop from the ramp across the pond and land inside the target.",
    "Hold the car upside down at the fan gantry, flip it and land upright." };
static const char *const KC_HOW[KC_COUNT] = {
    "Nothing to beat here. Practice with the thrusters.",
    "Standing start. Thrusters use gas: spend it where it counts.",
    "Fast down the strip, up the ramp, through the hoop. Cushion the landing with the hop jets. Centered pass, pad landing and gas left score.",
    "90 seconds. Hop off the pad, flip and roll, land on the wheels on the pad. Each landed trick scores.",
    "Run up to the ramp and cross the pond. The closer to the center and the more gas left, the better.",
    "Up the ramp, fly through the gantry upside down, land the right way up. Time inverted and a soft landing score." };
static const bool KC_BUILT[KC_COUNT] = { true, true, true, true, true, true };

// Medal times for the speed run, in seconds: gold, silver, bronze.
static const float KC_SPEED_MEDAL[3] = { 55.0f, 63.0f, 76.0f };
// Medal points for the others: gold, silver, bronze (these are first guesses, to be tuned on the Vita).
static const float KC_PTS_MEDAL[KC_COUNT][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 88.0f, 75.0f, 55.0f }, { 400.0f, 250.0f, 100.0f }, { 90.0f, 75.0f, 55.0f }, { 90.0f, 75.0f, 55.0f } };

enum { KS_IDLE = 0, KS_READY, KS_RUN, KS_DONE };
static int   kc_kind, kc_st;
static float kc_t, kc_cd, kc_go_t;           // the clock, the countdown, how long "GO" has been showing
static int   kc_gi;                          // speed run: how many of the five gates are behind you
static V3    kc_prev;                        // where the car was a frame ago (for the gate lines)
static bool  kc_fail, kc_newbest;
static char  kc_why[48], kc_msg[40];
static int   kc_msg_t;
static float kc_result;                      // the finished result: seconds for the speed run, points for the others
static int   kc_lift0, kc_land0;             // the lift-off and landing counters when the run began
static bool  kc_pass;                        // hoop or gantry passed
static float kc_pass_d, kc_pts;              // how far off center the pass was; points banked so far (trick run)
static int   kc_tricks;                      // trick run: tricks landed
static bool  kc_assisted;                    // this run has the computer assist on: silver is the most it can earn, and it has its own best
static bool  kc_unlocked_now;                // set on the run that completed the set of five golds: the result card shows the message
static const float KC_TRICK_TIME = 90.0f;
#define KC_PAD(x, z, x0, x1, z0, z1) ((x) >= (x0) && (x) <= (x1) && (z) >= (z0) && (z) <= (z1))

static const int KC_SEQ[5] = { 3, 2, 1, 4, 0 };     // the lap goes clockwise from the line: east, south-east, south-west, west gates, then the line again

static void kc_gate_pos(int g, float *x, float *z, bool *acx) {
    *x = SPG_SX(SPG_GATES[g][0]); *z = SPG_SZ(SPG_GATES[g][1]); *acx = SPG_GATES[g][2] > 0.5f;
}

// 0 = none, 1 = bronze, 2 = silver, 3 = gold (for the times of the speed run)
static int kc_medal_for(int kind, int hundredths) {
    if (hundredths <= 0 || kind < KC_SPEED) return 0;
    const float t = hundredths / 100.0f;
    if (kind == KC_SPEED) return t <= KC_SPEED_MEDAL[0] ? 3 : (t <= KC_SPEED_MEDAL[1] ? 2 : (t <= KC_SPEED_MEDAL[2] ? 1 : 0));
    return t >= KC_PTS_MEDAL[kind][0] ? 3 : (t >= KC_PTS_MEDAL[kind][1] ? 2 : (t >= KC_PTS_MEDAL[kind][2] ? 1 : 0));
}
// The medal for a run: with the assist, silver at most. The best medal on the menu: the unassisted best, or the assisted best (silver at most).
static int kc_medal_best(int k) {
    const int a = kc_medal_for(k, kc_best[k]); int b = kc_medal_for(k, kc_best_as[k]); if (b > 2) b = 2;
    return a > b ? a : b;
}
static int *kc_bestp(void) { return kc_assisted ? kc_best_as : kc_best; }
static const char *const KC_MEDAL_NAME[4] = { "NO MEDAL", "BRONZE", "SILVER", "GOLD" };
static unsigned int kc_medal_color(int m) { return m == 3 ? RGBA8(255, 206, 80, 255) : (m == 2 ? RGBA8(214, 220, 232, 255) : (m == 1 ? RGBA8(214, 140, 80, 255) : RGBA8(110, 130, 180, 255))); }

static void kc_place(float x, float z, float h) {
    memset(&pl, 0, sizeof(pl));
    pl.pos = v3(x, 0.0f, z);
    pl.hdg = pl.hdg_g = h;
    pl.on_ground = true; pl.fuel = 1.0f;
    sw_cold();
    pl.gear = 1.0f; pl.gear_cmd = true;
    engine_failed = false;
    ntrk = 0; trk_i = 0; trk_t = 0;
    kx_reset(h);
    sw.arm = false; sw.batt = true; sw.pump = sw.ign = true; eng_on = true; eng_n = 1.0f;
    pl.touched = true;
    set_axes(h, 0, 0);
    chase_dir = pl.f; look_ang = 0.0f;
}

static void kc_start(int kind) {
    kc_kind = kind;
    game_mode = MODE_KX; ch_state = CH_IDLE;
    ac_flying = AC_KX1; spec = &SPECS[AC_KX1];
    if (kind == KC_SPEED) {                                                // on the north straight, 45 m before the line, facing east
        float gx, gz; bool acx; kc_gate_pos(0, &gx, &gz, &acx);
        kc_place(gx - 45.0f, gz, PI_F * 0.5f);
    } else if (kind == KC_HOOP) {                                          // on the launch strip, 400 m before the ramp
        kc_place(SPG_SX(545.0f), SPG_SZ(72.0f), PI_F * 0.5f);
    } else if (kind == KC_PRECISE) {                                       // on the run-up to the pond ramp
        kc_place(SPG_SX(590.0f), SPG_SZ(500.0f), PI_F * 0.5f);
    } else if (kind == KC_TRICK) {                                         // on the trick pad
        kc_place(SPG_SX(495.0f), SPG_SZ(295.0f), PI_F * 0.5f);
    } else if (kind == KC_FLIP) {                                          // at the south end of the flip ramp's run-up, facing north
        kc_place(SPG_SX(680.0f), SPG_SZ(438.0f), 0.0f);
    } else {                                                               // at the pit, facing the road, past the garage and the tanks
        kc_place(SPG_SX(177.0f), SPG_SZ(550.0f), 0.0f);
    }
    if (kind >= KC_HOOP) sw.arm = true;                                    // the jets are the point of these: armed from the start
    kc_assisted = kx_assist; kc_unlocked_now = false; kc_lift0 = kx_ev_lift; kc_land0 = kx_ev_land; kc_pass = false; kc_pass_d = 0.0f; kc_pts = 0.0f; kc_tricks = 0;
    kc_gi = 0; kc_t = 0.0f; kc_go_t = 0.0f; kc_fail = false; kc_newbest = false; kc_why[0] = 0; kc_msg_t = 0;
    kc_prev = pl.pos;
    if (kind == KC_FREE) { kc_st = KS_RUN; kc_hold = false; }
    else { kc_st = KS_READY; kc_cd = 3.0f; kc_hold = true; }
}
static void kc_restart(void) { kc_start(kc_kind); }

static void kc_say(const char *s) { snprintf(kc_msg, sizeof(kc_msg), "%s", s); kc_msg_t = 120; }
// The assist switch was flipped: a challenge in progress starts over (so it cannot be switched on just for the landing).
static void kc_assist_changed(void) {
    if (game_mode != MODE_KX) return;
    if (kc_kind != KC_FREE && (kc_st == KS_READY || kc_st == KS_RUN)) { kc_restart(); kc_say(kx_assist ? "ASSIST ON: STARTING OVER" : "ASSIST OFF: STARTING OVER"); }
    else kc_say(kx_assist ? "COMPUTER ASSIST ON" : "COMPUTER ASSIST OFF");
}

// Is every flight challenge done in gold? (the unlock)
static bool kc_all_gold(void) {
    for (int k = KC_SPEED; k <= KC_FLIP; k++) if (kc_medal_for(k, kc_best[k]) < 3) return false;
    return true;
}

static void kc_finish_pts(bool fail, const char *why, float pts) {
    const bool was_gold = kc_all_gold();
    kc_st = KS_DONE; kc_hold = false; kc_fail = fail;
    snprintf(kc_why, sizeof(kc_why), "%s", why ? why : "");
    kc_result = (kc_kind == KC_SPEED) ? kc_t : pts; kc_newbest = false;
    if (!fail) {
        const int h = (int)(kc_result * 100.0f + 0.5f);
        int *bp = kc_bestp();
        if (kc_kind == KC_SPEED) { if (bp[kc_kind] <= 0 || h < bp[kc_kind]) { bp[kc_kind] = h; kc_newbest = true; } }
        else if (h > bp[kc_kind]) { bp[kc_kind] = h; kc_newbest = true; }
        scores_save();
    }
    kc_unlocked_now = !was_gold && kc_all_gold();
}
static void kc_finish(bool fail, const char *why) { kc_finish_pts(fail, why, kc_pts); }

// Did the car cross the gate between the last frame and this one (between its posts)?
static bool kc_crossed(int g, V3 a, V3 b) {
    float gx, gz; bool acx; kc_gate_pos(g, &gx, &gz, &acx);
    const float a0 = acx ? a.z - gz : a.x - gx, b0 = acx ? b.z - gz : b.x - gx;
    if ((a0 > 0.0f) == (b0 > 0.0f) || a0 == b0) return false;
    const float t = a0 / (a0 - b0);
    const float lat = acx ? (a.x + (b.x - a.x) * t) - gx : (a.z + (b.z - a.z) * t) - gz;
    return fabsf(lat) <= SPG_GATE_HW;
}

// Once a frame while a KX-1 session is on.
static void kc_update(void) {
    if (game_mode != MODE_KX) return;
    if (kc_msg_t > 0) kc_msg_t--;
    if (kc_go_t > 0.0f) kc_go_t -= DT;
    if (kc_st == KS_READY) {
        kc_hold = true; kc_cd -= DT;
        if (kc_cd <= 0.0f) { kc_st = KS_RUN; kc_hold = false; kc_t = 0.0f; kc_go_t = 1.0f; }
        kc_prev = pl.pos;
        return;
    }
    if (kc_st != KS_RUN) return;
    if (kc_kind != KC_FREE) kc_t += DT;
    if (!pl.crashed && (fabsf(pl.pos.x - SPG_X) > SPG_HX + 30.0f || fabsf(pl.pos.z - SPG_Z) > SPG_HZ + 30.0f)) {       // out of bounds
        if (kc_kind == KC_FREE) { kc_start(KC_FREE); kc_say("OUT OF BOUNDS - BACK AT THE PIT"); return; }
        kc_finish(true, "Left the proving ground"); return;
    }
    if (kc_kind == KC_FREE) { kc_prev = pl.pos; return; }
    if (pl.crashed) {
        if (kc_kind == KC_TRICK && kc_pts > 0.0f) { kc_finish_pts(false, "Crashed: the points so far are kept", kc_pts); return; }
        kc_finish(true, last_crash[0] ? last_crash : "Crashed"); return;
    }
    if (kc_kind == KC_SPEED) {
        if (kc_crossed(KC_SEQ[kc_gi], kc_prev, pl.pos)) {
            kc_gi++;
            snd_event(EV_CONFIRM, 1.0f);
            if (kc_gi >= 5) { kc_finish(false, ""); return; }
            char b[32]; snprintf(b, sizeof(b), "GATE %d / 5", kc_gi); kc_say(b);
        }
        if (kc_t > 240.0f) { kc_finish(true, "Out of time"); return; }
    }
    const bool landed = kx_ev_land != kc_land0;
    if (landed) kc_land0 = kx_ev_land;
    if (kc_kind == KC_HOOP) {
        if (!kc_pass && (kc_prev.x - SPG_HOOP_X) * (pl.pos.x - SPG_HOOP_X) < 0.0f) {
            const float t = (kc_prev.x - SPG_HOOP_X) / (kc_prev.x - pl.pos.x);
            const float y = kc_prev.y + (pl.pos.y - kc_prev.y) * t, z = kc_prev.z + (pl.pos.z - kc_prev.z) * t;
            const float d = sqrtf((y - SPG_HOOP_Y) * (y - SPG_HOOP_Y) + (z - SPG_HOOP_Z) * (z - SPG_HOOP_Z));
            if (d <= SPG_HOOP_R) { kc_pass = true; kc_pass_d = d; snd_event(EV_CONFIRM, 1.0f); kc_say("THROUGH THE HOOP"); }
            else { kc_finish(true, "Missed the hoop"); return; }
        }
        if (landed) {
            if (!kc_pass) { kc_finish(true, "Landed short of the hoop"); return; }
            if (!KC_PAD(pl.pos.x, pl.pos.z, SPG_SX(845.0f), SPG_SX(965.0f), SPG_SZ(88.0f), SPG_SZ(56.0f))) { kc_finish(true, "Missed the landing pad"); return; }
            kc_finish_pts(false, "", 40.0f - 20.0f * kc_pass_d / SPG_HOOP_R + 30.0f + 30.0f * kx.gas / 100.0f); return;
        }
        if (kc_t > 150.0f) { kc_finish(true, "Out of time"); return; }
    } else if (kc_kind == KC_PRECISE) {
        if (landed && kx_ev_lift != kc_lift0) {
            const float tx = SPG_SX(855.0f), tz = SPG_SZ(500.0f);
            if (!KC_PAD(pl.pos.x, pl.pos.z, SPG_SX(830.0f), SPG_SX(880.0f), SPG_SZ(530.0f), SPG_SZ(470.0f))) { kc_finish(true, "Missed the platform"); return; }
            const float d = sqrtf((pl.pos.x - tx) * (pl.pos.x - tx) + (pl.pos.z - tz) * (pl.pos.z - tz));
            kc_pass_d = d;
            kc_finish_pts(false, "", 70.0f * fmaxf(0.0f, 1.0f - d / 45.0f) + 30.0f * kx.gas / 100.0f); return;
        }
        if (kc_t > 150.0f) { kc_finish(true, "Out of time"); return; }
    } else if (kc_kind == KC_FLIP) {
        if (!kc_pass && (kc_prev.z - SPG_GANT_Z) * (pl.pos.z - SPG_GANT_Z) < 0.0f) {
            const float t = (kc_prev.z - SPG_GANT_Z) / (kc_prev.z - pl.pos.z);
            const float x = kc_prev.x + (pl.pos.x - kc_prev.x) * t, y = kc_prev.y + (pl.pos.y - kc_prev.y) * t;
            if (fabsf(x - SPG_GANT_X) > SPG_GANT_HW || y < SPG_GANT_Y0 || y > SPG_GANT_Y1) { kc_finish(true, "Missed the gantry"); return; }
            if (pl.u.y > -0.2f) { kc_finish(true, "Not upside down at the gantry"); return; }
            kc_pass = true; snd_event(EV_CONFIRM, 1.0f); kc_say("UPSIDE DOWN AT THE GANTRY");
        }
        if (landed) {
            if (!kc_pass) { kc_finish(true, "Landed before the gantry"); return; }
            kc_finish_pts(false, "", 40.0f + 30.0f * fminf(1.0f, kx_last_inv / 2.0f) + 30.0f * clampf(1.0f - kx_last_sink / 7.0f, 0.3f, 1.0f)); return;
        }
        if (kc_t > 150.0f) { kc_finish(true, "Out of time"); return; }
    } else if (kc_kind == KC_TRICK) {
        if (landed) {
            if (!KC_PAD(pl.pos.x, pl.pos.z, SPG_SX(420.0f), SPG_SX(570.0f), SPG_SZ(350.0f), SPG_SZ(240.0f))) kc_say("OFF THE PAD: NO POINTS");
            else {
                const float turn = 2.0f * PI_F * 0.85f;
                const int n = (int)(fabsf(kx_last_p) / turn) + (int)(fabsf(kx_last_r) / turn);
                if (n > 0) {
                    const float q = kx_last_sink < 4.0f ? 1.0f : 0.8f;
                    const float pts = 50.0f * n * (1.0f + 0.5f * (n - 1)) * q;
                    kc_pts += pts; kc_tricks += n;
                    char b[40]; snprintf(b, sizeof(b), "%d TRICK%s  +%d", n, n > 1 ? "S" : "", (int)pts); kc_say(b);
                    snd_event(EV_CONFIRM, 1.0f);
                } else kc_say("NO TRICK");
            }
        }
        if (kc_t >= KC_TRICK_TIME) { kc_finish_pts(false, "", kc_pts); return; }
    }
    kc_prev = pl.pos;
}

// ---- Drawing ----

static void kc_time_text(char *b, size_t n, float s) { int m = (int)(s / 60.0f); snprintf(b, n, "%d:%05.2f", m, s - m * 60.0f); }

// In the world: a column of light over the next gate.
static void kc_draw_world(void) {
    if (game_mode != MODE_KX || kc_st == KS_DONE) return;
    if (kc_kind == KC_PRECISE || kc_kind == KC_HOOP) {                        // a column of light over the target
        const float tx = kc_kind == KC_PRECISE ? SPG_SX(855.0f) : SPG_SX(905.0f), tz = kc_kind == KC_PRECISE ? SPG_SZ(500.0f) : SPG_SZ(72.0f);
        for (int k = 0; k < 9; k++) draw_light(tx, 2.0f + k * 4.0f, tz, 2.6f - 0.12f * k, RGBA8(255, 214, 70, 255));
        return;
    }
    if (kc_kind != KC_SPEED) return;
    float gx, gz; bool acx; kc_gate_pos(KC_SEQ[kc_gi < 5 ? kc_gi : 4], &gx, &gz, &acx);
    const unsigned int col = RGBA8(255, 214, 70, 255);
    for (int k = 0; k < 9; k++) draw_light(gx, 2.0f + k * 4.0f, gz, 2.6f - 0.12f * k, col);
    for (int s = -1; s <= 1; s += 2) for (int k = 0; k < 4; k++) draw_light(acx ? gx + s * SPG_GATE_HW : gx, 1.0f + k * 2.2f, acx ? gz : gz + s * SPG_GATE_HW, 1.4f, col);
}

static void kc_draw_hud(vita2d_pgf *pgf) {
    if (game_mode != MODE_KX) return;
    const unsigned int gold = RGBA8(255, 206, 130, 255), white = RGBA8(255, 255, 255, 255);
    char b[48];
    if (kc_kind == KC_FREE) {
        vita2d_pgf_draw_text(pgf, 540, 28, gold, 0.8f, "FREE ROAM    STRAKE PROVING GROUND");
    } else {
        char info[40];
        switch (kc_kind) {
            case KC_SPEED:   snprintf(info, sizeof(info), "GATE %d / 5", kc_gi < 5 ? kc_gi + 1 : 5); break;
            case KC_HOOP:    snprintf(info, sizeof(info), "%s", kc_pass ? "HOOP: THROUGH" : "HOOP: AHEAD"); break;
            case KC_PRECISE: snprintf(info, sizeof(info), "LAND ON THE TARGET"); break;
            case KC_TRICK:   snprintf(info, sizeof(info), "POINTS %d", (int)kc_pts); break;
            default:         snprintf(info, sizeof(info), "%s  INVERTED %.1f S", kc_pass ? "GANTRY: PASSED" : "GANTRY: AHEAD", kx_inv_t); break;
        }
        vita2d_pgf_draw_textf(pgf, 540, 28, gold, 0.8f, "%s    %s", KC_NAMES[kc_kind], info);
        if (kc_assisted) vita2d_pgf_draw_text(pgf, 540, 70, RGBA8(120, 210, 255, 255), 0.62f, "ASSIST: SILVER IS THE MOST");
        kc_time_text(b, sizeof(b), kc_kind == KC_TRICK ? fmaxf(0.0f, KC_TRICK_TIME - kc_t) : kc_t);
        int tw = vita2d_pgf_text_width(pgf, 1.5f, b);
        vita2d_pgf_draw_text(pgf, 480 - tw / 2, 70, white, 1.5f, b);
        if (kc_bestp()[kc_kind] > 0) {
            char c[40];
            if (kc_kind == KC_SPEED) kc_time_text(c, sizeof(c), kc_bestp()[kc_kind] / 100.0f); else snprintf(c, sizeof(c), "%d PTS", (kc_bestp()[kc_kind] + 50) / 100);
            vita2d_pgf_draw_textf(pgf, 540, 50, RGBA8(190, 205, 235, 255), 0.65f, "BEST %s", c);
        }
        if (kc_st == KS_READY) {
            snprintf(b, sizeof(b), "%d", (int)ceilf(kc_cd));
            tw = vita2d_pgf_text_width(pgf, 3.2f, b);
            vita2d_pgf_draw_text(pgf, 480 - tw / 2, 230, white, 3.2f, b);
            const char *h = kc_kind >= KC_HOOP ? "The thrusters are armed" : "Arm the thrusters if you want them";
            vita2d_pgf_draw_text(pgf, 480 - vita2d_pgf_text_width(pgf, 0.8f, h) / 2, 270, gold, 0.8f, h);
        } else if (kc_go_t > 0.0f) {
            tw = vita2d_pgf_text_width(pgf, 3.2f, "GO");
            vita2d_pgf_draw_text(pgf, 480 - tw / 2, 230, gold, 3.2f, "GO");
        }
        if (kc_kind == KC_SPEED && kc_st == KS_RUN && kc_gi < 5) {                     // which way is the next gate
            float gx, gz; bool acx; kc_gate_pos(KC_SEQ[kc_gi], &gx, &gz, &acx);
            float rel = atan2f(gx - pl.pos.x, gz - pl.pos.z) - pl.hdg;
            const float cx = 480.0f, cy = 106.0f;
            vita2d_draw_fill_circle(cx, cy, 15.0f, RGBA8(10, 22, 56, 200));
            vita2d_draw_fill_circle(cx + 11.0f * sinf(rel), cy - 11.0f * cosf(rel), 4.0f, gold);
        }
    }
    if (kx_assist_t > 0.0f && kc_st == KS_RUN) {
        const char *a = "COMPUTER ASSIST";
        vita2d_pgf_draw_text(pgf, 480 - vita2d_pgf_text_width(pgf, 0.8f, a) / 2, 196, RGBA8(120, 210, 255, 255), 0.8f, a);
    }
    if (kc_msg_t > 0) {
        int tw = vita2d_pgf_text_width(pgf, 1.1f, kc_msg);
        vita2d_pgf_draw_text(pgf, 480 - tw / 2, 160, gold, 1.1f, kc_msg);
    }
}

static void kc_draw_result(vita2d_pgf *pgf) {
    const unsigned int gold = RGBA8(255, 206, 130, 255), white = RGBA8(255, 255, 255, 255), card = RGBA8(10, 22, 56, 255), pale = RGBA8(190, 205, 235, 255);
    vita2d_draw_rectangle(250, 40, 460, 300, gold);
    vita2d_draw_rectangle(254, 44, 452, 292, card);
    vita2d_pgf_draw_text(pgf, 280, 90, white, 1.3f, KC_NAMES[kc_kind]);
    char b[48];
    if (kc_fail) {
        vita2d_pgf_draw_text(pgf, 280, 150, RGBA8(255, 120, 100, 255), 1.4f, kc_kind == KC_SPEED ? "NO TIME" : "NO SCORE");
        vita2d_pgf_draw_text(pgf, 280, 190, pale, 0.95f, kc_why);
    } else {
        if (kc_kind == KC_SPEED) kc_time_text(b, sizeof(b), kc_result); else snprintf(b, sizeof(b), "%d POINTS", (int)(kc_result + 0.5f));
        vita2d_pgf_draw_text(pgf, 280, 160, white, 2.0f, b);
        int m = kc_medal_for(kc_kind, (int)(kc_result * 100.0f + 0.5f));
        if (kc_assisted && m > 2) m = 2;
        vita2d_pgf_draw_text(pgf, 280, 205, kc_medal_color(m), 1.3f, KC_MEDAL_NAME[m]);
        if (kc_newbest) vita2d_pgf_draw_text(pgf, 280, 238, gold, 0.95f, "NEW BEST");
        else if (kc_bestp()[kc_kind] > 0) {
            char c[40];
            if (kc_kind == KC_SPEED) kc_time_text(c, sizeof(c), kc_bestp()[kc_kind] / 100.0f); else snprintf(c, sizeof(c), "%d POINTS", (kc_bestp()[kc_kind] + 50) / 100);
            vita2d_pgf_draw_textf(pgf, 280, 238, pale, 0.9f, "BEST %s", c);
        }
        if (kc_why[0]) vita2d_pgf_draw_text(pgf, 280, 268, pale, 0.8f, kc_why);
        else if (kc_assisted) vita2d_pgf_draw_text(pgf, 280, 268, RGBA8(120, 210, 255, 255), 0.8f, "ASSISTED: SILVER IS THE MOST");
        if (kc_unlocked_now) vita2d_pgf_draw_text(pgf, 280, 298, gold, 0.95f, "GOLD ON ALL FIVE: STRAKE TROPHY WON");
    }
    vita2d_pgf_draw_text(pgf, 280, 328, pale, 0.85f, "X: try again     O: menu");
}
