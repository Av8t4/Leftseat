// Left Seat: drawing one frame of the 3D scene.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

typedef struct { float dist; int idx; } DrawItem;
#define TREE_IDX0 (MAX_FACES + MAX_PFACES)      // items from here on are trees (index = TREE_IDX0 + position in trees[])
#define LIGHT_IDX0 (TREE_IDX0 + 256)           // and from here on, lights (index = LIGHT_IDX0 + position in flights[])
#define MAX_FLIGHTS 520
typedef struct { float x, y, z, px, far; unsigned int col; } FLight;
static FLight flights[MAX_FLIGHTS];
static int nflights;
static DrawItem items[MAX_FACES + MAX_PFACES + 256 + MAX_FLIGHTS];
static int cmp_item(const void *a, const void *b) {
    float da = ((const DrawItem *)a)->dist, db = ((const DrawItem *)b)->dist;
    return (da < db) - (da > db);          // far to near
}
// Far-to-near order for the painter's algorithm. A radix sort on the float bits (distances are never negative, so their bit
// patterns sort the same way the numbers do): linear time and stable, where qsort was about a third of the CPU time of a busy frame.
static DrawItem items_tmp[MAX_FACES + MAX_PFACES + 256 + MAX_FLIGHTS];
static void sort_items(int n) {
    DrawItem *src = items, *dst = items_tmp;
    for (int pass = 0; pass < 4; pass++) {
        int sh = pass * 8, cnt[256];
        memset(cnt, 0, sizeof cnt);
        for (int i = 0; i < n; i++) { unsigned u; memcpy(&u, &src[i].dist, 4); cnt[255 - ((u >> sh) & 255)]++; }   // 255 - byte: biggest first
        int one = 0;
        for (int b = 0; b < 256; b++) if (cnt[b] == n) { one = 1; break; }
        if (one) continue;                                        // every item has the same byte here: nothing to do in this pass
        int sum = 0;
        for (int b = 0; b < 256; b++) { int c = cnt[b]; cnt[b] = sum; sum += c; }
        for (int i = 0; i < n; i++) { unsigned u; memcpy(&u, &src[i].dist, 4); dst[cnt[255 - ((u >> sh) & 255)]++] = src[i]; }
        DrawItem *t = src; src = dst; dst = t;
    }
    if (src != items) memcpy(items, src, (size_t)n * sizeof(DrawItem));
}

// After dark, the lights on and around buildings join the painter's queue, so a roof covers a light behind it and a light on a wall
// shows in front of that wall. A light sorts a little nearer than its own distance, so it is drawn just after the building it sits on.
static void add_flight(float x, float y, float z, float px, float far, unsigned int col, int *ni) {
    float dx = x - cam.pos.x, dy = y - cam.pos.y, dz = z - cam.pos.z, d2 = dx * dx + dy * dy + dz * dz;
    if (d2 > far * far || nflights >= MAX_FLIGHTS) return;
    if (dx * cam.f.x + dy * cam.f.y + dz * cam.f.z < 1.0f) return;                    // behind us
    float d = sqrtf(d2) - 45.0f;
    d = d < 1.0f ? 1.0f : d;
    flights[nflights] = (FLight){ x, y, z, px, far, col };
    items[*ni].dist = d * d; items[*ni].idx = LIGHT_IDX0 + nflights; (*ni)++;
    nflights++;
}
static void collect_flights(int *ni) {
    nflights = 0;
    if (tod == TOD_DAY) return;
    unsigned int win = RGBA8(255, 214, 130, 255), sodium = RGBA8(255, 196, 120, 255);
    for (int i = 0; i < ntown_lights; i++) add_flight(town_lights[i].x, town_lights[i].y, town_lights[i].z, 1.5f, 3600.0f, win, ni);
    for (int i = 0; i < nroof_lights; i++) {
        const RoofLight *r = &roof_lights[i];
        if (r->blink) { if ((frame_count % 70) < 40) add_flight(r->x, r->y, r->z, r->px, 16000.0f, r->col, ni); }
        else add_flight(r->x, r->y, r->z, r->px, 6500.0f, r->col, ni);
    }
    for (int i = 0; i < nbridges; i++) {
        float L = br_len[i] - 36.0f;
        for (int e = -1; e <= 1; e += 2)
            for (int sd = -1; sd <= 1; sd += 2)
                add_flight(br_x[i] + br_dx[i] * e * L * 0.5f - br_dz[i] * sd * br_hw[i], 6.5f, br_z[i] + br_dz[i] * e * L * 0.5f + br_dx[i] * sd * br_hw[i], 1.6f, 3500.0f, sodium, ni);
    }
    if ((frame_count % 150) < 40) add_flight(sea_coast_x(LH_Z) - 240.0f, 31.5f, LH_Z, 3.4f, 40000.0f, RGBA8(255, 250, 220, 255), ni);   // the lighthouse
}

