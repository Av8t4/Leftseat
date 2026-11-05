// Left Seat: the 3D camera and polygon projection.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* 3D camera + polygon projection                                      */
/* ------------------------------------------------------------------ */

static void look_at(V3 eye, V3 target) {
    V3 f = vnorm(vsub(target, eye));
    V3 r = vcross(v3(0.0f, 1.0f, 0.0f), f);
    float rl = sqrtf(vdot(r, r));
    r = rl > 1e-4f ? vscale(r, 1.0f / rl) : v3(1.0f, 0.0f, 0.0f);
    cam.f = f; cam.r = r; cam.u = vcross(f, r); cam.pos = eye;
}

static void setup_camera(void) {
    g_focal = 520.0f;
    g_near = 2.0f;
    V3 pivot = v3(pl.pos.x, pl.pos.y + spec->piv_y, pl.pos.z);

    if (view_mode == 1) {                                   // chase: behind and above, level horizon
        V3 t = vnorm(v3(pl.f.x, pl.f.y * 0.5f, pl.f.z));
        chase_dir = vnorm(vadd(chase_dir, vscale(vsub(t, chase_dir), 0.08f)));
        float lc = cosf(-look_ang), ls = sinf(-look_ang);              // L / R swing the camera round the aircraft
        V3 cd = v3(chase_dir.x * lc + chase_dir.z * ls, chase_dir.y, -chase_dir.x * ls + chase_dir.z * lc);
        V3 eye = vadd(vsub(pivot, vscale(cd, spec->chase_dist)), v3(0.0f, spec->chase_up, 0.0f));
        if (eye.y < 1.8f) eye.y = 1.8f;
        look_at(eye, vadd(vadd(pivot, vscale(cd, 10.0f)), v3(0.0f, 1.0f, 0.0f)));
        return;
    }
    if (view_mode == 2) {                                   // control tower, zooming in on the aircraft
        V3 eye = v3(86.0f, 52.5f, 0.0f);          // on the tower cab roof edge, facing the runway
        V3 d = vsub(pivot, eye);
        g_focal = clampf(520.0f * sqrtf(vdot(d, d)) / 55.0f, 520.0f, 4000.0f);
        look_at(eye, pivot);
        return;
    }

    cam.f = pl.f;
    cam.r = pl.r;
    cam.u = pl.u;
    // Stall buffet: a small random shake as the wing nears the stall.
    float buf = clampf((pl.alpha - (ALPHA_S - 0.05f)) / 0.07f, 0.0f, 1.0f);
    if (!pl.on_ground && !pl.crashed && buf > 0.0f) {
        float e1 = frand(-1.0f, 1.0f) * buf * 0.012f;
        float e2 = frand(-1.0f, 1.0f) * buf * 0.012f;
        cam.f = vnorm(vadd(vadd(pl.f, vscale(pl.u, e1)), vscale(pl.r, e2)));
        cam.r = vnorm(vsub(pl.r, vscale(cam.f, vdot(pl.r, cam.f))));
        cam.u = vcross(cam.f, cam.r);
    }
    if (fabsf(look_ang) > 0.001f) {                         // turn your head: L = left, R = right, both = behind
        float c = cosf(look_ang), sn = sinf(look_ang);
        V3 f0 = cam.f, r0 = cam.r;
        cam.f = vadd(vscale(f0, c), vscale(r0, sn));
        cam.r = vsub(vscale(r0, c), vscale(f0, sn));
    }
    g_near = 0.3f;
    {                                                       // pilot's eyes: left seat, 1.62 m above the ground, fixed to the aircraft
        V3 eye = v3(pl.pos.x, pl.pos.y + spec->piv_y, pl.pos.z);
        eye = vadd(eye, vscale(pl.r, spec->eye_x));
        eye = vadd(eye, vscale(pl.u, spec->eye_y - spec->piv_y));
        eye = vadd(eye, vscale(pl.f, spec->eye_z));
        cam.pos = eye;
    }
}

// Transform a world polygon to the screen, near-clip it, and fill it.
static float poly_fog_cap = 1e30f;                 // polygons count as no farther than this for the fog (the far sea uses it, so it ends in a steady haze)
static void draw_poly_ex(const V3 *p, int n, unsigned int color, bool emit) {
    V3 c[8];
    for (int i = 0; i < n; i++) {
        V3 d = vsub(p[i], cam.pos);
        c[i] = v3(vdot(d, cam.r), vdot(d, cam.u), vdot(d, cam.f));
    }

    V3 q[12];
    int m = 0;
    for (int i = 0; i < n; i++) {
        V3 a = c[i], b = c[(i + 1) % n];
        bool ain = a.z >= NEAR_Z, bin = b.z >= NEAR_Z;
        if (ain) q[m++] = a;
        if (ain != bin) {
            float t = (NEAR_Z - a.z) / (b.z - a.z);
            q[m++] = v3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, NEAR_Z);
        }
    }
    if (m < 3) return;

    P2 s[12];
    for (int i = 0; i < m; i++) {
        s[i].x = CX + FOCAL * q[i].x / q[i].z;
        s[i].y = CY - FOCAL * q[i].y / q[i].z;
    }
    float cd = 0.0f;                                   // fog: blend toward the haze color with distance
    for (int i = 0; i < m; i++) cd += sqrtf(q[i].x * q[i].x + q[i].y * q[i].y + q[i].z * q[i].z);
    if (!emit) color = tod_apply(color);
    float fd = cd / m;
    if (fd > poly_fog_cap) fd = poly_fog_cap;
    color = wx_fog_apply(color, fd);
    fill_poly2d(s, m, color);
}
static void draw_world_poly(const V3 *p, int n, unsigned int color)      { draw_poly_ex(p, n, color, false); }
static void draw_world_poly_emit(const V3 *p, int n, unsigned int color) { draw_poly_ex(p, n, color, true); }   // lights

static void draw_ground_quad(float x0, float z0, float x1, float z1, unsigned int color) {
    V3 p[4] = { v3(x0, 0, z0), v3(x1, 0, z0), v3(x1, 0, z1), v3(x0, 0, z1) };
    draw_world_poly(p, 4, color);
}

