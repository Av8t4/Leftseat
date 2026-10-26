// Left Seat: sound. The Vita side (a background thread that plays what the sound engine makes) and the part that tells the
// sound engine what the aircraft is doing, once a frame. The engine itself is in soundcore.inc.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

#include <stdint.h>
#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>

#define SND_GRAIN 1024                                  // samples per buffer: about 21 ms at 48,000 Hz

static SndIn   snd_in;                                  // the latest description of the moment (written by the game, read by the audio thread)
static SndSt   snd_st;                                  // the sound engine's own memory (touched only by the audio thread)
static SndRs   snd_rs;                                  // the converter to 48,000 Hz (also only the audio thread)
static int     snd_port = -1;
static SceUID  snd_thread = -1;
static volatile int snd_run;

#ifndef SND_NO_DEVICE
static int snd_thread_main(SceSize args, void *argp) {
    (void)args; (void)argp;
    static int16_t buf[SND_GRAIN];
    SndIn local;
    while (snd_run) {
        local = snd_in;                                 // a snapshot, so a half-written update can't matter
        snd_render_out(&snd_rs, &snd_st, &local, buf, SND_GRAIN);
        sceAudioOutOutput(snd_port, buf);               // waits until the previous buffer has been played
    }
    return 0;
}
#endif

// Opens the sound hardware and starts the thread. If anything fails the game simply stays silent.
static void snd_start(void) {
    snd_init(&snd_st);
    snd_rs_init(&snd_rs);
    memset(&snd_in, 0, sizeof(snd_in));
#ifndef SND_NO_DEVICE
    snd_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, SND_GRAIN, SND_OUT_RATE, SCE_AUDIO_OUT_MODE_MONO);
    if (snd_port < 0) { if (logfile) { fprintf(logfile, "sound: no audio port (0x%08X)\n", (unsigned)snd_port); fflush(logfile); } snd_port = -1; return; }
    int vol[2] = { SCE_AUDIO_VOLUME_0DB, SCE_AUDIO_VOLUME_0DB };
    sceAudioOutSetVolume(snd_port, (SceAudioOutChannelFlag)(SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), vol);
    snd_run = 1;
    snd_thread = sceKernelCreateThread("leftseat_audio", snd_thread_main, 0x10000100, 0x10000, 0, 0, NULL);
    if (snd_thread < 0 || sceKernelStartThread(snd_thread, 0, NULL) < 0) {
        if (logfile) { fprintf(logfile, "sound: thread failed (0x%08X)\n", (unsigned)snd_thread); fflush(logfile); }
        snd_run = 0; sceAudioOutReleasePort(snd_port); snd_port = -1; snd_thread = -1;
        return;
    }
    if (logfile) { fputs("sound ok\n", logfile); fflush(logfile); }
#endif
}

static void snd_stop(void) {
#ifndef SND_NO_DEVICE
    if (snd_thread >= 0) {
        snd_run = 0;
        sceKernelWaitThreadEnd(snd_thread, NULL, NULL);
        sceKernelDeleteThread(snd_thread);
        snd_thread = -1;
    }
    if (snd_port >= 0) { sceAudioOutReleasePort(snd_port); snd_port = -1; }
#endif
}

// How rough the ground under the wheels is, 0 (smooth tarmac) to 1 (bumpy grass).
static float snd_ground_rough(void) {
    for (int k = 0; k < AP_COUNT; k++) {
        const Airport *a = &AIRPORTS[k];
        float lx, lz;
        ap_local(a, pl.pos.x, pl.pos.z, &lx, &lz);
        if (k == AP_PINECREST) { if (fabsf(lx) < a->hw + 14.0f && fabsf(lz) < a->hl + 40.0f) return 0.50f; continue; }      // the mown strip
        if (fabsf(lx) < 320.0f && fabsf(lz) < a->hl + 100.0f) return 0.08f;                                               // runway, taxiways and apron
    }
    return 0.90f;                                                                                                           // anywhere else is grass
}

