// Left Seat: the sound engine's core (see the sound preview folder for recordings made with it). No audio files at all: every
// sound is made on the fly from what the aircraft is doing. It knows nothing about the Vita: the game describes the moment
// in a SndIn and snd_render() fills a buffer with 16-bit mono samples. The Vita side (sound.inc) just plays the buffer.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

#include <math.h>
#include <stdint.h>
#include <string.h>

#define SND_RATE   22050
#define SND_BLOCK  64

enum { SND_PISTON = 0, SND_TURBOPROP, SND_JET };
enum { EV_TOUCHDOWN = 0, EV_CRASH, EV_CLICK, EV_CONFIRM, EV_CHIME_GOOD, EV_CHIME_OK, EV_CHIME_BAD, EV_FLAPS, EV_THUNDER, EV_COUNT };

// What the game tells the sound engine, about once a frame.
typedef struct {
    float volume;             // 0..1: the player's setting
    int   engine_kind;        // SND_PISTON, SND_TURBOPROP or SND_JET
    float rpm;                // piston: engine rpm
    float n1;                 // turboprop and jet: gas generator speed, 0..1.05
    float prop_rpm;           // turboprop: propeller rpm
    int   blades;
    float load;               // how hard the engine is working, 0..1
    int   engine_ok;          // 0 when it has failed or run out of fuel
    float airspeed;           // m/s
    float flaps;              // 0..3
    int   on_ground;
    float ground_speed;       // m/s
    float brake;              // 0..1
    float rough;              // 0..1: grass is rougher than tarmac
    float stall;              // 0..1: the stall warning
    float rain;               // 0..1
    float turb;               // 0..1: gusts and bumps
    float misfire;            // 0..1: sputtering
    int   view;               // 0 cockpit, 1 chase, 2 tower
    float doppler;            // pitch change from the aircraft moving towards or away from the tower
    float distance;           // metres to the aircraft (tower view)
    int   aircraft_sound;     // 0 in the menu: the engine is silent
    unsigned ev[EV_COUNT];    // event counters: the game adds one each time something happens
    float    ev_val[EV_COUNT];// how strong it was
} SndIn;

typedef struct { int type; float t, a, b; } SndVoice;

typedef struct {
    unsigned rng;
    // smoothed copies of the inputs, so nothing clicks when a value jumps
    float s_rpm, s_n1, s_load, s_air, s_prop, s_gs, s_brake, s_stall, s_rain, s_turb, s_flaps, s_vol, s_on, s_dist, s_dop, s_mis, s_gate;
    // oscillators
    float ph_fire, ph_bp, ph_w1, ph_w2, ph_horn, ph_squeal, ph_whistle, ph_lfo1, ph_lfo2, ph_motor, ph_flap;
    // filters
    float e_lp, e_lp2, c_lp, h_lp1, h_lp2, r_lp1, r_lp2, w_lp1, w_lp2, p_b0, g_lp1, g_lp2, k_lp, rain_lp1, rain_lp2, j_lp1, j_lp2, j_lp3, v_lp1, v_lp2, v_lp3, dc;
    float cyc_amp; float gate_t;
    float tab[1024];
    unsigned last_ev[EV_COUNT];
    SndVoice voice[10];
    float lp_out, cp_lp, cp_lp2;
} SndSt;

static float snd_rnd(SndSt *s) { s->rng ^= s->rng << 13; s->rng ^= s->rng >> 17; s->rng ^= s->rng << 5; return (float)(s->rng & 0xFFFFFF) / 8388608.0f - 1.0f; }
static float snd_clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
static float snd_lpa(float fc) { return 1.0f - expf(-6.2831853f * fc / SND_RATE); }
static float snd_soft(float x) { return x * (27.0f + x * x) / (27.0f + 9.0f * x * x); }      // a smooth limiter (like tanh)

