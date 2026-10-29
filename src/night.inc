// Left Seat: dusk and night - sky gradient, sun, moon and stars, airport lights and the landing light.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* Dusk and night                                                       */
/* ------------------------------------------------------------------ */

// Where something in the sky (a unit direction in the world) appears on the screen.
static bool sky_project(V3 dir, float *sx, float *sy) {
    float x = vdot(dir, cam.r), y = vdot(dir, cam.u), z = vdot(dir, cam.f);
    if (z < 0.05f) return false;
    *sx = CX + FOCAL * x / z;
    *sy = CY - FOCAL * y / z;
    return *sx > -90.0f && *sx < SCREEN_W + 90.0f && *sy > -90.0f && *sy < VIEW_H + 90.0f;
}
static void sky_disc(float x, float y, float r, unsigned int col) {
    P2 p[12];
    for (int i = 0; i < 12; i++) { float a = i * (2.0f * PI_F / 12.0f); p[i].x = x + cosf(a) * r; p[i].y = y + sinf(a) * r; }
    fill_poly2d(p, 12, col);
}
static V3 sky_dir(float az_deg, float el_deg) {                   // azimuth clockwise from north
    float a = az_deg * PI_F / 180.0f, e = el_deg * PI_F / 180.0f;
    return v3(sinf(a) * cosf(e), sinf(e), cosf(a) * cosf(e));
}

// Sky gradient (glowing horizon at dusk), the low sun, or the moon and stars. Drawn before the ground,
// so the land hides whatever is below the horizon.
static void draw_sky_extras(void) {
    V3 up = v3(0.0f, 1.0f, 0.0f);
    float ux = vdot(up, cam.r), uy = vdot(up, cam.u), uz = vdot(up, cam.f);
    float len = sqrtf(ux * ux + uy * uy);
    if (len > 1e-4f) {
        float nx = ux / len, ny = -uy / len, d0 = -uz * FOCAL / len, tx = -ny, ty = nx;
        float hx = CX + nx * d0, hy = CY + ny * d0;
        const float B = 6000.0f;
        float glow = (tod == TOD_DUSK) ? 0.60f * (1.0f - clampf(wx.cloud * 0.8f + wx.fog, 0.0f, 1.0f)) : 0.0f;
        float hr = fog_r + (255.0f - fog_r) * glow, hg = fog_g + (142.0f - fog_g) * glow, hb = fog_b + (74.0f - fog_b) * glow;
        const int NB = 9;
        const float STEP = 42.0f;
        for (int k = 0; k < NB; k++) {
            float t = powf((k + 0.5f) / NB, 0.75f);
            unsigned int col = RGBA8((int)(hr + (sky_r - hr) * t), (int)(hg + (sky_g - hg) * t), (int)(hb + (sky_b - hb) * t), 255);
            float o0 = k * STEP - 3.0f, o1 = (k + 1) * STEP;
            P2 q[4] = { { hx + nx * o0 - tx * B, hy + ny * o0 - ty * B }, { hx + nx * o0 + tx * B, hy + ny * o0 + ty * B },
                        { hx + nx * o1 + tx * B, hy + ny * o1 + ty * B }, { hx + nx * o1 - tx * B, hy + ny * o1 - ty * B } };
            fill_poly2d(q, 4, col);
        }
    }
    if (fog_vis < 7000.0f) return;                                  // haze or cloud hides the sky objects
    float sx, sy;
    if (tod == TOD_DAY) {
        float clear = 1.0f - clampf(wx.cloud * 1.3f + wx.fog, 0.0f, 1.0f);
        if (clear > 0.1f && sky_project(sky_dir(130.0f, 42.0f), &sx, &sy)) {
            sky_disc(sx, sy, 56.0f, RGBA8(255, 244, 205, (int)(46.0f * clear)));
            sky_disc(sx, sy, 28.0f, RGBA8(255, 248, 215, (int)(90.0f * clear)));
            sky_disc(sx, sy, 15.0f, RGBA8(255, 252, 235, 255));
        }
    } else if (tod == TOD_DUSK) {
        if (sky_project(sky_dir(252.0f, 4.0f), &sx, &sy)) {
            sky_disc(sx, sy, 78.0f, RGBA8(255, 150, 80, 60));
            sky_disc(sx, sy, 42.0f, RGBA8(255, 190, 110, 90));
            sky_disc(sx, sy, 20.0f, RGBA8(255, 236, 190, 255));
        }
    } else {
        float clear = 1.0f - clampf(wx.cloud * 1.2f + wx.fog, 0.0f, 1.0f);
        if (sky_project(sky_dir(112.0f, 38.0f), &sx, &sy)) {
            sky_disc(sx, sy, 30.0f, RGBA8(190, 200, 235, (int)(40.0f * clear + 10.0f)));
            sky_disc(sx, sy, 13.0f, RGBA8(238, 238, 222, 255));
        }
        int shown = (int)(70.0f * clear);                            // stars: fewer under cloud
        for (int i = 0; i < shown; i++) {
            float fa = fmodf(i * 0.61803f, 1.0f), fe = fmodf(i * 0.75487f + 0.31f, 1.0f), fb = fmodf(i * 0.31831f, 1.0f);
            if (sky_project(sky_dir(fa * 360.0f, 5.0f + fe * 80.0f), &sx, &sy)) {
                int br = 110 + (int)(fb * 130.0f);
                float s = fb > 0.8f ? 1.6f : 1.0f;
                P2 q[4] = { { sx - s, sy - s }, { sx + s, sy - s }, { sx + s, sy + s }, { sx - s, sy + s } };
                fill_poly2d(q, 4, RGBA8(br, br, br + 15 > 255 ? 255 : br + 15, 255));
            }
        }
    }
}

