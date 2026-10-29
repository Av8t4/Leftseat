// Left Seat: the start menu and the landing result card.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* Start menu                                                          */
/* ------------------------------------------------------------------ */

// Result card shown after a landing challenge.
static void draw_result(vita2d_pgf *pgf) {
    unsigned int gold = RGBA8(255, 206, 130, 255), white = RGBA8(255, 255, 255, 255);
    unsigned int dim = RGBA8(170, 184, 214, 255), good = RGBA8(120, 230, 140, 255), bad = RGBA8(255, 120, 100, 255);
    vita2d_draw_rectangle(170, 26, 620, 356, gold);
    vita2d_draw_rectangle(174, 30, 612, 348, RGBA8(10, 22, 56, 255));
    char title[40];
    if (chr.crashed) snprintf(title, sizeof(title), ch_rings() ? "NO SCORE" : "NO LANDING");
    else             snprintf(title, sizeof(title), "%s RESULT", CK_NAMES[ch_kind]);
    vita2d_pgf_draw_text(pgf, 204, 72, gold, 1.2f, title);

    char big[16];
    snprintf(big, sizeof(big), "%d", chr.total);
    vita2d_pgf_draw_text(pgf, 204, 150, chr.crashed ? bad : white, 3.2f, big);
    const char *grade = chr.crashed ? chr.note
                      : ch_rings() ? (chr.total >= 90 ? "PERFECT RUN" : (chr.total >= 75 ? "GREAT RUN"
                      : (chr.total >= 50 ? "GOOD RUN" : (chr.total >= 25 ? "KEEP PRACTISING" : "THE RINGS WIN"))))
                      : (chr.total >= 90 ? "PERFECT LANDING" : (chr.total >= 75 ? "GREAT LANDING"
                      : (chr.total >= 60 ? "GOOD LANDING" : (chr.total >= 40 ? "ROUGH LANDING" : "HARD LANDING"))));
    vita2d_pgf_draw_text(pgf, 330, 112, chr.crashed ? bad : good, 1.2f, grade);
    vita2d_pgf_draw_textf(pgf, 330, 142, dim, 0.8f, "out of 100     BEST %d     ATTEMPTS %d", ch_best, ch_attempts);
    if (chr.new_best) vita2d_pgf_draw_text(pgf, 330, 168, gold, 0.9f, "NEW BEST!");
    if (ch_trip())
        vita2d_pgf_draw_textf(pgf, 204, 192, dim, 0.8f, "Trip time %d:%02d      Fuel used %.1f gal", (int)(chr.trip_s / 60.0f), (int)chr.trip_s % 60, chr.fuel_used);

    if (!chr.crashed && ch_rings()) {
        char v[2][56];
        snprintf(v[0], 56, "%d of %d flown through", chr.rings_hit, NRINGS);
        snprintf(v[1], 56, "%d:%02d (par %d:%02d)", (int)(chr.trip_s / 60.0f), (int)chr.trip_s % 60, (int)(chr.par_s / 60.0f), (int)chr.par_s % 60);
        static const char *label[2] = { "Rings", "Speed" };
        int pts[2] = { chr.p_rings, chr.p_time }, mx[2] = { 80, 20 };
        for (int i = 0; i < 2; i++) {
            float y = 214.0f + i * 30.0f;
            vita2d_pgf_draw_text(pgf, 204, y, white, 0.9f, label[i]);
            vita2d_pgf_draw_text(pgf, 340, y, dim, 0.85f, v[i]);
            char p[24];
            snprintf(p, sizeof(p), "%d / %d", pts[i], mx[i]);
            int tw = vita2d_pgf_text_width(pgf, 0.9f, p);
            vita2d_pgf_draw_text(pgf, 756 - tw, y, pts[i] >= mx[i] * 0.8f ? good : white, 0.9f, p);
        }
        vita2d_pgf_draw_text(pgf, 204, 300, dim, 0.85f, "Beat the par time for the full speed bonus.");
    } else if (!chr.crashed) {
        float d = chr.z + 1000.0f;
        char v[5][56];
        snprintf(v[0], 56, "%.1f m/s (%d fpm)", chr.sink, (int)(chr.sink * 196.85f));
        snprintf(v[1], 56, "%.0f m %s of the center line", fabsf(chr.x), chr.x < 0 ? "left" : "right");
        if (d < 0) snprintf(v[2], 56, "%.0f m SHORT of the runway", -d);
        else       snprintf(v[2], 56, "%.0f m past the threshold", d);
        snprintf(v[3], 56, "heading %+.0f, bank %+.0f", chr.hdg_err, chr.bank_deg);
        snprintf(v[4], 56, "%d bounce%s%s", chr.bounces, chr.bounces == 1 ? "" : "s", chr.nose_first ? ", nose wheel first" : "");
        static const char *label[5] = { "Sink rate", "Center line", "Touchdown", "Attitude", "Penalties" };
        int pts[5] = { chr.p_sink, chr.p_line, chr.p_point, chr.p_att, -chr.penalty };
        int mx[5]  = { 40, 25, 25, 10, 0 };
        for (int i = 0; i < 5; i++) {
            float y = 214.0f + i * 30.0f;
            vita2d_pgf_draw_text(pgf, 204, y, white, 0.9f, label[i]);
            vita2d_pgf_draw_text(pgf, 340, y, dim, 0.85f, v[i]);
            char p[24];
            if (mx[i] > 0) snprintf(p, sizeof(p), "%d / %d", pts[i], mx[i]); else snprintf(p, sizeof(p), "%d", pts[i]);
            int tw = vita2d_pgf_text_width(pgf, 0.9f, p);
            vita2d_pgf_draw_text(pgf, 756 - tw, y, (mx[i] > 0 && pts[i] >= mx[i] * 0.8f) ? good : (pts[i] < 0 ? bad : white), 0.9f, p);
        }
    } else {
        vita2d_pgf_draw_text(pgf, 204, 230, dim, 0.9f, ch_rings() ? "Try again: keep the aircraft low and smooth, and" : "Try again: aim for the aiming-point markings and");
        vita2d_pgf_draw_text(pgf, 204, 258, dim, 0.9f, ch_rings() ? "line up each ring a good way before you reach it." : "use the glide-slope lights beside the runway.");
    }
    vita2d_pgf_draw_text(pgf, 204, 366, dim, 0.85f, "X: try again          O: menu");
}

enum { ST_MENU = 0, ST_FLY = 1, ST_RESULT = 2 };
static int  game_state = ST_MENU;
static int  menu_sel;
static int  menu_popup = -1;       // item whose "not implemented" box is showing
static int  menu_popup_t;
static int  menu_quit_t;           // frames left to confirm quitting
static int  menu_stick;            // left-stick latch so one push moves one step
static int  flight_world = -1;   // which world the flight in progress is in (-1 = none)
static const char *MENU_ITEMS[9] = { "OPEN WORLD", "AIRPORT", "CHALLENGES", "HANGAR", "WEATHER", "TIME", "OPTIONS", "CONTROLS", "QUIT" };
static int  menu_chopen;            // the challenge list is showing
static int  menu_chsel;             // highlighted row in that list
static int  menu_ch_pick = -1;      // result: -1 = resume the running challenge, otherwise the kind to start
static bool ch_running(void) { return ch_state == CH_FLYING || ch_state == CH_TOUCHED; }
static int  menu_ap;                // airport picked for the open world (start and Triangle reset)
static int  menu_ac;                // aircraft picked in the hangar for the next flight
static int  ar_request;             // the hangar asked for the AR view (main.c starts it)
static int  menu_hopen, menu_hsel;  // the hangar list is showing, and its highlighted row
static float menu_hang;             // the turn of the aircraft in the hangar preview (radians)
static float menu_hscroll;          // how far the hangar list is scrolled (pixels)
static struct { int mode; int id; float sx, sy, lx, ly; bool moved; } hg_t;     // the finger that is down in the hangar
// Hangar layout: the list on the left, the picture and the buttons on the right.
#define HG_LX 44.0f
#define HG_LY 72.0f
#define HG_LW 400.0f
#define HG_ROW 62.0f
#define HG_LH 412.0f                     // height of the list window
#define HG_PX 464.0f                     // the picture
#define HG_PY 66.0f
#define HG_PW 452.0f
#define HG_PH 282.0f
#define HG_BY 462.0f                     // the buttons
#define HG_BH 46.0f
static int  menu_kopen;             // the controls window is showing
static int  menu_popen;             // a picker (like the hangar) is showing: 0 = none, else the menu row it belongs to (1 airport, 4 weather, 5 time, 6 sound)
static int  menu_psel;              // its highlighted row
static int  menu_pparent;           // the picker to go back to when this one closes (8 = options), or 0
static int  menu_psnd;              // the volume when the sound picker opened (Back restores it)
#define PK_ROW 56.0f

// ---- Options: one screen with every setting on it, each shown as a row of buttons ----
static int  menu_oopen;             // the options screen is showing
static int  menu_osel;              // its highlighted row
#define OP_N   5
#define OP_X   44.0f
#define OP_Y   76.0f
#define OP_ROW 66.0f
#define OP_CX  236.0f                // where the buttons of a row start, and their total width
#define OP_CW  320.0f
#define OP_CH  44.0f
#define OP_DY  424.0f                // the DONE button
#define OP_PX  584.0f                // the tilt test panel
#define OP_PY  76.0f
#define OP_PW  332.0f
#define OP_PH  330.0f
static const int OP_COUNT[OP_N] = { 4, 2, 2, 2, 3 };
static const char *const OP_NAME[OP_N] = { "SOUND", "CONTROLS", "GYRO ROLL", "GYRO PITCH", "SENSITIVITY" };
static const char *const OP_CHIP[OP_N][4] = { { "OFF", "LOW", "MEDIUM", "HIGH" }, { "STICKS", "GYRO", "", "" }, { "NORMAL", "REVERSED", "", "" },
                                              { "NORMAL", "REVERSED", "", "" }, { "LOW", "MEDIUM", "HIGH", "" } };
static int op_get(int r) { return r == 0 ? (snd_level & 3) : (r == 1 ? gy_mode : (r == 2 ? gy_rinv : (r == 3 ? gy_pinv : gy_sens))); }
static void op_set(int r, int v) {
    if (v < 0 || v >= OP_COUNT[r] || v == op_get(r)) return;
    if (r == 0) snd_level = v; else if (r == 1) { gy_mode = v; if (v) gyro_recenter(); } else if (r == 2) gy_rinv = v; else if (r == 3) gy_pinv = v; else gy_sens = v;
    scores_save(); snd_event(EV_CLICK, 1.0f);
}
static const char *op_desc(int r) {
    if (r == 0) return "Engine and effects volume";
    if (r == 1) return gy_mode ? "Tilt to fly (sticks still work)" : "The sticks fly the aircraft";
    if (r == 2) return gy_rinv ? "Tilting right rolls LEFT" : "Tilting right rolls right";
    if (r == 3) return gy_pinv ? "Top edge toward you is nose DOWN" : "Top edge toward you is nose up";
    return "How far you tilt for full control";
}
static int  menu_tod;              // time of day for the next flight: day, dusk or night
static int  menu_wx;               // starting weather: 0..WX_COUNT-1, or WX_COUNT for random
static bool wx_dirty;              // the weather choice changed since the flight started

static int menu_count(void) { return 9; }                                   // 0 open world, 1 airport, 2 challenges, 3 hangar, 4 weather, 5 time, 6 options, 7 controls, 8 quit