static void snd_init(SndSt *s) {
    memset(s, 0, sizeof(*s));
    s->rng = 0x1234567u; s->cyc_amp = 1.0f;
    // One firing of a piston engine: a thump that rings down, with a sharp crack at the start.
    for (int i = 0; i < 1024; i++) {
        float x = i / 1024.0f;
        s->tab[i] = expf(-x * 7.0f) * sinf(6.2831853f * x * 2.3) + 0.55f * expf(-x * 40.0f) * sinf(6.2831853f * x * 11.0f)
                  + 0.35f * expf(-x * 14.0f) * sinf(6.2831853f * x * 5.1f + 1.0f);
    }
    float m = 0.0f; for (int i = 0; i < 1024; i++) if (fabsf(s->tab[i]) > m) m = fabsf(s->tab[i]);
    for (int i = 0; i < 1024; i++) s->tab[i] /= m;
}

static void snd_start_voice(SndSt *s, int type, float a, float b) {
    int slot = 0; float oldest = -1.0f;
    for (int i = 0; i < 10; i++) { if (s->voice[i].type == 0) { slot = i; oldest = 1e9f; break; } if (s->voice[i].t > oldest) { oldest = s->voice[i].t; slot = i; } }
    s->voice[slot] = (SndVoice){ type, 0.0f, a, b };
}
enum { V_NONE = 0, V_THUMP, V_CHIRP, V_CRASH, V_CLICK, V_CONFIRM, V_NOTE, V_FLAPS, V_THUNDER, V_CLUNK };

// Rendering ------------------------------------------------------------------------------------------------------