// A light: a small square that always faces the camera and never gets smaller than about `px` pixels,
// so runway lights stay visible from far away.
static float light_far = 9000.0f;                          // lights farther than this are skipped (the beacon raises it)
static void draw_light(float x, float y, float z, float px, unsigned int col) {
    V3 c = v3(x, y, z);
    V3 d = vsub(c, cam.pos);
    float cz = vdot(d, cam.f);
    if (cz < 1.0f || cz > light_far) return;
    float s = fmaxf(0.18f, cz * px / FOCAL);
    V3 r = vscale(cam.r, s), u = vscale(cam.u, s);
    V3 q[4] = { vsub(vsub(c, r), u), vadd(vsub(c, u), r), vadd(vadd(c, r), u), vadd(vsub(c, r), u) };
    draw_world_poly_emit(q, 4, col);
}

// The airport after dark: runway edge and threshold lights, approach lights, taxiway lights,
// the tower beacon and a few lit windows.
static void draw_night_lights(void) {
    if (tod == TOD_DAY) return;
    if (cam.pos.x * cam.pos.x + cam.pos.z * cam.pos.z > 8000.0f * 8000.0f) return;
    unsigned int wht = RGBA8(255, 246, 214, 255), red = RGBA8(255, 62, 44, 255), grn = RGBA8(80, 245, 120, 255);
    unsigned int blu = RGBA8(80, 140, 255, 255), win = RGBA8(255, 224, 140, 255);
    for (float z = -1000.0f; z <= 1000.1f; z += 60.0f) {                       // runway edge
        draw_light(-24.5f, 0.5f, z, 1.5f, wht);
        draw_light( 24.5f, 0.5f, z, 1.5f, wht);
    }
    for (int i = 0; i < 6; i++) {                                              // thresholds: green to land, red at the far end
        float x = -18.0f + i * 7.2f;
        draw_light(x, 0.5f, -1004.0f, 1.8f, grn);
        draw_light(x, 0.5f,  1004.0f, 1.8f, red);
    }
    for (int e = 0; e < 2; e++) {                                              // approach lights with two cross bars
        float sg = e ? 1.0f : -1.0f;
        for (int k = 1; k <= 20; k++) {
            float z = sg * (1000.0f + k * 30.0f);
            draw_light(0.0f, 0.6f, z, 1.7f, wht);
            if (k == 5 || k == 10)
                for (int j = 1; j <= 3; j++) { draw_light(3.0f * j, 0.6f, z, 1.7f, wht); draw_light(-3.0f * j, 0.6f, z, 1.7f, wht); }
        }
    }
    for (float z = -960.0f; z <= 960.0f; z += 80.0f) draw_light(-71.5f, 0.4f, z, 1.4f, blu);   // taxiway edge
    bool beacon = ((frame_count / 24) & 1) != 0;                              // tower beacon alternates white / green
    draw_light(95.0f, 52.5f, 0.0f, 3.0f, beacon ? wht : grn);
    for (int i = 0; i < 4; i++) {                                              // tower cab windows
        float o = -6.0f + i * 4.0f;
        draw_light(95.0f + o, 47.5f, -8.3f, 1.8f, win);
        draw_light(95.0f + o, 47.5f,  8.3f, 1.8f, win);
        draw_light(86.7f, 47.5f, o, 1.8f, win);
        draw_light(103.3f, 47.5f, o, 1.8f, win);
    }
    for (int i = 0; i < 7; i++) {                                              // terminal windows
        draw_light(120.0f + i * 9.0f, 5.0f, -343.3f, 1.8f, win);
        draw_light(114.7f, 5.0f, -340.0f + i * 3.8f, 1.8f, win);
    }
}

