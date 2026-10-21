// Left Seat: clouds, windsock and rain.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* Weather scenery: cloud puffs, a windsock and rain streaks            */
/* ------------------------------------------------------------------ */

// Cloud puffs on a drifting grid around the camera. Each is a low dome with a flat grey underside.
static void build_clouds(void) {
    float inside = 0.0f;
    if (wx.cloud >= 0.12f) {
        const float CELL = 2400.0f;
        int ci0 = (int)floorf((cam.pos.x - wx_off_x) / CELL), cj0 = (int)floorf((cam.pos.z - wx_off_z) / CELL);
        float dark = clampf(wx.rain * 0.9f, 0.0f, 1.0f);
        float cr = 240.0f - 130.0f * dark, cg = 242.0f - 130.0f * dark, cb = 247.0f - 118.0f * dark;
        for (int dj = -2; dj <= 2; dj++)
            for (int di = -2; di <= 2; di++) {
                int ci = ci0 + di, cj = cj0 + dj;
                unsigned int h = hash2(ci, cj), h2 = hash2(cj + 917, ci - 311);
                if (hf(h, 0) > wx.cloud) continue;
                float R    = CELL * (0.28f + 0.30f * wx.cloud) * (0.75f + 0.5f * hf(h, 1));
                float base = (wx.rain > 0.5f ? 900.0f : 1500.0f) + 700.0f * hf(h, 2);
                float th   = (200.0f + 300.0f * wx.cloud + 500.0f * wx.rain) * (0.7f + 0.6f * hf(h, 3));
                float cx   = (ci + 0.2f + 0.6f * hf(h2, 0)) * CELL + wx_off_x;
                float cz   = (cj + 0.2f + 0.6f * hf(h2, 1)) * CELL + wx_off_z;

                float s = (pl.pos.y - base) / th;                       // is the aircraft inside this puff?
                if (s > 0.0f && s < 1.0f) {
                    float dx = pl.pos.x - cx, dz = pl.pos.z - cz, rr = R * sqrtf(1.0f - s * s);
                    if (dx * dx + dz * dz < rr * rr) inside = 1.0f;
                }
                V3 ring0[5], ring1[5];
                V3 apex = v3(cx, base + th, cz), bc = v3(cx, base, cz), mid = v3(cx, base + 0.4f * th, cz);
                for (int j = 0; j < 5; j++) {
                    float a = (j + 0.5f) * 2.0f * PI_F / 5.0f;
                    ring0[j] = v3(cx + R * sinf(a), base, cz + R * cosf(a));
                    ring1[j] = v3(cx + 0.82f * R * sinf(a), base + 0.6f * th, cz + 0.82f * R * cosf(a));
                }
                for (int j = 0; j < 5; j++) {
                    int k = (j + 1) % 5;
                    V3 t0[3] = { bc, ring0[j], ring0[k] };
                    V3 q[4]  = { ring0[j], ring0[k], ring1[k], ring1[j] };
                    V3 t1[3] = { ring1[j], ring1[k], apex };
                    pf_poly(3, t0, &mid, cr * 0.74f, cg * 0.74f, cb * 0.76f, PF_EMIT, 255);   // soft grey underside
                    pf_poly(4, q,  &mid, cr, cg, cb, 0, 255);
                    pf_poly(3, t1, &mid, cr, cg, cb, 0, 255);
                }
            }
    }
    cloud_fx += (inside - cloud_fx) * 0.08f;
}

