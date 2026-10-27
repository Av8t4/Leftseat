// Left Seat: finding a square card with a thick black border in a camera picture (for the AR view in the hangar).
// No Vita calls in here, so it can be tested on a PC with simulated pictures. The picture is 480 x 272, 4 bytes per pixel (R, G, B, A).
// The detector works on every second pixel (240 x 136), finds dark blobs, keeps the ones that are a square ring, and then fits a
// straight line to each side so the four corners are accurate to well under a pixel (a steady picture).
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

#define ARD_W   480
#define ARD_H   272
#define ARD_GW  240
#define ARD_GH  136
#define ARD_N   (ARD_GW * ARD_GH)
#define ARD_PUSH 0.55                 // grid pixels
#define ARD_BIAS_CX bcx
#define ARD_BIAS_CY bcy

typedef struct {
    float raw[4][2];       // the corners in picture pixels (480 x 272), clockwise on the screen, starting anywhere
    int   shift;           // the card's own "first" corner is raw[shift] (found from the pattern inside the border)
    int   code;            // the 16 bits of the pattern inside the border (lowest of the four turns)
    float area;            // in picture pixels
} ArHit;

static uint8_t  ard_g[ARD_N];
static uint8_t  ard_dark[ARD_N];
static uint16_t ard_lab[ARD_N];
static uint16_t ard_stk[ARD_N];
static uint32_t ard_ii[(ARD_GW + 1) * (ARD_GH + 1)];

// Solve for the 8 numbers of the map from the 4 points s to the 4 points d (x' = (h0 x + h1 y + h2) / (h6 x + h7 y + 1), ...).
static bool ard_homog(const float s[4][2], const float d[4][2], float h[8]) {
    double A[8][9];
    for (int i = 0; i < 4; i++) {
        double x = s[i][0], y = s[i][1], u = d[i][0], v = d[i][1];
        double r0[9] = { x, y, 1, 0, 0, 0, -u * x, -u * y, u };
        double r1[9] = { 0, 0, 0, x, y, 1, -v * x, -v * y, v };
        for (int k = 0; k < 9; k++) { A[2 * i][k] = r0[k]; A[2 * i + 1][k] = r1[k]; }
    }
    for (int c = 0; c < 8; c++) {
        int p = c;
        for (int r = c + 1; r < 8; r++) if (fabs(A[r][c]) > fabs(A[p][c])) p = r;
        if (fabs(A[p][c]) < 1e-9) return false;
        if (p != c) for (int k = 0; k < 9; k++) { double t = A[p][k]; A[p][k] = A[c][k]; A[c][k] = t; }
        for (int r = 0; r < 8; r++) {
            if (r == c) continue;
            double f = A[r][c] / A[c][c];
            for (int k = c; k < 9; k++) A[r][k] -= f * A[c][k];
        }
    }
    for (int i = 0; i < 8; i++) h[i] = (float)(A[i][8] / A[i][i]);
    return true;
}

static void ard_apply(const float h[8], float x, float y, float *u, float *v) {
    float w = h[6] * x + h[7] * y + 1.0f;
    if (fabsf(w) < 1e-6f) w = 1e-6f;
    *u = (h[0] * x + h[1] * y + h[2]) / w;
    *v = (h[3] * x + h[4] * y + h[5]) / w;
}