// The aircraft's own lights, seen from outside (chase and tower views): a red light on the left wingtip, green on the right, white
// at the tail, a red beacon that flashes about once a second, and white wingtip strobes. Each is a small bright dot with a soft glow
// that stays visible from far away, so you can pick out an aircraft in the dark. Like the real ones, the red and green lights can only
// be seen from their own side and the white tail light only from behind and the sides.
typedef struct { float wx, wy, wz; float tx[2], ty[2], tz[2]; int ntail; float bx, by, bz; } AcLights;
static const AcLights AC_LIGHTS[AC_COUNT] = {
    /* K-4      */ { 5.50f, 2.23f,  0.74f, { 0.00f, 0.00f }, { 2.72f, 0.00f }, { -4.78f, 0.00f }, 1, 0.00f, 2.80f, -4.35f },
    /* K-10     */ { 7.94f, 2.91f,  0.70f, { 0.00f, 0.00f }, { 4.15f, 0.00f }, { -7.86f, 0.00f }, 1, 0.00f, 4.62f, -6.85f },
    /* Jet 1    */ { 2.74f, 0.905f, 0.33f, { 0.00f, 0.00f }, { 1.48f, 0.00f }, { -3.12f, 0.00f }, 1, 0.00f, 1.84f, -0.50f },
    /* K-30     */ { 7.20f, 1.72f, -1.66f, { 0.00f, 0.00f }, { 3.00f, 0.00f }, { -7.40f, 0.00f }, 1, 0.00f, 6.12f, -5.80f },
    /* Ashley A22 */ { 6.60f, 2.22f, 2.53f, { -4.05f, 4.05f }, { 2.29f, 2.29f }, { -7.05f, -7.05f }, 2, 0.00f, 4.08f, -5.43f },
    /* Ashley A11 */ { 4.90f, 1.17f, -0.41f, { 0.00f, 0.00f }, { 3.05f, 0.00f }, { -5.70f, 0.00f }, 1, 0.00f, 3.34f, -1.62f },
};

// A round glow: an octagon that always faces the camera, at least `px` pixels across from its middle to its edge.
static void plane_glow(V3 c, float px, unsigned int col) {
    V3 d = vsub(c, cam.pos);
    float cz = vdot(d, cam.f);
    if (cz < 1.0f || cz > 9000.0f) return;
    float s = fmaxf(0.10f, cz * px / FOCAL);
    V3 q[8];
    for (int i = 0; i < 8; i++) {
        float a = (i + 0.5f) * (PI_F / 4.0f);
        q[i] = vadd(c, vadd(vscale(cam.r, cosf(a) * s), vscale(cam.u, sinf(a) * s)));
    }
    draw_world_poly_emit(q, 8, col);
}