// Called once per frame, whatever the game is doing: describes the moment to the sound engine.
static void snd_game_update(const SceCtrlData *pad) {
    static bool prev_ground = true, prev_crashed = false, prev_dead = false, was_flying = false;
    static int prev_flaps = 0;
    static float air_t = 0.0f, last_vy = 0.0f, fail_t = 99.0f, prev_flash = 0.0f, thunder_t = -1.0f, fly_t = 0.0f;
    static const float LEVELS[4] = { 0.0f, 0.35f, 0.65f, 1.0f };
    const float dtf = 1.0f / 60.0f;
    SndIn in;
    memset(&in, 0, sizeof(in));                                     // every field starts at nothing, so nothing is left switched on
    in.volume = LEVELS[snd_level & 3];
    for (int e = 0; e < EV_COUNT; e++) { in.ev[e] = snd_evc[e]; in.ev_val[e] = snd_evv[e]; }
    const bool flying = (game_state == ST_FLY);
    in.aircraft_sound = flying ? 1 : 0;
    in.doppler = 1.0f;

    if (!flying) {                                                  // in the menu or on the score card: only the clicks and jingles
        was_flying = false;
        snd_in = in;
        return;
    }
    if (!was_flying) {                                              // just (re)started flying: forget what was going on before
        was_flying = true;
        prev_ground = pl.on_ground; prev_crashed = pl.crashed; prev_flaps = (int)(pl.flaps + 0.5f);
        prev_dead = (pl.fuel <= 0.0f || engine_failed); fail_t = 99.0f; air_t = 0.0f; thunder_t = -1.0f; prev_flash = wx_flash; fly_t = 0.0f;
    }

    // --- the engine
    const bool dead = (pl.fuel <= 0.0f || engine_failed);
    if (dead && !prev_dead) fail_t = 0.0f;                          // it has just stopped: a piston engine coughs for a couple of seconds first
    if (!dead) fail_t = 99.0f;
    prev_dead = dead;
    if (fail_t < 99.0f) fail_t += dtf;
    switch (spec->model) {
        case 1: case 4:                                             // turboprop (K-10, A22): the propeller speed gives the gas generator speed
            in.engine_kind = SND_TURBOPROP; in.blades = 3; in.prop_rpm = pl.rpm;
            in.n1 = clampf(0.36f + 0.64f * (pl.rpm - spec->rpm_idle) / spec->rpm_thr, 0.30f, 1.05f);
            in.load = clampf((in.n1 - 0.30f) / 0.70f, 0.0f, 1.0f);
            break;
        case 2: case 3:                                             // jet (Jet 1, K-30): the dial reads per cent N1
            in.engine_kind = SND_JET; in.n1 = clampf(pl.rpm / 100.0f, 0.0f, 1.05f); in.load = clampf(in.n1, 0.0f, 1.0f);
            break;
        default:
            in.engine_kind = SND_PISTON; in.blades = 2; in.rpm = pl.rpm;
            in.load = clampf((pl.rpm - 600.0f) / 1800.0f, 0.0f, 1.0f);
    }
    in.engine_ok = (!dead && !pl.crashed) ? 1 : 0;
    if (dead && spec->model == 0 && fail_t < 2.5f && !pl.crashed) { in.engine_ok = 1; in.misfire = 1.0f - fail_t / 2.5f; }    // sputtering, then silence

    // --- the air and the ground
    in.airspeed = pl.crashed ? 0.0f : pl.speed;
    in.flaps = pl.flaps;
    in.on_ground = pl.on_ground ? 1 : 0;
    in.ground_speed = pl.on_ground ? sqrtf(pl.vel.x * pl.vel.x + pl.vel.z * pl.vel.z) : 0.0f;
    in.brake = (pad && (pad->buttons & SCE_CTRL_DOWN) && pl.on_ground) ? 1.0f : 0.0f;
    in.rough = pl.on_ground ? snd_ground_rough() : 0.0f;
    if (!pl.on_ground && !pl.crashed) in.stall = clampf((pl.alpha - (ALPHA_S - 0.09f)) / 0.05f, 0.0f, 1.0f);   // the horn starts a few knots before the stall
    in.rain = clampf(wx.rain, 0.0f, 1.0f);
    in.turb = clampf(wx.turb * 2.0f, 0.0f, 1.0f);
    in.view = view_mode;
    if (view_mode == 2) {                                           // from the tower: quieter with distance, and the pitch shifts as it passes
        V3 d = vsub(cam.pos, pl.pos);
        float dist = sqrtf(vdot(d, d));
        in.distance = dist;
        if (dist > 1.0f) { float vr = vdot(pl.vel, vscale(d, 1.0f / dist)); in.doppler = clampf(340.0f / (340.0f - vr), 0.80f, 1.25f); }
    }

    // --- things that happen once
    fly_t += dtf;
    if (!pl.on_ground) { air_t += dtf; last_vy = pl.vel.y; }
    if (pl.on_ground && !prev_ground && air_t > 0.25f && fly_t > 1.5f)      // wheels down (not the aircraft settling onto the runway at the start)
        snd_event(EV_TOUCHDOWN, clampf(-last_vy / 3.0f, 0.15f, 1.0f));
    if (pl.on_ground) air_t = 0.0f;
    prev_ground = pl.on_ground;
    if (pl.crashed && !prev_crashed) snd_event(EV_CRASH, 1.0f);
    prev_crashed = pl.crashed;
    int fl = (int)(pl.flaps + 0.5f);
    if (fl != prev_flaps) snd_event(EV_FLAPS, 1.0f);
    prev_flaps = fl;
    if (wx_flash > 0.95f && prev_flash <= 0.95f) thunder_t = 0.4f + 3.5f * (float)(rand() % 100) / 100.0f;     // lightning: the thunder follows a little later
    prev_flash = wx_flash;
    if (thunder_t >= 0.0f) { thunder_t -= dtf; if (thunder_t < 0.0f) { snd_event(EV_THUNDER, 0.5f + 0.5f * (float)(rand() % 100) / 100.0f); thunder_t = -1.0f; } }

    for (int e = 0; e < EV_COUNT; e++) { in.ev[e] = snd_evc[e]; in.ev_val[e] = snd_evv[e]; }     // (including any event raised just now)
    snd_in = in;
}
