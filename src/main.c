// Left Seat - a flat-shaded, early-90s style flight simulator for PS Vita, by Kyle Knowles.
// All 3D is done in software (project + clip + painter's algorithm) and drawn as filled
// triangles through vita2d. There are no art assets to load: the world and the aircraft are
// built in code. See README.md for the controls.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>

/* The game is split into the files below. main.c includes them in order, so the whole
   program is still compiled as one unit (static functions and globals stay private). */
#include "defs.inc"
#include "aircraft.inc"
#include "mathx.inc"
#include "batch.inc"
#include "weather.inc"
#include "clip2d.inc"
#include "camera.inc"
#include "world.inc"
#include "flight.inc"
#include "gear.inc"
#include "soundcore.inc"
#include "challenge.inc"
#include "update.inc"
#include "models.inc"
#include "modelkit.inc"
#include "k10.inc"
#include "jet1.inc"
#include "k30.inc"
#include "fleet.inc"
#include "sky.inc"
#include "scenery.inc"
#include "night.inc"
#include "airports.inc"
#include "scene.inc"
#include "panel.inc"
#include "overlays.inc"
#include "chart.inc"
#include "menu.inc"
#include "sound.inc"

// New landing challenge with the weather chosen in the menu (it stays fixed during the attempt).
static void start_challenge(void) {
    ac_flying = menu_ac; spec = &SPECS[menu_ac];
    int pick = (menu_wx == WX_COUNT) ? (frame_count / 3) % WX_COUNT : menu_wx;
    if (ch_kind == CK_XWIND) pick = WX_CLEAR;                    // the crosswind challenge brings its own wind
    wx_start(pick, false);
    if (ch_kind == CK_XWIND) {
        wx.wind = wx_tgt.wind = 14.0f;                           // about 11 kt near the ground, 27 kt higher up
        wx.turb = wx_tgt.turb = 0.28f;
        wx_dir = wx_dir_tgt = ((rand() & 1) ? 1.0f : -1.0f) * frand(1.25f, 1.45f);   // from the right or the left
    }
    tod = menu_tod;
    challenge_begin();
    flight_world = 0;
    wx_dirty = false;
}

// Frame timing for the log file (ux0:data/skyhawk.log): average microseconds per phase over 300 flight frames.
static unsigned long long prof_t[6];
static int prof_n = 0;
static unsigned long long prof_last = 0;