// Grey picture and the "dark" map (a pixel that is clearly darker than the area around it, or simply very dark).
// rows: the camera picture, `stride` bytes per row.
static void ard_prepare(const uint8_t *img, int stride) {
    static uint8_t row[ARD_W * 4];
    for (int y = 0; y < ARD_GH; y++) {
        memcpy(row, img + (size_t)(y * 2) * stride, ARD_W * 4);      // one burst read: the camera memory is slow to read pixel by pixel
        for (int x = 0; x < ARD_GW; x++) {
            const uint8_t *p = row + x * 8;
            ard_g[y * ARD_GW + x] = (uint8_t)((p[0] * 77 + p[1] * 150 + p[2] * 29) >> 8);
        }
    }
    for (int x = 0; x <= ARD_GW; x++) ard_ii[x] = 0;
    for (int y = 0; y < ARD_GH; y++) {
        uint32_t run = 0;
        ard_ii[(y + 1) * (ARD_GW + 1)] = 0;
        for (int x = 0; x < ARD_GW; x++) {
            run += ard_g[y * ARD_GW + x];
            ard_ii[(y + 1) * (ARD_GW + 1) + x + 1] = ard_ii[y * (ARD_GW + 1) + x + 1] + run;
        }
    }
    const int R = 12;
    for (int y = 0; y < ARD_GH; y++) {
        int y0 = y - R < 0 ? 0 : y - R, y1 = y + R + 1 > ARD_GH ? ARD_GH : y + R + 1;
        for (int x = 0; x < ARD_GW; x++) {
            int x0 = x - R < 0 ? 0 : x - R, x1 = x + R + 1 > ARD_GW ? ARD_GW : x + R + 1;
            uint32_t s = ard_ii[y1 * (ARD_GW + 1) + x1] - ard_ii[y0 * (ARD_GW + 1) + x1] - ard_ii[y1 * (ARD_GW + 1) + x0] + ard_ii[y0 * (ARD_GW + 1) + x0];
            int mean = (int)(s / (uint32_t)((x1 - x0) * (y1 - y0)));
            int g = ard_g[y * ARD_GW + x];
            ard_dark[y * ARD_GW + x] = (g * 10 < mean * 9 - 20 || g < 40) ? 1 : 0;
        }
    }
}

static bool ard_is_dark(float gx, float gy) {
    int x = (int)(gx + 0.5f), y = (int)(gy + 0.5f);
    if (x < 0 || y < 0 || x >= ARD_GW || y >= ARD_GH) return false;
    return ard_dark[y * ARD_GW + x] != 0;
}

// The 16 bits inside the border for one choice of first corner (grid coordinates). Returns -1 if the inside is blank.
static int ard_code(const float P[4][2], int shift) {
    float src[4][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 1} }, dst[4][2], h[8];
    for (int i = 0; i < 4; i++) { dst[i][0] = P[(i + shift) & 3][0]; dst[i][1] = P[(i + shift) & 3][1]; }
    if (!ard_homog(src, dst, h)) return -1;
    int v[16], gmin = 255, gmax = 0;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) {
            float u = 1.0f / 6.0f + (c + 0.5f) / 4.0f * (4.0f / 6.0f), w = 1.0f / 6.0f + (r + 0.5f) / 4.0f * (4.0f / 6.0f), px, py;
            ard_apply(h, u, w, &px, &py);
            int x = (int)(px + 0.5f), y = (int)(py + 0.5f);
            x = x < 0 ? 0 : (x >= ARD_GW ? ARD_GW - 1 : x); y = y < 0 ? 0 : (y >= ARD_GH ? ARD_GH - 1 : y);
            int g = ard_g[y * ARD_GW + x];
            v[r * 4 + c] = g; if (g < gmin) gmin = g; if (g > gmax) gmax = g;
        }
    if (gmax - gmin < 30) return -1;
    int thr = (gmin + gmax) / 2, code = 0;
    for (int i = 0; i < 16; i++) if (v[i] > thr) code |= 1 << i;
    return code;
}

