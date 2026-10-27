// Left Seat: the AR view in the hangar. The Vita's back camera is the background, a card with a thick black square border (one of the
// Vita's own AR cards, or the printable one in docs/) is found in the picture, and the chosen aircraft is drawn sitting on it.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order (after menu.inc).

static bool         ar_on;                    // the AR view is showing (it takes over the hangar screen)
static bool         ar_cam_ok;                // the camera is open and running
static vita2d_texture *ar_tex;                // the camera picture (the camera writes straight into this texture)
static int          ar_ac;                    // aircraft being shown
static float        ar_ang;                   // how far the aircraft is turned on the card (radians)
static float        ar_size = 1.2f;           // how big the aircraft is compared with the card (L / R buttons)
static int          ar_fovi = 1;              // camera field of view setting (Triangle)
static int          ar_bad_reads;             // camera reads that failed in a row
static int          ar_frames, ar_lastframe = -1;
static int          ar_lost = 999;            // detections missed in a row (999 = never seen)
static bool         ar_have_c;                // the smoothed corners are valid
static float        ar_c[4][2];               // the card's corners (picture pixels, in the card's own order), smoothed
static float        ar_R[3][3], ar_t[3];      // the smoothed pose of the card
static bool         ar_have_pose;
static float        ar_card_px;               // how big the card looks (square root of its area, in picture pixels)
static int          ar_code = -1;
static int          ar_det_us;                // time the last detection took
static const char  *ar_msg = "";

static const float AR_FOV_DEG[4] = { 50.0f, 60.0f, 70.0f, 80.0f };           // horizontal field of view of the camera
static float ar_focal(void) { return (ARD_W * 0.5f) / tanf(AR_FOV_DEG[ar_fovi] * 0.5f * PI_F / 180.0f); }   // pixels of the 480 x 272 picture

static void ar_close(void) {
    if (ar_cam_ok) { sceCameraStop(1); sceCameraClose(1); ar_cam_ok = false; }
    if (ar_tex) { vita2d_wait_rendering_done(); vita2d_free_texture(ar_tex); ar_tex = NULL; }
    ar_on = false;
}

static void ar_open(int ac) {
    ar_ac = ac; ar_on = true; ar_lost = 999; ar_have_c = false; ar_have_pose = false; ar_code = -1;
    ar_lastframe = -1; ar_frames = 0; ar_bad_reads = 0; ar_msg = "";
    ar_tex = vita2d_create_empty_texture(ARD_W, ARD_H);
    if (!ar_tex) { ar_msg = "No memory for the camera picture"; return; }
    memset(vita2d_texture_get_datap(ar_tex), 0, vita2d_texture_get_stride(ar_tex) * ARD_H);
    int stride = (int)vita2d_texture_get_stride(ar_tex);
    SceCameraInfo ci;
    memset(&ci, 0, sizeof ci);
    ci.size = sizeof ci;
    ci.priority = SCE_CAMERA_PRIORITY_SHARE;
    ci.format = SCE_CAMERA_FORMAT_ABGR;
    ci.resolution = SCE_CAMERA_RESOLUTION_480_272;
    ci.framerate = SCE_CAMERA_FRAMERATE_30_FPS;
    ci.sizeIBase = 4 * ARD_W * ARD_H;
    ci.pitch = stride - 4 * ARD_W;
    ci.pIBase = vita2d_texture_get_datap(ar_tex);
    int r = sceCameraOpen(1, &ci);
    if (r < 0) { ar_msg = "The camera could not be opened"; return; }
    r = sceCameraStart(1);
    if (r < 0) { sceCameraClose(1); ar_msg = "The camera could not be started"; return; }
    ar_cam_ok = true;
}

// Turn the smoothed pose into the game's camera: the card is the ground, the aircraft stands on its middle.
// Card space: x right, y down, z away from you (into the table). Game space: x right, y up, z forward (nose along the card's top edge).
// A card side is `side` metres of aircraft.
static void ar_set_cam(float side) {
    float R[3][3], t[3];
    memcpy(R, ar_R, sizeof R); memcpy(t, ar_t, sizeof t);
    float p[3];                                                        // the camera in card space = -R^T t
    for (int j = 0; j < 3; j++) p[j] = -(R[0][j] * t[0] + R[1][j] * t[1] + R[2][j] * t[2]);
    cam.pos = v3(p[0] * side, -p[2] * side, -p[1] * side);
    cam.r = v3(R[0][0], -R[0][2], -R[0][1]);                           // picture right
    cam.u = v3(-R[1][0], R[1][2], R[1][1]);                            // picture up
    cam.f = v3(R[2][0], -R[2][2], -R[2][1]);                           // into the picture
}

