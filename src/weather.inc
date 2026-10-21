// Left Seat: weather state, wind, fog and sky colours.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* Weather: clouds, fog, rain and wind that start where you choose and */
/* then drift from one kind of weather to another                      */
/* ------------------------------------------------------------------ */

typedef struct { float cloud, fog, rain, wind, turb; } WxState;
enum { WX_CLEAR, WX_BREEZY, WX_OVERCAST, WX_FOG, WX_RAIN, WX_STORM, WX_COUNT };
static const char *const WX_NAMES[WX_COUNT] = { "CLEAR", "BREEZY", "OVERCAST", "FOG", "RAIN", "STORM" };
//                                               cloud  fog    rain   wind(m/s) gusts
static const WxState WX_PRESET[WX_COUNT] = {
    { 0.10f, 0.00f, 0.00f,  3.0f, 0.05f },     // clear
    { 0.40f, 0.00f, 0.00f,  8.0f, 0.18f },     // breezy, some clouds
    { 0.85f, 0.15f, 0.00f,  8.0f, 0.22f },     // overcast
    { 0.70f, 0.90f, 0.00f,  2.0f, 0.05f },     // fog
    { 0.95f, 0.40f, 0.60f, 10.0f, 0.40f },     // rain
    { 1.00f, 0.55f, 1.00f, 17.0f, 0.90f },     // storm
};
// Which weather can follow which (repeats make the likelier choices).
static const int WX_NEXT[WX_COUNT][3] = {
    { WX_BREEZY, WX_BREEZY, WX_FOG },
    { WX_CLEAR, WX_OVERCAST, WX_OVERCAST },
    { WX_BREEZY, WX_RAIN, WX_FOG },
    { WX_OVERCAST, WX_CLEAR, WX_BREEZY },
    { WX_OVERCAST, WX_STORM, WX_OVERCAST },
    { WX_RAIN, WX_RAIN, WX_OVERCAST },
};

static WxState wx     = { 0.10f, 0.0f, 0.0f, 3.0f, 0.05f };
static WxState wx_tgt = { 0.10f, 0.0f, 0.0f, 3.0f, 0.05f };
static float wx_dir, wx_dir_tgt;          // direction the wind blows FROM (radians, 0 = north)
static float wx_flash, wx_time;           // lightning flash, seconds of weather time
static float wx_off_x, wx_off_z;          // how far the clouds have drifted
static float cloud_fx;                    // 0..1: the aircraft is inside a cloud
static int   wx_hold, wx_cur;             // frames until the next change, current weather kind
static bool  wx_drift;                    // only the Random choice lets the weather change by itself
static float fog_r = 196.0f, fog_g = 214.0f, fog_b = 236.0f, fog_vis = 35000.0f;

static unsigned int hash2(int a, int b) {
    unsigned int h = (unsigned int)a * 374761393u + (unsigned int)b * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}
static float hf(unsigned int h, int k) { return (float)((h >> (k * 8)) & 255u) / 255.0f; }
static float wx_noise(float t) { return (sinf(t) + 0.6f * sinf(2.3f * t + 1.7f) + 0.35f * sinf(5.1f * t + 4.1f)) / 1.95f; }

static void wx_start(int preset, bool drift) {
    wx = WX_PRESET[preset]; wx_tgt = wx; wx_cur = preset; wx_drift = drift;
    wx_dir = wx_dir_tgt = frand(-0.7f, 0.7f);                    // wind from roughly along the runway
    wx_hold = (int)frand(5400.0f, 12000.0f);
    wx_flash = 0.0f; wx_time = 0.0f; wx_off_x = wx_off_z = 0.0f; cloud_fx = 0.0f;
}
static void wx_set_target(int preset, bool drift) {
    wx_tgt = WX_PRESET[preset]; wx_cur = preset; wx_drift = drift;
    wx_hold = (int)frand(5400.0f, 12000.0f);
}

static void wx_update(void) {
    wx_time += DT;
    const float k = 0.0008f;                                      // drift over roughly 20 seconds
    wx.cloud += (wx_tgt.cloud - wx.cloud) * k;
    wx.fog   += (wx_tgt.fog   - wx.fog)   * k;
    wx.rain  += (wx_tgt.rain  - wx.rain)  * k;
    wx.wind  += (wx_tgt.wind  - wx.wind)  * k;
    wx.turb  += (wx_tgt.turb  - wx.turb)  * k;
    wx_dir   += (wx_dir_tgt   - wx_dir)   * 0.0004f;
    if (wx_drift && --wx_hold <= 0) {                             // time for the weather to change (Random only)
        wx_cur = WX_NEXT[wx_cur][rand() % 3];
        wx_tgt = WX_PRESET[wx_cur];
        wx_dir_tgt = clampf(wx_dir + frand(-0.8f, 0.8f), -1.2f, 1.2f);
        wx_hold = (int)frand(5400.0f, 12000.0f);
    }
    wx_off_x += -sinf(wx_dir) * wx.wind * 0.8f * DT;              // clouds ride the wind
    wx_off_z += -cosf(wx_dir) * wx.wind * 0.8f * DT;
    if (wx_flash > 0.0f) wx_flash = fmaxf(0.0f, wx_flash - 0.08f);
    else if (wx.rain > 0.65f && (rand() % 420) == 0) wx_flash = 1.0f;   // lightning
}

