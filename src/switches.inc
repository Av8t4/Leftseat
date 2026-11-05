// Left Seat: the switch panel. Double-tap the back of the Vita and the gauges give way to the cockpit switches:
// battery, fuel pump, ignition and the START button (the engine has to be started now), the aircraft lights and a few
// extra switches. This file has the engine start-up and the touch handling; the drawing is in panel.inc.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

static void gear_say(const char *m);                       // (the message at the top right; defined in update.inc)

/* ------------------------------------------------------------------ */
/* The engine: start-up, running and shutdown                          */
/* ------------------------------------------------------------------ */

// Piston engines (K-4, A11) are started with magnetos and a starter; turbine engines (the turboprops and the jets) with
// fuel and ignition. Either way: battery on, fuel on, ignition on, then hold START until the engine lights.
static bool eng_piston(void) { return spec->model == 0 || spec->model == 5; }

static void engine_step(void) {
    const bool piston = eng_piston();
    eng_cranking = false;
    if (eng_on) {
        if (piston ? !sw.ign : !sw.pump) eng_on = false;               // magnetos off, or the fuel shut off: it stops
        eng_crank_t = 0.0f;
    } else if (starter_held && sw.batt && !pl.crashed) {
        eng_cranking = true;
        eng_crank_t += DT;
        const bool fuel_ok = sw.pump && sw.ign && pl.fuel > 0.0f && !engine_failed;
        const float need = piston ? 1.6f : 4.0f;                       // seconds of cranking before it lights
        if (fuel_ok && eng_crank_t >= need) { eng_on = true; eng_n = fmaxf(eng_n, 0.30f); eng_crank_t = 0.0f; eng_cranking = false; }
    } else {
        eng_crank_t = fmaxf(0.0f, eng_crank_t - DT * 2.0f);            // let go of START and the turning over is forgotten
    }
    const float up = piston ? 0.35f : 4.5f, down = piston ? 0.40f : 9.0f;
    if (ENG_RUN) eng_n += (1.0f - eng_n) * (1.0f - expf(-DT / up));
    else         eng_n -= eng_n * (1.0f - expf(-DT / down));
    if (eng_n < 0.002f) eng_n = 0.0f;
}

/* ------------------------------------------------------------------ */
/* The panel: where the switches are                                   */
/* ------------------------------------------------------------------ */

enum { SWI_BATT = 0, SWI_PUMP, SWI_IGN, SWI_START, SWI_BEACON, SWI_NAV, SWI_STROBE, SWI_LAND, SWI_TAXI, SWI_PITOT, SWI_AVI, SWI_CABIN, SWI_QUICK, SWI_N };

static void sw_cell(int i, float *x, float *y, float *w, float *h) {
    static const float X[SWI_N] = { 22, 86, 150, 214,  298, 362, 426, 490, 554,  640, 704, 768,  852 };
    *x = X[i]; *y = VIEW_H + 24.0f; *w = (i == SWI_QUICK) ? 86.0f : 60.0f; *h = 96.0f;
}
static bool *sw_flag(int i) {
    switch (i) {
        case SWI_BATT: return &sw.batt;   case SWI_PUMP: return &sw.pump;   case SWI_IGN: return &sw.ign;
        case SWI_BEACON: return &sw.beacon; case SWI_NAV: return &sw.nav;  case SWI_STROBE: return &sw.strobe;
        case SWI_LAND: return &sw.landing; case SWI_TAXI: return &sw.taxi; case SWI_PITOT: return &sw.pitot;
        case SWI_AVI: return &sw.avionics; case SWI_CABIN: return &sw.cabin;
        default: return NULL;
    }
}

/* ------------------------------------------------------------------ */
/* Touch: the double tap on the back, and the switches on the screen   */
/* ------------------------------------------------------------------ */

static const int SW_DT_WIN[4] = { 0, 14, 24, 38 };        // frames allowed between the two taps (off / fast / normal / slow)
static const int SW_TAP_MAX = 12;                           // a tap is a touch shorter than this (about a fifth of a second)

// nf front touches (x, y in screen pixels, with ids) and nback touches on the back pad, this frame.
static void sw_input(int nf, const float *fx, const float *fy, const int *fid, int nback) {
    static int frame, back_prev, back_start, back_last = -1000, front_last = -1000, prev_n, prev_id[2];
    static bool had_hold;
    frame++;
    bool toggle_panel = false;

    // The back pad: two quick taps (a finger that rests on it, as when holding the Vita, is not a tap).
    if (sw_dt > 0) {
        const int win = SW_DT_WIN[sw_dt & 3];
        bool down = nback > 0 && nback <= 2;
        if (down && !back_prev) back_start = frame;
        if (!down && back_prev) {
            if (frame - back_start <= SW_TAP_MAX) {
                if (frame - back_last <= win) { toggle_panel = true; back_last = -1000; }
                else back_last = frame;
            } else back_last = -1000;
        }
        back_prev = down;
    }

    // The front screen: a tap on a switch flips it; two quick taps on the bare panel do the same as the back pad
    // (for a Vita whose back pad is not working).
    bool start_down = false;
    for (int k = 0; k < nf; k++) {
        bool fresh = true;
        for (int q = 0; q < prev_n; q++) if (prev_id[q] == fid[k]) fresh = false;
        const float x = fx[k], y = fy[k];
        if (y < VIEW_H) continue;
        bool hit = false;
        if (sw_open) {
            for (int i = 0; i < SWI_N; i++) {
                float cx, cy, cw, ch; sw_cell(i, &cx, &cy, &cw, &ch);
                if (x < cx - 3.0f || x > cx + cw + 3.0f || y < cy - 3.0f || y > cy + ch + 3.0f) continue;
                hit = true;
                if (i == SWI_START) { start_down = true; if (fresh) snd_event(EV_CLICK, 1.0f); }
                else if (fresh && i == SWI_QUICK) { sw_run_all(); snd_event(EV_CONFIRM, 1.0f); gear_say("QUICK START: ENGINE RUNNING"); }
                else if (fresh) { bool *f = sw_flag(i); if (f) { *f = !*f; snd_event(EV_CLICK, 1.0f); } }
            }
        }
        if (fresh && !hit && !toggle_panel) {
            if (frame - front_last <= SW_DT_WIN[2]) { toggle_panel = true; front_last = -1000; }
            else front_last = frame;
        }
    }
    prev_n = nf > 2 ? 2 : nf;
    for (int k = 0; k < prev_n; k++) prev_id[k] = fid[k];
    starter_held = sw_open && start_down;
    (void)had_hold;
    if (toggle_panel) { sw_open = !sw_open; snd_event(EV_CLICK, 1.0f); starter_held = false; }
}
