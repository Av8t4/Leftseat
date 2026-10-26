// Left Seat: fields, roads, lakes, trees, airport ground detail and the cockpit window frame.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

/* ------------------------------------------------------------------ */
/* Scenery detail: fields, roads, lakes, trees, and the airport's      */
/* ground markings, taxiway, runway lights and glide-slope lights      */
/* ------------------------------------------------------------------ */

#define COCKPIT_NOSE 1           // draw the engine cowling and propeller blur in the cockpit view
#define COCKPIT_FRAME 1          // dashboard and windshield frame in the cockpit view
#define TREE_LIMIT 100           // most trees drawn at once (0 turns trees off; lower this if the frame rate dips)

#define ROAD_X 700.0f            // north-south road
#define ROAD_Z (-1700.0f)        // east-west road

static const unsigned int FIELD_DARK[5] = {
    RGBA8(54, 128, 46, 255), RGBA8(66, 134, 52, 255), RGBA8(92, 132, 52, 255),
    RGBA8(120, 124, 60, 255), RGBA8(74, 116, 60, 255) };
static const unsigned int FIELD_LIGHT[5] = {
    RGBA8(64, 146, 54, 255), RGBA8(76, 150, 60, 255), RGBA8(104, 148, 58, 255),
    RGBA8(134, 140, 66, 255), RGBA8(84, 132, 66, 255) };