static void draw_scene(void) {
    wx_frame_setup();
    setup_camera();
    draw_sky_extras();

    // 1. Ground fill. The horizon is where world-up is perpendicular to the view
    //    ray; work it out from the camera axes so any attitude (even inverted) works.
    {
        V3 up = v3(0.0f, 1.0f, 0.0f);
        float ux = vdot(up, cam.r), uy = vdot(up, cam.u), uz = vdot(up, cam.f);
        float len = sqrtf(ux * ux + uy * uy);
        const float B = 12000.0f;
        unsigned int gcol = wx_fog_apply(tod_apply(land_mid(cam.pos.x, cam.pos.z)), TILE * (float)wx_tile_radius());
        if (len < 1e-4f) {
            if (uz < 0.0f) {                      // looking straight down: all ground
                P2 q[4] = { {0, 0}, {SCREEN_W, 0}, {SCREEN_W, VIEW_H}, {0, VIEW_H} };
                fill_poly2d(q, 4, gcol);
            }
        } else {
            float nx = ux / len, ny = -uy / len;  // screen normal, points to the sky
            float d0 = -uz * FOCAL / len;         // horizon offset along that normal
            float tx = -ny, ty = nx;              // direction along the horizon
            float hx = CX + nx * d0, hy = CY + ny * d0;
            P2 q[4] = {
                { hx - tx * B,            hy - ty * B },
                { hx + tx * B,            hy + ty * B },
                { hx + tx * B - nx * B,   hy + ty * B - ny * B },
                { hx - tx * B - nx * B,   hy - ty * B - ny * B },
            };
            fill_poly2d(q, 4, gcol);
        }
    }

    // 2. Checkerboard of darker tiles gives a sense of speed and altitude.
    {
        int i0 = (int)floorf(cam.pos.x / TILE), j0 = (int)floorf(cam.pos.z / TILE);
        int tr = wx_tile_radius();
        bool alltiles = fog_vis < 9000.0f;               // in haze the base colour fades, so draw every tile
        for (int dj = -tr; dj <= tr; dj++) {
            for (int di = -tr; di <= tr; di++) {
                int i = i0 + di, j = j0 + dj;
                bool dark = ((i + j) & 1) != 0;
                if (!dark && !alltiles) continue;
                V3 cc = v3((i + 0.5f) * TILE - cam.pos.x, -cam.pos.y, (j + 0.5f) * TILE - cam.pos.z);
                if (vdot(cc, cam.f) < -TILE * 1.5f) continue;     // well behind us
                draw_ground_quad(i * TILE, j * TILE, (i + 1) * TILE, (j + 1) * TILE,
                                 field_color(i, j, dark));
            }
        }
        draw_lakes();
        draw_sea();
        draw_main_road_shoulders();
        draw_roads(i0, j0, tr);
        draw_main_road_marks();
        draw_river();
        draw_highway();
        draw_country_roads();
        draw_town_streets();
        draw_bridges();
    }

    // 3. Runway.
    draw_ground_quad(-22, -1000, 22, 1000, RGBA8(72, 72, 78, 255));
    draw_ground_quad(-22, -1000, -20.5f, 1000, RGBA8(235, 235, 235, 255));
    draw_ground_quad( 20.5f, -1000, 22, 1000, RGBA8(235, 235, 235, 255));
    for (float z = -940; z < 940; z += 60)
        draw_ground_quad(-1, z, 1, z + 30, RGBA8(235, 235, 235, 255));
    for (int e = 0; e < 2; e++) {
        float z0 = e ? 960.0f : -990.0f;
        for (float x = -16; x <= 16; x += 6)
            draw_ground_quad(x - 1, z0, x + 1, z0 + 30, RGBA8(235, 235, 235, 255));
    }

    draw_airport_ground();
    draw_papi();
    draw_night_lights();
    draw_redrock_all();
    draw_pinecrest_all();
    draw_lana_all();
    draw_spaceport_all();
    draw_landing_light();

    // 3b. The aircraft's shadow (not needed in the cockpit).
    if (view_mode != 0) {
        draw_plane_shadow();
    }

    collect_trees();

    // 4. Solid objects and the aircraft, sorted far to near (painter's algorithm).
    int ni = 0;
    float cull = fminf(fmaxf(9000.0f, fog_vis * 1.3f), 45000.0f);
    for (int i = 0; i < nfaces; i++) {
        const Face *f = &faces[i];
        V3 d = vsub(f->c, cam.pos);
        float d2 = vdot(d, d), rad = f->rad;
        if (vdot(d, cam.f) < -rad) continue;                               // entirely behind us
        if (d2 > (cull + rad) * (cull + rad)) continue;                    // lost in the haze
        if (f->maxd > 0.0f && d2 > f->maxd * f->maxd) continue;           // city detail: not worth drawing from far away
        if (f->top * f->top * FOCAL * FOCAL < 6.25f * d2) continue;        // too low or too far to show even a few pixels
        items[ni].dist = d2; items[ni].idx = i; ni++;
    }
    for (int k = 0; k < ntrees; k++) {                                         // trees take their place in the same order
        V3 d = v3(trees[k].x - cam.pos.x, trees[k].y0 + trees[k].h * 0.4f - cam.pos.y, trees[k].z - cam.pos.z);
        items[ni].dist = vdot(d, d); items[ni].idx = TREE_IDX0 + k; ni++;
    }
    collect_flights(&ni);
    npf = 0; pf_last = -1;
    if (view_mode != 0) build_plane();
    else {
        if (fabsf(look_ang) > 0.25f) build_cockpit_parts();                              // wings and struts
#if COCKPIT_NOSE
        if (fabsf(look_ang) < 0.8f) build_cockpit_nose();
#endif
    }
    build_clouds();
    build_windsock();
    build_outlying_windsocks();
    build_rings();
    for (int k = 0; k < npf; k++) {
        const PFace *pf = &pfaces[k];
        if (pf->isdecal) continue;
        if (!pf->twosided && vdot(pf->nrm, vsub(cam.pos, pf->c)) <= 0.0f) continue;   // facing away
        V3 d = vsub(pf->c, cam.pos);
        float d2 = vdot(d, d);
        if (pf->twosided && d2 > 1e-6f) {                      // thin two-sided panels (wings, tail): the side facing us is drawn later,
            float dl = sqrtf(d2);                              // so the near skin covers the far skin instead of the far one showing through
            dl -= 0.3f * (-vdot(pf->nrm, d) / dl);
            d2 = dl * dl;
        }
        items[ni].dist = d2; items[ni].idx = MAX_FACES + k; ni++;
    }
    sort_items(ni);
    for (int i = 0; i < ni; i++) {
        int idx = items[i].idx;
        if (idx >= LIGHT_IDX0) {
            const FLight *fl = &flights[idx - LIGHT_IDX0];
            light_far = fl->far;
            draw_light(fl->x, fl->y, fl->z, fl->px, fl->col);
            light_far = 9000.0f;
        } else if (idx >= TREE_IDX0) {
            draw_tree(&trees[idx - TREE_IDX0]);
        } else if (idx < MAX_FACES) {
            const Face *f = &faces[idx];
            draw_world_poly(f->p, f->n, f->color);
        } else {
            const PFace *pf = &pfaces[idx - MAX_FACES];
            if (pf->emit) draw_world_poly_emit(pf->p, pf->n, pf->color); else draw_world_poly(pf->p, pf->n, pf->color);
            for (int d = 1; d <= pf->ndecals; d++) {              // stripes and windows ride on top
                const PFace *dc = &pfaces[idx - MAX_FACES + d];
                if (pf->twosided && !dc->emit && vdot(dc->nrm, vsub(cam.pos, dc->c)) <= 0.0f) continue;   // a stripe on a thin wing must not show through to the far side
                if (dc->emit) draw_world_poly_emit(dc->p, dc->n, dc->color); else draw_world_poly(dc->p, dc->n, dc->color);
            }
        }
    }

    flush_batch();
    draw_rain();
    flush_batch();
#if COCKPIT_FRAME
    if (view_mode == 0) { draw_cockpit_frame(); flush_batch(); }
#endif

    // 5. Nose reference mark.
    unsigned int w = RGBA8(255, 255, 255, 255);
    if (view_mode == 0 && fabsf(look_ang) < 0.35f) {
        vita2d_draw_line(CX - 34, CY, CX - 12, CY, w);
        vita2d_draw_line(CX + 12, CY, CX + 34, CY, w);
        vita2d_draw_line(CX, CY - 12, CX, CY - 4, w);
    }

#if SHOW_FLIGHT_PATH_MARKER
    // Flight-path marker: where the plane is actually going (not where the nose points).
    // Put it on the horizon for level flight; above the horizon means you are climbing.
    float V = sqrtf(vdot(pl.vel, pl.vel));
    if (view_mode == 0 && fabsf(look_ang) < 0.35f && V > 5.0f && !pl.crashed) {
        V3 vh = vscale(pl.vel, 1.0f / V);
        float zc = vdot(vh, cam.f);
        if (zc > 0.05f) {
            float sx = CX + FOCAL * vdot(vh, cam.r) / zc;
            float sy = CY - FOCAL * vdot(vh, cam.u) / zc;
            if (sx > 24 && sx < SCREEN_W - 24 && sy > 24 && sy < VIEW_H - 24) {
                unsigned int gr = RGBA8(120, 255, 120, 255);
                for (int i = 0; i < 16; i++) {
                    float a0 = i * PI_F / 8.0f, a1 = (i + 1) * PI_F / 8.0f;
                    vita2d_draw_line(sx + cosf(a0) * 8, sy + sinf(a0) * 8,
                                     sx + cosf(a1) * 8, sy + sinf(a1) * 8, gr);
                }
                vita2d_draw_line(sx - 20, sy, sx - 8, sy, gr);
                vita2d_draw_line(sx + 8, sy, sx + 20, sy, gr);
                vita2d_draw_line(sx, sy - 8, sx, sy - 15, gr);
            }
        }
    }
#endif
}

