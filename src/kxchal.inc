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
    "(Coming in the next update)", "(Coming in the next update)", "(Coming in the next update)", "(Coming in the next update)" };
static const bool KC_BUILT[KC_COUNT] = { true, true, false, false, false, false };

// Medal times for the speed run, in seconds: gold, silver, bronze.
static const float KC_SPEED_MEDAL[3] = { 55.0f, 63.0f, 76.0f };

enum { KS_IDLE = 0, KS_READY, KS_RUN, KS_DONE };
static int   kc_kind, kc_st;
static float kc_t, kc_cd, kc_go_t;           // the clock, the countdown, how long "GO" has been showing
static int   kc_gi;                          // speed run: how many of the five gates are behind you
static V3    kc_prev;                        // where the car was a frame ago (for the gate lines)
static bool  kc_fail, kc_newbest;
static char  kc_why[48], kc_msg[40];
static int   kc_msg_t;
static float kc_result;                      // the finished result: seconds for the speed run

static const int KC_SEQ[5] = { 3, 2, 1, 4, 0 };     // the lap goes clockwise from the line: east, south-east, south-west, west gates, then the line again

static void kc_gate_pos(int g, float *x, float *z, bool *acx) {
    *x = SPG_SX(SPG_GATES[g][0]); *z = SPG_SZ(SPG_GATES[g][1]); *acx = SPG_GATES[g][2] > 0.5f;
}

// 0 = none, 1 = bronze, 2 = silver, 3 = gold (for the times of the speed run)
static int kc_medal_for(int kind, int hundredths) {
    if (hundredths <= 0 || kind != KC_SPEED) return 0;
    const float t = hundredths / 100.0f;
    return t <= KC_SPEED_MEDAL[0] ? 3 : (t <= KC_SPEED_MEDAL[1] ? 2 : (t <= KC_SPEED_MEDAL[2] ? 1 : 0));
}
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
    } else {                                                               // at the pit, facing the road, past the garage and the tanks
        kc_place(SPG_SX(177.0f), SPG_SZ(550.0f), 0.0f);
    }
    kc_gi = 0; kc_t = 0.0f; kc_go_t = 0.0f; kc_fail = false; kc_newbest = false; kc_why[0] = 0; kc_msg_t = 0;
    kc_prev = pl.pos;
    if (kind == KC_FREE) { kc_st = KS_RUN; kc_hold = false; }
    else { kc_st = KS_READY; kc_cd = 3.0f; kc_hold = true; }
}
static void kc_restart(void) { kc_start(kc_kind); }

static void kc_say(const char *s) { snprintf(kc_msg, sizeof(kc_msg), "%s", s); kc_msg_t = 120; }

static void kc_finish(bool fail, const char *why) {
    kc_st = KS_DONE; kc_hold = false; kc_fail = fail;
    snprintf(kc_why, sizeof(kc_why), "%s", why ? why : "");
    kc_result = kc_t; kc_newbest = false;
    if (!fail && kc_kind == KC_SPEED) {
        const int h = (int)(kc_t * 100.0f + 0.5f);
        if (kc_best[kc_kind] <= 0 || h < kc_best[kc_kind]) { kc_best[kc_kind] = h; kc_newbest = true; }
        scores_save();
    }
}

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
    if (pl.crashed) { kc_finish(true, last_crash[0] ? last_crash : "Crashed"); return; }
    if (kc_kind == KC_SPEED) {
        if (kc_crossed(KC_SEQ[kc_gi], kc_prev, pl.pos)) {
            kc_gi++;
            snd_event(EV_CONFIRM, 1.0f);
            if (kc_gi >= 5) { kc_finish(false, ""); return; }
            char b[32]; snprintf(b, sizeof(b), "GATE %d / 5", kc_gi); kc_say(b);
        }
        if (kc_t > 240.0f) { kc_finish(true, "Out of time"); return; }
    }
    kc_prev = pl.pos;
}

// ---- Drawing ----

static void kc_time_text(char *b, size_t n, float s) { int m = (int)(s / 60.0f); snprintf(b, n, "%d:%05.2f", m, s - m * 60.0f); }

// In the world: a column of light over the next gate.
static void kc_draw_world(void) {
    if (game_mode != MODE_KX || kc_kind != KC_SPEED || kc_st == KS_DONE) return;
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
        vita2d_pgf_draw_textf(pgf, 540, 28, gold, 0.8f, "%s    GATE %d / 5", KC_NAMES[kc_kind], kc_gi < 5 ? kc_gi + 1 : 5);
        kc_time_text(b, sizeof(b), kc_t);
        int tw = vita2d_pgf_text_width(pgf, 1.5f, b);
        vita2d_pgf_draw_text(pgf, 480 - tw / 2, 70, white, 1.5f, b);
        if (kc_best[kc_kind] > 0) { char c[40]; kc_time_text(c, sizeof(c), kc_best[kc_kind] / 100.0f); vita2d_pgf_draw_textf(pgf, 540, 50, RGBA8(190, 205, 235, 255), 0.65f, "BEST %s", c); }
        if (kc_st == KS_READY) {
            snprintf(b, sizeof(b), "%d", (int)ceilf(kc_cd));
            tw = vita2d_pgf_text_width(pgf, 3.2f, b);
            vita2d_pgf_draw_text(pgf, 480 - tw / 2, 230, white, 3.2f, b);
            const char *h = "Arm the thrusters if you want them";
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
        vita2d_pgf_draw_text(pgf, 280, 150, RGBA8(255, 120, 100, 255), 1.4f, "NO TIME");
        vita2d_pgf_draw_text(pgf, 280, 190, pale, 0.95f, kc_why);
    } else {
        kc_time_text(b, sizeof(b), kc_result);
        vita2d_pgf_draw_text(pgf, 280, 160, white, 2.0f, b);
        const int m = kc_medal_for(kc_kind, (int)(kc_result * 100.0f + 0.5f));
        vita2d_pgf_draw_text(pgf, 280, 205, kc_medal_color(m), 1.3f, KC_MEDAL_NAME[m]);
        if (kc_newbest) vita2d_pgf_draw_text(pgf, 280, 238, gold, 0.95f, "NEW BEST");
        else if (kc_best[kc_kind] > 0) { char c[40]; kc_time_text(c, sizeof(c), kc_best[kc_kind] / 100.0f); vita2d_pgf_draw_textf(pgf, 280, 238, pale, 0.9f, "BEST %s", c); }
    }
    vita2d_pgf_draw_text(pgf, 280, 320, pale, 0.85f, "X: try again     O: menu");
}