// One camera frame: smooth the corners, then the pose.
static void ar_track(const ArHit *h) {
    float n[4][2];
    int sh = h->shift;
    if (ar_have_c && ar_lost < 25) {                                   // keep the corner numbering the tracker already has (steadier than the pattern)
        float best = 1e30f;
        for (int s = 0; s < 4; s++) {
            float e = 0.0f;
            for (int i = 0; i < 4; i++) { float dx = h->raw[(i + s) & 3][0] - ar_c[i][0], dy = h->raw[(i + s) & 3][1] - ar_c[i][1]; e += dx * dx + dy * dy; }
            if (e < best) { best = e; sh = s; }
        }
    }
    for (int i = 0; i < 4; i++) { n[i][0] = h->raw[(i + sh) & 3][0]; n[i][1] = h->raw[(i + sh) & 3][1]; }
    float px = sqrtf(h->area);
    ar_card_px = px;
    float a = px > 110.0f ? 0.75f : (px > 70.0f ? 0.55f : 0.40f);     // a small card is noisier: trust each frame less
    if (!ar_have_c || ar_lost >= 25) { memcpy(ar_c, n, sizeof n); }
    else {
        for (int i = 0; i < 4; i++) {
            float dx = n[i][0] - ar_c[i][0], dy = n[i][1] - ar_c[i][1];
            float aa = (dx * dx + dy * dy > 30.0f * 30.0f) ? 1.0f : a;      // a quick move: follow at once
            ar_c[i][0] += dx * aa; ar_c[i][1] += dy * aa;
        }
    }
    ar_have_c = true; ar_lost = 0; ar_code = h->code;
    float R[3][3], t[3];
    if (!ar_pose(ar_c, ar_focal(), R, t)) return;
    if (!ar_have_pose) { memcpy(ar_R, R, sizeof R); memcpy(ar_t, t, sizeof t); ar_have_pose = true; return; }
    float k = px > 110.0f ? 0.7f : (px > 70.0f ? 0.45f : 0.30f);
    for (int i = 0; i < 3; i++) { for (int j = 0; j < 3; j++) ar_R[i][j] += (R[i][j] - ar_R[i][j]) * k; ar_t[i] += (t[i] - ar_t[i]) * 0.6f; }
    float *c0 = &ar_R[0][0];                                           // tidy the turn (Gram-Schmidt on the columns)
    float c1v[3] = { ar_R[0][0], ar_R[1][0], ar_R[2][0] }, c2v[3] = { ar_R[0][1], ar_R[1][1], ar_R[2][1] };
    (void)c0;
    float l = sqrtf(c1v[0] * c1v[0] + c1v[1] * c1v[1] + c1v[2] * c1v[2]);
    for (int i = 0; i < 3; i++) c1v[i] /= l;
    float d = c1v[0] * c2v[0] + c1v[1] * c2v[1] + c1v[2] * c2v[2];
    for (int i = 0; i < 3; i++) c2v[i] -= d * c1v[i];
    l = sqrtf(c2v[0] * c2v[0] + c2v[1] * c2v[1] + c2v[2] * c2v[2]);
    for (int i = 0; i < 3; i++) c2v[i] /= l;
    float c3v[3] = { c1v[1] * c2v[2] - c1v[2] * c2v[1], c1v[2] * c2v[0] - c1v[0] * c2v[2], c1v[0] * c2v[1] - c1v[1] * c2v[0] };
    for (int i = 0; i < 3; i++) { ar_R[i][0] = c1v[i]; ar_R[i][1] = c2v[i]; ar_R[i][2] = c3v[i]; }
}

static void ar_step(const SceCtrlData *pad, unsigned int pressed) {
    if (pressed & SCE_CTRL_CIRCLE) { ar_close(); snd_event(EV_CLICK, 1.0f); return; }
    if (pressed & SCE_CTRL_UP)   { ar_ac = (ar_ac + AC_COUNT - 1) % AC_COUNT; menu_hsel = ar_ac; snd_event(EV_CLICK, 1.0f); }
    if (pressed & SCE_CTRL_DOWN) { ar_ac = (ar_ac + 1) % AC_COUNT; menu_hsel = ar_ac; snd_event(EV_CLICK, 1.0f); }
    if (pressed & SCE_CTRL_CROSS) { menu_ac = ar_ac; menu_hsel = ar_ac; snd_event(EV_CONFIRM, 1.0f); }
    if (pressed & SCE_CTRL_TRIANGLE) { ar_fovi = (ar_fovi + 1) & 3; ar_have_pose = false; snd_event(EV_CLICK, 1.0f); }
    if (pad->buttons & SCE_CTRL_LEFT)  ar_ang -= 0.04f;
    if (pad->buttons & SCE_CTRL_RIGHT) ar_ang += 0.04f;
    if (pad->buttons & SCE_CTRL_LTRIGGER) ar_size = fmaxf(0.4f, ar_size - 0.012f);
    if (pad->buttons & SCE_CTRL_RTRIGGER) ar_size = fminf(2.5f, ar_size + 0.012f);
    if (!ar_cam_ok) return;
    SceCameraRead rd;
    memset(&rd, 0, sizeof rd);
    rd.size = sizeof rd;
    rd.mode = 0;                                                       // wait for the next picture (the camera makes 30 a second)
    int r = sceCameraRead(1, &rd);
    if (r < 0) { if (++ar_bad_reads > 90) ar_msg = "The camera is not giving pictures"; return; }
    ar_bad_reads = 0;
    if (rd.frame == ar_lastframe) return;
    ar_lastframe = rd.frame; ar_frames++;
    unsigned long long t0 = sceKernelGetProcessTimeWide();
    ArHit h;
    bool got = ar_detect((const uint8_t *)vita2d_texture_get_datap(ar_tex), (int)vita2d_texture_get_stride(ar_tex), &h);
    ar_det_us = (int)(sceKernelGetProcessTimeWide() - t0);
    if (got) ar_track(&h);
    else if (ar_lost < 999) ar_lost++;
}