int main(void) {
    logfile = fopen("ux0:data/skyhawk.log", "w");           // (the old file name is kept)
    if (logfile) { fputs("start\n", logfile); fflush(logfile); }
    vita2d_init();
    if (logfile) { fputs("vita2d_init ok\n", logfile); fflush(logfile); }
    vita2d_set_clear_color(RGBA8(118, 176, 236, 255));     // sky
    vita2d_pgf *pgf = vita2d_load_default_pgf();
    if (logfile) { fprintf(logfile, "pgf %s\n", pgf ? "ok" : "NULL"); fflush(logfile); }

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    build_world();
    reset_plane();
    scores_load();
    snd_start();
    if (logfile) { fprintf(logfile, "world ok, faces=%d boxes=%d\n", nfaces, nboxes); fflush(logfile); }

    SceCtrlData pad, prev;
    memset(&prev, 0, sizeof(prev));

    for (;;) {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        unsigned int pressed = pad.buttons & ~prev.buttons;

        if (game_state == ST_MENU) {
            int act = menu_update(&pad, pressed);
            if (act == 2) break;
            if (act == 1) {
                int pick = (menu_wx == WX_COUNT) ? (frame_count / 3) % WX_COUNT : menu_wx;
                bool drift = (menu_wx == WX_COUNT);                  // only Random drifts; every other choice stays put
                int want = menu_ac;
                int want_ap = menu_ap;
                if (flight_world != 0 || game_mode != MODE_FREE || ac_flying != want || start_ap != want_ap) {
                    ac_flying = want; spec = &SPECS[want];
                    start_ap = want_ap;
                    reset_free(); flight_world = 0; game_mode = MODE_FREE; ch_state = CH_IDLE;
                    wx_start(pick, drift); wx_dirty = false;
                }
                else if (wx_dirty)     { wx_set_target(pick, drift); wx_dirty = false; }
                tod = menu_tod;
                game_state = ST_FLY;
            }
            if (act == 4) {                                           // a challenge picked from the list: start it, or resume
                if (menu_ch_pick >= 0) { ch_kind = menu_ch_pick; start_challenge(); }
                tod = menu_tod;
                game_state = ST_FLY;
            }

            vita2d_start_drawing();
            vita2d_clear_screen();
            draw_menu(pgf);
            vita2d_end_drawing();
            vita2d_swap_buffers();
        } else if (pressed & SCE_CTRL_START) {              // START in flight: back to the menu
            game_state = ST_MENU;
            ff_level = 0;
            menu_quit_t = 0;
            menu_stick = 0;
        } else if (game_state == ST_RESULT) {                         // landing score card over the frozen scene
            if (pressed & SCE_CTRL_CROSS)       { snd_event(EV_CONFIRM, 1.0f); start_challenge(); game_state = ST_FLY; }
            else if (pressed & (SCE_CTRL_CIRCLE | SCE_CTRL_START)) { snd_event(EV_CLICK, 1.0f); ch_state = CH_IDLE; game_state = ST_MENU; menu_quit_t = 0; menu_stick = 0; }

            vita2d_start_drawing();
            vita2d_clear_screen();
            draw_scene();
            draw_overlays(pgf);
            draw_panel(pgf);
            draw_result(pgf);
            vita2d_end_drawing();
            vita2d_swap_buffers();
        } else {
            if (game_mode != MODE_FREE) ff_level = 0;                         // fast forward is for free flight only, never in a challenge
            else if (pressed & SCE_CTRL_UP) ff_level = (ff_level + 1) % 3;   // D-pad up: normal -> x2 -> x5 -> normal
            unsigned long long pt0 = sceKernelGetProcessTimeWide();
            update(&pad, pressed);
            challenge_update();
            for (int k = 1; k < FF_MULT[ff_level] && !pl.crashed && ch_state != CH_DONE; k++) {   // fast forward: extra sim steps, no new button presses
                ff_sub = true;
                update(&pad, 0);
                challenge_update();
                ff_sub = false;
            }
            if (pl.crashed || (pressed & SCE_CTRL_TRIANGLE)) ff_level = 0;
            if (game_mode == MODE_LANDING && ch_state == CH_DONE) { game_state = ST_RESULT; ff_level = 0; }

            unsigned long long pt1 = sceKernelGetProcessTimeWide();
            vita2d_start_drawing();
            vita2d_clear_screen();
            draw_scene();
            unsigned long long pt2 = sceKernelGetProcessTimeWide();
            draw_overlays(pgf);
            if (chart_open) draw_chart(pgf);                          // holding Select
            draw_panel(pgf);
            if (game_state == ST_RESULT) draw_result(pgf);
            unsigned long long pt3 = sceKernelGetProcessTimeWide();
            vita2d_end_drawing();
            vita2d_swap_buffers();
            unsigned long long pt4 = sceKernelGetProcessTimeWide();
            prof_t[0] += pt1 - pt0; prof_t[1] += pt2 - pt1; prof_t[2] += pt3 - pt2; prof_t[3] += pt4 - pt3;
            prof_t[4] += pt4 - (prof_last ? prof_last : pt0);        // whole frame, start to start
            prof_last = pt4;
            if (++prof_n >= 300) {
                if (logfile) {
                    fprintf(logfile, "timing (avg us/frame over 300): sim %llu  scene %llu  hud+panel %llu  gpu/vsync %llu  total %llu  = %.1f fps  [ac=%d view=%d ff=%d]\n",
                            prof_t[0] / 300, prof_t[1] / 300, prof_t[2] / 300, prof_t[3] / 300, prof_t[4] / 300,
                            prof_t[4] ? 300.0e6 / (double)prof_t[4] : 0.0, ac_flying, view_mode, ff_level);
                    fflush(logfile);
                }
                memset(prof_t, 0, sizeof prof_t); prof_n = 0;
            }
        }

        snd_game_update(&pad);
        prev = pad;
        frame_count++;
        if (view_banner > 0) view_banner--;
        if (logfile && (frame_count <= 3 || frame_count % 300 == 0)) {
            fprintf(logfile, "frame %d\n", frame_count);
            fflush(logfile);
        }
    }

    snd_stop();
    vita2d_free_pgf(pgf);
    vita2d_fini();
    sceKernelExitProcess(0);
    return 0;
}
