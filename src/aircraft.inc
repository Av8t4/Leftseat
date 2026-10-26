// Left Seat: the aircraft you can fly. Everything that makes one aircraft handle differently from another lives here.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* Aircraft specifications                                              */
/* ------------------------------------------------------------------ */

typedef struct { float x, y, z, ks, cs; } GearLeg;      // wheel-bottom position, spring and damper (see gear.inc)

typedef struct {
    const char *name, *key, *tag;                       // name for the menus; key for the scores file (as in older files, so saved scores carry over); short name for the menu row
    const char *line1, *line2;                          // a couple of facts for the hangar
    // Aerodynamics, per unit mass at full weight (SI units).
    float kl, cla, alpha0, alpha_s, cl_max;             // lift: accel per V^2 per unit CL, lift slope, zero-lift and stall angles
    float kd0, kdi, kd_windmill, kside;                 // parasite drag, induced drag, idling-propeller drag, side force
    float tstat, thrust_v0;                             // static thrust acceleration; thrust fades to zero at this speed
    float vref, alpha_trim, vne_ms;                     // control authority speed, neutral-stick angle of attack, never-exceed speed
    float flap_cl[4], flap_drag[4];                     // flaps 0 / 10 / 20 / 30 degrees
    float phugoid_damp, pitch_md, pitch_ma, pitch_mq;   // pitch control power, stiffness, damping
    float roll_pow, roll_damp, yaw_pow, yaw_damp, weathercock;
    // Fuel and engine.
    float fuel_full, weight_full, lb_per_gal, burn_idle, burn_full;     // gallons, lb at full fuel, gallons per hour
    float rpm_idle, rpm_thr, rpm_air, rpm_cap, windmill_k, windmill_cap, blade_vis_rpm;
    // Ground handling.
    float rot_speed, gpitch_max, brake;
    float piv_y;                                        // height of the point the aircraft rotates about, above the wheels
    float ixx, iyy, izz;                                // moments of inertia per unit mass (m^2)
    GearLeg gear[3];                                    // left main, right main, nose
    float tip_x, tip_y, tail_z, tail_y, prop_z, prop_y; // points that strike the ground
    // Cameras.
    float eye_x, eye_y, eye_z, chase_dist, chase_up;
    // Instruments (knots and rpm).
    float asi_white0, asi_white1, asi_green0, asi_green1, asi_yellow1;
    float tach_max, tach_green0, tach_green1, tach_red;
    // Challenge starts.
    float appr_kt; int appr_flaps; float glide_ms, glide_gamma, eng_h0, eng_h1;
    float prop_cy, prop_pz, prop_r, spin_r, spin_tip, spin_back; int nblades;      // propeller and spinner
    int model;                                          // which 3D model to draw
    // Engine type and instrument scales.
    float spool_up, spool_down;                         // seconds for a jet to spool up / down (0 = instant, as for the propeller aircraft)
    float thr_exp, thr_idle;                            // thrust grows as throttle^exp; idle thrust as a fraction of full (a jet pushes at idle)
    float pfactor, gpull;                               // propeller torque effects in the air and on the ground (zero for a jet)
    float asi_max, asi_step, asi_major;                 // airspeed dial: full scale, spacing of the numbers, ticks between long marks
    float tach_tick, tach_major, tach_mark, tach_round; // engine dial: unit between ticks and between numbers, red mark half-width, rounding
    const char *tach_unit;
    int canopy;                                         // 1 = bubble canopy (thin frame in the cockpit view)
    // Retractable landing gear (zero for the fixed-gear aircraft).
    int retract;                                        // 1 = the gear can be raised and lowered (Square)
    float kd_gear, gear_vmax, gear_time;                // extra drag with the gear out, fastest speed (m/s) at which it may move, seconds to move
} Spec;

enum { AC_K4 = 0, AC_K10, AC_JET1, AC_K30, AC_A22, AC_A11, AC_COUNT };