// The main menu is a big OPEN WORLD button over a grid of 4 x 2 tiles. GRID[g] is the menu item in grid slot g.
static const int MT_GRID[8] = { 1, 3, 4, 5, 2, 6, 7, 8 };
static int  menu_col;                // the grid column to come back to when moving between the big button and the grid
static int  menu_stickx;             // left-stick latch for left / right
static void mt_rect(int i, float *x, float *y, float *w, float *h) {
    if (i == 0) { *x = 40.0f; *y = 176.0f; *w = 460.0f; *h = 92.0f; return; }
    int g = 0; for (int k = 0; k < 8; k++) if (MT_GRID[k] == i) g = k;
    *x = 40.0f + (g % 4) * 225.0f; *y = 286.0f + (g / 4) * 108.0f; *w = 205.0f; *h = 94.0f;
}
static int mt_move(int sel, int dx, int dy) {
    if (sel == 0) { if (dy > 0) return MT_GRID[menu_col]; if (dy < 0) return MT_GRID[4 + menu_col]; return 0; }
    int g = 0; for (int k = 0; k < 8; k++) if (MT_GRID[k] == sel) g = k;
    int col = g % 4, row = g / 4;
    if (dx) { col = (col + dx + 4) % 4; menu_col = col; return MT_GRID[row * 4 + col]; }
    row += dy; menu_col = col;
    if (row < 0 || row > 1) return 0;
    return MT_GRID[row * 4 + col];
}

// ---- The pickers: Airport, Weather, Time and Sound open a list like the hangar's ----
static int pk_count(int kind) {
    if (kind == 1) return ap_nshown();
    if (kind == 4) return WX_COUNT + 1;
    if (kind == 5) return TOD_COUNT;
    if (kind == 7 || kind == 9 || kind == 10) return 2;
    if (kind == 8) return 5;
    if (kind == 11) return 3;
    return 4;
}
static int pk_idx(int kind) { return kind == 6 ? 0 : (kind == 7 ? 1 : (kind == 9 ? 2 : (kind == 10 ? 3 : 4))); }       // the row of the options list a sub-list belongs to
static int pk_ap(int i) { int n = 0; for (int k = 0; k < AP_COUNT; k++) if (ap_shown(k)) { if (n == i) return k; n++; } return 0; }
static int pk_cur(int kind) {
    if (kind == 1) { int n = 0; for (int k = 0; k < AP_COUNT; k++) if (ap_shown(k)) { if (k == menu_ap) return n; n++; } return 0; }
    if (kind == 4) return menu_wx;
    if (kind == 5) return menu_tod;
    if (kind == 7) return gy_mode;
    if (kind == 9) return gy_rinv;
    if (kind == 10) return gy_pinv;
    if (kind == 11) return gy_sens;
    if (kind == 8) return -1;
    return snd_level & 3;
}
static void pk_apply(int kind, int i) {
    if (kind == 1) menu_ap = pk_ap(i);
    else if (kind == 4) { menu_wx = i; wx_dirty = true; }
    else if (kind == 5) menu_tod = i;
    else if (kind == 7) gy_mode = i;
    else if (kind == 9) gy_rinv = i;
    else if (kind == 10) gy_pinv = i;
    else if (kind == 11) gy_sens = i;
    else if (kind == 6) snd_level = i & 3;
}
static const char *pk_name(int kind, int i) {
    if (kind == 1) return AIRPORTS[pk_ap(i)].name;
    if (kind == 4) return i == WX_COUNT ? "RANDOM" : WX_NAMES[i];
    if (kind == 5) return TOD_NAMES[i];
    if (kind == 7) return i ? "GYRO" : "STICKS";
    if (kind == 8) { static const char *const O[5] = { "SOUND", "CONTROLS", "GYRO ROLL", "GYRO PITCH", "GYRO SENSITIVITY" }; return O[i]; }
    if (kind == 9 || kind == 10) return i ? "REVERSED" : "NORMAL";
    if (kind == 11) { static const char *const N[3] = { "LOW", "MEDIUM", "HIGH" }; return N[i]; }
    return SND_NAMES[i & 3];
}
static const char *pk_l1(int kind, int i) {
    static const char *const W[WX_COUNT + 1] = { "Clear sky, light wind", "Some cloud and a steady wind", "Low solid cloud and haze", "Thick fog, almost no wind",
                                                 "Rain under heavy cloud", "Storm: heavy rain and violent gusts", "A different weather each time you start" };
    static const char *const T[TOD_COUNT] = { "Full daylight", "Low sun and long shadows", "Dark sky and lit runways" };
    static const char *const S[4] = { "No sound at all", "Quiet: engine and wind in the background", "A comfortable level", "Full volume" };
    if (kind == 1) { static char b[48]; const Airport *a = &AIRPORTS[pk_ap(i)]; snprintf(b, sizeof(b), "%s   %s", a->id, a->place); return b; }
    if (kind == 4) return W[i];
    if (kind == 5) return T[i];
    if (kind == 7) return i ? "Tilt the Vita like a control column" : "Left stick pitch and roll, right stick throttle and rudder";
    if (kind == 9) return i ? "Tilt right to roll left" : "Tilt right to roll right";
    if (kind == 10) return i ? "Top edge toward you is nose down" : "Top edge toward you is nose up";
    if (kind == 11) { static const char *const N[3] = { "Tilt about 40 degrees for full control", "Tilt about 28 degrees for full control", "Tilt about 18 degrees for full control" }; return N[i]; }
    if (kind == 8) {
        static char b[48];
        if (i == 0) snprintf(b, sizeof(b), "Volume: %s", SND_NAMES[snd_level & 3]);
        else if (i == 1) snprintf(b, sizeof(b), "Flight controls: %s", gy_mode ? "GYRO" : "STICKS");
        else if (i == 2) snprintf(b, sizeof(b), "Roll: %s", gy_rinv ? "REVERSED" : "NORMAL");
        else if (i == 3) snprintf(b, sizeof(b), "Pitch: %s", gy_pinv ? "REVERSED" : "NORMAL");
        else snprintf(b, sizeof(b), "Sensitivity: %s", gy_sens == 0 ? "LOW" : (gy_sens == 1 ? "MEDIUM" : "HIGH"));
        return b;
    }
    return S[i & 3];
}
static const char *pk_l2(int kind, int i) {
    static const char *const W[WX_COUNT + 1] = { "Best for learning. Unlimited visibility.", "Gusts of about 15 kt. Expect some bumps.", "Reduced visibility, moderate wind.",
                                                 "The runway appears late: fly a steady approach.", "Strong gusty wind, reduced visibility.", "Wind about 33 kt. For experts only.",
                                                 "It drifts and changes during the flight." };
    static const char *const T[TOD_COUNT] = { "Best visibility.", "Airfield lights come on. A good time to look at the sky.", "Runway lights and beacons. The spaceport glows." };
    static const char *const S[4] = { "Effects and engine are muted.", "Easy on the ears.", "The recommended setting.", "Everything at full strength." };
    if (kind == 1) {
        static char b[64]; const Airport *a = &AIRPORTS[pk_ap(i)];
        int dg = (int)(a->hdg * 57.29578f + 0.5f), rw = ((dg + 5) / 10) % 36; if (rw == 0) rw = 36;
        snprintf(b, sizeof(b), "Runway %02d/%02d   %d ft long   %s", rw, (rw + 18 > 36) ? rw - 18 : rw + 18, (int)(a->hl * 2.0f * 3.281f / 10.0f + 0.5f) * 10, pk_ap(i) == AP_PINECREST ? "grass" : "paved");
        return b;
    }
    if (kind == 4) return W[i];
    if (kind == 5) return T[i];
    if (kind == 7) return i ? "D-pad down + Circle centers the tilt in flight." : "The standard way to fly.";
    if (kind == 9 || kind == 10 || kind == 11) return "Tilt the Vita and watch the bars. D-pad down + Circle centers it.";
    if (kind == 8) { static const char *const D[5] = { "Set how loud the engine and effects are.", "Choose sticks or tilt to steer the aircraft.", "Which way the Vita rolls the aircraft.", "Which way the Vita pitches the aircraft.", "How far you have to tilt for full control." }; return D[i]; }
    return S[i & 3];
}

// Touch on the menus: a tap is turned into the button press it stands for, so the touch screen and the buttons share one set of rules.
// Main menu: tap a row (Airport, Weather, Time and Sound open a list like the hangar's).
// Challenge list: tap a row to start it, tap outside the list to go back. Controls window and message box: tap to close.
static unsigned int menu_touch_taps(void) {
    static struct { int on; int id; float sx, sy, lx, ly; bool moved; } mt;
    const ChTouch *t = &g_touch;
    unsigned int vp = 0;
    int pi = -1;
    for (int i = 0; i < t->n; i++) if (mt.on && t->id[i] == mt.id) pi = i;
    if (mt.on && pi < 0) {                                             // the finger came up
        mt.on = 0;
        if (!mt.moved) {
            float tx = mt.lx, ty = mt.ly;
            if (menu_kopen || menu_popup >= 0) vp = SCE_CTRL_CIRCLE;
            else if (menu_oopen) {
                if (ty >= OP_DY && ty <= OP_DY + HG_BH && tx >= HG_LX && tx <= HG_LX + 210.0f) vp = SCE_CTRL_CIRCLE;                     // DONE
                else for (int r = 0; r < OP_N; r++) {
                    float ry = OP_Y + r * OP_ROW;
                    if (ty < ry || ty >= ry + OP_ROW || tx < OP_X || tx > OP_CX + OP_CW + 10.0f) continue;
                    menu_osel = r;
                    int n = OP_COUNT[r];
                    float cw = (OP_CW - 8.0f * (n - 1)) / n;
                    for (int c = 0; c < n; c++) {
                        float cx0 = OP_CX + c * (cw + 8.0f);
                        if (tx >= cx0 - 4.0f && tx <= cx0 + cw + 4.0f) op_set(r, c);
                    }
                }
            }
            else if (menu_popen) {
                if (tx >= HG_LX && tx <= HG_LX + HG_LW && ty >= HG_LY && ty < HG_LY + pk_count(menu_popen) * PK_ROW) {
                    int row = (int)floorf((ty - HG_LY) / PK_ROW);
                    if (row == menu_psel) vp = SCE_CTRL_CROSS;                       // a second tap on the same row chooses it
                    else { menu_psel = row; snd_event(EV_CLICK, 1.0f); if (menu_popen == 6) snd_level = row & 3; }
                } else if (ty >= HG_BY && ty <= HG_BY + HG_BH && tx >= HG_PX && tx <= HG_PX + 210.0f) vp = SCE_CTRL_CROSS;
                else if (ty >= HG_BY && ty <= HG_BY + HG_BH && tx >= HG_PX + 230.0f) vp = SCE_CTRL_CIRCLE;
            } else if (menu_chopen) {
                bool run = ch_running();
                int n = CK_COUNT + (run ? 1 : 0);
                bool tight = (n >= 6);
                int rs = tight ? 48 : 58, rh = tight ? 44 : 52, top = tight ? 56 : 84, h = 70 + n * rs + 36;
                if (tx < 150.0f || tx > 810.0f || ty < top || ty > top + h) vp = SCE_CTRL_CIRCLE;
                else for (int i = 0; i < n; i++) {
                    int ry = top + 60 + i * rs;
                    if (tx >= 172.0f && tx <= 788.0f && ty >= ry && ty < ry + rh) { menu_chsel = i; vp = SCE_CTRL_CROSS; }
                }
            } else {
                for (int i = 0; i < menu_count(); i++) {
                    float x, y, w, h; mt_rect(i, &x, &y, &w, &h);
                    if (tx < x - 6.0f || tx > x + w + 6.0f || ty < y - 6.0f || ty > y + h + 6.0f) continue;
                    menu_sel = i; vp = SCE_CTRL_CROSS;
                }
            }
        }
    }
    if (!mt.on && t->n > 0) { mt.on = 1; mt.id = t->id[0]; mt.sx = mt.lx = t->x[0]; mt.sy = mt.ly = t->y[0]; mt.moved = false; pi = 0; }
    if (mt.on && pi >= 0) {
        mt.lx = t->x[pi]; mt.ly = t->y[pi];
        if (fabsf(mt.lx - mt.sx) + fabsf(mt.ly - mt.sy) > 14.0f) mt.moved = true;
    }
    return vp;
}