static void ar_draw(vita2d_pgf *pgf) {
    const unsigned int white = RGBA8(255, 255, 255, 255), gold = RGBA8(255, 206, 130, 255), pale = RGBA8(190, 205, 235, 255);
    if (ar_tex) vita2d_draw_texture_scale(ar_tex, 0.0f, CY - ARD_H, 2.0f, 2.0f);              // picture centre on the 3D window's centre
    else vita2d_draw_rectangle(0, 0, SCREEN_W, VIEW_H, RGBA8(20, 24, 34, 255));
    bool show = ar_have_pose && ar_lost < 30;
    if (show) {                                                                              // the aircraft on the card
        Plane sv_pl = pl; Camera sv_cam = cam; const Spec *sv_spec = spec;
        int sv_ac = ac_flying, sv_tod = tod; float sv_f = g_focal, sv_n = g_near, sv_vis = fog_vis, sv_ph = prop_phase;
        spec = &SPECS[ar_ac]; ac_flying = ar_ac; tod = TOD_DAY; fog_vis = 35000.0f;
        memset(&pl, 0, sizeof pl);
        pl.pos = v3(0.0f, 0.0f, 0.0f); pl.gear = 1.0f; pl.gear_cmd = true; pl.on_ground = true; pl.rpm = 700.0f;
        set_axes(ar_ang, 0.0f, 0.0f);
        g_focal = ar_focal() * 2.0f; g_near = 0.3f;                                          // the screen is twice as wide as the picture
        float rm = fmaxf(spec->tip_x, fmaxf(-spec->tail_z, 4.5f));
        ar_set_cam(2.0f * rm / ar_size);
        hangar_draw_model();
        pl = sv_pl; cam = sv_cam; spec = sv_spec; ac_flying = sv_ac; tod = sv_tod; g_focal = sv_f; g_near = sv_n; fog_vis = sv_vis; prop_phase = sv_ph;
    }
    if (ar_have_c && ar_lost < 3) {                                                          // the card outline, so you can see what it found
        float sx = 2.0f, oy = CY - ARD_H;
        for (int i = 0; i < 4; i++) {
            int j = (i + 1) & 3;
            vita2d_draw_line(ar_c[i][0] * sx, ar_c[i][1] * sx + oy, ar_c[j][0] * sx, ar_c[j][1] * sx + oy, i == 0 ? RGBA8(255, 90, 80, 255) : RGBA8(90, 255, 120, 255));
        }
    }
    // The panel under the picture.
    vita2d_draw_rectangle(0, VIEW_H, SCREEN_W, SCREEN_H - VIEW_H, RGBA8(10, 22, 56, 255));
    vita2d_draw_rectangle(0, VIEW_H, SCREEN_W, 2, gold);
    vita2d_pgf_draw_text(pgf, 24, VIEW_H + 34, white, 1.2f, SPECS[ar_ac].name);
    const char *st;
    if (ar_msg[0]) st = ar_msg;
    else if (!ar_cam_ok) st = "No camera";
    else if (ar_lost >= 999) st = "Point the back camera at a card (flat on the desk, whole black border in view)";
    else if (ar_lost >= 30) st = "Card lost: bring it back into view";
    else st = "Tracking the card";
    vita2d_pgf_draw_text(pgf, 24, VIEW_H + 62, ar_msg[0] || !ar_cam_ok || ar_lost >= 30 ? RGBA8(255, 150, 120, 255) : RGBA8(140, 255, 170, 255), 0.85f, st);
    vita2d_pgf_draw_textf(pgf, 24, VIEW_H + 90, pale, 0.7f, "Card size %d px   camera view %d deg   detect %.1f ms   aircraft size %d%%",
                          (int)ar_card_px, (int)AR_FOV_DEG[ar_fovi], ar_det_us / 1000.0f, (int)(ar_size * 100.0f + 0.5f));
    vita2d_pgf_draw_text(pgf, 24, VIEW_H + 116, pale, 0.7f, "Up / Down: aircraft    Left / Right: turn it    L / R: size    Triangle: camera view");
    vita2d_pgf_draw_text(pgf, 24, VIEW_H + 138, pale, 0.7f, "X: choose this aircraft for flying    O: back to the hangar");
}