// Fill a convex polygon anywhere on screen (used by the instruments).
static void fill_poly_panel(const P2 *pts, int n, unsigned int color) {
    vita2d_color_vertex v[3 * 62];
    int k = 0;
    for (int i = 1; i < n - 1 && i < 62; i++) {
        v[k++] = (vita2d_color_vertex){ pts[0].x,     pts[0].y,     0.5f, color };
        v[k++] = (vita2d_color_vertex){ pts[i].x,     pts[i].y,     0.5f, color };
        v[k++] = (vita2d_color_vertex){ pts[i + 1].x, pts[i + 1].y, 0.5f, color };
    }
    draw_triangles(v, k);
}

// Attitude indicator: sky/ground ball that tilts and slides like the real horizon.
static void draw_attitude(float cx, float cy, float r) {
    const float PPD = 1.3f;                         // pixels per degree of pitch
    unsigned int white = RGBA8(240, 240, 240, 255);
    float sr = sinf(pl.roll), cr = cosf(pl.roll);
    float nx = sr, ny = cr;                         // toward the ground, on the ball
    float tx = cr, ty = -sr;                        // along the horizon
    float d  = pl.pitch * 57.2958f * PPD;           // horizon offset along n (nose up -> down)

    vita2d_draw_fill_circle(cx, cy, r + 4, RGBA8(90, 90, 96, 255));
    vita2d_draw_fill_circle(cx, cy, r, RGBA8(70, 130, 205, 255));          // sky

    float phi = acosf(clampf(d / r, -1.0f, 1.0f));  // half-angle of the ground arc
    if (phi > 0.02f) {
        P2 g[40];
        const int N = 36;
        float an = atan2f(ny, nx);
        for (int i = 0; i <= N; i++) {
            float a = an - phi + 2.0f * phi * i / N;
            g[i].x = cx + r * cosf(a);
            g[i].y = cy + r * sinf(a);
        }
        fill_poly_panel(g, N + 1, RGBA8(140, 96, 52, 255));                // ground
    }
    if (fabsf(d) < r - 1.0f) {                                             // horizon line
        float hx = cx + nx * d, hy = cy + ny * d;
        float h = sqrtf(r * r - d * d);
        vita2d_draw_line(hx - tx * h, hy - ty * h, hx + tx * h, hy + ty * h, white);
    }
    // Pitch ladder (5 / 10 / 20 degrees).
    static const int marks[] = { -20, -10, -5, 5, 10, 20 };
    for (int i = 0; i < 6; i++) {
        float m = d - marks[i] * PPD;               // positive pitch sits on the sky side
        if (fabsf(m) > r * 0.72f) continue;
        float qx = cx + nx * m, qy = cy + ny * m;
        float hl = (abs(marks[i]) == 5) ? 9.0f : 18.0f;
        vita2d_draw_line(qx - tx * hl, qy - ty * hl, qx + tx * hl, qy + ty * hl, white);
    }
    // Fixed bank scale at 0 / 30 / 60 degrees, and the pointer that rides with the ball.
    static const float bank_marks[] = { 0.0f, 30.0f, -30.0f, 60.0f, -60.0f };
    for (int i = 0; i < 5; i++) {
        float a = (-90.0f + bank_marks[i]) * PI_F / 180.0f;
        vita2d_draw_line(cx + cosf(a) * r * 0.86f, cy + sinf(a) * r * 0.86f,
                         cx + cosf(a) * r * 0.99f, cy + sinf(a) * r * 0.99f, white);
    }
    P2 tri[3] = {
        { cx - nx * r * 0.74f,                 cy - ny * r * 0.74f },
        { cx - nx * r * 0.97f - tx * 5.0f,     cy - ny * r * 0.97f - ty * 5.0f },
        { cx - nx * r * 0.97f + tx * 5.0f,     cy - ny * r * 0.97f + ty * 5.0f },
    };
    fill_poly_panel(tri, 3, RGBA8(255, 160, 40, 255));
    // Fixed aircraft symbol.
    unsigned int yel = RGBA8(255, 215, 60, 255);
    vita2d_draw_line(cx - 30, cy, cx - 10, cy, yel);
    vita2d_draw_line(cx - 10, cy, cx - 10, cy + 6, yel);
    vita2d_draw_line(cx + 10, cy, cx + 30, cy, yel);
    vita2d_draw_line(cx + 10, cy, cx + 10, cy + 6, yel);
    vita2d_draw_fill_circle(cx, cy, 2.5f, yel);
}

