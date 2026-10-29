// Left Seat: flying with the Vita's motion sensor. Tilt the Vita like a control column: left / right is roll, top edge toward you is nose up.
// The tilt is measured from the way you hold it when the flight starts (or when you press D-pad down + Circle), so any comfortable grip works.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

static int   gy_mode;                     // flight controls: 0 = sticks, 1 = gyro (tilt)
static int   gy_rinv, gy_pinv;            // 1 = roll / pitch direction reversed
static int   gy_sens = 1;                 // 0 = low (tilt far), 1 = medium, 2 = high (tilt a little)
static float gy_f[3] = { 0.0f, 1.0f, 0.0f };   // the filtered direction of gravity in the Vita's own axes
static float gy_n[3] = { 0.0f, 1.0f, 0.0f };   // ... as held when centerd
static bool  gy_have, gy_ncal;
static float gy_roll_deg, gy_pitch_deg;   // tilt from the centerd position in degrees: roll right is positive, top edge away from you is positive
static float gy_ail, gy_ele;              // what the tilt asks for: -1..1 (aileron right positive, elevator nose up positive)

static const float GY_FULL[3] = { 40.0f, 28.0f, 18.0f };      // degrees of tilt for full control
#define GY_DEAD 2.5f

static void gyro_recenter(void) { for (int i = 0; i < 3; i++) gy_n[i] = gy_f[i]; gy_ncal = gy_have; }

// Read the sensor once per frame (the menu shows the tilt live too).
static void gyro_sample(void) {
    SceMotionState ms;
    if (sceMotionGetState(&ms) != 0) return;
    float a[3] = { ms.acceleration.x, ms.acceleration.y, ms.acceleration.z };
    float m = sqrtf(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    if (m < 0.3f) return;                                          // free fall or no data
    for (int i = 0; i < 3; i++) a[i] /= m;
    if (!gy_have) { for (int i = 0; i < 3; i++) gy_f[i] = a[i]; gy_have = true; }
    else for (int i = 0; i < 3; i++) gy_f[i] += (a[i] - gy_f[i]) * 0.30f;
    if (!gy_ncal) { for (int i = 0; i < 3; i++) gy_n[i] = gy_f[i]; gy_ncal = true; }
    // Roll turns the Vita about the axis out of the screen: the angle between the two directions as seen in the x-y plane.
    // Pitch turns it about its left-right axis: the same, in the y-z plane. (Either is left alone if the Vita is held flat, where the angle is meaningless.)
    float rx0 = gy_n[0], ry0 = gy_n[1], rx = gy_f[0], ry = gy_f[1];
    if (sqrtf(rx0 * rx0 + ry0 * ry0) > 0.3f && sqrtf(rx * rx + ry * ry) > 0.25f)
        gy_roll_deg = atan2f(rx0 * ry - ry0 * rx, rx0 * rx + ry0 * ry) * 57.29578f;
    float py0 = gy_n[1], pz0 = gy_n[2], py = gy_f[1], pz = gy_f[2];
    if (sqrtf(py0 * py0 + pz0 * pz0) > 0.3f && sqrtf(py * py + pz * pz) > 0.25f)
        gy_pitch_deg = atan2f(py0 * pz - pz0 * py, py0 * py + pz0 * pz) * 57.29578f;
    float full = GY_FULL[gy_sens < 0 ? 0 : (gy_sens > 2 ? 2 : gy_sens)];
    float r = gy_roll_deg, p = -gy_pitch_deg;                      // pulling the top edge toward you is nose up
    r = (fabsf(r) <= GY_DEAD) ? 0.0f : (r > 0.0f ? r - GY_DEAD : r + GY_DEAD);
    p = (fabsf(p) <= GY_DEAD) ? 0.0f : (p > 0.0f ? p - GY_DEAD : p + GY_DEAD);
    gy_ail = clampf(r / (full - GY_DEAD), -1.0f, 1.0f) * (gy_rinv ? -1.0f : 1.0f);
    gy_ele = clampf(p / (full - GY_DEAD), -1.0f, 1.0f) * (gy_pinv ? -1.0f : 1.0f);
}
