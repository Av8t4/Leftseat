// Left Seat: building the static world (buildings, hills, towers) and its collisions.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* world building (static, flat-shaded faces)                          */
/* ------------------------------------------------------------------ */

static void add_face(int n, const V3 *pts, float r, float g, float b) {
    if (nfaces >= MAX_FACES) return;
    Face *f = &faces[nfaces];
    f->n = n;
    V3 c = v3(0, 0, 0);
    for (int i = 0; i < n; i++) {
        f->p[i] = pts[i];
        c.x += pts[i].x; c.y += pts[i].y; c.z += pts[i].z;
    }
    f->c = v3(c.x / n, c.y / n, c.z / n);
    f->rad = 0.0f; f->top = 0.0f;
    for (int i = 0; i < n; i++) {
        if (pts[i].y > f->top) f->top = pts[i].y;
        float dx = pts[i].x - f->c.x, dy = pts[i].y - f->c.y, dz = pts[i].z - f->c.z;
        f->rad = fmaxf(f->rad, sqrtf(dx * dx + dy * dy + dz * dz));
    }

    // Fixed "sun" lighting baked into the colour (flat shading).
    V3 nrm = vcross(vsub(pts[1], pts[0]), vsub(pts[2], pts[0]));
    float len = sqrtf(vdot(nrm, nrm));
    float shade = 0.7f;
    if (len > 1e-4f) {
        V3 sun = v3(0.50f, 0.78f, 0.37f);
        shade = 0.50f + 0.50f * fabsf(vdot(nrm, sun) / len);
    }
    f->color = RGBA8((int)clampf(r * shade, 0, 255),
                     (int)clampf(g * shade, 0, 255),
                     (int)clampf(b * shade, 0, 255), 255);
    nfaces++;
}

static void add_box_faces(float x, float z, float w, float d, float y0, float y1,
                          float r, float g, float b) {
    float x0 = x - w / 2, x1 = x + w / 2, z0 = z - d / 2, z1 = z + d / 2;
    V3 s0[4] = { v3(x0,y0,z0), v3(x1,y0,z0), v3(x1,y1,z0), v3(x0,y1,z0) };
    V3 s1[4] = { v3(x1,y0,z0), v3(x1,y0,z1), v3(x1,y1,z1), v3(x1,y1,z0) };
    V3 s2[4] = { v3(x1,y0,z1), v3(x0,y0,z1), v3(x0,y1,z1), v3(x1,y1,z1) };
    V3 s3[4] = { v3(x0,y0,z1), v3(x0,y0,z0), v3(x0,y1,z0), v3(x0,y1,z1) };
    V3 rf[4] = { v3(x0,y1,z0), v3(x1,y1,z0), v3(x1,y1,z1), v3(x0,y1,z1) };
    add_face(4, s0, r, g, b);
    add_face(4, s1, r, g, b);
    add_face(4, s2, r, g, b);
    add_face(4, s3, r, g, b);
    add_face(4, rf, r * 1.15f, g * 1.15f, b * 1.15f);
}

static void add_building(float x, float z, float w, float d, float h,
                         float r, float g, float b) {
    if (nboxes < MAX_BOXES) boxes[nboxes++] = (Box){ x, z, w, d, h };
    add_box_faces(x, z, w, d, 0, h, r, g, b);
}

#define MT_STEP 0.65f   // fraction of height where rock turns to snow
#define MT_TOP  0.35f   // half-width of the snow base relative to hb

// Mountains are built in two steps: the callers below only record a mountain and its colours; once the rivers, roads
// and towns exist, settle_mountains() moves any mountain that sits on top of one of them and then builds the faces.
static float mt_col[MAX_MOUNTS][6];
static bool  mt_has[MAX_MOUNTS];
static int   mt_pine_first = MAX_MOUNTS;                       // mountains from here on belong to Pinecrest's valley and stay put