// Find the best card in the picture. Returns false if there is none.
static bool ar_detect(const uint8_t *img, int stride, ArHit *out) {
    ard_prepare(img, stride);
    memset(ard_lab, 0, sizeof ard_lab);
    bool found = false;
    float best_area = 0.0f;
    unsigned id = 0;
    for (int s0 = 0; s0 < ARD_N; s0++) {
        if (!ard_dark[s0] || ard_lab[s0]) continue;
        if (++id > 60000) break;
        int sp = 0, count = 0;
        long sumx = 0, sumy = 0;
        int bx0 = ARD_GW, bx1 = 0, by0 = ARD_GH, by1 = 0;
        ard_stk[sp++] = (uint16_t)s0; ard_lab[s0] = (uint16_t)id;
        while (sp) {
            int p = ard_stk[--sp], x = p % ARD_GW, y = p / ARD_GW;
            count++;
            sumx += x; sumy += y;
            bx0 = x < bx0 ? x : bx0; bx1 = x > bx1 ? x : bx1; by0 = y < by0 ? y : by0; by1 = y > by1 ? y : by1;
            if (x > 0          && ard_dark[p - 1]      && !ard_lab[p - 1])      { ard_lab[p - 1] = (uint16_t)id;      ard_stk[sp++] = (uint16_t)(p - 1); }
            if (x < ARD_GW - 1 && ard_dark[p + 1]      && !ard_lab[p + 1])      { ard_lab[p + 1] = (uint16_t)id;      ard_stk[sp++] = (uint16_t)(p + 1); }
            if (y > 0          && ard_dark[p - ARD_GW] && !ard_lab[p - ARD_GW]) { ard_lab[p - ARD_GW] = (uint16_t)id; ard_stk[sp++] = (uint16_t)(p - ARD_GW); }
            if (y < ARD_GH - 1 && ard_dark[p + ARD_GW] && !ard_lab[p + ARD_GW]) { ard_lab[p + ARD_GW] = (uint16_t)id; ard_stk[sp++] = (uint16_t)(p + ARD_GW); }
        }
        if (count < 100 || count > 22000) continue;
        if (bx0 <= 1 || by0 <= 1 || bx1 >= ARD_GW - 2 || by1 >= ARD_GH - 2) continue;       // touching the edge of the picture: cut off
        // The four corners: the pixel farthest from the middle, the one farthest from that, then the farthest on either side of the line
        // between them (this works at any turn of the card, unlike taking the extremes along the diagonals).
        float P[4][2];
        {
            float mx = (float)sumx / count, my = (float)sumy / count;
            float ax = 0, ay = 0, bx = 0, by = 0, cx = 0, cy = 0, dx = 0, dy = 0, bd = -1.0f;
            for (int y = by0; y <= by1; y++) for (int x = bx0; x <= bx1; x++) {
                if (ard_lab[y * ARD_GW + x] != id) continue;
                float d2 = (x - mx) * (x - mx) + (y - my) * (y - my);
                if (d2 > bd) { bd = d2; ax = (float)x; ay = (float)y; }
            }
            bd = -1.0f;
            for (int y = by0; y <= by1; y++) for (int x = bx0; x <= bx1; x++) {
                if (ard_lab[y * ARD_GW + x] != id) continue;
                float d2 = (x - ax) * (x - ax) + (y - ay) * (y - ay);
                if (d2 > bd) { bd = d2; bx = (float)x; by = (float)y; }
            }
            float lx = bx - ax, ly = by - ay, ll = sqrtf(lx * lx + ly * ly);
            if (ll < 1.0f) continue;
            float pmax = 0.0f, nmax = 0.0f;
            for (int y = by0; y <= by1; y++) for (int x = bx0; x <= bx1; x++) {
                if (ard_lab[y * ARD_GW + x] != id) continue;
                float sd = ((x - ax) * ly - (y - ay) * lx) / ll;
                if (sd > pmax) { pmax = sd; cx = (float)x; cy = (float)y; }
                if (sd < nmax) { nmax = sd; dx = (float)x; dy = (float)y; }
            }
            if (pmax < 4.0f || nmax > -4.0f) continue;
            float Q[4][2] = { {ax, ay}, {cx, cy}, {bx, by}, {dx, dy} }, ang[4];
            for (int i = 0; i < 4; i++) ang[i] = atan2f(Q[i][1] - my, Q[i][0] - mx);
            int idx[4] = { 0, 1, 2, 3 };
            for (int i = 0; i < 4; i++) for (int j = i + 1; j < 4; j++) if (ang[idx[j]] < ang[idx[i]]) { int t = idx[i]; idx[i] = idx[j]; idx[j] = t; }
            for (int i = 0; i < 4; i++) { P[i][0] = Q[idx[i]][0]; P[i][1] = Q[idx[i]][1]; }   // increasing angle with y down is clockwise on the screen
        }
        // A sensible, convex four-sided shape.
        float area = 0.0f; bool ok = true;
        for (int i = 0; i < 4; i++) {
            int j = (i + 1) & 3, k = (i + 2) & 3;
            area += P[i][0] * P[j][1] - P[j][0] * P[i][1];
            float ex = P[j][0] - P[i][0], ey = P[j][1] - P[i][1], fx = P[k][0] - P[j][0], fy = P[k][1] - P[j][1];
            if (ex * fy - ey * fx <= 0.0f) ok = false;                      // clockwise on the screen means this is positive
            if (ex * ex + ey * ey < 12.0f * 12.0f) ok = false;
        }
        area *= 0.5f;
        if (!ok || area < 220.0f) continue;
        float fill = (float)count / area;
        if (fill < 0.10f || fill > 0.92f) continue;                         // a ring is part-filled; a solid blob or a scribble is not
        // Fit a line to the outer edge of the blob along each side, and meet them at the corners.
        double sn[4] = { 0 }, sx[4] = { 0 }, sy[4] = { 0 }, sxx[4] = { 0 }, sxy[4] = { 0 }, syy[4] = { 0 };
        for (int y = by0; y <= by1; y++)
            for (int x = bx0; x <= bx1; x++) {
                int p = y * ARD_GW + x;
                if (ard_lab[p] != id) continue;
                if (ard_lab[p - 1] == id && ard_lab[p + 1] == id && ard_lab[p - ARD_GW] == id && ard_lab[p + ARD_GW] == id) continue;
                for (int e = 0; e < 4; e++) {
                    int f = (e + 1) & 3;
                    float ex = P[f][0] - P[e][0], ey = P[f][1] - P[e][1], L2 = ex * ex + ey * ey, L = sqrtf(L2);
                    float dx = x - P[e][0], dy = y - P[e][1];
                    float tt = (dx * ex + dy * ey) / L2, dist = (dx * ey - dy * ex) / L;      // positive on the outside of a clockwise shape? (checked by abs below)
                    if (tt < 0.15f || tt > 0.85f) continue;
                    if (fabsf(dist) > 0.05f * L + 1.2f) continue;
                    sn[e] += 1; sx[e] += x; sy[e] += y; sxx[e] += (double)x * x; sxy[e] += (double)x * y; syy[e] += (double)y * y;
                    break;
                }
            }
        double lc[4][2], ld[4][2]; bool fit = true;
        double bcx = (P[0][0] + P[1][0] + P[2][0] + P[3][0]) * 0.25, bcy = (P[0][1] + P[1][1] + P[2][1] + P[3][1]) * 0.25;
        for (int e = 0; e < 4; e++) {
            if (sn[e] < 5) { fit = false; break; }
            double mx = sx[e] / sn[e], my = sy[e] / sn[e];
            double cxx = sxx[e] / sn[e] - mx * mx, cxy = sxy[e] / sn[e] - mx * my, cyy = syy[e] / sn[e] - my * my;
            double a = 0.5 * atan2(2.0 * cxy, cxx - cyy);
            lc[e][0] = mx; lc[e][1] = my; ld[e][0] = cos(a); ld[e][1] = sin(a);
            {                                                               // the blob's edge pixels sit a little inside the real edge: move the line out
                double nx = -ld[e][1], ny = ld[e][0];
                if (nx * (ARD_BIAS_CX - mx) + ny * (ARD_BIAS_CY - my) > 0) { nx = -nx; ny = -ny; }
                lc[e][0] += nx * ARD_PUSH; lc[e][1] += ny * ARD_PUSH;
            }
        }
        if (fit) {
            float Q[4][2];
            for (int i = 0; i < 4; i++) {                                   // corner i is where side i-1 meets side i
                int a = (i + 3) & 3, b = i;
                double den = ld[a][0] * ld[b][1] - ld[a][1] * ld[b][0];
                if (fabs(den) < 0.2) { fit = false; break; }
                double wx = lc[b][0] - lc[a][0], wy = lc[b][1] - lc[a][1];
                double t = (wx * ld[b][1] - wy * ld[b][0]) / den;
                Q[i][0] = (float)(lc[a][0] + t * ld[a][0]); Q[i][1] = (float)(lc[a][1] + t * ld[a][1]);
                if (fabsf(Q[i][0] - P[i][0]) > 8.0f || fabsf(Q[i][1] - P[i][1]) > 8.0f) { fit = false; break; }
            }
            if (fit) for (int i = 0; i < 4; i++) { P[i][0] = Q[i][0]; P[i][1] = Q[i][1]; }
        }
        // The border really is dark all the way along each side?
        int dark_n = 0, dark_t = 0;
        for (int e = 0; e < 4; e++) {
            int f = (e + 1) & 3;
            float ex = P[f][0] - P[e][0], ey = P[f][1] - P[e][1], L = sqrtf(ex * ex + ey * ey);
            float nx = -ey / L, ny = ex / L;                               // towards the inside of a clockwise shape (y down)
            for (int k = 0; k < 5; k++) {
                float t = 0.2f + 0.15f * k, px = P[e][0] + ex * t + nx * 0.045f * L, py = P[e][1] + ey * t + ny * 0.045f * L;
                dark_t++; if (ard_is_dark(px, py)) dark_n++;
            }
        }
        if (dark_n * 10 < dark_t * 8) continue;
        // The pattern inside: the lowest of the four turns says which corner is "first".
        int best_c = 0x7fffffff, best_s = 0;
        for (int s = 0; s < 4; s++) {
            int c = ard_code(P, s);
            if (c < 0) { best_c = -1; break; }
            if (c < best_c) { best_c = c; best_s = s; }
        }
        if (best_c < 0) continue;                                          // a plain inside: a picture frame or a screen, not a card
        float ar2 = area * 4.0f;
        if (ar2 > best_area) {
            best_area = ar2; found = true;
            for (int i = 0; i < 4; i++) { out->raw[i][0] = P[i][0] * 2.0f + 0.5f; out->raw[i][1] = P[i][1] * 2.0f + 0.5f; }
            out->shift = best_s; out->code = best_c; out->area = ar2;
        }
    }
    return found;
}