// Returns 0 = nothing, 1 = start / resume the flight, 2 = quit the game.
static int menu_update(const SceCtrlData *pad, unsigned int pressed) {
    if (menu_quit_t > 0) menu_quit_t--;
    {
        unsigned int vp = menu_touch_taps();
        if (!menu_hopen) pressed |= vp;
    }
    if (menu_hopen) {                                         // hangar: choose an aircraft (D-pad / stick, or touch)
        int mv = 0;
        if (pressed & SCE_CTRL_UP)   mv = -1;
        if (pressed & SCE_CTRL_DOWN) mv = 1;
        int sy = pad->ly < 64 ? -1 : (pad->ly > 192 ? 1 : 0);
        if (sy != 0 && menu_stick == 0) mv = sy;
        menu_stick = sy;
        float hmax = fmaxf(0.0f, AC_COUNT * HG_ROW - HG_LH);
        if (mv) {
            menu_hsel = (menu_hsel + mv + AC_COUNT) % AC_COUNT; snd_event(EV_CLICK, 1.0f);
            float top = menu_hsel * HG_ROW;                   // keep the highlighted row on show
            if (top < menu_hscroll) menu_hscroll = top;
            if (top + HG_ROW > menu_hscroll + HG_LH) menu_hscroll = top + HG_ROW - HG_LH;
        }
        if (pad->buttons & SCE_CTRL_LEFT)  menu_hang -= 0.035f;       // turn the picture
        if (pad->buttons & SCE_CTRL_RIGHT) menu_hang += 0.035f;
        if (pressed & SCE_CTRL_SQUARE) { ar_request = 1; snd_event(EV_CONFIRM, 1.0f); }
        if (pressed & SCE_CTRL_CIRCLE) { menu_hopen = 0; snd_event(EV_CLICK, 1.0f); }
        if (pressed & SCE_CTRL_CROSS)  { menu_ac = menu_hsel; menu_hopen = 0; snd_event(EV_CONFIRM, 1.0f); }
        // Touch: tap a row to look at that aircraft, drag the list to scroll it, drag the picture to turn it, tap the buttons.
        const ChTouch *t = &g_touch;
        int pi = -1;
        for (int i = 0; i < t->n; i++) if (hg_t.mode && t->id[i] == hg_t.id) pi = i;
        bool turning = false;
        if (hg_t.mode && pi < 0) {                                     // the finger came up
            if (!hg_t.moved) {
                if (hg_t.mode == 1) {
                    int row = (int)floorf((hg_t.ly - HG_LY + menu_hscroll) / HG_ROW);
                    if (row >= 0 && row < AC_COUNT) { menu_hsel = row; snd_event(EV_CLICK, 1.0f); }
                } else if (hg_t.mode == 3 && hg_t.lx >= HG_PX && hg_t.lx <= HG_PX + 130.0f && hg_t.ly >= HG_BY && hg_t.ly <= HG_BY + HG_BH) {
                    menu_ac = menu_hsel; menu_hopen = 0; snd_event(EV_CONFIRM, 1.0f);
                } else if (hg_t.mode == 3 && hg_t.lx >= HG_PX + 140.0f && hg_t.lx <= HG_PX + 290.0f && hg_t.ly >= HG_BY && hg_t.ly <= HG_BY + HG_BH) {
                    ar_request = 1; snd_event(EV_CONFIRM, 1.0f);
                } else if (hg_t.mode == 3 && hg_t.lx >= HG_PX + 300.0f && hg_t.ly >= HG_BY && hg_t.ly <= HG_BY + HG_BH) {
                    menu_hopen = 0; snd_event(EV_CLICK, 1.0f);
                }
            }
            hg_t.mode = 0;
        }
        if (!hg_t.mode && t->n > 0 && menu_hopen) {                    // a finger went down
            float fx = t->x[0], fy = t->y[0];
            hg_t.id = t->id[0]; hg_t.sx = hg_t.lx = fx; hg_t.sy = hg_t.ly = fy; hg_t.moved = false;
            if (fx >= HG_LX && fx <= HG_LX + HG_LW && fy >= HG_LY && fy <= HG_LY + HG_LH) hg_t.mode = 1;
            else if (fx >= HG_PX && fx <= HG_PX + HG_PW && fy >= HG_PY && fy <= HG_PY + HG_PH) hg_t.mode = 2;
            else if (fy >= HG_BY && fy <= HG_BY + HG_BH) hg_t.mode = 3;
            pi = 0;
        }
        if (hg_t.mode && pi >= 0) {
            float fx = t->x[pi], fy = t->y[pi];
            if (fabsf(fx - hg_t.sx) + fabsf(fy - hg_t.sy) > 10.0f) hg_t.moved = true;
            if (hg_t.mode == 1 && hg_t.moved) menu_hscroll = clampf(menu_hscroll - (fy - hg_t.ly), 0.0f, hmax);
            if (hg_t.mode == 2) { menu_hang += (fx - hg_t.lx) * 0.012f; turning = hg_t.moved; }
            hg_t.lx = fx; hg_t.ly = fy;
        }
        if (!turning && !(pad->buttons & (SCE_CTRL_LEFT | SCE_CTRL_RIGHT))) menu_hang += 0.008f;     // a slow turntable
        return 0;
    }
    if (menu_oopen) {                                         // options: Up / Down choose a row, Left / Right change it
        int dy = 0, dx = 0;
        if (pressed & SCE_CTRL_UP)    dy = -1;
        if (pressed & SCE_CTRL_DOWN)  dy = 1;
        if (pressed & SCE_CTRL_LEFT)  dx = -1;
        if (pressed & SCE_CTRL_RIGHT) dx = 1;
        int sy = pad->ly < 64 ? -1 : (pad->ly > 192 ? 1 : 0), sx = pad->lx < 64 ? -1 : (pad->lx > 192 ? 1 : 0);
        if (sy != 0 && menu_stick == 0) dy = sy;
        if (sx != 0 && menu_stickx == 0) dx = sx;
        menu_stick = sy; menu_stickx = sx;
        if (dy) { menu_osel = (menu_osel + dy + OP_N) % OP_N; snd_event(EV_CLICK, 1.0f); }
        if (dx) op_set(menu_osel, op_get(menu_osel) + dx);
        if (pressed & SCE_CTRL_CROSS) { int v = op_get(menu_osel) + 1; op_set(menu_osel, v >= OP_COUNT[menu_osel] ? 0 : v); }     // X steps on, round to the start
        if (pressed & (SCE_CTRL_CIRCLE | SCE_CTRL_START)) { menu_oopen = 0; snd_event(EV_CLICK, 1.0f); }
        return 0;
    }
    if (menu_popen) {                                         // a picker: choose with D-pad / stick or touch
        int n = pk_count(menu_popen), mv = 0;
        if (pressed & (SCE_CTRL_UP | SCE_CTRL_LEFT))    mv = -1;
        if (pressed & (SCE_CTRL_DOWN | SCE_CTRL_RIGHT)) mv = 1;
        int sy = pad->ly < 64 ? -1 : (pad->ly > 192 ? 1 : 0);
        if (sy != 0 && menu_stick == 0) mv = sy;
        menu_stick = sy;
        if (mv) {
            menu_psel = (menu_psel + mv + n) % n; snd_event(EV_CLICK, 1.0f);
            if (menu_popen == 6) snd_level = menu_psel & 3;                 // hear the volume as you choose it
        }
        if (pressed & SCE_CTRL_CIRCLE) {
            if (menu_popen == 6) snd_level = menu_psnd;
            if (menu_pparent) { menu_psel = pk_idx(menu_popen); menu_popen = menu_pparent; menu_pparent = 0; }
            else menu_popen = 0;
            snd_event(EV_CLICK, 1.0f);
        } else if (pressed & SCE_CTRL_CROSS) {
            if (menu_popen == 8) {                                                   // options: open the sound, controls or one of the gyro lists
                static const int SUB[5] = { 6, 7, 9, 10, 11 };
                menu_pparent = 8; menu_popen = SUB[menu_psel]; menu_psel = pk_cur(menu_popen); menu_psnd = snd_level & 3;
                if (menu_popen >= 7) gyro_recenter();
                snd_event(EV_CONFIRM, 1.0f);
            } else {
                if (menu_popen == 6) snd_level = menu_psel & 3; else pk_apply(menu_popen, menu_psel);
                if (menu_popen == 6 || menu_popen >= 7) scores_save();
                if (menu_popen == 7 && menu_psel == 1) gyro_recenter();
                snd_event(EV_CONFIRM, 1.0f);
                if (menu_pparent) { menu_psel = pk_idx(menu_popen); menu_popen = menu_pparent; menu_pparent = 0; }
                else menu_popen = 0;
            }
        }
        return 0;
    }
    if (menu_kopen) {                                         // controls window: X or O closes it
        if (pressed & (SCE_CTRL_CIRCLE | SCE_CTRL_CROSS)) { menu_kopen = 0; snd_event(EV_CLICK, 1.0f); }
        return 0;
    }
    if (menu_chopen) {                                        // challenge list
        int n = CK_COUNT + (ch_running() ? 1 : 0), mv = 0;
        if (pressed & SCE_CTRL_UP)   mv = -1;
        if (pressed & SCE_CTRL_DOWN) mv = 1;
        int sy = pad->ly < 64 ? -1 : (pad->ly > 192 ? 1 : 0);
        if (sy != 0 && menu_stick == 0) mv = sy;
        menu_stick = sy;
        if (mv) { menu_chsel = (menu_chsel + mv + n) % n; snd_event(EV_CLICK, 1.0f); }
        if (pressed & SCE_CTRL_CIRCLE) { menu_chopen = 0; snd_event(EV_CLICK, 1.0f); }
        if (pressed & SCE_CTRL_CROSS) {
            snd_event(EV_CONFIRM, 1.0f);
            int k = menu_chsel - (ch_running() ? 1 : 0);
            menu_chopen = 0;
            menu_ch_pick = k < 0 ? -1 : k;
            return 4;
        }
        return 0;
    }
    if (menu_popup >= 0) {                                    // message box: X / O (or a timeout) closes it
        if (menu_popup_t > 0) menu_popup_t--;
        if ((pressed & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE)) || menu_popup_t == 0) menu_popup = -1;
        return 0;
    }
    int dy = 0, dx = 0;
    if (pressed & SCE_CTRL_UP)    dy = -1;
    if (pressed & SCE_CTRL_DOWN)  dy = 1;
    if (pressed & SCE_CTRL_LEFT)  dx = -1;
    if (pressed & SCE_CTRL_RIGHT) dx = 1;
    int sy = pad->ly < 64 ? -1 : (pad->ly > 192 ? 1 : 0), sx = pad->lx < 64 ? -1 : (pad->lx > 192 ? 1 : 0);
    if (sy != 0 && menu_stick == 0) dy = sy;
    if (sx != 0 && menu_stickx == 0) dx = sx;
    menu_stick = sy; menu_stickx = sx;
    if (dy || dx) { int ns = mt_move(menu_sel, dx, dy); if (ns != menu_sel) { menu_sel = ns; snd_event(EV_CLICK, 1.0f); } }

    if (pressed & SCE_CTRL_CROSS) {
        if (menu_sel == 8) {                                  // quit: press twice
            if (menu_quit_t > 0) return 2;
            menu_quit_t = 150; snd_event(EV_CLICK, 1.0f); return 0;
        }
        snd_event(EV_CONFIRM, 1.0f);
        if (menu_sel == 1 || menu_sel == 4 || menu_sel == 5 || menu_sel == 6) {   // airport, weather, time, options: open the list
            if (menu_sel == 6) { menu_oopen = 1; menu_osel = 0; return 0; }
            menu_popen = menu_sel; menu_psel = pk_cur(menu_popen); menu_psnd = snd_level & 3; menu_pparent = 0;
            return 0;
        }
        if (menu_sel == 0) return 1;
        if (menu_sel == 1) {                                  // X also steps through the airports
            menu_ap = ap_next(menu_ap, 1);
            return 0;
        }
        if (menu_sel == 2) {                                  // challenges: open the list
            menu_chopen = 1;
            menu_chsel = ch_running() ? 0 : ch_kind;
            return 0;
        }
        if (menu_sel == 3) {                                  // hangar: open the list of aircraft
            menu_hopen = 1;
            menu_hsel = menu_ac;
            menu_hang = 0.6f; menu_hscroll = 0.0f; hg_t.mode = 0;
            return 0;
        }
        if (menu_sel == 7) {                                  // controls: open the window
            menu_kopen = 1;
            return 0;
        }
        if (menu_sel == 4) {                                  // X also steps through the weather
            menu_wx = (menu_wx + 1) % (WX_COUNT + 1);
            wx_dirty = true;
            return 0;
        }
        if (menu_sel == 5) {                                  // X also steps through the time of day
            menu_tod = (menu_tod + 1) % TOD_COUNT;
            return 0;
        }
        menu_popup = menu_sel;                                // everything else is a placeholder for now
        menu_popup_t = 240;
    }
    if (pressed & SCE_CTRL_START) {
        if (menu_quit_t > 0) return 2;
        menu_quit_t = 150;
    }
    return 0;
}