static void add_mountain2(const Mount *mp, float r, float g, float b, float cr, float cg, float cb) {
    int i = (int)(mp - mounts);
    if (i < 0 || i >= MAX_MOUNTS) return;
    mt_col[i][0] = r; mt_col[i][1] = g; mt_col[i][2] = b; mt_col[i][3] = cr; mt_col[i][4] = cg; mt_col[i][5] = cb;
    mt_has[i] = true;
}
static void emit_mountain(Mount m, float r, float g, float b, float cr, float cg, float cb) {
    float hb = m.hb, ht = m.hb * MT_TOP, yh = m.h * MT_STEP;
    V3 bs[4] = { v3(m.x-hb,0,m.z-hb), v3(m.x+hb,0,m.z-hb), v3(m.x+hb,0,m.z+hb), v3(m.x-hb,0,m.z+hb) };
    V3 ts[4] = { v3(m.x-ht,yh,m.z-ht), v3(m.x+ht,yh,m.z-ht), v3(m.x+ht,yh,m.z+ht), v3(m.x-ht,yh,m.z+ht) };
    V3 apex = v3(m.x, m.h, m.z);
    for (int i = 0; i < 4; i++) {
        int j = (i + 1) & 3;
        V3 side[4] = { bs[i], bs[j], ts[j], ts[i] };
        add_face(4, side, r, g, b);
        V3 cap[3] = { ts[i], ts[j], apex };
        add_face(3, cap, cr, cg, cb);
    }
}
static void add_mountain(const Mount *mp, float r, float g, float b) { add_mountain2(mp, r, g, b, 245, 245, 250); }   // rock with a snow cap

// A flat-topped butte (mesa): the sides slope up to a flat top.
static void add_mesa(Mesa m, float r, float g, float b) {
    float hb = m.hb, ht = m.top;
    V3 bs[4] = { v3(m.x-hb,0,m.z-hb), v3(m.x+hb,0,m.z-hb), v3(m.x+hb,0,m.z+hb), v3(m.x-hb,0,m.z+hb) };
    V3 ts[4] = { v3(m.x-ht,m.h,m.z-ht), v3(m.x+ht,m.h,m.z-ht), v3(m.x+ht,m.h,m.z+ht), v3(m.x-ht,m.h,m.z+ht) };
    for (int i = 0; i < 4; i++) {
        int j = (i + 1) & 3;
        V3 side[4] = { bs[i], bs[j], ts[j], ts[i] };
        add_face(4, side, r * (i & 1 ? 0.86f : 1.0f), g * (i & 1 ? 0.86f : 1.0f), b * (i & 1 ? 0.86f : 1.0f));
    }
    add_face(4, ts, r * 1.18f, g * 1.12f, b * 1.05f);
    if (nmesas < MAX_MESAS) mesas[nmesas++] = m;
}
static float mesa_surface(const Mesa *m, float x, float z) {
    float d = fmaxf(fabsf(x - m->x), fabsf(z - m->z));
    if (d >= m->hb) return 0.0f;
    if (d <= m->top) return m->h;
    return m->h * (m->hb - d) / (m->hb - m->top);
}
static float mountain_surface(const Mount *m, float x, float z) {
    float d = fmaxf(fabsf(x - m->x), fabsf(z - m->z));
    float ht = m->hb * MT_TOP, yh = m->h * MT_STEP;
    if (d >= m->hb) return 0.0f;
    if (d >= ht)    return yh * (m->hb - d) / (m->hb - ht);
    return m->h - (m->h - yh) * d / ht;
}