static void snd_render(SndSt *s, const SndIn *inp, int16_t *out, int n) {
    SndIn in = *inp;
    const float dt = 1.0f / SND_RATE;
    for (int e = 0; e < EV_COUNT; e++) {                                       // new events since last time
        unsigned fresh = in.ev[e] - s->last_ev[e];
        if (fresh > 4) fresh = 4;
        for (unsigned k = 0; k < fresh; k++) {
            float v = in.ev_val[e];
            switch (e) {
                case EV_TOUCHDOWN: snd_start_voice(s, V_THUMP, v, 0); snd_start_voice(s, V_CHIRP, v, 0); break;
                case EV_CRASH:     snd_start_voice(s, V_CRASH, 1.0f, 0); break;
                case EV_CLICK:     snd_start_voice(s, V_CLICK, 1.0f, 0); break;
                case EV_CONFIRM:   snd_start_voice(s, V_CONFIRM, 1.0f, 0); break;
                case EV_CHIME_GOOD: for (int q = 0; q < 4; q++) snd_start_voice(s, V_NOTE, 0.20f * -(q) , 523.25f * powf(2.0f, (q == 0 ? 0 : q == 1 ? 4 : q == 2 ? 7 : 12) / 12.0f)); break;
                case EV_CHIME_OK:  snd_start_voice(s, V_NOTE, 0.0f, 523.25f); snd_start_voice(s, V_NOTE, -0.22f, 659.25f); break;
                case EV_CHIME_BAD: snd_start_voice(s, V_NOTE, 0.0f, 220.0f); snd_start_voice(s, V_NOTE, -0.28f, 164.8f); break;
                case EV_FLAPS:     snd_start_voice(s, V_FLAPS, v, 0); break;
                case EV_THUNDER:   snd_start_voice(s, V_THUNDER, v, 0); break;
            }
        }
        s->last_ev[e] = in.ev[e];
    }
    for (int base = 0; base < n; base += SND_BLOCK) {
        int m = n - base < SND_BLOCK ? n - base : SND_BLOCK;
        // --- once per block: smooth the inputs and work out the filter settings
        const float k = 1.0f - expf(-(float)SND_BLOCK / SND_RATE / 0.06f), kf = 1.0f - expf(-(float)SND_BLOCK / SND_RATE / 0.02f);
        float live = in.aircraft_sound ? 1.0f : 0.0f;
        s->s_on    += (live - s->s_on) * (1.0f - expf(-(float)SND_BLOCK / SND_RATE / 0.25f));
        s->s_vol   += (in.volume - s->s_vol) * kf;
        s->s_rpm   += (in.rpm - s->s_rpm) * k;
        s->s_n1    += (in.n1 - s->s_n1) * k;
        s->s_prop  += (in.prop_rpm - s->s_prop) * k;
        s->s_load  += (in.load - s->s_load) * k;
        s->s_air   += (in.airspeed - s->s_air) * k;
        s->s_gs    += (in.ground_speed - s->s_gs) * k;
        s->s_brake += (in.brake - s->s_brake) * kf;
        s->s_stall += (in.stall - s->s_stall) * (in.stall > s->s_stall ? kf : k * 0.5f);
        s->s_rain  += (in.rain - s->s_rain) * k * 0.3f;
        s->s_turb  += (in.turb - s->s_turb) * k * 0.5f;
        s->s_flaps += (in.flaps - s->s_flaps) * k;
        s->s_dist  += (in.distance - s->s_dist) * k;
        s->s_dop   += ((in.doppler > 0.0f ? in.doppler : 1.0f) - s->s_dop) * k;
        s->s_mis   += (in.misfire - s->s_mis) * k;
        s->gate_t -= (float)SND_BLOCK / SND_RATE;
        if (s->gate_t <= 0.0f) { s->gate_t = 0.05f + 0.08f * (snd_rnd(s) * 0.5f + 0.5f); s->s_gate = (snd_rnd(s) * 0.5f + 0.5f) < s->s_mis ? 0.0f : 1.0f; }
        const int view = in.view;
        const float bright = view == 1 ? 1.0f : (view == 2 ? 0.8f : (in.engine_kind == SND_JET ? 0.75f : 0.55f));
        const float far_k = view == 2 ? 1.0f / (1.0f + s->s_dist / 90.0f) : 1.0f;
        const float dop = view == 2 ? s->s_dop : 1.0f;
        const float eng_gain = (view == 0 ? 0.80f : (view == 1 ? 1.0f : 1.35f * far_k)) * s->s_on;
        const float wind_gain = (view == 0 ? 1.0f : (view == 1 ? 0.55f : 0.12f * far_k)) * s->s_on;
        float a_e = view == 0 ? snd_lpa(520.0f + 2000.0f * s->s_load * bright) : snd_lpa(1500.0f + 3200.0f * s->s_load * bright);   // piston exhaust brightness
        float a_e2 = view == 0 ? snd_lpa(950.0f + 1700.0f * bright) : snd_lpa(1200.0f + 3000.0f * bright);
        float a_cp = snd_lpa(3200.0f);                                           // the cabin muffles everything a little
        float a_chop = snd_lpa(450.0f + 500.0f * s->s_load);
        float a_wind = snd_lpa(110.0f + 24.0f * s->s_air);
        float a_rum = snd_lpa(170.0f), a_bump = snd_lpa(55.0f);
        float f_fire = s->s_rpm / 60.0f * 2.0f * dop, f_bp = (in.engine_kind == SND_PISTON ? s->s_rpm / 60.0f * 2.0f : s->s_prop / 60.0f * (float)(in.blades > 0 ? in.blades : 2)) * dop;
        float f_w1 = (480.0f + 1500.0f * s->s_n1) * dop * (1.0f + 0.012f * sinf(6.2831853f * s->ph_lfo2 * 4.7f));   // turboprop whine, with a little flutter
        float f_j1 = (480.0f + 4300.0f * powf(snd_clampf(s->s_n1, 0.0f, 1.1f), 1.15f)) * dop;                // jet spool tone
        float n1 = snd_clampf(s->s_n1, 0.0f, 1.1f);
        float a_hiss_hi = snd_lpa(3000.0f * (0.6f + 0.4f * bright)), a_hiss_lo = snd_lpa(900.0f);
        float a_roar_hi = snd_lpa(650.0f + 2300.0f * n1 * (0.6f + 0.4f * bright)), a_roar_lo = snd_lpa(130.0f + 150.0f * n1);
        float wind_amp = powf(snd_clampf(s->s_air / 80.0f, 0.0f, 1.4f), 1.7f) * 0.62f * (1.0f + 0.08f * s->s_flaps);
        float whistle = snd_clampf((s->s_air - 55.0f) / 55.0f, 0.0f, 1.0f);
        float rain_amp = s->s_rain * (0.10f + 0.22f * snd_clampf(s->s_air / 60.0f, 0.0f, 1.0f)) * (view == 0 ? 1.0f : 0.35f) * s->s_on;
        float roll_amp = in.on_ground ? snd_clampf(s->s_gs / 38.0f, 0.0f, 1.2f) * (0.20f + 0.65f * in.rough) * (view == 0 ? 0.9f : 0.5f) * s->s_on : 0.0f;
        float brake_amp = in.on_ground ? s->s_brake * snd_clampf(s->s_gs / 14.0f, 0.0f, 1.0f) * s->s_on : 0.0f;
        float horn_amp = s->s_stall * 0.20f * (view == 2 ? far_k : 1.0f) * s->s_on;
        float run = in.engine_ok ? 1.0f : 0.0f;
        for (int i = 0; i < m; i++) {
            float y = 0.0f, ya = 0.0f;                                         // ya: the engine and the air noise, which the cabin muffles
            float nz = snd_rnd(s);
            // ---------------- engine
            if (in.engine_kind == SND_PISTON) {
                s->ph_fire += f_fire * dt;
                if (s->ph_fire >= 1.0f) { s->ph_fire -= 1.0f; s->cyc_amp = 0.86f + 0.14f * (snd_rnd(s) * 0.5f + 0.5f) * 2.0f * 0.5f + 0.06f * snd_rnd(s); }
                float pos = s->ph_fire * 1024.0f; int i0 = (int)pos; float fr = pos - i0;
                float th = s->tab[i0 & 1023] * (1.0f - fr) + s->tab[(i0 + 1) & 1023] * fr;
                float body = th * s->cyc_amp * (0.55f + 0.55f * s->s_load) * run * (in.misfire > 0.01f ? s->s_gate : 1.0f);
                s->e_lp += (body - s->e_lp) * a_e; s->e_lp2 += (s->e_lp - s->e_lp2) * a_e2;
                s->ph_bp += f_bp * dt; if (s->ph_bp >= 1.0f) s->ph_bp -= 1.0f;
                float chop = 0.5f + 0.5f * sinf(6.2831853f * s->ph_bp); chop *= chop;
                s->c_lp += (nz - s->c_lp) * a_chop;
                float prop_noise = s->c_lp * chop * (0.30f + 0.40f * s->s_load + 0.0030f * s->s_air);
                ya += eng_gain * (1.35f * s->e_lp2 + 1.10f * prop_noise * (s->s_rpm > 300.0f ? 1.0f : 0.0f));
            } else if (in.engine_kind == SND_TURBOPROP) {
                s->ph_w1 += f_w1 * dt; if (s->ph_w1 >= 1.0f) s->ph_w1 -= 1.0f;
                float whine = sinf(6.2831853f * s->ph_w1) * 0.60f + sinf(12.566371f * s->ph_w1) * 0.10f;
                float whine_amp = (0.25f + 0.75f * n1) * (0.25f + 0.75f * n1) * 0.42f * run + 0.04f * (1.0f - run) * n1;
                s->h_lp1 += (nz - s->h_lp1) * a_hiss_hi; s->h_lp2 += (nz - s->h_lp2) * a_hiss_lo;
                float hiss = (s->h_lp1 - s->h_lp2) * (0.12f + 0.88f * n1 * n1) * 2.4f;
                s->ph_bp += f_bp * dt; if (s->ph_bp >= 1.0f) s->ph_bp -= 1.0f;
                float sw = 0.5f + 0.5f * sinf(6.2831853f * s->ph_bp); float thrum = (sw * sw * sw - 0.3125f) * 2.6f;
                s->e_lp += (thrum - s->e_lp) * snd_lpa(340.0f + 520.0f * n1 * bright);
                s->c_lp += (nz - s->c_lp) * a_chop;
                float chop = 0.5f + 0.5f * sinf(6.2831853f * s->ph_bp + 0.7f); chop *= chop;
                float prop_part = (0.85f * s->e_lp + 0.75f * s->c_lp * chop) * (0.35f + 0.65f * s->s_load) * (s->s_prop > 200.0f ? 1.0f : 0.3f);
                s->r_lp1 += (nz - s->r_lp1) * snd_lpa(160.0f);
                s->r_lp2 += (nz - s->r_lp2) * snd_lpa(420.0f);                                                       // the turbine growl
                float throb = 1.0f + 0.10f * sinf(6.2831853f * s->ph_bp / (float)(in.blades > 0 ? in.blades : 3));      // the whole engine pulses at the propeller's turn
                ya += eng_gain * 0.45f * throb * (whine * whine_amp * (0.6f + 0.4f * bright) + hiss * 0.30f + prop_part * 0.85f + s->r_lp1 * 0.14f * n1 + s->r_lp2 * 0.45f * (0.2f + 0.8f * n1));
            } else {
                s->ph_w1 += f_j1 * dt; if (s->ph_w1 >= 1.0f) s->ph_w1 -= 1.0f;
                s->ph_w2 += f_j1 * 2.013f * dt; if (s->ph_w2 >= 1.0f) s->ph_w2 -= 1.0f;
                float tone = sinf(6.2831853f * s->ph_w1) * 0.6f + sinf(6.2831853f * s->ph_w2) * 0.25f;
                float tone_amp = (0.50f + 0.50f * n1) * 0.17f * (0.3f + 0.7f * run + 0.2f * (1.0f - run));
                s->j_lp1 += (nz - s->j_lp1) * a_roar_hi; s->j_lp2 += (nz - s->j_lp2) * a_roar_lo;
                float roar = (s->j_lp1 - s->j_lp2) * (0.16f + 0.84f * powf(n1, 1.7f)) * 2.0f * (0.35f + 0.65f * run);
                s->j_lp3 += (nz - s->j_lp3) * snd_lpa(110.0f);
                float rumble = s->j_lp3 * powf(n1, 1.2f) * 0.8f;
                ya += eng_gain * 0.50f * (tone * tone_amp + roar + rumble);
            }
            // ---------------- air noise
            s->w_lp1 += (nz - s->w_lp1) * a_wind; s->w_lp2 += (s->w_lp1 - s->w_lp2) * a_wind;
            float gust = 1.0f;
            s->ph_lfo1 += 0.31f * dt; s->ph_lfo2 += 0.77f * dt; if (s->ph_lfo1 > 1.0f) s->ph_lfo1 -= 1.0f; if (s->ph_lfo2 > 1.0f) s->ph_lfo2 -= 1.0f;
            gust += s->s_turb * (0.5f * sinf(6.2831853f * s->ph_lfo1) + 0.35f * sinf(6.2831853f * s->ph_lfo2));
            ya += wind_gain * s->w_lp2 * 2.6f * wind_amp * gust;
            if (whistle > 0.01f) {
                s->ph_whistle += (1700.0f + 9.0f * s->s_air) * dt; if (s->ph_whistle >= 1.0f) s->ph_whistle -= 1.0f;
                ya += wind_gain * sinf(6.2831853f * s->ph_whistle) * 0.035f * whistle * whistle * (0.6f + 0.4f * sinf(6.2831853f * s->ph_lfo2 * 2.0f));
            }
            // ---------------- rain
            if (rain_amp > 0.001f) {
                s->rain_lp1 += (nz - s->rain_lp1) * snd_lpa(6500.0f); s->rain_lp2 += (nz - s->rain_lp2) * snd_lpa(1800.0f);
                float drop = 0.0f;
                if ((snd_rnd(s) * 0.5f + 0.5f) < s->s_rain * 0.004f) drop = (snd_rnd(s) * 0.5f + 0.5f) * 1.6f;
                s->k_lp += (drop - s->k_lp) * 0.5f;
                y += rain_amp * ((s->rain_lp1 - s->rain_lp2) * 1.5f + s->k_lp * 1.4f);
            }
            // ---------------- the ground
            if (roll_amp > 0.001f) {
                s->g_lp1 += (nz - s->g_lp1) * a_rum; s->g_lp2 += (nz - s->g_lp2) * a_bump;
                float bump = s->g_lp2 * (0.4f + 2.2f * in.rough);
                y += roll_amp * (s->g_lp1 * 1.2f + bump * 1.6f);
            }
            if (brake_amp > 0.001f) {
                s->dc += (nz - s->dc) * snd_lpa(2400.0f);
                s->ph_squeal += (3100.0f + 120.0f * snd_rnd(s)) * dt; if (s->ph_squeal >= 1.0f) s->ph_squeal -= 1.0f;
                y += brake_amp * ((nz - s->dc) * 0.07f + (s->s_brake > 0.7f && s->s_gs > 12.0f ? sinf(6.2831853f * s->ph_squeal) * 0.035f : 0.0f));
            }
            // ---------------- the stall warning horn
            if (horn_amp > 0.002f) {
                s->ph_horn += 1090.0f * dt; if (s->ph_horn >= 1.0f) s->ph_horn -= 1.0f;
                s->ph_motor += 27.0f * dt; if (s->ph_motor >= 1.0f) s->ph_motor -= 1.0f;
                float sq = (s->ph_horn < 0.5f ? 1.0f : -1.0f) * 0.55f + sinf(6.2831853f * s->ph_horn) * 0.45f;
                s->p_b0 += (sq - s->p_b0) * snd_lpa(3200.0f);
                y += horn_amp * s->p_b0 * (0.72f + 0.28f * sinf(6.2831853f * s->ph_motor));
            }
            // ---------------- one-shot sounds
            for (int v = 0; v < 10; v++) {
                SndVoice *vo = &s->voice[v];
                if (vo->type == V_NONE) continue;
                float t = vo->t; vo->t += dt;
                float g = 0.0f;
                switch (vo->type) {
                    case V_THUMP: { float st = snd_clampf(vo->a, 0.1f, 1.0f); g = st * 0.75f * expf(-t * 16.0f) * sinf(6.2831853f * (48.0f + 30.0f * expf(-t * 12.0f)) * t); if (t > 0.5f) vo->type = V_NONE; break; }
                    case V_CHIRP: { float st = snd_clampf(vo->a, 0.1f, 1.0f); s->v_lp1 += (nz - s->v_lp1) * 0.5f; g = st * 0.32f * expf(-t * 22.0f) * (nz - s->v_lp1) * (0.6f + 0.4f * sinf(t * 2.0f * 6.2831853f * 90.0f)); if (t > 0.35f) vo->type = V_NONE; break; }
                    case V_CRASH: { s->v_lp2 += (nz - s->v_lp2) * 0.07f; float low = s->v_lp2 * expf(-t * 1.7f) * 2.3f; float crunch = nz * expf(-t * 11.0f) * 0.55f; float boom = sinf(6.2831853f * 38.0f * t) * expf(-t * 4.0f) * 0.8f; g = low + crunch + boom; if (t > 2.6f) vo->type = V_NONE; break; }
                    case V_CLICK: { g = sinf(6.2831853f * 1900.0f * t) * expf(-t * 130.0f) * 0.20f; if (t > 0.06f) vo->type = V_NONE; break; }
                    case V_CONFIRM: { float f = t < 0.06f ? 1250.0f : 1760.0f; g = sinf(6.2831853f * f * t) * expf(-t * 22.0f) * 0.22f; if (t > 0.22f) vo->type = V_NONE; break; }
                    case V_NOTE: { float tt = t + vo->a; if (tt < 0.0f) { g = 0.0f; break; } float f = vo->b; g = (sinf(6.2831853f * f * tt) + 0.35f * sinf(12.566371f * f * tt) + 0.12f * sinf(18.849556f * f * tt)) * expf(-tt * 3.4f) * 0.17f * (tt < 0.01f ? tt / 0.01f : 1.0f); if (tt > 1.8f) vo->type = V_NONE; break; }
                    case V_FLAPS: { float dur = 0.85f; if (t < dur) { float f = 210.0f + 55.0f * (t / dur); s->ph_flap += f * dt; if (s->ph_flap >= 1.0f) s->ph_flap -= 1.0f; g = (sinf(6.2831853f * s->ph_flap) * 0.5f + nz * 0.12f * 0.5f) * 0.11f * (view == 0 ? 1.0f : 0.25f) * snd_clampf(t / 0.05f, 0, 1) * snd_clampf((dur - t) / 0.08f, 0, 1); } else if (t < dur + 0.12f) { float tt = t - dur; g = sinf(6.2831853f * 140.0f * tt) * expf(-tt * 50.0f) * 0.22f * (view == 0 ? 1.0f : 0.3f); } else vo->type = V_NONE; break; }
                    case V_THUNDER: { s->v_lp3 += (nz - s->v_lp3) * 0.045f; float env = (1.0f - expf(-t * 4.0f)) * expf(-t * 0.55f) * (0.75f + 0.25f * sinf(t * 7.0f)); g = s->v_lp3 * env * 3.0f * snd_clampf(vo->a, 0.2f, 1.0f); if (t > 7.0f) vo->type = V_NONE; break; }
                    default: vo->type = V_NONE;
                }
                y += g;
            }
            if (view == 0) { s->cp_lp += (ya - s->cp_lp) * a_cp; ya = s->cp_lp; }
            y += ya;
            { float hp = y - s->lp_out; s->lp_out += hp * 0.00228f; y = hp; }
            y *= s->s_vol * 0.85f;
            out[base + i] = (int16_t)(snd_soft(y) * 30000.0f);
        }
    }
}