static P2 rot_pt(float x, float y, float cx, float cy, float s, float ca, float sa) {
    P2 p = { cx + (x * ca - y * sa) * s, cy + (x * sa + y * ca) * s };
    return p;
}

// Top-down aircraft silhouette (nose up at angle 0), drawn with simple polygons.
static void draw_menu_plane(float cx, float cy, float s, float ang, float spin, int shadow) {
    float ca = cosf(ang), sa = sinf(ang);
    unsigned int white = shadow ? RGBA8(6, 14, 40, 255) : RGBA8(246, 247, 250, 255);
    unsigned int red   = shadow ? RGBA8(6, 14, 40, 255) : RGBA8(204, 36, 40, 255);
    unsigned int glass = shadow ? RGBA8(6, 14, 40, 255) : RGBA8(30, 62, 118, 255);
    unsigned int dark  = shadow ? RGBA8(6, 14, 40, 255) : RGBA8(40, 42, 52, 255);
    static const float TAIL[4][2] = { {-.40f, .30f}, {.40f, .30f}, {.37f, .40f}, {-.37f, .40f} };
    static const float FUS[13][2] = { {0, -.50f}, {.07f, -.44f}, {.095f, -.30f}, {.10f, -.12f}, {.09f, .06f},
        {.045f, .26f}, {.022f, .42f}, {-.022f, .42f}, {-.045f, .26f}, {-.09f, .06f}, {-.10f, -.12f},
        {-.095f, -.30f}, {-.07f, -.44f} };
    static const float WING[4][2] = { {-.88f, -.16f}, {-.12f, -.16f}, {-.12f, .06f}, {-.88f, -.07f} };
    static const float TIP[4][2]  = { {-.88f, -.16f}, {-.74f, -.15f}, {-.74f, -.08f}, {-.88f, -.07f} };
    static const float GLS[4][2]  = { {-.11f, -.18f}, {.11f, -.18f}, {.075f, -.31f}, {-.075f, -.31f} };
    P2 q[13];
    for (int i = 0; i < 4; i++) q[i] = rot_pt(TAIL[i][0], TAIL[i][1], cx, cy, s, ca, sa);
    fill_poly_panel(q, 4, white);
    for (int sx = -1; sx <= 1; sx += 2) {
        for (int i = 0; i < 4; i++) q[i] = rot_pt(sx * WING[i][0], WING[i][1], cx, cy, s, ca, sa);
        fill_poly_panel(q, 4, white);
        for (int i = 0; i < 4; i++) q[i] = rot_pt(sx * TIP[i][0], TIP[i][1], cx, cy, s, ca, sa);
        fill_poly_panel(q, 4, red);
    }
    for (int i = 0; i < 13; i++) q[i] = rot_pt(FUS[i][0], FUS[i][1], cx, cy, s, ca, sa);
    fill_poly_panel(q, 13, white);
    for (int i = 0; i < 4; i++) q[i] = rot_pt(GLS[i][0], GLS[i][1], cx, cy, s, ca, sa);
    fill_poly_panel(q, 4, glass);
    float L = 0.30f * fabsf(cosf(spin));                       // propeller seen edge-on, spinning
    P2 pr[4] = { rot_pt(-L, -.545f, cx, cy, s, ca, sa), rot_pt(L, -.545f, cx, cy, s, ca, sa),
                 rot_pt(L, -.515f, cx, cy, s, ca, sa), rot_pt(-L, -.515f, cx, cy, s, ca, sa) };
    fill_poly_panel(pr, 4, dark);
}

// Block-letter title built from plain rectangles, so it stays crisp at any size
// (scaled PGF text smears into boxes at this size).
static void draw_title_block(float x, float y, unsigned int col) {
    const float w = 36, h = 52, t = 10, gap = 8;
    const char *s = "LEFT SEAT";
    for (; *s; s++) {
        char c = *s;
        if (c == ' ') { x += 28; continue; }
        float m = y + h / 2 - t / 2;
        switch (c) {
        case 'L': vita2d_draw_rectangle(x, y, t, h, col); vita2d_draw_rectangle(x, y + h - t, w, t, col); break;
        case 'E': vita2d_draw_rectangle(x, y, t, h, col); vita2d_draw_rectangle(x, y, w, t, col);
                  vita2d_draw_rectangle(x, m, w - 4, t, col); vita2d_draw_rectangle(x, y + h - t, w, t, col); break;
        case 'F': vita2d_draw_rectangle(x, y, t, h, col); vita2d_draw_rectangle(x, y, w, t, col);
                  vita2d_draw_rectangle(x, m, w - 4, t, col); break;
        case 'T': vita2d_draw_rectangle(x, y, w, t, col); vita2d_draw_rectangle(x + (w - t) / 2, y, t, h, col); break;
        case 'S': vita2d_draw_rectangle(x, y, w, t, col); vita2d_draw_rectangle(x, y, t, h / 2 + t / 2, col);
                  vita2d_draw_rectangle(x, m, w, t, col); vita2d_draw_rectangle(x + w - t, m, t, h / 2 + t / 2, col);
                  vita2d_draw_rectangle(x, y + h - t, w, t, col); break;
        case 'A': vita2d_draw_rectangle(x, y, t, h, col); vita2d_draw_rectangle(x + w - t, y, t, h, col);
                  vita2d_draw_rectangle(x, y, w, t, col); vita2d_draw_rectangle(x, m, w, t, col); break;
        }
        x += w + gap;
    }
}

// Shadow and model of the aircraft in `spec`, standing at the origin with the current camera: shared by the hangar picture and the AR view.
static void hangar_draw_model(void) {
    draw_plane_shadow();
    npf = 0; pf_last = -1;
    build_plane();
    int ni = 0;
    for (int k = 0; k < npf; k++) {
        const PFace *pf = &pfaces[k];
        if (pf->isdecal) continue;
        if (!pf->twosided && vdot(pf->nrm, vsub(cam.pos, pf->c)) <= 0.0f) continue;
        V3 d = vsub(pf->c, cam.pos);
        float d2 = vdot(d, d);
        if (pf->twosided && d2 > 1e-6f) { float dl = sqrtf(d2); dl -= 0.3f * (-vdot(pf->nrm, d) / dl); d2 = dl * dl; }
        items[ni].dist = d2; items[ni].idx = MAX_FACES + k; ni++;
    }
    sort_items(ni);
    for (int i = 0; i < ni; i++) {
        const PFace *pf = &pfaces[items[i].idx - MAX_FACES];
        if (pf->emit) draw_world_poly_emit(pf->p, pf->n, pf->color); else draw_world_poly(pf->p, pf->n, pf->color);
        for (int d = 1; d <= pf->ndecals; d++) {
            const PFace *dc = &pfaces[items[i].idx - MAX_FACES + d];
            if (pf->twosided && !dc->emit && vdot(dc->nrm, vsub(cam.pos, dc->c)) <= 0.0f) continue;
            if (dc->emit) draw_world_poly_emit(dc->p, dc->n, dc->color); else draw_world_poly(dc->p, dc->n, dc->color);
        }
    }
    flush_batch();
    npf = 0; pf_last = -1;
}

// The real 3D model of an aircraft, standing on a round pad, turned by `ang`, drawn in the picture box of the hangar. It borrows the
// flight's camera and aircraft state for the moment and gives them back, so it is safe to call from the menu in the middle of a flight.
static void draw_hangar_preview(int ac, float ang) {
    Plane sv_pl = pl; Camera sv_cam = cam; const Spec *sv_spec = spec;
    int sv_ac = ac_flying, sv_tod = tod; float sv_f = g_focal, sv_n = g_near, sv_vis = fog_vis, sv_ph = prop_phase;
    spec = &SPECS[ac]; ac_flying = ac; tod = TOD_DAY; fog_vis = 35000.0f;
    memset(&pl, 0, sizeof pl);
    pl.pos = v3(0.0f, 0.0f, 0.0f); pl.gear = 1.0f; pl.gear_cmd = true; pl.on_ground = true; pl.rpm = 700.0f;
    set_axes(ang, 0.0f, 0.0f);
    g_focal = 520.0f; g_near = 0.5f;
    float rm = fmaxf(spec->tip_x, fmaxf(-spec->tail_z, 4.5f)), D = 3.3f * rm;
    V3 pivot = v3(0.0f, spec->piv_y * 0.8f, 0.0f);
    look_at(vadd(pivot, v3(0.0f, D * 0.30f, -D * 0.95f)), pivot);
    float cxp = HG_PX + HG_PW * 0.5f, cyp = HG_PY + HG_PH * 0.52f;       // where the aircraft should appear on the screen
    cam.pos = vsub(cam.pos, vscale(cam.r, (cxp - CX) * D / g_focal));
    cam.pos = vadd(cam.pos, vscale(cam.u, (cyp - CY) * D / g_focal));
    {                                                                      // the pad
        V3 o[8], q[8];
        for (int i = 0; i < 8; i++) {
            float a = (i + 0.5f) * PI_F / 4.0f;
            o[i] = v3(cosf(a) * rm * 0.85f, 0.0f, sinf(a) * rm * 0.85f);
            q[i] = v3(cosf(a) * rm * 0.79f, 0.02f, sinf(a) * rm * 0.79f);
        }
        draw_world_poly(o, 8, RGBA8(52, 56, 66, 255));
        draw_world_poly(q, 8, RGBA8(112, 116, 126, 255));
    }
    hangar_draw_model();
    pl = sv_pl; cam = sv_cam; spec = sv_spec; ac_flying = sv_ac; tod = sv_tod; g_focal = sv_f; g_near = sv_n; fog_vis = sv_vis; prop_phase = sv_ph;
}