static const Spec SPECS[AC_COUNT] = {
    {   // ---- K-4 Trainer ------------------------------------------------------------
        "K-4 TRAINER", "skyhawk", "K-4",
        "180 hp piston, fixed gear, cruise 107 kt", "Stalls at 53 kt. Forgiving and easy to land.",
        0.00858f, 4.6f, -0.065f, 0.261f, 1.5f,
        0.00036f, 0.00049f, 0.00006f, 0.0027f,
        2.0f, 450.0f,
        55.0f, 0.065f, 83.8f,
        { 0.0f, 0.15f, 0.27f, 0.375f }, { 0.0f, 0.00008f, 0.00020f, 0.00034f },
        0.02f, 2.7f, 9.0f, 4.2f,
        1.3f, 2.8f, 0.9f, 2.5f, 5.0f,
        53.0f, 2550.0f, 6.0f, 2.0f, 9.5f,
        700.0f, 1600.0f, 7.0f, 2800.0f, 14.0f, 1200.0f, 1100.0f,
        15.0f, 0.24f, 3.5f,
        1.15f,
        0.82f, 1.16f, 1.70f,
        { { -1.25f, -0.118f, -0.30f, 34.7f, 9.0f }, { 1.25f, -0.118f, -0.30f, 34.7f, 9.0f }, { 0.0f, -0.162f, 1.62f, 10.0f, 5.0f } },
        5.5f, 2.2f, -4.6f, 1.1f, 3.10f, 0.14f,
        -0.32f, 1.62f, 0.45f, 15.0f, 4.0f,
        40.0f, 85.0f, 48.0f, 129.0f, 163.0f,
        3000.0f, 2100.0f, 2600.0f, 2700.0f,
        75.0f, 1, 36.0f, -0.098f, 440.0f, 520.0f,
        1.09f, 3.10f, 0.95f, 0.20f, 3.46f, 3.0f, 2,
        0,
        0.0f, 0.0f, 1.0f, 0.0f, 0.12f, 0.03f,
        200.0f, 40.0f, 2.0f,
        100.0f, 500.0f, 10.0f, 10.0f, "RPM",
        0
    },
    {   // ---- K-10 Hauler --------------------------------------------------------
        "K-10 HAULER", "caravan", "K-10",
        "675 hp turboprop, fixed gear, cruise 175 kt", "Stalls at 56 kt. Heavy, stable and powerful.",
        0.004769f, 4.9f, -0.060f, 0.270f, 1.62f,
        0.000153f, 0.000238f, 0.00004f, 0.0022f,
        2.2f, 231.0f,
        60.0f, 0.050f, 90.0f,
        { 0.0f, 0.35f, 0.70f, 0.98f }, { 0.0f, 0.00005f, 0.00014f, 0.00032f },
        0.02f, 2.2f, 8.0f, 4.6f,
        1.0f, 2.8f, 0.8f, 2.6f, 4.5f,
        335.0f, 7350.0f, 6.7f, 30.0f, 32.0f,
        1000.0f, 900.0f, 0.0f, 2000.0f, 14.0f, 1100.0f, 1350.0f,
        26.0f, 0.22f, 3.5f,
        1.55f,
        1.90f, 2.80f, 3.90f,
        { { -1.80f, -0.160f, -0.55f, 25.4f, 9.0f }, { 1.80f, -0.160f, -0.55f, 25.4f, 9.0f }, { 0.0f, -0.180f, 2.75f, 9.3f, 5.0f } },
        7.94f, 2.90f, -7.62f, 1.94f, 4.55f, 0.40f,
        -0.40f, 2.40f, 1.45f, 20.0f, 5.5f,
        61.0f, 125.0f, 70.0f, 150.0f, 175.0f,
        2400.0f, 1600.0f, 1900.0f, 1900.0f,
        85.0f, 2, 44.8f, -0.088f, 430.0f, 510.0f,
        1.70f, 4.55f, 1.30f, 0.28f, 5.00f, 4.40f, 3,
        1,
        0.0f, 0.0f, 1.0f, 0.0f, 0.12f, 0.03f,
        200.0f, 40.0f, 2.0f,
        100.0f, 500.0f, 10.0f, 10.0f, "RPM",
        0
    },
    {   // ---- Jet 1 Sprint ---------------------------------------------------------------
        "JET 1 SPRINT", "subsonex", "JET 1",
        "247 lb turbojet, V-tail, cruise 210 kt", "Stalls at 56 kt. Tiny, fast and quick on the controls.",
        0.007885f, 4.4f, -0.041f, 0.300f, 1.50f,
        0.000152f, 0.00062f, 0.00002f, 0.0030f,
        2.45f, 700.0f,
        60.0f, 0.022f, 128.0f,
        { 0.0f, 0.14f, 0.26f, 0.38f }, { 0.0f, 0.00005f, 0.00012f, 0.00022f },
        0.02f, 3.4f, 10.0f, 5.0f,
        4.2f, 3.0f, 1.1f, 2.8f, 4.5f,
        40.0f, 960.0f, 6.7f, 6.0f, 26.0f,
        30.0f, 70.0f, 0.0f, 105.0f, 0.25f, 16.0f, 0.0f,
        22.0f, 0.22f, 4.0f,
        0.70f,
        0.22f, 0.40f, 0.55f,
        { { -0.80f, -0.070f, -0.25f, 61.0f, 10.0f }, { 0.80f, -0.070f, -0.25f, 61.0f, 10.0f }, { 0.0f, -0.080f, 1.50f, 17.0f, 6.0f } },
        2.74f, 0.86f, -3.03f, 0.95f, 0.0f, 9.0f,
        0.0f, 1.30f, 0.50f, 9.5f, 2.0f,
        50.0f, 100.0f, 56.0f, 187.0f, 249.0f,
        110.0f, 30.0f, 100.0f, 103.0f,
        85.0f, 2, 50.0f, -0.085f, 420.0f, 500.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,
        2,
        2.8f, 1.6f, 1.3f, 0.04f, 0.0f, 0.0f,
        300.0f, 50.0f, 5.0f,
        5.0f, 20.0f, 0.8f, 1.0f, "%",
        1
    },
    {   // ---- K-30 Courier ---------------------------------------------------------------
        "K-30 COURIER", "k30", "K-30",
        "Twin turbofan, T-tail, canards, retractable gear", "Stalls at 82 kt. Fast and stable. Gear on Square.",
        0.003674f, 4.4f, -0.041f, 0.300f, 1.50f,
        0.0000698f, 0.00030f, 0.00002f, 0.0030f,
        3.4f, 600.0f,
        85.0f, 0.020f, 200.0f,
        { 0.0f, 0.14f, 0.26f, 0.38f }, { 0.0f, 0.000025f, 0.00006f, 0.00010f },
        0.02f, 2.6f, 9.0f, 5.0f,
        2.6f, 2.4f, 0.8f, 2.4f, 4.0f,
        380.0f, 11500.0f, 6.7f, 60.0f, 300.0f,
        30.0f, 70.0f, 0.0f, 105.0f, 0.25f, 16.0f, 0.0f,
        36.0f, 0.30f, 5.0f,
        1.50f,
        3.0f, 12.0f, 14.0f,
        { { -1.50f, -0.120f, -0.40f, 37.0f, 8.0f }, { 1.50f, -0.120f, -0.40f, 37.0f, 8.0f }, { 0.0f, -0.140f, 4.95f, 14.0f, 6.0f } },
        7.20f, 1.70f, -7.30f, 2.65f, 0.0f, 9.0f,
        0.0f, 3.05f, 5.50f, 26.0f, 5.0f,
        75.0f, 140.0f, 82.0f, 300.0f, 390.0f,
        110.0f, 30.0f, 100.0f, 103.0f,
        115.0f, 3, 62.0f, -0.09f, 600.0f, 700.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,
        3,
        3.2f, 2.0f, 1.3f, 0.04f, 0.0f, 0.0f,
        400.0f, 100.0f, 5.0f,
        5.0f, 20.0f, 0.8f, 1.0f, "%",
        0,
        1, 0.00004f, 103.0f, 4.5f
    },
    {   // ---- Ashley A22 --------------------------------------------------------
        "ASHLEY TWIN A22", "a22", "A22",
        "Twin pusher turboprop, retractable gear, cruise 200 kt", "Stalls at 62 kt. Fast, smooth and different. Gear on Square.",
        0.0041f, 4.9f, -0.060f, 0.270f, 1.45f,
        0.000124f, 0.000238f, 0.00006f, 0.0022f,
        3.1f, 275.0f,
        62.0f, 0.050f, 112.0f,
        { 0.0f, 0.35f, 0.70f, 0.90f }, { 0.0f, 0.00005f, 0.00014f, 0.00032f },
        0.02f, 2.2f, 8.0f, 4.6f,
        1.0f, 2.8f, 0.8f, 2.6f, 4.5f,
        320.0f, 7600.0f, 6.7f, 34.0f, 58.0f,
        1000.0f, 900.0f, 0.0f, 2000.0f, 14.0f, 1100.0f, 1350.0f,
        29.0f, 0.22f, 3.5f,
        1.55f,
        2.60f, 3.00f, 4.80f,
        { { -2.45f, -0.160f, -0.30f, 30.0f, 9.0f }, { 2.45f, -0.160f, -0.30f, 30.0f, 9.0f }, { 0.0f, -0.180f, 3.20f, 11.0f, 5.0f } },
        6.60f, 1.30f, -6.40f, 2.20f, 0.0f, 0.0f,
        -0.40f, 2.20f, 1.90f, 20.0f, 5.5f,
        64.0f, 130.0f, 74.0f, 175.0f, 205.0f,
        2400.0f, 1600.0f, 1900.0f, 1900.0f,
        82.0f, 2, 43.0f, -0.088f, 430.0f, 510.0f,
        1.90f, 2.50f, 1.05f, 0.20f, 3.00f, 2.35f, 3,
        4,
        0.0f, 0.0f, 1.0f, 0.0f, 0.12f, 0.03f,
        240.0f, 40.0f, 2.0f,
        100.0f, 500.0f, 10.0f, 10.0f, "RPM",
        0,
        1, 0.00004f, 103.0f, 4.5f
    },
    {   // ---- Ashley A11 ------------------------------------------------------------
        "ASHLEY SINGLE A11", "a11", "A11",
        "Single pusher piston, twin booms, fixed gear, cruise 150 kt", "Stalls at 52 kt. Small, quick and easy to fly.",
        0.0062f, 4.7f, -0.065f, 0.261f, 1.5f,
        0.00029f, 0.00049f, 0.00006f, 0.0027f,
        2.6f, 450.0f,
        55.0f, 0.060f, 100.0f,
        { 0.0f, 0.25f, 0.50f, 0.75f }, { 0.0f, 0.00008f, 0.00020f, 0.00034f },
        0.02f, 2.7f, 9.0f, 4.2f,
        1.3f, 2.8f, 0.9f, 2.5f, 5.0f,
        70.0f, 3300.0f, 6.0f, 3.0f, 14.0f,
        700.0f, 1600.0f, 7.0f, 2800.0f, 14.0f, 1200.0f, 1100.0f,
        17.0f, 0.22f, 3.5f,
        1.15f,
        1.00f, 1.30f, 2.00f,
        { { -1.15f, -0.118f, -0.55f, 34.7f, 9.0f }, { 1.15f, -0.118f, -0.55f, 34.7f, 9.0f }, { 0.0f, -0.162f, 3.95f, 10.0f, 5.0f } },
        4.9f, 1.15f, -5.20f, 1.75f, -1.75f, 1.66f,
        -0.36f, 1.96f, 3.10f, 15.0f, 4.0f,
        44.0f, 95.0f, 52.0f, 135.0f, 165.0f,
        3000.0f, 2100.0f, 2600.0f, 2700.0f,
        72.0f, 1, 37.0f, -0.098f, 440.0f, 520.0f,
        2.28f, -1.65f, 0.62f, 0.16f, -2.10f, -1.30f, 2,
        5,
        0.0f, 0.0f, 1.0f, 0.0f, 0.12f, 0.03f,
        200.0f, 40.0f, 2.0f,
        100.0f, 500.0f, 10.0f, 10.0f, "RPM",
        0
    }
};