static void draw_plane_lights(void) {
    if (pl.crashed || !sw.batt) return;
    const AcLights *L = &AC_LIGHTS[ac_flying];
    const bool night = (tod != TOD_DAY);
    const unsigned int redc = RGBA8(255, 50, 40, 255), grnc = RGBA8(60, 255, 110, 255), whtc = RGBA8(255, 252, 235, 255);
    const bool beacon_on = ((frame_count / 6) % 10) == 0;                             // same timing as the beacon patch on the model
    const bool strobe_l = (((frame_count) / 3) % 17) == 0, strobe_r = (((frame_count + 7) / 3) % 17) == 0;
    const float core = night ? 2.6f : 1.8f, halo = night ? 11.0f : 5.0f;
    const int ha = night ? 95 : 55;
    struct { V3 p; unsigned int col; int vis; bool bright; } lt[8];
    int n = 0;
    V3 wl = body_pt(-L->wx, L->wy, L->wz), wr = body_pt(L->wx, L->wy, L->wz);
    V3 dl = vsub(cam.pos, wl), dr = vsub(cam.pos, wr);
    float ll = sqrtf(vdot(dl, dl)) + 0.01f, lr = sqrtf(vdot(dr, dr)) + 0.01f;
    lt[n].p = wl; lt[n].col = redc; lt[n].vis = lt_nav() && vdot(dl, pl.r) < 0.30f * ll; lt[n].bright = false; n++;
    lt[n].p = wr; lt[n].col = grnc; lt[n].vis = lt_nav() && vdot(dr, pl.r) > -0.30f * lr; lt[n].bright = false; n++;
    lt[n].p = wl; lt[n].col = whtc; lt[n].vis = strobe_l && lt_strobe(); lt[n].bright = true; n++;       // strobes ride on the wingtips
    lt[n].p = wr; lt[n].col = whtc; lt[n].vis = strobe_r && lt_strobe(); lt[n].bright = true; n++;
    for (int i = 0; i < L->ntail; i++) {
        V3 p = body_pt(L->tx[i], L->ty[i], L->tz[i]), d = vsub(cam.pos, p);
        lt[n].p = p; lt[n].col = whtc; lt[n].vis = lt_nav() && vdot(d, pl.f) < 0.25f * (sqrtf(vdot(d, d)) + 0.01f); lt[n].bright = false; n++;
    }
    lt[n].p = body_pt(L->bx, L->by, L->bz); lt[n].col = redc; lt[n].vis = beacon_on && lt_beacon(); lt[n].bright = true; n++;
    for (int i = 0; i < n; i++) {
        if (!lt[i].vis) continue;
        float k = lt[i].bright ? 1.5f : 1.0f;
        unsigned int c = lt[i].col, rgb = c & 0x00FFFFFFu;
        plane_glow(lt[i].p, halo * k, rgb | ((unsigned)(ha / 2) << 24));           // a wide faint glow,
        plane_glow(lt[i].p, halo * 0.55f * k, rgb | ((unsigned)ha << 24));         // a tighter one,
        plane_glow(lt[i].p, core * k, c);                                          // and the bright centre
    }
}

static void draw_landing_light(void) {
    if (tod == TOD_DAY || pl.crashed || !lt_landing()) return;
    float eye = pl.pos.y + 1.6f;
    if (eye > 450.0f) return;
    V3 b = vnorm(vadd(pl.f, vscale(pl.u, -0.06f)));                            // beam points a little below the nose
    if (b.y > -0.02f) return;
    float t = clampf(-eye / b.y, 10.0f, 600.0f);
    V3 hd = vnorm(v3(pl.f.x, 0.0f, pl.f.z));
    V3 sd = v3(hd.z, 0.0f, -hd.x);
    V3 ctr = v3(pl.pos.x + b.x * t, 0.04f, pl.pos.z + b.z * t);
    float L = 0.70f * t + 10.0f, W = 0.38f * t + 8.0f;                         // the pool gets longer and wider farther out
    float fade = 1.0f / (1.0f + eye / 140.0f);
    int total = (int)(78.0f * fade * (tod == TOD_DUSK ? 0.55f : 1.0f));
    const int LAYERS = 7;
    int per = total / LAYERS;
    if (per < 1) return;
    // A soft pool of light: stacked, shrinking ellipses. Where they overlap the light adds up, so it is brightest
    // in the middle and fades out to nothing at the edge. Every other layer is turned a little to hide the corners.
    for (int k = 0; k < LAYERS; k++) {
        float s = 1.0f - 0.84f * k / (LAYERS - 1);
        float rot = (k & 1) ? 0.3927f : 0.0f;
        V3 q[8];
        for (int i = 0; i < 8; i++) {
            float an = rot + i * (PI_F / 4.0f);
            q[i] = vadd(ctr, vadd(vscale(hd, cosf(an) * L * 0.5f * s), vscale(sd, sinf(an) * W * 0.5f * s)));
        }
        draw_world_poly_emit(q, 8, RGBA8(255, 244, 205, per));
    }
}