// The land turns to red desert toward Redrock: 0 = farmland, 1 = full desert.
static float desert_k(float x, float z) {
    float d = sat_dist(x, z, AIRPORTS[AP_REDROCK].cx, AIRPORTS[AP_REDROCK].cz);
    float t = clampf((17000.0f - d) / 9000.0f, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
static unsigned int blend_col(unsigned int a, unsigned int b, float t) {
    if (t <= 0.0f) return a;
    float r = (float)(a & 255u), g = (float)((a >> 8) & 255u), bl = (float)((a >> 16) & 255u);
    r += ((float)(b & 255u) - r) * t; g += ((float)((b >> 8) & 255u) - g) * t; bl += ((float)((b >> 16) & 255u) - bl) * t;
    return RGBA8((int)r, (int)g, (int)bl, 255);
}
// Dense forest round Pinecrest: 0 = the usual country, 1 = deep forest.
static float forest_k(float x, float z) {
    float d = sat_dist(x, z, AIRPORTS[AP_PINECREST].cx, AIRPORTS[AP_PINECREST].cz);
    float t = clampf((9000.0f - d) / 6000.0f, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
static const unsigned int FOREST_DARK[5] = {
    RGBA8(34, 92, 46, 255), RGBA8(38, 98, 48, 255), RGBA8(30, 86, 44, 255), RGBA8(42, 102, 52, 255), RGBA8(28, 82, 42, 255) };
static const unsigned int FOREST_LIGHT[5] = {
    RGBA8(44, 108, 56, 255), RGBA8(48, 114, 58, 255), RGBA8(40, 102, 54, 255), RGBA8(52, 118, 62, 255), RGBA8(38, 98, 52, 255) };
static const unsigned int DESERT_DARK[5] = {
    RGBA8(146, 104, 74, 255), RGBA8(154, 108, 76, 255), RGBA8(138, 100, 72, 255), RGBA8(160, 116, 84, 255), RGBA8(128, 92, 68, 255) };
static const unsigned int DESERT_LIGHT[5] = {
    RGBA8(168, 120, 84, 255), RGBA8(176, 126, 88, 255), RGBA8(158, 114, 80, 255), RGBA8(182, 134, 96, 255), RGBA8(148, 108, 78, 255) };

static unsigned int field_color(int i, int j, bool dark) {
    int k = (int)(hf(hash2(i * 3 + 1, j * 5 + 2), 0) * 4.99f);
    unsigned int base = dark ? FIELD_DARK[k] : FIELD_LIGHT[k];
    float dk = desert_k((i + 0.5f) * TILE, (j + 0.5f) * TILE);
    unsigned int col = dk <= 0.0f ? base : blend_col(base, dark ? DESERT_DARK[k] : DESERT_LIGHT[k], dk);
    float fk = forest_k((i + 0.5f) * TILE, (j + 0.5f) * TILE);
    return fk <= 0.0f ? col : blend_col(col, dark ? FOREST_DARK[k] : FOREST_LIGHT[k], fk);
}

// Lakes: irregular octagons with a sandy shore.
typedef struct { float x, z, rx, rz; } Lake;
#define NLAKES 5
static const Lake LAKES[NLAKES] = { { -1800.0f, 2300.0f, 520.0f, 380.0f }, { 2600.0f, 1200.0f, 700.0f, 450.0f },
                                    { -2600.0f, -2600.0f, 600.0f, 500.0f },
                                    { 18450.0f, -1400.0f, 850.0f, 620.0f },       // the river ends in this one
                                    { 5234.0f, -14424.0f, 600.0f, 420.0f } };      // Lana Lake, 1100 m to the left of Lana Lake Regional's runway

static void lake_outline(int n, float k, V3 *out) {
    const Lake *L = &LAKES[n];
    for (int i = 0; i < 8; i++) {
        float a = (i + 0.5f) * PI_F / 4.0f;
        float jitter = 1.0f + 0.22f * (hf(hash2(n + 11, i + 5), 0) - 0.5f) * 2.0f;
        out[i] = v3(L->x + L->rx * k * jitter * cosf(a), 0.02f, L->z + L->rz * k * jitter * sinf(a));
    }
}
static void draw_lakes(void) {
    for (int n = 0; n < NLAKES; n++) {
        float dx = LAKES[n].x - cam.pos.x, dz = LAKES[n].z - cam.pos.z;
        if (dx * dx + dz * dz > 9000.0f * 9000.0f) continue;
        V3 shore[8], water[8];
        lake_outline(n, 1.10f, shore);
        lake_outline(n, 1.00f, water);
        draw_world_poly(shore, 8, RGBA8(196, 184, 130, 255));
        draw_world_poly(water, 8, RGBA8(52, 108, 172, 255));
    }
}
static bool in_lake(float x, float z, float grow) {
    for (int n = 0; n < NLAKES; n++) {
        float ux = (x - LAKES[n].x) / (LAKES[n].rx * grow), uz = (z - LAKES[n].z) / (LAKES[n].rz * grow);
        if (ux * ux + uz * uz < 1.0f) return true;
    }
    return false;
}

static void draw_roads(int i0, int j0, int tr) {
    const unsigned int road = RGBA8(86, 86, 92, 255);
    for (int j = j0 - tr; j <= j0 + tr; j++) draw_ground_quad(ROAD_X - 4.5f, j * TILE, ROAD_X + 4.5f, (j + 1) * TILE, road);
    for (int i = i0 - tr; i <= i0 + tr; i++) draw_ground_quad(i * TILE, ROAD_Z - 4.5f, (i + 1) * TILE, ROAD_Z + 4.5f, road);
    draw_ground_quad(190.0f, -304.0f, 460.0f, -296.0f, road);                 // access road from the terminal
    draw_ground_quad(460.0f, -304.0f, 704.0f, -296.0f, road);
}

// ---- trees -------------------------------------------------------------------------------
typedef struct { float x, z, r, h, d, y0; float shade; unsigned char cr, cg, cb; } Tree;     // y0: the ground the tree stands on
static Tree trees[256];
static int  ntrees;
static int cmp_tree_near(const void *a, const void *b) {
    float da = ((const Tree *)a)->d, db = ((const Tree *)b)->d;
    return (da > db) - (da < db);
}
static int cmp_tree_far(const void *a, const void *b) { return -cmp_tree_near(a, b); }

static bool tree_ok(float x, float z) {
    if (desert_k(x, z) > 0.5f) return false;                                 // no forests in the desert
    {                                                                         // keep the grass strip and its buildings clear
        float lx, lz;
        ap_local(&AIRPORTS[AP_PINECREST], x, z, &lx, &lz);
        if (lx > -50.0f && lx < 90.0f && fabsf(lz) < AIRPORTS[AP_PINECREST].hl + 70.0f) return false;
    }
    {                                                                         // keep Lana Lake Regional clear
        float lx, lz;
        ap_local(&AIRPORTS[AP_LANA], x, z, &lx, &lz);
        if (lx > -120.0f && lx < 260.0f && fabsf(lz) < AIRPORTS[AP_LANA].hl + 150.0f) return false;
    }
    if (sea_depth(x, z) > -250.0f) return false;                             // no trees in the sea or on its beach
    {                                                                         // or on the spaceport and its build site
        float lx, lz;
        ap_local(&AIRPORTS[AP_SPACE], x, z, &lx, &lz);
        if (lx > -120.0f && lx < 1500.0f && lz > -700.0f && lz < 2500.0f) return false;
    }
    if (fabsf(x) < 320.0f && fabsf(z) < 1400.0f) return false;              // airport
    if (fabsf(x - 150.0f) < 120.0f && fabsf(z) < 500.0f) return false;      // terminal area
    if (fabsf(x - ROAD_X) < 26.0f || fabsf(z - ROAD_Z) < 26.0f) return false;
    if (z > -330.0f && z < -270.0f && x > 150.0f && x < 720.0f) return false;
    return !in_lake(x, z, 1.30f);
}

// Trees are collected once a frame and then drawn in the same far-to-near order as the buildings and mountains (see
// draw_scene), so a mountain behind them no longer paints over them. They stand on a hill's slope, and not on the
// bare rock of the tall mountains.
static void collect_trees(void) {
    ntrees = 0;
    if (TREE_LIMIT <= 0) return;
    const int R = 4, MAXT = TREE_LIMIT;
    int i0 = (int)floorf(cam.pos.x / TILE), j0 = (int)floorf(cam.pos.z / TILE);
    for (int dj = -R; dj <= R; dj++)
        for (int di = -R; di <= R; di++) {
            int i = i0 + di, j = j0 + dj;
            unsigned int hp = hash2((int)floorf(i / 3.0f) + 50, (int)floorf(j / 3.0f) - 50);
            if (hf(hp, 0) > 0.30f + 0.55f * forest_k((i + 0.5f) * TILE, (j + 0.5f) * TILE)) continue;      // forest patches: about 30% of the land, most of it near Pinecrest
            for (int t = 0; t < 6 && ntrees < 256; t++) {
                unsigned int h = hash2(i * 7 + t, j * 13 + t * 5);
                float x = (i + hf(h, 0)) * TILE, z = (j + hf(h, 1)) * TILE;
                if (!tree_ok(x, z)) continue;
                V3 d = v3(x - cam.pos.x, -cam.pos.y, z - cam.pos.z);
                if (vdot(d, cam.f) < -80.0f) continue;                         // behind us
                float dist = sqrtf(d.x * d.x + d.z * d.z);
                if (dist > 1700.0f) continue;
                float y0 = 0.0f;
                bool rock = false;
                for (int m = 0; m < nmounts && !rock; m++) {
                    float sh = mountain_surface(&mounts[m], x, z);
                    if (sh <= 0.0f) continue;
                    if (mounts[m].h >= 200.0f) rock = true;                     // no trees on the tall mountains
                    else if (sh > y0) y0 = sh;                                  // on a hill: stand on its slope
                }
                for (int m = 0; m < nmesas && !rock; m++) if (mesa_surface(&mesas[m], x, z) > 0.0f) rock = true;
                if (rock) continue;
                Tree *tr = &trees[ntrees++];
                tr->x = x; tr->z = z; tr->y0 = y0; tr->r = 4.5f + 4.0f * hf(h, 2); tr->h = 11.0f + 9.0f * hf(h, 3); tr->d = dist;
                float g = hf(h, 2);
                tr->cr = (unsigned char)(30 + 24 * g); tr->cg = (unsigned char)(84 + 40 * hf(h, 3)); tr->cb = (unsigned char)(34 + 20 * g);
            }
        }
    if (ntrees > MAXT) { qsort(trees, ntrees, sizeof(Tree), cmp_tree_near); ntrees = MAXT; }
}
static void draw_tree(const Tree *t) {
    V3 apex = v3(t->x, t->y0 + t->h, t->z), b[4];
    for (int k = 0; k < 4; k++) {
        float a = PI_F / 4.0f + k * PI_F / 2.0f;
        b[k] = v3(t->x + t->r * cosf(a), t->y0, t->z + t->r * sinf(a));
    }
    for (int k = 0; k < 4; k++) {
        V3 tri[3] = { b[k], b[(k + 1) & 3], apex };
        V3 nrm = vcross(vsub(tri[1], tri[0]), vsub(tri[2], tri[0]));
        float len = sqrtf(vdot(nrm, nrm));
        float shade = 0.55f + 0.45f * fabsf(len > 1e-4f ? vdot(nrm, SUN) / len : 0.7f);
        draw_world_poly(tri, 3, RGBA8((int)(t->cr * shade), (int)(t->cg * shade), (int)(t->cb * shade), 255));
    }
}

// ---- airport ground detail ----------------------------------------------------------------
// Runway coordinates: lateral l (across) and along a (from the threshold of the end being used).
static void rwy_rect(int north, float l0, float l1, float a0, float a1, unsigned int col) {
    float xa = north ? -l0 : l0, xb = north ? -l1 : l1;
    float za = north ? 1000.0f - a0 : -1000.0f + a0, zb = north ? 1000.0f - a1 : -1000.0f + a1;
    draw_ground_quad(fminf(xa, xb), fminf(za, zb), fmaxf(xa, xb), fmaxf(za, zb), col);
}

static void draw_digit(int d, int north, float lat0, unsigned int col) {
    static const int MASK[10] = { 0, 6, 0, 79, 0, 0, 125, 0, 127, 0 };    // only the digits used: 1 3 6 8
    const float W = 7.0f, H = 18.0f, t = 1.6f, A0 = 45.0f;
    float seg[7][4] = { { 0, W, H - t, H }, { W - t, W, H / 2, H }, { W - t, W, 0, H / 2 }, { 0, W, 0, t },
                        { 0, t, 0, H / 2 }, { 0, t, H / 2, H }, { 0, W, H / 2 - t / 2, H / 2 + t / 2 } };
    for (int s = 0; s < 7; s++)
        if (MASK[d] & (1 << s))
            rwy_rect(north, lat0 + seg[s][0], lat0 + seg[s][1], A0 + seg[s][2], A0 + seg[s][3], col);
}

static void draw_airport_ground(void) {
    float cdx = cam.pos.x, cdz = cam.pos.z;
    if (cdx * cdx + cdz * cdz > 3000.0f * 3000.0f) return;                  // too far to see the detail
    const unsigned int tw = RGBA8(86, 86, 92, 255), yel = RGBA8(232, 202, 64, 255), wht = RGBA8(240, 240, 244, 255);

    draw_ground_quad(-70.0f, -980.0f, -58.0f, 980.0f, tw);                  // taxiway, connectors and apron
    draw_ground_quad(-58.0f, -966.0f, -22.0f, -954.0f, tw);
    draw_ground_quad(-58.0f, -6.0f, -22.0f, 6.0f, tw);
    draw_ground_quad(-58.0f, 954.0f, -22.0f, 966.0f, tw);
    draw_ground_quad(-90.0f, -700.0f, -70.0f, -200.0f, tw);
    draw_ground_quad(-64.3f, -980.0f, -63.7f, 980.0f, yel);

    for (int north = 0; north < 2; north++) {                                // numbers and marks at both ends
        draw_digit(north ? 1 : 3, north, -9.0f, wht);
        draw_digit(north ? 8 : 6, north,  2.0f, wht);
        rwy_rect(north, -12.0f, -6.0f, 300.0f, 345.0f, wht);                 // aiming point
        rwy_rect(north,   6.0f, 12.0f, 300.0f, 345.0f, wht);
        for (int s = -1; s <= 1; s += 2) {
            rwy_rect(north, s * 4.0f, s * 6.0f, 150.0f, 172.0f, wht);        // touchdown zone bars
            rwy_rect(north, s * 8.0f, s * 10.0f, 150.0f, 172.0f, wht);
            rwy_rect(north, s * 4.0f, s * 6.0f, 450.0f, 472.0f, wht);
        }
    }
    // Runway edge lights and end lights (green at the south end, red at the north end).
    for (float z = -990.0f; z <= 990.0f; z += 90.0f)
        for (int s = -1; s <= 1; s += 2) {
            float x = s * 24.5f, dx = x - cdx, dz = z - cdz;
            if (dx * dx + dz * dz < 2200.0f * 2200.0f)
                draw_ground_quad(x - 0.7f, z - 0.7f, x + 0.7f, z + 0.7f, RGBA8(255, 246, 190, 255));
        }
    for (float x = -20.0f; x <= 20.0f; x += 4.0f) {
        draw_ground_quad(x - 0.6f, -1005.0f, x + 0.6f, -1003.8f, RGBA8(70, 230, 90, 255));
        draw_ground_quad(x - 0.6f, 1003.8f, x + 0.6f, 1005.0f, RGBA8(235, 60, 50, 255));
    }
}

// Glide-slope lights beside the runway at the aiming point: white = high, red = low, mixed = on slope.
static void draw_papi(void) {
    float cdx = cam.pos.x, cdz = cam.pos.z;
    if (cdx * cdx + cdz * cdz > 3500.0f * 3500.0f) return;
    static const float THR[4] = { 3.50f, 3.17f, 2.83f, 2.50f };               // degrees, inner unit first
    float eye = pl.pos.y + 2.0f;
    for (int end = 0; end < 2; end++) {
        float zp = end ? 700.0f : -700.0f;
        float d = end ? pl.pos.z - zp : zp - pl.pos.z;                        // distance before the units
        float ang = d > 5.0f ? atan2f(eye, d) * 57.2958f : -1.0f;
        for (int i = 0; i < 4; i++) {
            float x = (end ? 1.0f : -1.0f) * (30.0f + 4.0f * i);
            unsigned int col = RGBA8(110, 110, 116, 255);                      // dark unless you are on that approach
            if (ang > 0.0f && d < 6000.0f) col = ang > THR[i] ? RGBA8(255, 255, 255, 255) : RGBA8(255, 50, 40, 255);
            V3 q[4] = { v3(x - 0.8f, 1.0f, zp), v3(x + 0.8f, 1.0f, zp), v3(x + 0.8f, 2.3f, zp), v3(x - 0.8f, 2.3f, zp) };
            if (ang > 0.0f && d < 6000.0f) draw_world_poly_emit(q, 4, col);      // lit units glow
            else                           draw_world_poly(q, 4, col);
        }
    }
}

// The cabin around you: windshield posts, door pillars, rear quarter panels, roof and sill. They are fixed to
// the aircraft, so they slide across the view as you turn your head and you always look out through a window.
static float cabin_rel(float az) {                 // direction relative to where you look (degrees, wrapped)
    float d = az - look_ang * 57.2958f;
    while (d > 180.0f) d -= 360.0f;
    while (d < -180.0f) d += 360.0f;
    return d;
}
static float cabin_x(float d_deg) {
    d_deg = clampf(d_deg, -80.0f, 80.0f);
    return CX + FOCAL * tanf(d_deg * PI_F / 180.0f);
}
static void cabin_wall(float az0, float az1, unsigned int dark, unsigned int rim) {
    float d0 = cabin_rel(az0), d1 = d0 + (az1 - az0);
    if (d1 < -80.0f || d0 > 80.0f) return;
    float x0 = cabin_x(d0), x1 = cabin_x(d1), VH = (float)VIEW_H;
    P2 q[4] = { {x0, 0}, {x1, 0}, {x1, VH}, {x0, VH} };
    fill_poly2d(q, 4, dark);
    if (d0 > -80.0f) { P2 e[4] = { {x0 - 4, 0}, {x0, 0}, {x0, VH}, {x0 - 4, VH} }; fill_poly2d(e, 4, rim); }
    if (d1 < 80.0f)  { P2 e[4] = { {x1, 0}, {x1 + 4, 0}, {x1 + 4, VH}, {x1, VH} }; fill_poly2d(e, 4, rim); }
}

static void draw_cockpit_frame(void) {
    const float VH = (float)VIEW_H;
    unsigned int dark = RGBA8(30, 32, 38, 255), rim = RGBA8(78, 82, 94, 255);
    if (spec->canopy) {                                                                    // bubble canopy: a thin front bow and low coaming
        cabin_wall(42.0f, 43.5f, dark, rim);
        cabin_wall(-43.5f, -42.0f, dark, rim);
        P2 bow[4] = { {0, 0}, {960, 0}, {960, 3}, {0, 2} };
        fill_poly2d(bow, 4, dark);
        P2 coam[4] = { {0, VH}, {0, VH - 11}, {960, VH - 11}, {960, VH} };
        fill_poly2d(coam, 4, dark);
        float cen = cabin_rel(0.0f);
        if (fabsf(cen) < 100.0f) {
            static const float AZ[5] = { -40.0f, -20.0f, 0.0f, 20.0f, 40.0f }, HH[5] = { 11, 14, 17, 14, 11 };
            P2 dash[7];
            for (int k = 0; k < 5; k++) { dash[k].x = cabin_x(cen + AZ[k]); dash[k].y = VH - HH[k]; }
            dash[5].x = dash[4].x; dash[5].y = VH;
            dash[6].x = dash[0].x; dash[6].y = VH;
            fill_poly2d(dash, 7, dark);
            for (int k = 0; k < 4; k++) {
                P2 lip[4] = { {dash[k].x, dash[k].y}, {dash[k + 1].x, dash[k + 1].y}, {dash[k + 1].x, dash[k + 1].y + 2}, {dash[k].x, dash[k].y + 2} };
                fill_poly2d(lip, 4, rim);
            }
        }
        return;
    }
    for (int sgn = -1; sgn <= 1; sgn += 2) {
        cabin_wall(sgn > 0 ? 40.0f : -48.0f,  sgn > 0 ? 48.0f : -40.0f,  dark, rim);     // windshield post
        cabin_wall(sgn > 0 ? 112.0f : -120.0f, sgn > 0 ? 120.0f : -112.0f, dark, rim);   // door pillar
    }
    P2 head[4] = { {0, 0}, {960, 0}, {960, 8}, {0, 6} };                                   // roof
    fill_poly2d(head, 4, dark);
    P2 sill[4] = { {0, VH}, {0, VH - 16}, {960, VH - 16}, {960, VH} };                     // window sill all round
    fill_poly2d(sill, 4, dark);
    float centre = cabin_rel(0.0f);
    if (fabsf(centre) < 100.0f) {                                                          // glareshield over the instrument panel
        static const float AZ[5] = { -45.0f, -22.0f, 0.0f, 22.0f, 45.0f }, HH[5] = { 16, 21, 25, 21, 16 };
        P2 dash[7];
        for (int k = 0; k < 5; k++) { dash[k].x = cabin_x(centre + AZ[k]); dash[k].y = VH - HH[k]; }
        dash[5].x = dash[4].x; dash[5].y = VH;
        dash[6].x = dash[0].x; dash[6].y = VH;
        fill_poly2d(dash, 7, dark);
        for (int k = 0; k < 4; k++) {                                                      // lit lip along the top
            P2 lip[4] = { {dash[k].x, dash[k].y}, {dash[k + 1].x, dash[k + 1].y},
                          {dash[k + 1].x, dash[k + 1].y + 3}, {dash[k].x, dash[k].y + 3} };
            fill_poly2d(lip, 4, rim);
        }
    }
}