/* ------------------------------------------------------------------ */
/* Playing at 48,000 Hz                                                 */
/* ------------------------------------------------------------------ */
// The Vita's main audio port only accepts 48,000 Hz, and the sounds are made at 22,050 Hz (which is what they were tuned at),
// so the output is converted with a smooth (cubic) interpolation.

#define SND_OUT_RATE 48000

typedef struct { float y[4]; float mu; int16_t blk[SND_BLOCK]; int blk_pos, blk_n; } SndRs;
static void snd_rs_init(SndRs *r) { memset(r, 0, sizeof(*r)); r->mu = 1.0f; }
static float snd_rs_next(SndRs *r, SndSt *s, const SndIn *in) {
    if (r->blk_pos >= r->blk_n) { snd_render(s, in, r->blk, SND_BLOCK); r->blk_n = SND_BLOCK; r->blk_pos = 0; }
    return r->blk[r->blk_pos++] / 32768.0f;
}
// Fills n samples at SND_OUT_RATE.
static void snd_render_out(SndRs *r, SndSt *s, const SndIn *in, int16_t *out, int n) {
    const float ratio = (float)SND_RATE / (float)SND_OUT_RATE;
    for (int i = 0; i < n; i++) {
        while (r->mu >= 1.0f) { r->mu -= 1.0f; r->y[0] = r->y[1]; r->y[1] = r->y[2]; r->y[2] = r->y[3]; r->y[3] = snd_rs_next(r, s, in); }
        float t = r->mu, a = r->y[0], b = r->y[1], c = r->y[2], d = r->y[3];
        float v = b + 0.5f * t * (c - a + t * (2.0f * a - 5.0f * b + 4.0f * c - d + t * (3.0f * (b - c) + d - a)));      // Catmull-Rom
        if (v > 1.0f) v = 1.0f; else if (v < -1.0f) v = -1.0f;
        out[i] = (int16_t)(v * 32767.0f);
        r->mu += ratio;
    }
}

/* ------------------------------------------------------------------ */
/* What the rest of the game uses                                       */
/* ------------------------------------------------------------------ */

static int snd_level = 2;                                          // the player's volume setting: 0 off, 1 low, 2 medium, 3 high (kept in the scores file)
static const char *SND_NAMES[4] = { "OFF", "LOW", "MEDIUM", "HIGH" };
static unsigned snd_evc[EV_COUNT];                                 // event counters and strengths, read by snd_game_update()
static float    snd_evv[EV_COUNT];
static void snd_event(int e, float strength) { snd_evv[e] = strength; snd_evc[e]++; }     // something happened: a click, a touchdown ...