// Wind at a point: steady wind (weaker near the ground) plus smooth gusts and up/down-drafts.
static V3 wx_wind_at(V3 pos) {
    float sp = wx.wind * (0.45f + 0.55f * clampf(pos.y / 250.0f, 0.0f, 1.0f));
    V3 w = v3(-sinf(wx_dir) * sp, 0.0f, -cosf(wx_dir) * sp);
    float t = wx_time, a = wx.turb;
    w.x += a * 6.0f * wx_noise(0.9f * t + 0.0041f * pos.x + 0.0029f * pos.z + 0.3f);
    w.z += a * 6.0f * wx_noise(1.1f * t + 0.0035f * pos.z - 0.0027f * pos.x + 2.1f);
    w.y += a * 3.5f * wx_noise(1.3f * t + 0.0049f * (pos.x + pos.z) + 4.4f);
    return w;
}

static const char *wx_label(void) {
    int best = 0;
    float bd = 1e9f;
    for (int i = 0; i < WX_COUNT; i++) {
        const WxState *p = &WX_PRESET[i];
        float d = fabsf(wx.cloud - p->cloud) + 1.5f * fabsf(wx.fog - p->fog) + 1.5f * fabsf(wx.rain - p->rain)
                + fabsf(wx.wind - p->wind) / 12.0f + fabsf(wx.turb - p->turb);
        if (d < bd) { bd = d; best = i; }
    }
    return WX_NAMES[best];
}

// Per-frame fog colour, visibility and sky colour.
// ---- Time of day: fixed for a flight, chosen in the menu -----------------------------------
enum { TOD_DAY = 0, TOD_DUSK, TOD_NIGHT, TOD_COUNT };
static const char *const TOD_NAMES[TOD_COUNT] = { "DAY", "DUSK", "NIGHT" };
static int   tod = TOD_DAY;
static float sky_r = 118.0f, sky_g = 176.0f, sky_b = 236.0f;       // sky colour straight overhead this frame

// Colour of a lit surface at this time of day. Lights and other glowing things do not use this.
static unsigned int tod_apply(unsigned int col) {
    if (tod == TOD_DAY) return col;
    float r = (float)(col & 255u), g = (float)((col >> 8) & 255u), b = (float)((col >> 16) & 255u);
    if (tod == TOD_DUSK) { r = r * 0.88f + 6.0f; g = g * 0.66f + 4.0f; b = b * 0.60f + 8.0f; }
    else                 { r = r * 0.16f + 3.0f; g = g * 0.20f + 5.0f; b = b * 0.34f + 12.0f; }
    return RGBA8((int)clampf(r, 0.0f, 255.0f), (int)clampf(g, 0.0f, 255.0f), (int)clampf(b, 0.0f, 255.0f), (int)(col >> 24));
}

static void wx_frame_setup(void) {
    float f = clampf(fmaxf(wx.fog, cloud_fx * 0.97f), 0.0f, 1.0f);
    fog_vis = 35000.0f * powf(0.02f, f);
    float gl = clampf(wx.cloud * 1.1f, 0.0f, 1.0f), dk = clampf(wx.rain * 0.85f, 0.0f, 1.0f);
    float r = 196.0f + (178.0f - 196.0f) * gl, g = 214.0f + (184.0f - 214.0f) * gl, b = 236.0f + (194.0f - 236.0f) * gl;
    r += (92.0f - r) * dk; g += (98.0f - g) * dk; b += (112.0f - b) * dk;
    float wr = 232.0f - 70.0f * wx.rain;                          // inside a cloud everything goes white-grey
    r += (wr - r) * cloud_fx; g += (wr + 2.0f - g) * cloud_fx; b += (wr + 8.0f - b) * cloud_fx;
    r += (244.0f - r) * wx_flash; g += (246.0f - g) * wx_flash; b += (255.0f - b) * wx_flash;
    fog_r = r; fog_g = g; fog_b = b;
    if (tod != TOD_DAY) {                                         // haze takes on the evening / night colours
        unsigned int fc = tod_apply(RGBA8((int)fog_r, (int)fog_g, (int)fog_b, 255));
        fog_r = (float)(fc & 255u); fog_g = (float)((fc >> 8) & 255u); fog_b = (float)((fc >> 16) & 255u);
    }

    float sr = 118.0f, sg = 176.0f, sb = 236.0f;
    if (tod == TOD_DUSK)       { sr = 52.0f; sg = 70.0f; sb = 130.0f; }
    else if (tod == TOD_NIGHT) { sr = 5.0f;  sg = 9.0f;  sb = 28.0f; }
    float sm = fmaxf(clampf(wx.cloud * 0.85f + wx.fog, 0.0f, 1.0f), cloud_fx);
    sr += (r - sr) * sm; sg += (g - sg) * sm; sb += (b - sb) * sm;
    sky_r = sr; sky_g = sg; sky_b = sb;
    vita2d_set_clear_color(RGBA8((int)sr, (int)sg, (int)sb, 255));
}

// Blend a colour toward the fog colour for something `dist` metres away.
static unsigned int wx_fog_apply(unsigned int col, float dist) {
    float k = dist / fog_vis;
    if (k * k < 0.0039f && wx_flash <= 0.0f) return col;      // 1 - exp(-x) is never more than x, so this is the "< 0.004: no fog" case without the exp
    float m = 1.0f - expf(-k * k);
    if (wx_flash > 0.0f) m = fmaxf(m, 0.65f * wx_flash);
    if (m < 0.004f) return col;
    float r = (float)(col & 255u), g = (float)((col >> 8) & 255u), b = (float)((col >> 16) & 255u);
    r += (fog_r - r) * m; g += (fog_g - g) * m; b += (fog_b - b) * m;
    return RGBA8((int)r, (int)g, (int)b, (int)(col >> 24));
}

static int wx_tile_radius(void) {
    if (fog_vis >= 9000.0f) return TILE_R;
    int r = (int)(fog_vis * 2.4f / TILE) + 2;
    return r < 3 ? 3 : (r > TILE_R ? TILE_R : r);
}

