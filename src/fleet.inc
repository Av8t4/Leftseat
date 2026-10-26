// Left Seat: which 3D model is drawn for the aircraft being flown.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

// Which model to draw for the aircraft being flown.
static void build_plane(void) {
    switch (spec->model) { case 1: build_k10(); break; case 2: build_jet1(); break; case 3: build_k30(); break; case 4: build_a22(); break; default: build_k4(); }
}
static void build_cockpit_parts(void) {
    switch (spec->model) { case 1: build_cockpit_parts_k10(); break; case 2: build_cockpit_parts_jet1(); break; case 3: build_cockpit_parts_k30(); break; case 4: build_cockpit_parts_a22(); break; default: build_cockpit_parts_k4(); }
}
static void build_cockpit_nose(void) {
    switch (spec->model) { case 1: build_cockpit_nose_k10(); break; case 2: build_cockpit_nose_jet1(); break; case 3: build_cockpit_nose_k30(); break; case 4: build_cockpit_nose_a22(); break; default: build_cockpit_nose_k4(); }
}
static void draw_plane_shadow(void) {
    switch (spec->model) { case 1: draw_plane_shadow_k10(); break; case 2: draw_plane_shadow_jet1(); break; case 3: draw_plane_shadow_k30(); break; case 4: draw_plane_shadow_a22(); break; default: draw_plane_shadow_k4(); }
}