// A windsock by the runway: limp when calm, stretched out downwind in a breeze.
static void build_windsock_at(float px, float pz) {
    const float ph = 7.0f;
    V3 b[4] = { v3(px - 0.12f, 0, pz - 0.12f), v3(px + 0.12f, 0, pz - 0.12f), v3(px + 0.12f, 0, pz + 0.12f), v3(px - 0.12f, 0, pz + 0.12f) };
    V3 t[4] = { v3(px - 0.12f, ph, pz - 0.12f), v3(px + 0.12f, ph, pz - 0.12f), v3(px + 0.12f, ph, pz + 0.12f), v3(px - 0.12f, ph, pz + 0.12f) };
    V3 pc = v3(px, ph * 0.5f, pz);
    for (int i = 0; i < 4; i++) {
        int j = (i + 1) & 3;
        V3 q[4] = { b[i], b[j], t[j], t[i] };
        pf_poly(4, q, &pc, 230, 230, 236, 0, 255);
    }
    float lift = clampf(wx.wind * 0.45f / 7.0f, 0.0f, 1.0f);
    float dx = -sinf(wx_dir), dz = -cosf(wx_dir);
    V3 side = v3(dz, 0.0f, -dx);
    V3 c[4], ring[4][4];
    V3 pos = v3(px, ph + 0.05f, pz);
    for (int k = 0; k < 4; k++) {
        float droop = (1.0f - lift) * 1.25f + 0.06f * sinf(wx_time * 6.3f + k * 0.9f) * (0.3f + lift);
        V3 dir = v3(cosf(droop) * dx, -sinf(droop), cosf(droop) * dz);
        V3 up2 = vnorm(vcross(side, dir));
        float r = 0.50f - 0.09f * k;
        c[k] = pos;
        for (int j = 0; j < 4; j++) {
            float a = j * PI_F / 2.0f + PI_F / 4.0f;
            ring[k][j] = vadd(pos, vadd(vscale(side, r * cosf(a)), vscale(up2, r * sinf(a))));
        }
        pos = vadd(pos, dir);
    }
    for (int k = 0; k < 3; k++) {
        V3 cc = vscale(vadd(c[k], c[k + 1]), 0.5f);
        for (int j = 0; j < 4; j++) {
            int n = (j + 1) & 3;
            V3 q[4] = { ring[k][j], ring[k][n], ring[k + 1][n], ring[k + 1][j] };
            if (k & 1) pf_poly(4, q, &cc, 246, 246, 246, 0, 255);
            else       pf_poly(4, q, &cc, 224, 70, 44, 0, 255);
        }
    }
}

static void build_windsock(void) { build_windsock_at(62.0f, -430.0f); }              // the one beside the home runway

// Rain: streaks that fly out from the centre of the view.
static float wrapf(float x, float b) { return x - b * floorf(x / b + 0.5f); }

// Rain falls through the world (slanted by the wind) and we fly through it. Each drop is a short streak whose
// direction is its motion relative to the aircraft: straight down when parked, slanting back as you fly, and
// sliding past sideways when you look out of a side window.
static void draw_rain(void) {
    if (wx.rain < 0.04f) return;
    const float BX = 70.0f, BY = 44.0f, BZ = 70.0f;                    // a box of rain that follows the camera
    int n = (int)(160.0f + 360.0f * wx.rain);
    V3 w = wx_wind_at(cam.pos);
    V3 vr = v3(w.x * 0.8f, -9.0f, w.z * 0.8f);                         // how a drop moves through the world
    V3 rel = vsub(vr, pl.vel);                                         // ... and as seen from the aircraft
    float t = wx_time;
    unsigned int col = tod_apply(RGBA8(196, 210, 230, 200));
    for (int i = 0; i < n; i++) {
        unsigned int h = hash2(i, 4242);
        V3 d = v3(wrapf(hf(h, 0) * BX + vr.x * t - cam.pos.x, BX),
                  wrapf(hf(h, 1) * BY + vr.y * t - cam.pos.y, BY),
                  wrapf(hf(h, 2) * BZ + vr.z * t - cam.pos.z, BZ));
        float cz = vdot(d, cam.f);
        if (cz < 3.0f) continue;                                       // not right in front of the lens
        V3 e = vsub(d, vscale(rel, 0.06f));                            // where this drop was a moment ago
        float ez = vdot(e, cam.f);
        if (ez < 1.0f) continue;
        float x1 = CX + FOCAL * vdot(d, cam.r) / cz, y1 = CY - FOCAL * vdot(d, cam.u) / cz;
        float x2 = CX + FOCAL * vdot(e, cam.r) / ez, y2 = CY - FOCAL * vdot(e, cam.u) / ez;
        float dx = x2 - x1, dy = y2 - y1, len = sqrtf(dx * dx + dy * dy);
        if (len < 0.6f) continue;
        if (len > 110.0f) { x2 = x1 + dx * 110.0f / len; y2 = y1 + dy * 110.0f / len; len = 110.0f; }
        float px = -dy / len * 0.8f, py = dx / len * 0.8f;
        P2 q[4] = { { x1 + px, y1 + py }, { x2 + px, y2 + py }, { x2 - px, y2 - py }, { x1 - px, y1 - py } };
        fill_poly2d(q, 4, col);
    }
}