// Solid of revolution around a vertical axis. Ring i has radius rad[i] at height hgt[i];
// gaps from `split` upwards use the second colour. Rings of radius 0 become a point.
static void add_lathe(float cx, float cz, const float *rad, const float *hgt, int nrings, int sides, int split,
                      float r, float g, float b, float r2, float g2, float b2) {
    for (int i = 0; i + 1 < nrings; i++) {
        int second = (i >= split);
        for (int j = 0; j < sides; j++) {
            float a0 = (j + 0.5f) * 2.0f * PI_F / sides, a1 = (j + 1.5f) * 2.0f * PI_F / sides;
            V3 p0 = v3(cx + rad[i] * sinf(a0),     hgt[i],     cz + rad[i] * cosf(a0));
            V3 p1 = v3(cx + rad[i] * sinf(a1),     hgt[i],     cz + rad[i] * cosf(a1));
            V3 p2 = v3(cx + rad[i + 1] * sinf(a1), hgt[i + 1], cz + rad[i + 1] * cosf(a1));
            V3 p3 = v3(cx + rad[i + 1] * sinf(a0), hgt[i + 1], cz + rad[i + 1] * cosf(a0));
            float cr = second ? r2 : r, cg = second ? g2 : g, cb = second ? b2 : b;
            if (rad[i + 1] < 0.001f)      { V3 t[3] = { p0, p1, p2 }; add_face(3, t, cr, cg, cb); }
            else if (rad[i] < 0.001f)     { V3 t[3] = { p0, p2, p3 }; add_face(3, t, cr, cg, cb); }
            else                          { V3 q[4] = { p0, p1, p2, p3 }; add_face(4, q, cr, cg, cb); }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Airports. Positions on an airport are in its own frame: lx across   */
/* (right is positive) and lz along the runway, from -1000 to +1000.   */
/* ------------------------------------------------------------------ */

typedef struct { float cx, cz, hdg, s, c; const char *name, *id; float hl, hw; const char *place; } Airport;      // s, c: sine and cosine of the runway heading; hl, hw: half length and half width of the runway
enum { AP_HOME = 0, AP_REDROCK, AP_PINECREST, AP_LANA, AP_COUNT };
static const Airport AIRPORTS[AP_COUNT] = {
    {     0.0f,     0.0f, 0.0f,       0.0f,       1.0f,       "KNOWLES FIELD",       "KAK", 1000.0f, 22.0f, "Knowles Field" },
    { 14500.0f, 25500.0f, 1.2217305f, 0.9396926f, 0.3420201f, "REDROCK MUNICIPAL",  "RDR", 1000.0f, 22.0f, "Redrock" },        // runway 07 / 25
    {-19000.0f,  8900.0f, 2.0943951f, 0.8660254f, -0.5f,      "PINECREST AIRSTRIP", "PNC",  325.0f, 12.0f, "Pinecrest" },      // grass, runway 12 / 30
    {  4200.0f,-14800.0f, 2.7925268f, 0.3420201f,-0.9396926f, "LANA LAKE REGIONAL", "LLK",  800.0f, 18.0f, "Lana Lake" },     // 8 NM south of home, runway 16 / 34, the lake to its left
};
static V3 ap_pt(const Airport *a, float lx, float y, float lz) {
    return v3(a->cx + lx * a->c + lz * a->s, y, a->cz - lx * a->s + lz * a->c);
}
static void ap_local(const Airport *a, float x, float z, float *lx, float *lz) {
    float dx = x - a->cx, dz = z - a->cz;
    *lx = dx * a->c - dz * a->s;
    *lz = dx * a->s + dz * a->c;
}

// A box on an airport, turned with the runway. The collision box is the rectangle around it.
static void add_box_rot(float cx, float cz, float w, float d, float y0, float y1, const Airport *a,
                        float r, float g, float b) {
    float ox[4] = { -w / 2, w / 2, w / 2, -w / 2 }, oz[4] = { -d / 2, -d / 2, d / 2, d / 2 };
    V3 q[4];
    for (int i = 0; i < 4; i++) q[i] = v3(cx + ox[i] * a->c + oz[i] * a->s, 0.0f, cz - ox[i] * a->s + oz[i] * a->c);
    for (int i = 0; i < 4; i++) {
        int j = (i + 1) & 3;
        V3 s[4] = { v3(q[i].x, y0, q[i].z), v3(q[j].x, y0, q[j].z), v3(q[j].x, y1, q[j].z), v3(q[i].x, y1, q[i].z) };
        add_face(4, s, r, g, b);
    }
    V3 roof[4] = { v3(q[0].x, y1, q[0].z), v3(q[1].x, y1, q[1].z), v3(q[2].x, y1, q[2].z), v3(q[3].x, y1, q[3].z) };
    add_face(4, roof, r * 1.15f, g * 1.15f, b * 1.15f);
}
static void ap_building(const Airport *a, float lx, float lz, float w, float d, float h, float r, float g, float b) {
    V3 c0 = ap_pt(a, lx, 0.0f, lz);
    if (nboxes < MAX_BOXES) boxes[nboxes++] = (Box){ c0.x, c0.z, fabsf(w * a->c) + fabsf(d * a->s), fabsf(w * a->s) + fabsf(d * a->c), h };
    add_box_rot(c0.x, c0.z, w, d, 0.0f, h, a, r, g, b);
}

// Things you can see on the chart and light up at night.
#define MAX_TOWNS 8
static float town_x[MAX_TOWNS], town_z[MAX_TOWNS];
static int   ntowns;
static V3    town_lights[160];
static int   ntown_lights;
#define MAX_RIVER 52
static float river_x[MAX_RIVER], river_z[MAX_RIVER];
static int   nriver;

// Country roads: polylines of ground points. road_new marks the first point of each road, road_dirt a gravel road.
#define MAX_ROADP 220
static float road_x[MAX_ROADP], road_z[MAX_ROADP];
static unsigned char road_new[MAX_ROADP], road_dirt[MAX_ROADP];
static int   nroadp;
// Add a gently winding road from A to B (no random numbers, so nothing else in the world moves).
static void road_leg(float ax, float az, float bx, float bz, int dirt, int first, int seed) {
    float dx = bx - ax, dz = bz - az, len = sqrtf(dx * dx + dz * dz);
    if (len < 1.0f) return;
    int n = (int)(len / 700.0f) + 1;
    float nx = -dz / len, nz = dx / len, amp = dirt ? 170.0f : 120.0f;
    for (int k = first ? 0 : 1; k <= n && nroadp < MAX_ROADP; k++) {
        float t = (float)k / n, off = amp * sinf(PI_F * t) * sinf(2.6f * t * (1.0f + 0.15f * seed) + seed * 1.7f);
        road_x[nroadp] = ax + dx * t + nx * off;
        road_z[nroadp] = az + dz * t + nz * off;
        road_new[nroadp] = (first && k == 0);
        road_dirt[nroadp] = (unsigned char)dirt;
        nroadp++;
    }
}
static void road_poly(const float *p, int npts, int dirt, int seed) {      // p = x0,z0, x1,z1, ...
    for (int i = 0; i + 1 < npts; i++) road_leg(p[i * 2], p[i * 2 + 1], p[i * 2 + 2], p[i * 2 + 3], dirt, i == 0, seed + i);
}

// Bridges where a road crosses the river: centre, direction along the road, length along it and half width across it.
#define MAX_BRIDGES 10
static float br_x[MAX_BRIDGES], br_z[MAX_BRIDGES], br_dx[MAX_BRIDGES], br_dz[MAX_BRIDGES], br_len[MAX_BRIDGES], br_hw[MAX_BRIDGES];
static int   nbridges;
static void bridge_check(float ax, float az, float bx, float bz, float road_half) {
    float rx = bx - ax, rz = bz - az, rl = sqrtf(rx * rx + rz * rz);
    if (rl < 1.0f) return;
    for (int j = 0; j + 1 < nriver && nbridges < MAX_BRIDGES; j++) {
        float sx = river_x[j + 1] - river_x[j], sz = river_z[j + 1] - river_z[j];
        float den = rx * sz - rz * sx;
        if (fabsf(den) < 1e-3f) continue;
        float t = ((river_x[j] - ax) * sz - (river_z[j] - az) * sx) / den;       // along the road
        float u = ((river_x[j] - ax) * rz - (river_z[j] - az) * rx) / den;       // along the river segment
        if (t < 0.0f || t > 1.0f || u < 0.0f || u > 1.0f) continue;
        float cx = ax + rx * t, cz = az + rz * t;
        bool dup = false;
        for (int k = 0; k < nbridges; k++) if (fabsf(br_x[k] - cx) + fabsf(br_z[k] - cz) < 200.0f) dup = true;
        if (dup) continue;
        float hw = 11.0f + 24.0f * (float)j / (float)(nriver - 2);
        br_x[nbridges] = cx; br_z[nbridges] = cz; br_dx[nbridges] = rx / rl; br_dz[nbridges] = rz / rl;
        br_len[nbridges] = 2.0f * (hw + 10.0f) + 36.0f;                        // across the banks, plus a little ramp each side
        br_hw[nbridges] = road_half + 1.5f;
        nbridges++;
    }
}

static float sat_dist(float x, float z, float ax, float az) { return sqrtf((x - ax) * (x - ax) + (z - az) * (z - az)); }

// Redrock Municipal: a desert airport with the apron, terminal and tower on the other side from home.
static void build_redrock(void) {
    const Airport *a = &AIRPORTS[AP_REDROCK];
    ap_building(a,  195.0f, -220.0f, 36.0f, 110.0f,  9.0f, 206, 176, 142);    // terminal
    ap_building(a,  150.0f,  -90.0f, 24.0f,  34.0f,  6.0f, 214, 190, 150);    // annex
    for (int k = 0; k < 4; k++)                                                 // hangars
        ap_building(a, 125.0f, 300.0f + 82.0f * k, 52.0f, 62.0f, 13.0f, 188, 152, 112);
    ap_building(a, -80.0f, -100.0f, 8.0f, 8.0f, 30.0f, 214, 208, 198);        // tower
    {
        V3 c0 = ap_pt(a, -80.0f, 0.0f, -100.0f);
        add_box_rot(c0.x, c0.z, 14.0f, 14.0f, 30.0f, 36.0f, a, 80.0f, 120.0f, 172.0f);   // tower cab
    }
    ap_building(a,  235.0f, -20.0f, 9.0f, 9.0f, 10.0f, 224, 224, 228);         // fuel tanks
    ap_building(a,  235.0f,  -2.0f, 9.0f, 9.0f, 10.0f, 224, 224, 228);
}

// Pinecrest: a few wooden buildings beside a short grass strip.
static void build_pinecrest(void) {
    const Airport *a = &AIRPORTS[AP_PINECREST];
    ap_building(a, 34.0f, -40.0f, 14.0f, 18.0f, 6.0f, 150, 108, 70);          // open hangar
    ap_building(a, 52.0f,  20.0f,  8.0f,  6.0f, 3.6f, 122, 84, 56);           // cabin
    ap_building(a, 40.0f,  72.0f,  3.0f,  3.0f, 2.4f, 178, 62, 48);           // fuel store
}

// Lana Lake Regional: a small paved airport with a terminal, tower and three hangars on the right of the runway.
static void build_lana(void) {
    const Airport *a = &AIRPORTS[AP_LANA];
    ap_building(a, 165.0f, -160.0f, 30.0f, 70.0f,  8.0f, 205, 196, 170);      // terminal
    ap_building(a, 160.0f,  -40.0f,  7.0f,  7.0f, 20.0f, 214, 208, 198);      // tower
    {
        V3 c0 = ap_pt(a, 160.0f, 0.0f, -40.0f);
        add_box_rot(c0.x, c0.z, 12.0f, 12.0f, 20.0f, 25.0f, a, 80.0f, 120.0f, 172.0f);   // tower cab
    }
    for (int k = 0; k < 3; k++)                                                 // hangars
        ap_building(a, 135.0f, 170.0f + 75.0f * k, 50.0f, 60.0f, 12.0f, 150 + 14 * k, 156 + 10 * k, 172 + 6 * k);
    ap_building(a, 165.0f,  -8.0f, 8.0f, 8.0f, 9.0f, 224, 224, 228);          // fuel tank
}

#if WORLD_DETAIL >= 1
// The valley round Pinecrest: ridges either side of the strip, wooded hills and peaks at the ends. The two approach
// paths along the runway are kept clear.
static void build_pinecrest_country(void) {
    const Airport *P = &AIRPORTS[AP_PINECREST];
    mt_pine_first = nmounts;
    for (int side = -1; side <= 1; side += 2)
        for (int k = 0; k < 5 && nmounts < MAX_MOUNTS; k++) {
            float lat = side * (2300.0f + frand(-150.0f, 350.0f)), along = (k - 2) * 1500.0f + frand(-250.0f, 250.0f);
            float hb = frand(900.0f, 1300.0f), h = frand(650.0f, 1150.0f);
            V3 p = ap_pt(P, lat, 0.0f, along);
            mounts[nmounts] = (Mount){ p.x, p.z, hb, h };
            bool tall = h > 950.0f;
            add_mountain2(&mounts[nmounts], 108, 98, 88, tall ? 240.0f : 96.0f, tall ? 240.0f : 120.0f, tall ? 246.0f : 74.0f);
            nmounts++;
        }
    for (int k = 0; k < 6 && nmounts < MAX_MOUNTS; k++) {                            // wooded hills, well to the sides of the approaches
        float side = (k & 1) ? 1.0f : -1.0f;
        float lat = side * frand(1400.0f, 1900.0f), along = ((k % 3) - 1) * 2300.0f + frand(-500.0f, 500.0f);
        float hb = frand(380.0f, 620.0f), h = frand(70.0f, 150.0f);
        V3 p = ap_pt(P, lat, 0.0f, along);
        mounts[nmounts] = (Mount){ p.x, p.z, hb, h };
        add_mountain2(&mounts[nmounts], 44, 86, 48, 54, 100, 56);
        nmounts++;
    }
    for (int end = -1; end <= 1; end += 2)                                           // a peak at each end of the valley, off the approach line
        if (nmounts < MAX_MOUNTS) {
            V3 p = ap_pt(P, end * 1900.0f, 0.0f, end * 5600.0f);
            mounts[nmounts] = (Mount){ p.x, p.z, 1250.0f, 1050.0f };
            add_mountain2(&mounts[nmounts], 108, 98, 88, 240, 240, 246);
            nmounts++;
        }
}

// The country between the two airports: a mountain pass, low hills, red buttes near Redrock, towns and a river.
static bool world_spot_ok(float x, float z, float clear) {
    for (int k = 0; k < AP_COUNT; k++) if (sat_dist(x, z, AIRPORTS[k].cx, AIRPORTS[k].cz) < clear) return false;
    return true;
}
static void build_countryside(void) {
    const Airport *B = &AIRPORTS[AP_REDROCK];
    float L = sat_dist(B->cx, B->cz, 0.0f, 0.0f);
    float ux = B->cx / L, uz = B->cz / L;                  // along the route, home to Redrock
    float px = uz, pz = -ux;                                // across it (to the right)

    // The pass: two ridges of peaks with a gap about 2.6 km wide, half way along the route.
    float mx = ux * L * 0.46f, mz = uz * L * 0.46f;
    for (int side = -1; side <= 1; side += 2)
        for (int k = 0; k < 5 && nmounts < MAX_MOUNTS; k++) {
            float lat = side * (2100.0f + k * 900.0f + frand(-150, 150)), along = frand(-700.0f, 700.0f) + (k & 1) * 500.0f;
            float hb = frand(650, 1050), h = frand(520, 1250) + k * 120.0f;
            mounts[nmounts] = (Mount){ mx + ux * along + px * lat, mz + uz * along + pz * lat, hb, h };
            add_mountain2(&mounts[nmounts], 116, 102, 92, h > 950.0f ? 240.0f : 150.0f, h > 950.0f ? 240.0f : 138.0f, h > 950.0f ? 246.0f : 122.0f);
            nmounts++;
        }

    // Low rolling hills along the route.
    static const unsigned char HILL[4][3] = { {104,96,62}, {116,100,66}, {96,88,58}, {122,106,72} };   // earthy browns and olives, so a hill never looks like a flat green field
    for (int k = 0, placed = 0; k < 120 && placed < 28 && nmounts < MAX_MOUNTS; k++) {
        float t = frand(0.04f, 0.97f), lat = frand(-4200.0f, 4200.0f);
        float x = ux * L * t + px * lat, z = uz * L * t + pz * lat;
        if (fabsf(lat) < 650.0f || !world_spot_ok(x, z, 3200.0f)) continue;
        float hb = frand(380, 850), h = frand(55, 150);
        const unsigned char *c = HILL[k & 3];
        mounts[nmounts] = (Mount){ x, z, hb, h };
        add_mountain2(&mounts[nmounts], c[0], c[1], c[2], c[0] * 1.10f, c[1] * 1.10f, c[2] * 1.06f);
        nmounts++; placed++;
    }
#if WORLD_DETAIL >= 2
    // Red buttes around Redrock.
    static const float MES[6][2] = { {3200,-1500}, {-3600,1800}, {5200,2600}, {-5200,-2200}, {2800,4200}, {-2400,-4800} };
    for (int k = 0; k < 6; k++) {
        V3 p = ap_pt(B, MES[k][0], 0.0f, MES[k][1]);
        float hb = frand(450, 700);
        add_mesa((Mesa){ p.x, p.z, hb, frand(140, 260), hb * 0.55f }, 176, 98, 66);
    }
    // Towns.
    static const float TT[5] = { 0.16f, 0.34f, 0.60f, 0.78f, 0.93f }, TL[5] = { 1800.0f, -2300.0f, 1500.0f, -1900.0f, 2600.0f };
    for (int n = 0; n < 5 && ntowns < MAX_TOWNS; n++) {
        float tx = ux * L * TT[n] + px * TL[n], tz = uz * L * TT[n] + pz * TL[n];
        if (!world_spot_ok(tx, tz, 2200.0f)) continue;
        town_x[ntowns] = tx; town_z[ntowns] = tz; ntowns++;
        int count = 14 + (int)frand(0.0f, 7.0f);
        for (int k = 0; k < count && nboxes < MAX_BOXES; k++) {
            float ang = frand(0.0f, 2.0f * PI_F), rr = 40.0f + 340.0f * sqrtf(frand(0.0f, 1.0f));
            float x = tx + cosf(ang) * rr, z = tz + sinf(ang) * rr;
            float w = frand(14, 42), d = frand(14, 42), h = frand(6, 26);
            float tone = frand(150, 225);
            float kw = w * 0.5f + 8.0f, kd = d * 0.5f + 8.0f;                 // keep clear of the two main streets (see draw_town_streets)
            if (fabsf(x - tx) < kw) x = tx + (x >= tx ? kw : -kw);
            if (fabsf(z - tz) < kd) z = tz + (z >= tz ? kd : -kd);
            add_building(x, z, w, d, h, tone, tone * 0.90f, tone * 0.80f);
            if (ntown_lights < 160) town_lights[ntown_lights++] = v3(x, 3.5f, z);
        }
    }
    // A river wandering across the route, a third of the way along. It rises in the hills to the north-west and
    // runs on past the route to the south-east.
    {
        float cx0 = ux * L * 0.27f, cz0 = uz * L * 0.27f;
        nriver = 0;
        for (int k = 0; k < MAX_RIVER; k++) {
            float s = (k - MAX_RIVER / 2) * 650.0f;
            float wob = 380.0f * sinf(s / 1700.0f) + 160.0f * sinf(s / 620.0f + 1.0f);
            river_x[nriver] = cx0 + px * s + ux * wob;
            river_z[nriver] = cz0 + pz * s + uz * wob;
            nriver++;
        }
    }
    // Roads. A country road from the road beside the airfield through every town to Redrock, spurs down to the highway,
    // and a gravel road from Pinecrest out of its valley to the east-west road.
    {
        nroadp = 0;
        float hx0 = 704.0f, hz0 = -300.0f;
        V3 he = ap_pt(B, 330.0f, 0.0f, -220.0f);
        float chain[16];
        int nc = 0;
        chain[nc * 2] = 700.0f; chain[nc * 2 + 1] = (ntowns > 0 ? town_z[0] : 3000.0f); nc++;       // starts on the north-south road by the airfield
        for (int n = 0; n < ntowns && nc < 7; n++) { chain[nc * 2] = town_x[n]; chain[nc * 2 + 1] = town_z[n]; nc++; }
        chain[nc * 2] = he.x; chain[nc * 2 + 1] = he.z; nc++;                                       // ends where the highway meets Redrock
        road_poly(chain, nc, 0, 0);
        for (int n = 1; n < ntowns; n += 2) {                             // spurs from the highway to every other town
            float dx = he.x - hx0, dz = he.z - hz0, hl2 = dx * dx + dz * dz;
            float t = clampf(((town_x[n] - hx0) * dx + (town_z[n] - hz0) * dz) / hl2, 0.0f, 1.0f);
            float sp[4] = { hx0 + dx * t, hz0 + dz * t, town_x[n], town_z[n] };
            road_poly(sp, 2, 0, 5 + n);
        }
        const Airport *P = &AIRPORTS[AP_PINECREST];
        V3 p0 = ap_pt(P, 80.0f, 0.0f, 10.0f), p1 = ap_pt(P, 80.0f, 0.0f, 1300.0f), p2 = ap_pt(P, 220.0f, 0.0f, 3600.0f);   // out along the valley, towards home
        float gp[10] = { p0.x, p0.z, p1.x, p1.z, p2.x, p2.z, -11800.0f, 2200.0f, -8500.0f, -1700.0f };   // (the east-west road is at z = -1700)
        road_poly(gp, 5, 1, 2);
        nbridges = 0;                                                      // where the roads and the highway cross the river
        bridge_check(hx0, hz0, he.x, he.z, 5.5f);
        for (int i = 0; i + 1 < nroadp; i++)
            if (!road_new[i + 1]) bridge_check(road_x[i], road_z[i], road_x[i + 1], road_z[i + 1], road_dirt[i] ? 3.0f : 4.5f);
    }
#endif
}
#endif

// Does a square mountain footprint (centre x,z, half width hb) sit on a river, road, highway, town or airport?
static bool seg_hits_square(float ax, float az, float bx, float bz, float cx, float cz, float h) {
    float t0 = 0.0f, t1 = 1.0f, dx = bx - ax, dz = bz - az;
    float p[4] = { -dx, dx, -dz, dz }, q[4] = { ax - (cx - h), (cx + h) - ax, az - (cz - h), (cz + h) - az };
    for (int k = 0; k < 4; k++) {
        if (fabsf(p[k]) < 1e-6f) { if (q[k] < 0.0f) return false; continue; }
        float t = q[k] / p[k];
        if (p[k] < 0.0f) { if (t > t1) return false; if (t > t0) t0 = t; }
        else             { if (t < t0) return false; if (t < t1) t1 = t; }
    }
    return t0 <= t1;
}
static bool mt_conflict(float x, float z, float hb, bool near_ap) {
    const float M = 100.0f;
    if (fabsf(x - 700.0f) < hb + M || fabsf(z + 1700.0f) < hb + M) return true;          // the two long roads past the airfield
    for (int j = 0; j + 1 < nriver; j++)
        if (seg_hits_square(river_x[j], river_z[j], river_x[j + 1], river_z[j + 1], x, z, hb + M + 40.0f)) return true;
    for (int i = 0; i + 1 < nroadp; i++)
        if (!road_new[i + 1] && seg_hits_square(road_x[i], road_z[i], road_x[i + 1], road_z[i + 1], x, z, hb + M)) return true;
    {
        V3 he = ap_pt(&AIRPORTS[AP_REDROCK], 330.0f, 0.0f, -220.0f);
        if (seg_hits_square(704.0f, -300.0f, he.x, he.z, x, z, hb + M)) return true;      // the highway
    }
    for (int n = 0; n < ntowns; n++)
        if (fmaxf(fabsf(x - town_x[n]), fabsf(z - town_z[n])) < hb + 520.0f) return true;
    if (near_ap)
        for (int k = 0; k < AP_COUNT; k++)
            if (sat_dist(x, z, AIRPORTS[k].cx, AIRPORTS[k].cz) < hb + 1200.0f) return true;
    return false;
}
static void settle_mountains(void) {
    for (int i = 0; i < nmounts; i++) {
        if (!mt_has[i]) continue;
        Mount *m = &mounts[i];
        bool valley = (i >= mt_pine_first);                                  // Pinecrest's ridges are placed by hand: only nudge them a little
        bool hill = m->h < 200.0f;
        if (!mt_conflict(m->x, m->z, m->hb, !valley)) continue;
        float out = atan2f(m->x, m->z), maxr = valley ? 1200.0f : 9000.0f;
        bool found = false;
        for (float r = 250.0f; r <= maxr && !found; r += 250.0f)
            for (int a = 0; a < 16 && !found; a++) {
                float ang = out + ((a & 1) ? -1.0f : 1.0f) * ((a + 1) >> 1) * (PI_F / 8.0f);
                float nx = m->x + sinf(ang) * r, nz = m->z + cosf(ang) * r;
                if (!mt_conflict(nx, nz, m->hb, !valley)) { m->x = nx; m->z = nz; found = true; }
            }
        if (!found && hill) { m->hb = 0.0f; m->h = 0.0f; mt_has[i] = false; }   // no room for a little hill: leave it out
    }
    for (int i = 0; i < nmounts; i++)
        if (mt_has[i] && mounts[i].hb > 0.0f)
            emit_mountain(mounts[i], mt_col[i][0], mt_col[i][1], mt_col[i][2], mt_col[i][3], mt_col[i][4], mt_col[i][5]);
}

static void build_world(void) {
    srand(1990);
    nfaces = 0; nboxes = 0; nmesas = 0; ntowns = 0; ntown_lights = 0; nriver = 0;
    nmounts = NUM_MOUNTS; nbridges = 0; nroadp = 0; mt_pine_first = MAX_MOUNTS; memset(mt_has, 0, sizeof mt_has);
    vita2d_set_clear_color(RGBA8(118, 176, 236, 255));   // sky

    // Airport: hangars along the west side, control tower on the east.
    add_building(-110, -650, 40, 70, 14, 190, 170, 130);
    add_building(-110, -450, 40, 70, 14, 190, 170, 130);
    add_building(-110, -250, 40, 70, 14, 170, 150, 120);
    add_building(  95,    0,  9,  9, 44, 200, 200, 205);
    add_box_faces( 95,    0, 16, 16, 44, 51, 90, 140, 190);   // tower cab
    add_building( 150, -330, 70, 26, 10, 176, 180, 192);      // terminal
    add_building( 150, -215, 36, 20,  6, 198, 178, 150);      // annex
    add_building(-110,  -20, 40, 60, 14, 170, 150, 120);      // fourth hangar

    // Scattered town.
    for (int k = 0; k < 18; k++) {
        float x = (rand() & 1 ? 1.0f : -1.0f) * frand(250, 2600);
        float z = frand(-3000, 3000);
        if (fabsf(x) < 300 && fabsf(z) < 1300) continue;
        float w = frand(20, 70), d = frand(20, 70), h = frand(12, 80);
        float tone = frand(120, 215);
        add_building(x, z, w, d, h, tone, tone * 0.92f, tone * 0.85f);
    }

    // Distant mountains in a ring.
    for (int k = 0; k < NUM_MOUNTS; k++) {
        float a = (2.0f * PI_F * k) / NUM_MOUNTS + frand(-0.2f, 0.2f);
        float dist = frand(7000, 11000);
        mounts[k] = (Mount){ sinf(a) * dist, cosf(a) * dist,
                             frand(1500, 2700), frand(1200, 2500) };
        add_mountain(&mounts[k], 118, 104, 92);
    }

    // The second airport and the country around the route to it.
    build_redrock();
    build_pinecrest();
    build_lana();
#if WORLD_DETAIL >= 1
    build_countryside();
    build_pinecrest_country();
#endif
    settle_mountains();
}