static void draw_menu(vita2d_pgf *pgf) {
    float t = frame_count / 60.0f;
    unsigned int gold = RGBA8(255, 206, 130, 255), white = RGBA8(255, 255, 255, 255);

    // Sky.
    for (int i = 0; i < 34; i++) {
        float u = i / 33.0f;
        int r, g, b;
        if (u < 0.55f) { float k = u / 0.55f;          r = (int)(10 + 50 * k);  g = (int)(24 + 86 * k);  b = (int)(70 + 120 * k); }
        else           { float k = (u - 0.55f) / 0.45f; r = (int)(60 + 195 * k); g = (int)(110 + 60 * k); b = (int)(190 - 80 * k); }
        vita2d_draw_rectangle(0, i * 16, SCREEN_W, 17, RGBA8(r, g, b, 255));
    }
    vita2d_draw_fill_circle(760, 478, 50, RGBA8(255, 228, 170, 255));       // low sun
    // Mountains and ground.
    P2 m1[3] = { {-40, 520}, {150, 372}, {340, 520} };      fill_poly_panel(m1, 3, RGBA8(46, 62, 120, 255));
    P2 m2[3] = { {230, 520}, {430, 396}, {640, 520} };      fill_poly_panel(m2, 3, RGBA8(52, 70, 128, 255));
    P2 m3[3] = { {560, 520}, {800, 380}, {1010, 520} };     fill_poly_panel(m3, 3, RGBA8(40, 56, 112, 255));
    P2 h1[3] = { {-100, 560}, {130, 450}, {380, 560} };     fill_poly_panel(h1, 3, RGBA8(22, 40, 84, 255));
    P2 h2[3] = { {300, 560}, {560, 458}, {820, 560} };      fill_poly_panel(h2, 3, RGBA8(18, 34, 74, 255));
    vita2d_draw_rectangle(0, 506, SCREEN_W, 38, RGBA8(12, 26, 56, 255));

    // Compass with a slowly turning aircraft.
    const float cx = 730.0f, cy = 148.0f, R = 108.0f;
    vita2d_draw_fill_circle(cx, cy, R + 5, gold);
    vita2d_draw_fill_circle(cx, cy, R, RGBA8(12, 26, 62, 255));
    for (int k = 0; k < 72; k++) {
        float a = k * 5.0f * PI_F / 180.0f;
        int major = (k % 18 == 0), mid = (k % 6 == 0);
        float r0 = R * (major ? 0.84f : (mid ? 0.88f : 0.92f)), r1 = R * 0.97f;
        vita2d_draw_line(cx + sinf(a) * r0, cy - cosf(a) * r0, cx + sinf(a) * r1, cy - cosf(a) * r1,
                         major ? RGBA8(255, 220, 160, 255) : RGBA8(130, 160, 215, 255));
    }
    static const char *CARD[4] = { "N", "E", "S", "W" };
    for (int k = 0; k < 4; k++) {
        float a = k * PI_F / 2.0f, rr = R * 0.70f;
        int tw = vita2d_pgf_text_width(pgf, 0.95f, CARD[k]);
        vita2d_pgf_draw_text(pgf, cx + sinf(a) * rr - tw / 2, cy - cosf(a) * rr + 7,
                             k == 0 ? RGBA8(255, 90, 80, 255) : RGBA8(235, 240, 255, 255), 0.95f, CARD[k]);
    }
    float ang = t * 0.35f;
    draw_menu_plane(cx + 5, cy + 6, 78.0f, ang, t * 30.0f, 1);               // shadow
    draw_menu_plane(cx, cy, 78.0f, ang, t * 30.0f, 0);

    // Title.
    draw_title_block(63, 73, RGBA8(8, 16, 44, 255));
    draw_title_block(60, 70, white);
    vita2d_pgf_draw_text(pgf, 64, 160, gold, 1.05f, "A KYLE KNOWLES FLIGHT SIMULATOR");

    // The big OPEN WORLD button and the tiles. A tile is easy to hit with a thumb.
    for (int i = 0; i < menu_count(); i++) {
        float x, y, w, h; mt_rect(i, &x, &y, &w, &h);
        bool sel = (i == menu_sel);
        unsigned int base = (i == 0) ? RGBA8(18, 52, 110, 255) : (i == 8 ? RGBA8(52, 22, 40, 255) : RGBA8(12, 26, 62, 255));
        unsigned int hi = (i == 0) ? RGBA8(30, 76, 150, 255) : (i == 8 ? RGBA8(86, 34, 58, 255) : RGBA8(24, 50, 112, 255));
        vita2d_draw_rectangle(x - 2, y - 2, w + 4, h + 4, sel ? gold : RGBA8(70, 94, 150, 255));
        vita2d_draw_rectangle(x, y, w, h, sel ? hi : base);
        unsigned int lc = sel ? white : RGBA8(176, 192, 228, 255), vc = sel ? gold : RGBA8(130, 152, 200, 255);
        char vb[40] = "", sb[48] = "";
        const char *label = MENU_ITEMS[i];
        if (i == 0) {
            snprintf(sb, sizeof(sb), "Fly from %s", AIRPORTS[menu_ap].name);
            vita2d_pgf_draw_text(pgf, x + 24, y + 52, lc, 1.9f, label);
            vita2d_pgf_draw_text(pgf, x + 26, y + 78, vc, 0.85f, sb);
            if (flight_world == 0 && game_mode == MODE_FREE && ac_flying == menu_ac && start_ap == menu_ap)
                vita2d_pgf_draw_text(pgf, x + w - 24 - vita2d_pgf_text_width(pgf, 1.0f, "RESUME"), y + 52, gold, 1.0f, "RESUME");
            continue;
        }
        if (i == 1) { snprintf(vb, sizeof(vb), "%s", AIRPORTS[menu_ap].id); snprintf(sb, sizeof(sb), "%s", AIRPORTS[menu_ap].place); }
        else if (i == 2) { bool run = (ch_state == CH_FLYING || ch_state == CH_TOUCHED); snprintf(vb, sizeof(vb), run ? "RESUME" : "%d TO FLY", CK_COUNT); }
        else if (i == 3) { snprintf(vb, sizeof(vb), "%s", SPECS[menu_ac].tag); for (char *q = vb; *q; q++) if (*q >= 'a' && *q <= 'z') *q -= 32; }
        else if (i == 4) snprintf(vb, sizeof(vb), "%s", menu_wx == WX_COUNT ? "RANDOM" : WX_NAMES[menu_wx]);
        else if (i == 5) snprintf(vb, sizeof(vb), "%s", TOD_NAMES[menu_tod]);
        else if (i == 6) { snprintf(vb, sizeof(vb), "SOUND %s", SND_NAMES[snd_level & 3]); snprintf(sb, sizeof(sb), "Controls: %s", gy_mode ? "GYRO" : "STICKS"); }
        else if (i == 7) snprintf(vb, sizeof(vb), "SHOW KEYS");
        else if (i == 8) snprintf(vb, sizeof(vb), "%s", menu_quit_t > 0 ? "TAP AGAIN" : "");
        vita2d_pgf_draw_text(pgf, x + 16, y + 34, lc, 1.25f, label);
        vita2d_pgf_draw_text(pgf, x + 16, y + 64, vc, 0.95f, vb);
        if (sb[0]) vita2d_pgf_draw_text(pgf, x + 16, y + 84, RGBA8(130, 152, 200, 255), 0.66f, sb);
    }

    // Footer.
    vita2d_pgf_draw_text(pgf, 40, 528, RGBA8(190, 205, 235, 255), 0.8f, "Touch: tap a tile     D-pad / left stick: move     X: select");
    if (menu_quit_t > 0) vita2d_pgf_draw_text(pgf, 700, 528, gold, 0.8f, "Press QUIT again to quit");

    // The hangar: the list of aircraft on the left, a turning picture of the chosen one on the right.
    if (menu_hopen) {
        const unsigned int card = RGBA8(10, 22, 56, 255), dimc = RGBA8(150, 172, 215, 255), pale = RGBA8(190, 205, 235, 255);
        vita2d_draw_rectangle(20, 12, 920, 520, gold);
        vita2d_draw_rectangle(24, 16, 912, 512, card);
        for (int i = 0; i < 12; i++) {                                    // the picture's sky, and the ground under it
            float u = i / 11.0f;
            vita2d_draw_rectangle(HG_PX, HG_PY + i * (HG_PH * 0.56f / 12.0f), HG_PW, HG_PH * 0.56f / 12.0f + 1.0f, RGBA8((int)(58 + 120 * u), (int)(104 + 100 * u), (int)(176 + 56 * u), 255));
        }
        vita2d_draw_rectangle(HG_PX, HG_PY + HG_PH * 0.56f, HG_PW, HG_PH * 0.44f, RGBA8(64, 92, 66, 255));
        draw_hangar_preview(menu_hsel, menu_hang);
        // Hide anything that spilled outside the picture, then frame it.
        vita2d_draw_rectangle(24, 16, 912, HG_PY - 16, card);
        vita2d_draw_rectangle(24, HG_PY + HG_PH, 912, 528 - HG_PY - HG_PH, card);
        vita2d_draw_rectangle(24, 16, HG_PX - 24, 512, card);
        vita2d_draw_rectangle(HG_PX + HG_PW, 16, 936 - HG_PX - HG_PW, 512, card);
        vita2d_draw_rectangle(HG_PX - 2, HG_PY - 2, HG_PW + 4, 2, gold); vita2d_draw_rectangle(HG_PX - 2, HG_PY + HG_PH, HG_PW + 4, 2, gold);
        vita2d_draw_rectangle(HG_PX - 2, HG_PY - 2, 2, HG_PH + 4, gold); vita2d_draw_rectangle(HG_PX + HG_PW, HG_PY - 2, 2, HG_PH + 4, gold);
        vita2d_pgf_draw_text(pgf, HG_LX, 54, white, 1.3f, "HANGAR");
        for (int i = 0; i < AC_COUNT; i++) {                                // the list: a row is drawn only if it fits completely in the window
            float ry = HG_LY + i * HG_ROW - menu_hscroll;
            if (ry < HG_LY - 1.0f || ry + HG_ROW - 4.0f > HG_LY + HG_LH + 1.0f) continue;
            bool s = (i == menu_hsel);
            vita2d_draw_rectangle(HG_LX, ry, HG_LW, HG_ROW - 6.0f, s ? RGBA8(22, 46, 104, 255) : RGBA8(14, 30, 70, 255));
            if (s) vita2d_draw_rectangle(HG_LX, ry, 6, HG_ROW - 6.0f, gold);
            vita2d_pgf_draw_text(pgf, HG_LX + 20, ry + 26, s ? white : dimc, 1.1f, SPECS[i].name);
            vita2d_pgf_draw_text(pgf, HG_LX + 20, ry + 47, pale, 0.62f, SPECS[i].line1);
            if (i == menu_ac) vita2d_pgf_draw_text(pgf, HG_LX + HG_LW - 16 - vita2d_pgf_text_width(pgf, 0.7f, "SELECTED"), ry + 22, gold, 0.7f, "SELECTED");
        }
        if (AC_COUNT * HG_ROW > HG_LH) {                                    // scroll bar (only when the list is longer than the window)
            float full = AC_COUNT * HG_ROW, th = fmaxf(24.0f, HG_LH * HG_LH / full), hm = full - HG_LH;
            vita2d_draw_rectangle(HG_LX + HG_LW + 6, HG_LY, 3, HG_LH, RGBA8(40, 56, 90, 255));
            vita2d_draw_rectangle(HG_LX + HG_LW + 6, HG_LY + (HG_LH - th) * (menu_hscroll / hm), 3, th, gold);
        }
        {                                                                    // the facts about the aircraft in the picture
            const char *maker = (menu_hsel == AC_A22 || menu_hsel == AC_A11) ? "ASHLEY AEROSPACE" : "KNOWLES AIRCRAFT CO.";
            vita2d_pgf_draw_text(pgf, HG_PX, HG_PY + HG_PH + 32, white, 1.25f, SPECS[menu_hsel].name);
            vita2d_pgf_draw_text(pgf, HG_PX + HG_PW - vita2d_pgf_text_width(pgf, 0.65f, maker), HG_PY + HG_PH + 32, RGBA8(176, 150, 96, 255), 0.65f, maker);
            vita2d_pgf_draw_text(pgf, HG_PX, HG_PY + HG_PH + 58, pale, 0.78f, SPECS[menu_hsel].line1);
            vita2d_pgf_draw_text(pgf, HG_PX, HG_PY + HG_PH + 80, dimc, 0.72f, SPECS[menu_hsel].line2);
        }
        {                                                                    // buttons
            bool chosen = (menu_hsel == menu_ac);
            const float bxs[3] = { HG_PX, HG_PX + 140.0f, HG_PX + 300.0f }, bws[3] = { 130.0f, 150.0f, HG_PW - 300.0f };
            const char *bl[3] = { chosen ? "SELECTED" : "SELECT", "AR VIEW", "BACK" };
            for (int b = 0; b < 3; b++) {
                unsigned int fillc = (b == 0 && chosen) ? RGBA8(40, 70, 60, 255) : (b == 1 ? RGBA8(36, 64, 110, 255) : RGBA8(30, 52, 96, 255));
                vita2d_draw_rectangle(bxs[b], HG_BY, bws[b], HG_BH, fillc);
                vita2d_draw_rectangle(bxs[b], HG_BY, bws[b], 1, gold); vita2d_draw_rectangle(bxs[b], HG_BY + HG_BH - 1, bws[b], 1, gold);
                vita2d_draw_rectangle(bxs[b], HG_BY, 1, HG_BH, gold); vita2d_draw_rectangle(bxs[b] + bws[b] - 1, HG_BY, 1, HG_BH, gold);
                vita2d_pgf_draw_text(pgf, (int)(bxs[b] + (bws[b] - vita2d_pgf_text_width(pgf, 1.0f, bl[b])) * 0.5f), HG_BY + 31, white, 1.0f, bl[b]);
            }
        }
        vita2d_pgf_draw_text(pgf, HG_LX, 508, pale, 0.66f, "Touch: tap an aircraft, drag the picture to turn it");
        vita2d_pgf_draw_text(pgf, HG_LX, 525, pale, 0.66f, "Up / Down: choose   Left / Right: turn   X: select   Square: AR view   O: back");
    }


    // A picker (airport, weather, time or sound): the list on the left, a picture and the details on the right.
    if (menu_popen) {
        const unsigned int card = RGBA8(10, 22, 56, 255), dimc = RGBA8(150, 172, 215, 255), pale = RGBA8(190, 205, 235, 255);
        const int kind = menu_popen, n = pk_count(kind);
        static const char *const TITLE[12] = { "", "AIRPORT", "", "", "WEATHER", "TIME OF DAY", "SOUND", "FLIGHT CONTROLS", "OPTIONS", "GYRO ROLL", "GYRO PITCH", "GYRO SENSITIVITY" };
        vita2d_draw_rectangle(20, 12, 920, 520, gold);
        vita2d_draw_rectangle(24, 16, 912, 512, card);
        float bx = HG_PX, by = HG_PY, bw = HG_PW, bh = HG_PH, hz = by + bh * 0.56f;      // the picture box and its horizon
        int wxk = menu_psel, todk = TOD_DAY;
        if (kind == 4 && menu_psel == WX_COUNT) wxk = (frame_count / 90) % WX_COUNT;
        if (kind == 5) todk = menu_psel;
        float cloud = 0.1f, fog = 0.0f, rain = 0.0f;
        if (kind == 4) { cloud = WX_PRESET[wxk].cloud; fog = WX_PRESET[wxk].fog; rain = WX_PRESET[wxk].rain; }
        float dark = 1.0f; int sr = 58, sg = 104, sb = 176, hr = 178, hg2 = 204, hb = 232;     // sky overhead and at the horizon
        if (todk == TOD_DUSK)  { sr = 44; sg = 60; sb = 120; hr = 250; hg2 = 150; hb = 100; dark = 0.62f; }
        if (todk == TOD_NIGHT) { sr = 4;  sg = 8;  sb = 30;  hr = 16;  hg2 = 26;  hb = 64;  dark = 0.26f; }
        for (int i = 0; i < 14; i++) {
            float u = i / 13.0f, g = fminf(1.0f, cloud * 0.9f) * 0.55f;
            int r = (int)((sr + (hr - sr) * u) * (1 - g) + 112 * dark * g), gg = (int)((sg + (hg2 - sg) * u) * (1 - g) + 118 * dark * g), b = (int)((sb + (hb - sb) * u) * (1 - g) + 128 * dark * g);
            vita2d_draw_rectangle(bx, by + i * ((hz - by) / 14.0f), bw, (hz - by) / 14.0f + 1.0f, RGBA8(r, gg, b, 255));
        }
        vita2d_draw_rectangle(bx, hz, bw, by + bh - hz, RGBA8((int)(64 * dark), (int)(98 * dark), (int)(66 * dark), 255));
        if (kind == 5) {                                                                   // sun, moon, stars
            if (todk == TOD_DAY)  vita2d_draw_fill_circle(bx + bw * 0.72f, by + 70, 30, RGBA8(255, 244, 190, 255));
            if (todk == TOD_DUSK) vita2d_draw_fill_circle(bx + bw * 0.72f, hz - 20, 36, RGBA8(255, 190, 110, 255));
            if (todk == TOD_NIGHT) {
                vita2d_draw_fill_circle(bx + bw * 0.72f, by + 62, 24, RGBA8(235, 238, 250, 255));
                vita2d_draw_fill_circle(bx + bw * 0.72f + 9, by + 56, 21, RGBA8(6, 12, 40, 255));
                for (int i = 0; i < 28; i++) vita2d_draw_rectangle(bx + 10 + (i * 97) % (int)(bw - 20), by + 8 + (i * 53) % (int)(hz - by - 30), 2, 2, RGBA8(230, 235, 255, 255));
            }
        }
        if (kind == 4 || kind == 5) {                                                      // clouds
            int nc = kind == 4 ? (int)(cloud * 9.0f + 0.5f) : 2;
            for (int i = 0; i < nc; i++) {
                float cx0 = bx + 40 + (i * 131) % (int)(bw - 80), cy0 = by + 34 + (i * 47) % 70;
                unsigned int cc = kind == 4 ? RGBA8((int)(236 - 90 * cloud), (int)(240 - 90 * cloud), (int)(246 - 80 * cloud), 255) : RGBA8((int)(240 * dark), (int)(240 * dark), (int)(246 * dark), 255);
                vita2d_draw_fill_circle(cx0, cy0, 22, cc); vita2d_draw_fill_circle(cx0 + 24, cy0 + 4, 18, cc); vita2d_draw_fill_circle(cx0 - 24, cy0 + 6, 16, cc);
            }
        }
        if (kind == 1) {                                                                   // a runway running away to the horizon
            const Airport *a = &AIRPORTS[pk_ap(menu_psel)];
            float hw = a->hw * 4.6f;
            P2 rq[4] = { { bx + bw * 0.5f - 3, hz }, { bx + bw * 0.5f + 3, hz }, { bx + bw * 0.5f + hw, by + bh }, { bx + bw * 0.5f - hw, by + bh } };
            bool grass = (pk_ap(menu_psel) == AP_PINECREST);
            fill_poly_panel(rq, 4, grass ? RGBA8(96, 128, 70, 255) : RGBA8(58, 60, 66, 255));
            for (int i = 0; i < 5; i++) {
                float u0 = 0.10f + i * 0.17f, u1 = u0 + 0.07f;
                float y0 = hz + (by + bh - hz) * u0 * u0, y1 = hz + (by + bh - hz) * u1 * u1;
                float w0 = 1.0f + 5.0f * u0 * u0 * (a->hw > 15.0f ? 1.0f : 0.8f), w1 = 1.0f + 5.0f * u1 * u1;
                if (!grass) { P2 d[4] = { { bx + bw * 0.5f - w0, y0 }, { bx + bw * 0.5f + w0, y0 }, { bx + bw * 0.5f + w1, y1 }, { bx + bw * 0.5f - w1, y1 } }; fill_poly_panel(d, 4, RGBA8(236, 236, 230, 255)); }
            }
            int tw = vita2d_pgf_text_width(pgf, 2.2f, a->id);
            vita2d_pgf_draw_text(pgf, bx + (bw - tw) * 0.5f, by + 74, RGBA8(255, 255, 255, 235), 2.2f, a->id);
        }
        if (kind == 4) {
            if (rain > 0.0f) for (int i = 0; i < (int)(rain * 90.0f); i++) {
                float rx = bx + (i * 83 + frame_count * 3) % (int)bw, ry = by + (i * 61 + frame_count * 11) % (int)(bh - 14.0f);
                vita2d_draw_line(rx, ry, rx - 3, ry + 11, RGBA8(190, 205, 235, 255));
            }
            if (fog > 0.0f) vita2d_draw_rectangle(bx, by, bw, bh, RGBA8(200, 204, 208, (int)(fog * 200.0f)));
            if (wxk == WX_STORM && (frame_count % 150) < 4) vita2d_draw_rectangle(bx, by, bw, bh, RGBA8(255, 255, 255, 140));
        }
        {
            bool gyp = (kind == 7 && menu_psel == 1) || kind >= 9 || (kind == 8 && menu_psel >= 2);        // the picture follows the tilt of the Vita
            if (kind == 7 || (kind == 8 && menu_psel == 1) || gyp) {
                float tcx = bx + bw * 0.5f - 20.0f, tcy = by + 112.0f, an = gyp ? clampf(gy_roll_deg, -42.0f, 42.0f) * (PI_F / 180.0f) : 0.0f, ca = cosf(an), sa = sinf(an);
                P2 body[4]; float ox[4] = { -120, 120, 120, -120 }, oy[4] = { -50, -50, 50, 50 };
                for (int q = 0; q < 4; q++) { body[q].x = tcx + ox[q] * ca - oy[q] * sa; body[q].y = tcy + ox[q] * sa + oy[q] * ca; }
                fill_poly_panel(body, 4, RGBA8(36, 44, 70, 255));
                P2 scr[4]; float sx[4] = { -70, 70, 70, -70 }, sy2[4] = { -42, -42, 42, 42 };
                for (int q = 0; q < 4; q++) { scr[q].x = tcx + sx[q] * ca - sy2[q] * sa; scr[q].y = tcy + sx[q] * sa + sy2[q] * ca; }
                fill_poly_panel(scr, 4, RGBA8(70, 110, 170, 255));
                for (int q = 0; q < 2; q++) { float lx2 = q ? 99.0f : -99.0f; vita2d_draw_fill_circle(tcx + lx2 * ca + 13 * sa, tcy + lx2 * sa - 13 * ca, 14, gyp ? RGBA8(90, 100, 130, 255) : gold); }
                if (gyp) {                                                                       // what the tilt is asking for
                    const float rc = bx + bw * 0.5f - 20.0f, ry = by + 258.0f;
                    vita2d_draw_rectangle(rc - 150, ry, 300, 14, RGBA8(40, 56, 90, 255));
                    vita2d_draw_rectangle(gy_ail >= 0 ? rc : rc + gy_ail * 150.0f, ry, fabsf(gy_ail) * 150.0f, 14, gold);
                    vita2d_draw_rectangle(rc - 1, ry - 4, 2, 22, white);
                    vita2d_pgf_draw_text(pgf, rc - 150, ry - 8, pale, 0.62f, "ROLL LEFT");
                    vita2d_pgf_draw_text(pgf, rc + 150 - vita2d_pgf_text_width(pgf, 0.62f, "ROLL RIGHT"), ry - 8, pale, 0.62f, "ROLL RIGHT");
                    const float pc = by + 130.0f, px = bx + bw - 44.0f;
                    vita2d_draw_rectangle(px, pc - 90, 14, 180, RGBA8(40, 56, 90, 255));
                    vita2d_draw_rectangle(px, gy_ele >= 0 ? pc - gy_ele * 90.0f : pc, 14, fabsf(gy_ele) * 90.0f, gold);
                    vita2d_draw_rectangle(px - 4, pc - 1, 22, 2, white);
                    vita2d_pgf_draw_text(pgf, px - 44, by + 28, pale, 0.62f, "NOSE UP");
                    vita2d_pgf_draw_text(pgf, px - 56, by + 238, pale, 0.62f, "NOSE DOWN");
                    if (!gy_have) vita2d_pgf_draw_text(pgf, bx + 20, by + 24, RGBA8(255, 120, 100, 255), 0.8f, "No motion data");
                }
            }
        }
        if (kind == 6 || (kind == 8 && menu_psel == 0)) {                                  // a speaker and four bars
            vita2d_draw_rectangle(bx + 90, by + 120, 40, 44, RGBA8(200, 210, 235, 255));
            P2 sp[4] = { { bx + 130, by + 120 }, { bx + 180, by + 84 }, { bx + 180, by + 200 }, { bx + 130, by + 164 } };
            fill_poly_panel(sp, 4, RGBA8(200, 210, 235, 255));
            for (int i = 0; i < 3; i++) {
                bool on = ((kind == 8 ? snd_level : menu_psel) & 3) > i;
                float hh = 40 + i * 34;
                vita2d_draw_rectangle(bx + 230 + i * 56, by + 142 - hh * 0.5f + 10, 30, hh, on ? gold : RGBA8(60, 78, 120, 255));
            }
            if (((kind == 8 ? snd_level : menu_psel) & 3) == 0) { vita2d_draw_line(bx + 230, by + 100, bx + 380, by + 190, RGBA8(255, 90, 80, 255)); vita2d_draw_line(bx + 231, by + 100, bx + 381, by + 190, RGBA8(255, 90, 80, 255)); }
        }
        vita2d_draw_rectangle(bx - 2, by - 2, bw + 4, 2, gold); vita2d_draw_rectangle(bx - 2, by + bh, bw + 4, 2, gold);
        vita2d_draw_rectangle(bx - 2, by - 2, 2, bh + 4, gold); vita2d_draw_rectangle(bx + bw, by - 2, 2, bh + 4, gold);
        vita2d_pgf_draw_text(pgf, HG_LX, 54, white, 1.3f, TITLE[kind]);
        for (int i = 0; i < n; i++) {
            float ry = HG_LY + i * PK_ROW;
            bool s = (i == menu_psel);
            vita2d_draw_rectangle(HG_LX, ry, HG_LW, PK_ROW - 6.0f, s ? RGBA8(22, 46, 104, 255) : RGBA8(14, 30, 70, 255));
            if (s) vita2d_draw_rectangle(HG_LX, ry, 6, PK_ROW - 6.0f, gold);
            vita2d_pgf_draw_text(pgf, HG_LX + 20, ry + 24, s ? white : dimc, 1.1f, pk_name(kind, i));
            vita2d_pgf_draw_text(pgf, HG_LX + 20, ry + 43, pale, 0.62f, pk_l1(kind, i));
            if (i == pk_cur(kind) && !(kind == 6)) vita2d_pgf_draw_text(pgf, HG_LX + HG_LW - 16 - vita2d_pgf_text_width(pgf, 0.7f, "SELECTED"), ry + 22, gold, 0.7f, "SELECTED");
            if (kind == 6 && i == menu_psnd) vita2d_pgf_draw_text(pgf, HG_LX + HG_LW - 16 - vita2d_pgf_text_width(pgf, 0.7f, "SELECTED"), ry + 22, gold, 0.7f, "SELECTED");
        }
        vita2d_pgf_draw_text(pgf, HG_PX, HG_PY + HG_PH + 32, white, 1.25f, pk_name(kind, menu_psel));
        vita2d_pgf_draw_text(pgf, HG_PX, HG_PY + HG_PH + 58, pale, 0.78f, pk_l1(kind, menu_psel));
        vita2d_pgf_draw_text(pgf, HG_PX, HG_PY + HG_PH + 80, dimc, 0.72f, pk_l2(kind, menu_psel));
        {
            bool chosen = (kind != 8) && (menu_psel == (kind == 6 ? menu_psnd : pk_cur(kind)));
            vita2d_draw_rectangle(HG_PX, HG_BY, 210, HG_BH, chosen ? RGBA8(40, 70, 60, 255) : RGBA8(30, 52, 96, 255));
            vita2d_draw_rectangle(HG_PX + 230, HG_BY, HG_PW - 230, HG_BH, RGBA8(30, 52, 96, 255));
            for (int b = 0; b < 2; b++) {
                float x0 = b ? HG_PX + 230 : HG_PX, w0 = b ? HG_PW - 230 : 210;
                vita2d_draw_rectangle(x0, HG_BY, w0, 1, gold); vita2d_draw_rectangle(x0, HG_BY + HG_BH - 1, w0, 1, gold);
                vita2d_draw_rectangle(x0, HG_BY, 1, HG_BH, gold); vita2d_draw_rectangle(x0 + w0 - 1, HG_BY, 1, HG_BH, gold);
            }
            const char *l1 = kind == 8 ? "OPEN" : (chosen ? "CHOSEN" : "SELECT"), *l2 = "BACK";
            vita2d_pgf_draw_text(pgf, (int)(HG_PX + (210 - vita2d_pgf_text_width(pgf, 1.1f, l1)) * 0.5f), HG_BY + 31, white, 1.1f, l1);
            vita2d_pgf_draw_text(pgf, (int)(HG_PX + 230 + (HG_PW - 230 - vita2d_pgf_text_width(pgf, 1.1f, l2)) * 0.5f), HG_BY + 31, white, 1.1f, l2);
        }
        vita2d_pgf_draw_text(pgf, HG_LX, 508, pale, 0.66f, "Touch: tap a row to look, tap it again or SELECT to choose");
        vita2d_pgf_draw_text(pgf, HG_LX, 525, pale, 0.66f, "Up / Down: choose   X: select   O: back");
    }

    // Options: every setting on one screen.
    if (menu_oopen) {
        const unsigned int card = RGBA8(10, 22, 56, 255), dimc = RGBA8(150, 172, 215, 255), pale = RGBA8(190, 205, 235, 255);
        vita2d_draw_rectangle(20, 12, 920, 520, gold);
        vita2d_draw_rectangle(24, 16, 912, 512, card);
        vita2d_pgf_draw_text(pgf, OP_X, 54, white, 1.3f, "OPTIONS");
        for (int r = 0; r < OP_N; r++) {
            float ry = OP_Y + r * OP_ROW;
            bool f = (r == menu_osel), gyrow = (r >= 2);
            vita2d_draw_rectangle(OP_X, ry, OP_CX + OP_CW + 14.0f - OP_X, OP_ROW - 6.0f, f ? RGBA8(22, 46, 104, 255) : RGBA8(14, 30, 70, 255));
            if (f) vita2d_draw_rectangle(OP_X, ry, 6, OP_ROW - 6.0f, gold);
            unsigned int nc = f ? white : dimc;
            if (gyrow && !gy_mode) nc = RGBA8(110, 126, 166, 255);
            vita2d_pgf_draw_text(pgf, OP_X + 18, ry + 26, nc, 1.0f, OP_NAME[r]);
            vita2d_pgf_draw_text(pgf, OP_X + 18, ry + 47, RGBA8(150, 172, 215, 255), 0.55f, op_desc(r));
            int n = OP_COUNT[r], cur = op_get(r);
            float cw = (OP_CW - 8.0f * (n - 1)) / n;
            for (int c = 0; c < n; c++) {
                float cx0 = OP_CX + c * (cw + 8.0f), cy0 = ry + (OP_ROW - 6.0f - OP_CH) * 0.5f;
                bool on = (c == cur);
                vita2d_draw_rectangle(cx0, cy0, cw, OP_CH, on ? RGBA8(214, 160, 70, 255) : RGBA8(30, 52, 96, 255));
                if (!on) { vita2d_draw_rectangle(cx0, cy0, cw, 1, RGBA8(70, 94, 150, 255)); vita2d_draw_rectangle(cx0, cy0 + OP_CH - 1, cw, 1, RGBA8(70, 94, 150, 255)); }
                int tw = vita2d_pgf_text_width(pgf, 0.8f, OP_CHIP[r][c]);
                vita2d_pgf_draw_text(pgf, (int)(cx0 + (cw - tw) * 0.5f), cy0 + 29, on ? RGBA8(20, 24, 40, 255) : pale, 0.8f, OP_CHIP[r][c]);
            }
        }
        // The tilt test: the Vita turns with your tilt, and the bars show what the tilt asks of the aircraft.
        vita2d_draw_rectangle(OP_PX - 2, OP_PY - 2, OP_PW + 4, OP_PH + 4, gold);
        vita2d_draw_rectangle(OP_PX, OP_PY, OP_PW, OP_PH, RGBA8(14, 30, 70, 255));
        vita2d_pgf_draw_text(pgf, OP_PX + 14, OP_PY + 26, pale, 0.8f, "TILT TEST");
        {
            float tcx = OP_PX + OP_PW * 0.5f - 22.0f, tcy = OP_PY + 112.0f;
            float an = clampf(gy_roll_deg, -42.0f, 42.0f) * (PI_F / 180.0f), ca = cosf(an), sa = sinf(an);
            P2 body[4]; float ox[4] = { -96, 96, 96, -96 }, oy[4] = { -40, -40, 40, 40 };
            for (int q = 0; q < 4; q++) { body[q].x = tcx + ox[q] * ca - oy[q] * sa; body[q].y = tcy + ox[q] * sa + oy[q] * ca; }
            fill_poly_panel(body, 4, RGBA8(36, 44, 70, 255));
            P2 scr[4]; float sx[4] = { -56, 56, 56, -56 }, sy2[4] = { -33, -33, 33, 33 };
            for (int q = 0; q < 4; q++) { scr[q].x = tcx + sx[q] * ca - sy2[q] * sa; scr[q].y = tcy + sx[q] * sa + sy2[q] * ca; }
            fill_poly_panel(scr, 4, RGBA8(70, 110, 170, 255));
            for (int q = 0; q < 2; q++) { float lx2 = q ? 79.0f : -79.0f; vita2d_draw_fill_circle(tcx + lx2 * ca + 10 * sa, tcy + lx2 * sa - 10 * ca, 11, RGBA8(90, 100, 130, 255)); }
            const float rc = tcx, rby = OP_PY + 244.0f;
            vita2d_draw_rectangle(rc - 120, rby, 240, 14, RGBA8(40, 56, 90, 255));
            vita2d_draw_rectangle(gy_ail >= 0 ? rc : rc + gy_ail * 120.0f, rby, fabsf(gy_ail) * 120.0f, 14, gold);
            vita2d_draw_rectangle(rc - 1, rby - 4, 2, 22, white);
            vita2d_pgf_draw_text(pgf, rc - 120, rby - 8, pale, 0.6f, "ROLL LEFT");
            vita2d_pgf_draw_text(pgf, rc + 120 - vita2d_pgf_text_width(pgf, 0.6f, "ROLL RIGHT"), rby - 8, pale, 0.6f, "ROLL RIGHT");
            const float pc = OP_PY + 112.0f, px = OP_PX + OP_PW - 40.0f;
            vita2d_draw_rectangle(px, pc - 78, 14, 156, RGBA8(40, 56, 90, 255));
            vita2d_draw_rectangle(px, gy_ele >= 0 ? pc - gy_ele * 78.0f : pc, 14, fabsf(gy_ele) * 78.0f, gold);
            vita2d_draw_rectangle(px - 4, pc - 1, 22, 2, white);
            vita2d_pgf_draw_text(pgf, px - 24, OP_PY + 22, pale, 0.6f, "NOSE UP");
            vita2d_pgf_draw_text(pgf, px - 32, OP_PY + 206, pale, 0.6f, "NOSE DOWN");
            vita2d_pgf_draw_text(pgf, OP_PX + 14, OP_PY + OP_PH - 38, dimc, 0.6f, gy_have ? "Tilt the Vita: the bars show what it asks of the aircraft." : "No motion data");
            vita2d_pgf_draw_text(pgf, OP_PX + 14, OP_PY + OP_PH - 18, dimc, 0.6f, "In flight, D-pad down + Circle centers the tilt.");
        }
        vita2d_draw_rectangle(HG_LX, OP_DY, 210, HG_BH, RGBA8(30, 52, 96, 255));
        vita2d_draw_rectangle(HG_LX, OP_DY, 210, 1, gold); vita2d_draw_rectangle(HG_LX, OP_DY + HG_BH - 1, 210, 1, gold);
        vita2d_draw_rectangle(HG_LX, OP_DY, 1, HG_BH, gold); vita2d_draw_rectangle(HG_LX + 209, OP_DY, 1, HG_BH, gold);
        vita2d_pgf_draw_text(pgf, (int)(HG_LX + (210 - vita2d_pgf_text_width(pgf, 1.1f, "DONE")) * 0.5f), OP_DY + 31, white, 1.1f, "DONE");
        vita2d_pgf_draw_text(pgf, HG_LX, 508, pale, 0.66f, "Touch: tap a button     Up / Down: choose a row     Left / Right or X: change it");
        vita2d_pgf_draw_text(pgf, HG_LX, 525, pale, 0.66f, "O: done");
    }

    // The controls window.
    if (menu_kopen) {
        static const char *const KEYS[][2] = {
            { "Left stick",                "Pitch and roll" },
            { "Right stick up / down",     "Throttle" },
            { "Right stick left / right",  "Rudder" },
            { "L / R",                     "Look left / right (chase view: both swing the camera to the front)" },
            { "Circle / Cross",            "Flaps down / up" },
            { "D-pad left / right",        "Pitch trim: a tap is a fine step, hold for more" },
            { "D-pad down (hold)",         "Wheel brakes on the ground. It is also the shift key for the three rows below" },
            { "D-pad down + Triangle",     "Trim for level flight at your current airspeed (in the air)" },
            { "D-pad down + Circle",       "Gyro controls: center the tilt where the Vita is now" },
            { "D-pad up",                  "Fast forward x2 / x5 / normal (free flight only)" },
            { "Square",                    "Landing gear up / down (K-30 and A22)" },
            { "D-pad down + Square",       "Aircraft lights: all on / navigation and beacon / off" },
            { "Triangle",                  "Reset the aircraft (restart the attempt in a challenge)" },
            { "Select (tap)",              "Switch camera: cockpit / chase" },
                        { "Select (hold)",             "Chart (L / R scroll the list)" },
            { "L + R + Select",            "Invert the pitch axis" },
            { "START",                     "Pause and return to the menu" },
            { "Select + START",            "Save a screenshot (ux0:data/leftseat_001.png, 002, ...)" } };
        const int nk = (int)(sizeof(KEYS) / sizeof(KEYS[0])), rowh = 21;
        int h = 52 + nk * rowh + 88, top = (SCREEN_H - h) / 2;
        vita2d_draw_rectangle(60, top, 840, h, gold);
        vita2d_draw_rectangle(64, top + 4, 832, h - 8, RGBA8(10, 22, 56, 255));
        vita2d_pgf_draw_text(pgf, 90, top + 38, white, 1.3f, "CONTROLS");
        for (int i = 0; i < nk; i++) {
            int ry = top + 66 + i * rowh;
            if (i & 1) vita2d_draw_rectangle(80, ry - 16, 800, rowh, RGBA8(14, 30, 70, 255));
            vita2d_pgf_draw_text(pgf, 94, ry, gold, 0.75f, KEYS[i][0]);
            vita2d_pgf_draw_text(pgf, 340, ry, RGBA8(210, 222, 245, 255), 0.75f, KEYS[i][1]);
        }
        int ty = top + 66 + nk * rowh + 14;
        vita2d_pgf_draw_text(pgf, 94, ty, RGBA8(190, 205, 235, 255), 0.72f, "Takeoff (K-4): flaps 10, full throttle, hold right rudder, rotate at 55 kt, climb at 74 kt.");
        vita2d_pgf_draw_text(pgf, 94, ty + 22, RGBA8(190, 205, 235, 255), 0.72f, "Landing: about 65 kt with full flaps, and flare just above the runway.");
        vita2d_pgf_draw_text(pgf, 94, top + h - 14, RGBA8(190, 205, 235, 255), 0.8f, "X or O: close");
    }

    // The challenge list.
    if (menu_chopen) {
        static const char *const DESC[CK_COUNT] = {
            "Final approach. Land smoothly on the aiming point.",
            "The engine fails at 1,500 ft. Glide in and land.",
            "Strong wind across the runway. Crab in, then straighten up.",
            "Fly 16 NM to Redrock Municipal and land. Hold SELECT for the chart.",
            "Fly 11 NM to a short grass strip in the mountains. Touch down early.",
            "Fly through a course of rings over and around Lana Lake." };
        bool run = ch_running();
        int n = CK_COUNT + (run ? 1 : 0);
        bool tight = (n >= 6);                                    // with the RESUME row the list gets a little more compact
        int rs = tight ? 48 : 58, rh = tight ? 44 : 52, top = tight ? 56 : 84, tn = tight ? 19 : 24, td = tight ? 36 : 44;
        int h = 70 + n * rs + 36;
        vita2d_draw_rectangle(150, top, 660, h, gold);
        vita2d_draw_rectangle(154, top + 4, 652, h - 8, RGBA8(10, 22, 56, 255));
        vita2d_pgf_draw_text(pgf, 180, top + 42, white, 1.3f, "CHALLENGES");
        for (int i = 0; i < n; i++) {
            int k = i - (run ? 1 : 0), ry = top + 60 + i * rs;
            bool s = (i == menu_chsel);
            vita2d_draw_rectangle(172, ry, 616, rh, s ? RGBA8(22, 46, 104, 255) : RGBA8(14, 30, 70, 255));
            if (s) vita2d_draw_rectangle(172, ry, 6, rh, gold);
            if (k < 0) {
                vita2d_pgf_draw_text(pgf, 194, ry + tn, s ? white : RGBA8(150, 172, 215, 255), 1.1f, "RESUME");
                vita2d_pgf_draw_textf(pgf, 194, ry + td, RGBA8(150, 172, 215, 255), 0.65f, "Continue the %s challenge in progress (%s)", CK_NAMES[ch_kind], SPECS[ac_flying].key);
            } else {
                vita2d_pgf_draw_text(pgf, 194, ry + tn, s ? white : RGBA8(150, 172, 215, 255), 1.1f, CK_NAMES[k]);
                vita2d_pgf_draw_text(pgf, 194, ry + td, RGBA8(150, 172, 215, 255), 0.65f, DESC[k]);
                char bb[24];
                if (ch_best_a[menu_ac][k] > 0) snprintf(bb, sizeof(bb), "BEST %d", ch_best_a[menu_ac][k]); else snprintf(bb, sizeof(bb), "NOT FLOWN");
                int tw = vita2d_pgf_text_width(pgf, 0.85f, bb);
                vita2d_pgf_draw_text(pgf, 768 - tw, ry + rh / 2 + 6, ch_best_a[menu_ac][k] > 0 ? gold : RGBA8(110, 130, 180, 255), 0.85f, bb);
            }
        }
        vita2d_pgf_draw_text(pgf, 194, top + h - 14, RGBA8(190, 205, 235, 255), 0.8f, "Up / Down: choose     X: start     O: back");
    }

    // "Not implemented" message box.
    if (menu_popup >= 0) {
        vita2d_draw_rectangle(220, 180, 520, 170, gold);
        vita2d_draw_rectangle(224, 184, 512, 162, RGBA8(10, 22, 56, 255));
        vita2d_pgf_draw_text(pgf, 256, 232, white, 1.3f, MENU_ITEMS[menu_popup]);
        vita2d_pgf_draw_text(pgf, 256, 276, gold, 1.1f, "FUTURE UPDATE");
        vita2d_pgf_draw_text(pgf, 256, 308, RGBA8(200, 214, 240, 255), 0.8f, "Not implemented yet - coming in a later version.");
        vita2d_pgf_draw_text(pgf, 256, 334, RGBA8(150, 172, 215, 255), 0.75f, "Press X to close");
    }
    draw_shot_msg(pgf);
}