// The pose of the card from its four corners (picture pixels, in the order of the card: top left, top right, bottom right, bottom left),
// for a camera of focal length `f` pixels with its centre in the middle of the picture. The card is a unit square lying in the x-y plane
// (x to the right, y down, z away from you), centred on the origin. R is the card-to-camera turn, t the card centre in camera space.
static bool ar_pose(const float c[4][2], float f, float R[3][3], float t[3]) {
    float src[4][2] = { {-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f} }, dst[4][2], h[8];
    for (int i = 0; i < 4; i++) { dst[i][0] = (c[i][0] - ARD_W * 0.5f) / f; dst[i][1] = (c[i][1] - ARD_H * 0.5f) / f; }
    if (!ard_homog(src, dst, h)) return false;
    float h1[3] = { h[0], h[3], h[6] }, h2[3] = { h[1], h[4], h[7] }, h3[3] = { h[2], h[5], 1.0f };
    float l1 = sqrtf(h1[0] * h1[0] + h1[1] * h1[1] + h1[2] * h1[2]), l2 = sqrtf(h2[0] * h2[0] + h2[1] * h2[1] + h2[2] * h2[2]);
    if (l1 < 1e-6f || l2 < 1e-6f) return false;
    float sc = 2.0f / (l1 + l2);
    float r1[3], r2[3], r3[3], tt[3];
    for (int i = 0; i < 3; i++) { r1[i] = h1[i] * sc; r2[i] = h2[i] * sc; tt[i] = h3[i] * sc; }
    if (tt[2] < 0.0f) { for (int i = 0; i < 3; i++) { r1[i] = -r1[i]; r2[i] = -r2[i]; tt[i] = -tt[i]; } }
    float n1 = sqrtf(r1[0] * r1[0] + r1[1] * r1[1] + r1[2] * r1[2]);
    for (int i = 0; i < 3; i++) r1[i] /= n1;
    float d = r1[0] * r2[0] + r1[1] * r2[1] + r1[2] * r2[2];
    for (int i = 0; i < 3; i++) r2[i] -= d * r1[i];
    float n2 = sqrtf(r2[0] * r2[0] + r2[1] * r2[1] + r2[2] * r2[2]);
    if (n2 < 1e-6f) return false;
    for (int i = 0; i < 3; i++) r2[i] /= n2;
    r3[0] = r1[1] * r2[2] - r1[2] * r2[1]; r3[1] = r1[2] * r2[0] - r1[0] * r2[2]; r3[2] = r1[0] * r2[1] - r1[1] * r2[0];
    for (int i = 0; i < 3; i++) { R[i][0] = r1[i]; R[i][1] = r2[i]; R[i][2] = r3[i]; t[i] = tt[i]; }
    return tt[2] > 0.05f;
}