static const Spec *spec = &SPECS[AC_K4];           // the aircraft being flown
static int ac_flying = AC_K4;                      // ... as an index

// The names the flight code has always used now read from the aircraft being flown.
#define KL           (spec->kl)
#define CLA          (spec->cla)
#define ALPHA0       (spec->alpha0)
#define ALPHA_S      (spec->alpha_s)
#define CL_MAX       (spec->cl_max)
#define KD0          (spec->kd0)
#define KDI          (spec->kdi)
#define KD_WINDMILL  (spec->kd_windmill)
#define KSIDE        (spec->kside)
#define TSTAT        (spec->tstat)
#define THRUST_V0    (spec->thrust_v0)
#define VREF         (spec->vref)
#define ALPHA_TRIM   (spec->alpha_trim)
#define FUEL_FULL    (spec->fuel_full)
#define WEIGHT_FULL  (spec->weight_full)
#define VNE_MS       (spec->vne_ms)
#define PHUGOID_DAMP (spec->phugoid_damp)
#define PITCH_MD     (spec->pitch_md)
#define PITCH_MA     (spec->pitch_ma)
#define PITCH_MQ     (spec->pitch_mq)
#define FLAP_CL      (spec->flap_cl)
#define FLAP_DRAG    (spec->flap_drag)
#define GEAR         (spec->gear)
#define IXX_M        (spec->ixx)
#define IYY_M        (spec->iyy)
#define IZZ_M        (spec->izz)

