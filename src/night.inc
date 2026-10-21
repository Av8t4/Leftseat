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

// The aircraft's landing light: a soft pool of light on the ground where the beam lands.
static void draw_landing_light(void) {
    if (tod == TOD_DAY || pl.crashed) return;
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

