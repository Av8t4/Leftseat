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
    if (chr.crashed) snprintf(title, sizeof(title), "NO LANDING");
    else             snprintf(title, sizeof(title), "%s RESULT", CK_NAMES[ch_kind]);
    vita2d_pgf_draw_text(pgf, 204, 72, gold, 1.2f, title);

    char big[16];
    snprintf(big, sizeof(big), "%d", chr.total);
    vita2d_pgf_draw_text(pgf, 204, 150, chr.crashed ? bad : white, 3.2f, big);
    const char *grade = chr.crashed ? chr.note
                      : (chr.total >= 90 ? "PERFECT LANDING" : (chr.total >= 75 ? "GREAT LANDING"
                      : (chr.total >= 60 ? "GOOD LANDING" : (chr.total >= 40 ? "ROUGH LANDING" : "HARD LANDING"))));
    vita2d_pgf_draw_text(pgf, 330, 112, chr.crashed ? bad : good, 1.2f, grade);
    vita2d_pgf_draw_textf(pgf, 330, 142, dim, 0.8f, "out of 100     BEST %d     ATTEMPTS %d", ch_best, ch_attempts);
    if (chr.new_best) vita2d_pgf_draw_text(pgf, 330, 168, gold, 0.9f, "NEW BEST!");
    if (ch_trip())
        vita2d_pgf_draw_textf(pgf, 204, 192, dim, 0.8f, "Trip time %d:%02d      Fuel used %.1f gal", (int)(chr.trip_s / 60.0f), (int)chr.trip_s % 60, chr.fuel_used);

    if (!chr.crashed) {
        float d = chr.z + 1000.0f;
        char v[5][56];
        snprintf(v[0], 56, "%.1f m/s (%d fpm)", chr.sink, (int)(chr.sink * 196.85f));
        snprintf(v[1], 56, "%.0f m %s of the centre line", fabsf(chr.x), chr.x < 0 ? "left" : "right");
        if (d < 0) snprintf(v[2], 56, "%.0f m SHORT of the runway", -d);
        else       snprintf(v[2], 56, "%.0f m past the threshold", d);
        snprintf(v[3], 56, "heading %+.0f, bank %+.0f", chr.hdg_err, chr.bank_deg);
        snprintf(v[4], 56, "%d bounce%s%s", chr.bounces, chr.bounces == 1 ? "" : "s", chr.nose_first ? ", nose wheel first" : "");
        static const char *label[5] = { "Sink rate", "Centre line", "Touchdown", "Attitude", "Penalties" };
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
        vita2d_pgf_draw_text(pgf, 204, 230, dim, 0.9f, "Try again: aim for the aiming-point markings and");
        vita2d_pgf_draw_text(pgf, 204, 258, dim, 0.9f, "use the glide-slope lights beside the runway.");
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
static const char *MENU_ITEMS[8] = { "OPEN WORLD", "AIRPORT", "CHALLENGES", "HANGAR", "WEATHER", "TIME", "SOUND", "CONTROLS" };
static int  menu_chopen;            // the challenge list is showing
static int  menu_chsel;             // highlighted row in that list
static int  menu_ch_pick = -1;      // result: -1 = resume the running challenge, otherwise the kind to start
static bool ch_running(void) { return ch_state == CH_FLYING || ch_state == CH_TOUCHED; }
static int  menu_ap;                // airport picked for the open world (start and Triangle reset)
static int  menu_ac;                // aircraft picked in the hangar for the next flight
static int  menu_hopen, menu_hsel;  // the hangar list is showing, and its highlighted row
static int  menu_kopen;             // the controls window is showing
static int  menu_tod;              // time of day for the next flight: day, dusk or night
static int  menu_wx;               // starting weather: 0..WX_COUNT-1, or WX_COUNT for random
static bool wx_dirty;              // the weather choice changed since the flight started

static int menu_count(void) { return 8; }

// Returns 0 = nothing, 1 = start / resume the flight, 2 = quit the game.
static int menu_update(const SceCtrlData *pad, unsigned int pressed) {
    if (menu_quit_t > 0) menu_quit_t--;
    if (menu_hopen) {                                         // hangar: choose an aircraft
        int mv = 0;
        if (pressed & SCE_CTRL_UP)   mv = -1;
        if (pressed & SCE_CTRL_DOWN) mv = 1;
        int sy = pad->ly < 64 ? -1 : (pad->ly > 192 ? 1 : 0);
        if (sy != 0 && menu_stick == 0) mv = sy;
        menu_stick = sy;
        if (mv) { menu_hsel = (menu_hsel + mv + AC_COUNT) % AC_COUNT; snd_event(EV_CLICK, 1.0f); }
        if (pressed & SCE_CTRL_CIRCLE) { menu_hopen = 0; snd_event(EV_CLICK, 1.0f); }
        if (pressed & SCE_CTRL_CROSS)  { menu_ac = menu_hsel; menu_hopen = 0; snd_event(EV_CONFIRM, 1.0f); }
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
    int move = 0;
    if (pressed & SCE_CTRL_UP)   move = -1;
    if (pressed & SCE_CTRL_DOWN) move = 1;
    int sy = pad->ly < 64 ? -1 : (pad->ly > 192 ? 1 : 0);
    if (sy != 0 && menu_stick == 0) move = sy;
    menu_stick = sy;
    if (move) { menu_sel = (menu_sel + move + menu_count()) % menu_count(); snd_event(EV_CLICK, 1.0f); }
    if (menu_sel == 1) {                                      // airport row: left / right picks where the open world starts
        if (pressed & SCE_CTRL_RIGHT) { menu_ap = (menu_ap + 1) % AP_COUNT; snd_event(EV_CLICK, 1.0f); }
        if (pressed & SCE_CTRL_LEFT)  { menu_ap = (menu_ap + AP_COUNT - 1) % AP_COUNT; snd_event(EV_CLICK, 1.0f); }
    }
    if (menu_sel == 4) {                                      // weather row: left / right picks the starting weather
        if (pressed & SCE_CTRL_RIGHT) { menu_wx = (menu_wx + 1) % (WX_COUNT + 1); wx_dirty = true; }
        if (pressed & SCE_CTRL_LEFT)  { menu_wx = (menu_wx + WX_COUNT) % (WX_COUNT + 1); wx_dirty = true; }
    }

    if (menu_sel == 5) {                                      // time row: left / right picks day, dusk or night
        if (pressed & SCE_CTRL_RIGHT) menu_tod = (menu_tod + 1) % TOD_COUNT;
        if (pressed & SCE_CTRL_LEFT)  menu_tod = (menu_tod + TOD_COUNT - 1) % TOD_COUNT;
    }

    if (menu_sel == 6) {                                      // sound row: left / right sets the volume
        if (pressed & SCE_CTRL_RIGHT) { snd_level = (snd_level + 1) % 4; snd_event(EV_CLICK, 1.0f); scores_save(); }
        if (pressed & SCE_CTRL_LEFT)  { snd_level = (snd_level + 3) % 4; snd_event(EV_CLICK, 1.0f); scores_save(); }
    }

    if (pressed & SCE_CTRL_CROSS) {
        if (menu_sel == 6) { snd_level = (snd_level + 1) % 4; snd_event(EV_CLICK, 1.0f); scores_save(); return 0; }       // X also steps through the volume
        snd_event(EV_CONFIRM, 1.0f);
        if (menu_sel == 0) return 1;
        if (menu_sel == 1) {                                  // X also steps through the airports
            menu_ap = (menu_ap + 1) % AP_COUNT;
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
    const float cx = 712.0f, cy = 232.0f, R = 160.0f;
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
        int tw = vita2d_pgf_text_width(pgf, 1.3f, CARD[k]);
        vita2d_pgf_draw_text(pgf, cx + sinf(a) * rr - tw / 2, cy - cosf(a) * rr + 9,
                             k == 0 ? RGBA8(255, 90, 80, 255) : RGBA8(235, 240, 255, 255), 1.3f, CARD[k]);
    }
    float ang = t * 0.35f;
    draw_menu_plane(cx + 7, cy + 9, 118.0f, ang, t * 30.0f, 1);               // shadow
    draw_menu_plane(cx, cy, 118.0f, ang, t * 30.0f, 0);

    // Title.
    vita2d_pgf_draw_text(pgf, 63, 123, RGBA8(8, 16, 44, 255), 3.0f, "LEFT SEAT");
    vita2d_pgf_draw_text(pgf, 60, 120, white, 3.0f, "LEFT SEAT");
    vita2d_pgf_draw_text(pgf, 64, 160, gold, 1.05f, "A KYLE KNOWLES FLIGHT SIMULATOR");

    // Menu items.
    for (int i = 0; i < menu_count(); i++) {
        int step = 37;
        int y = 204 + i * step;
        int sel = (i == menu_sel);
        vita2d_draw_rectangle(52, y - 33, 420, step - 3, sel ? RGBA8(22, 46, 104, 255) : RGBA8(12, 26, 62, 255));
        if (sel) vita2d_draw_rectangle(52, y - 33, 8, step - 3, gold);
        vita2d_pgf_draw_text(pgf, 80, y, sel ? white : RGBA8(150, 172, 215, 255), 1.5f, MENU_ITEMS[i]);
        if (i == 0) {
            if (flight_world == 0 && game_mode == MODE_FREE && ac_flying == menu_ac && start_ap == menu_ap)
                vita2d_pgf_draw_text(pgf, 340, y - 4, gold, 0.8f, "RESUME");
        } else if (i == 1) {
            char ab[40];
            snprintf(ab, sizeof(ab), "< %s >", AIRPORTS[menu_ap].name);
            int tw = vita2d_pgf_text_width(pgf, 0.8f, ab);
            vita2d_pgf_draw_text(pgf, 462 - tw, y - 2, sel ? gold : RGBA8(130, 152, 200, 255), 0.8f, ab);
        } else if (i == 2) {
            char cb[24];
            bool running = (ch_state == CH_FLYING || ch_state == CH_TOUCHED);
            if (running) snprintf(cb, sizeof(cb), "RESUME");
            else         snprintf(cb, sizeof(cb), "%d TO FLY", CK_COUNT);
            vita2d_pgf_draw_text(pgf, 330, y - 4, running || sel ? gold : RGBA8(110, 130, 180, 255), 0.75f, cb);
        } else if (i == 4) {
            char wb[32];
            snprintf(wb, sizeof(wb), "< %s >", menu_wx == WX_COUNT ? "RANDOM" : WX_NAMES[menu_wx]);
            int tw = vita2d_pgf_text_width(pgf, 0.85f, wb);
            vita2d_pgf_draw_text(pgf, 462 - tw, y - 2, sel ? gold : RGBA8(130, 152, 200, 255), 0.85f, wb);
        } else if (i == 5) {
            char tb[32];
            snprintf(tb, sizeof(tb), "< %s >", TOD_NAMES[menu_tod]);
            int tw = vita2d_pgf_text_width(pgf, 0.85f, tb);
            vita2d_pgf_draw_text(pgf, 462 - tw, y - 2, sel ? gold : RGBA8(130, 152, 200, 255), 0.85f, tb);
        } else if (i == 6) {
            char sb[32];
            snprintf(sb, sizeof(sb), "< %s >", SND_NAMES[snd_level & 3]);
            int tw = vita2d_pgf_text_width(pgf, 0.85f, sb);
            vita2d_pgf_draw_text(pgf, 462 - tw, y - 2, sel ? gold : RGBA8(130, 152, 200, 255), 0.85f, sb);
        } else if (i == 3) {
            char nm[24];
            snprintf(nm, sizeof(nm), "%s", SPECS[menu_ac].tag);
            for (char *p = nm; *p; p++) if (*p >= 'a' && *p <= 'z') *p -= 32;
            int tw = vita2d_pgf_text_width(pgf, 0.8f, nm);
            vita2d_pgf_draw_text(pgf, 462 - tw, y - 2, sel ? gold : RGBA8(130, 152, 200, 255), 0.8f, nm);
        }
    }

    // Footer.
    vita2d_pgf_draw_text(pgf, 60, 528, RGBA8(190, 205, 235, 255), 0.8f,
                         menu_sel == 1 ? "Left / Right: choose the starting airport     Up / Down: move"
                       : (menu_sel == 4 ? "Left / Right: change weather     Up / Down: move"
                       : (menu_sel == 5 ? "Left / Right: change time of day     Up / Down: move"
                       : (menu_sel == 6 ? "Left / Right: change the volume     Up / Down: move"
                       : (menu_sel == 7 ? "X: show the controls     Up / Down: move" : "D-pad / left stick: move     X: select")))));
    if (menu_quit_t > 0) vita2d_pgf_draw_text(pgf, 640, 528, gold, 0.8f, "Press START again to quit");
    else                 vita2d_pgf_draw_text(pgf, 770, 528, RGBA8(190, 205, 235, 255), 0.8f, "START: quit");

    // The hangar.
    if (menu_hopen) {
        const int pitch = 82, bh = 76;
        int h = 50 + AC_COUNT * pitch + 34, top = (SCREEN_H - h) / 2;
        vita2d_draw_rectangle(150, top, 660, h, gold);
        vita2d_draw_rectangle(154, top + 4, 652, h - 8, RGBA8(10, 22, 56, 255));
        vita2d_pgf_draw_text(pgf, 180, top + 36, white, 1.3f, "HANGAR");
        for (int i = 0; i < AC_COUNT; i++) {
            int ry = top + 50 + i * pitch;
            bool s = (i == menu_hsel);
            vita2d_draw_rectangle(172, ry, 616, bh, s ? RGBA8(22, 46, 104, 255) : RGBA8(14, 30, 70, 255));
            if (s) vita2d_draw_rectangle(172, ry, 6, bh, gold);
            vita2d_pgf_draw_text(pgf, 194, ry + 26, s ? white : RGBA8(150, 172, 215, 255), 1.1f, SPECS[i].name);
            vita2d_pgf_draw_text(pgf, 194, ry + 47, RGBA8(190, 205, 235, 255), 0.75f, SPECS[i].line1);
            vita2d_pgf_draw_text(pgf, 194, ry + 67, RGBA8(150, 172, 215, 255), 0.7f, SPECS[i].line2);
            if (i == menu_ac) vita2d_pgf_draw_text(pgf, 700, ry + 26, gold, 0.8f, "SELECTED");
            vita2d_pgf_draw_text(pgf, 778 - vita2d_pgf_text_width(pgf, 0.6f, "KNOWLES AIRCRAFT CO."), ry + 67, RGBA8(176, 150, 96, 255), 0.6f, "KNOWLES AIRCRAFT CO.");
        }
        vita2d_pgf_draw_text(pgf, 194, top + h - 12, RGBA8(190, 205, 235, 255), 0.8f, "Up / Down: choose     X: select     O: back");
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
            { "D-pad down (in the air)",   "Trim for level flight at your current airspeed" },
            { "D-pad down (on the ground)", "Wheel brakes" },
            { "D-pad up",                  "Fast forward x2 / x5 / normal (free flight only)" },
            { "Square",                    "Landing gear up / down (K-30 only)" },
            { "Triangle",                  "Reset the aircraft (restart the attempt in a challenge)" },
            { "Select (tap)",              "Switch camera: cockpit / chase" },
            { "Select (hold)",             "Chart of the whole area" },
            { "L + R + Select",            "Invert the pitch axis" },
            { "START",                     "Pause and return to the menu" } };
        const int nk = (int)(sizeof(KEYS) / sizeof(KEYS[0])), rowh = 22;
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
            "Fly 11 NM to a short grass strip in the mountains. Touch down early." };
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
}

