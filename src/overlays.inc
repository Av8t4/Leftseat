// Left Seat: on-screen messages and warnings.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

static void draw_overlays(vita2d_pgf *pgf) {
    unsigned int white = RGBA8(255, 255, 255, 255);

    if (view_banner > 0) {
        static const char *names[3] = { "COCKPIT VIEW", "CHASE VIEW", "TOWER VIEW" };
        vita2d_pgf_draw_text(pgf, 20, 388, white, 0.95f, names[view_mode]);
    }
    if (game_mode == MODE_LANDING && (ch_state == CH_FLYING || ch_state == CH_TOUCHED)) {
        unsigned int gold = RGBA8(255, 206, 130, 255);
        float to_thr = (-1000.0f - pl.pos.z) / 1000.0f;
        if (ch_trip()) {
            float brg, dist;
            ap_bearing(&AIRPORTS[ch_dest_ap()], &brg, &dist);
            char place[24];
            snprintf(place, sizeof(place), "%s", AIRPORTS[ch_dest_ap()].place);
            for (char *c = place; *c; c++) if (*c >= 'a' && *c <= 'z') *c -= 32;
            vita2d_pgf_draw_textf(pgf, 480, 28, gold, 0.8f, "%s    %s %03d   %.1f KM", CK_NAMES[ch_kind], place, (int)(brg + 0.5f) % 360, dist / 1000.0f);
        }
        else if (to_thr > 0.0f) vita2d_pgf_draw_textf(pgf, 540, 28, gold, 0.8f, "%s CHALLENGE    RUNWAY %.1f KM", CK_NAMES[ch_kind], to_thr);
        else                    vita2d_pgf_draw_textf(pgf, 540, 28, gold, 0.8f, "%s CHALLENGE", CK_NAMES[ch_kind]);
        if (ch_state == CH_FLYING && ch_t < (ch_trip() ? 14.0f : 10.0f)) {
            static const char *const L1[CK_COUNT] = {
                "Land smoothly near the aiming-point markings, on the centre line.",
                "The engine has failed! Hold about 70 kt and glide to the runway.",
                "Strong crosswind: point the nose into the wind to track the runway.",
                "Take off and fly to Redrock Municipal, about 29 km to the north-north-east.",
                "Take off and fly west-northwest to Pinecrest, a short grass strip 21 km away." };
            static const char *const L2[CK_COUNT] = {
                "Use the glide-slope lights beside the runway. Triangle restarts.",
                "Keep the flaps for the last part of the approach. Triangle restarts.",
                "Straighten up with rudder just before touchdown. Triangle restarts.",
                "Hold SELECT to see the chart. Land on the Redrock runway. Triangle restarts.",
                "Hold SELECT for the chart. The strip is short: touch down near the start. Triangle restarts." };
            vita2d_pgf_draw_text(pgf, 20, 60, white, 0.85f, L1[ch_kind]);
            vita2d_pgf_draw_text(pgf, 20, 86, white, 0.85f, L2[ch_kind]);
        }
        if (engine_failed && ((frame_count / 20) & 1))
            vita2d_pgf_draw_text(pgf, 360, 140, RGBA8(255, 90, 70, 255), 1.3f, "ENGINE OUT");
    }
    if (ff_level > 0 && !pl.crashed) {                                    // top right, clear of the instrument panel
        const char *ff = ff_level == 1 ? ">> FAST FORWARD x2" : ">> FAST FORWARD x5";
        int fw = vita2d_pgf_text_width(pgf, 1.0f, ff);
        vita2d_draw_rectangle(SCREEN_W - fw - 32, 8, fw + 24, 34, RGBA8(0, 0, 0, 120));
        vita2d_pgf_draw_text(pgf, SCREEN_W - fw - 20, 33, RGBA8(255, 210, 90, 255), 1.0f, ff);
    }
    if (spec->retract && !pl.crashed && gear_msg_t > 0) {                 // gear refusals ("too fast" etc.): top right
        int mw = vita2d_pgf_text_width(pgf, 0.8f, gear_msg);
        vita2d_pgf_draw_text(pgf, SCREEN_W - mw - 20, 68, RGBA8(255, 150, 60, 255), 0.8f, gear_msg);
    }
    if (pl.crashed) {
        vita2d_pgf_draw_text(pgf, 330, 170, RGBA8(255, 70, 60, 255), 2.2f, "CRASHED");
        vita2d_pgf_draw_text(pgf, 330, 215, white, 1.1f, "Press TRIANGLE to restart");
        return;
    }
    if (!pl.on_ground && pl.alpha > ALPHA_S - 0.04f && ((frame_count / 15) & 1))
        vita2d_pgf_draw_text(pgf, 400, 120, RGBA8(255, 70, 60, 255), 1.6f, "STALL");
    if (!pl.on_ground && pl.speed > VNE_MS && ((frame_count / 10) & 1))
        vita2d_pgf_draw_text(pgf, 360, 160, RGBA8(255, 70, 60, 255), 1.4f, "OVERSPEED");
    if (pl.fuel <= 0.0f && ((frame_count / 30) & 1))
        vita2d_pgf_draw_text(pgf, 350, 200, RGBA8(255, 150, 60, 255), 1.2f, "OUT OF FUEL");

    if (pl.on_ground && pl.gs < 1.0f) {
        vita2d_pgf_draw_text(pgf, 20, 30,  white, 0.85f, "Left stick: pitch / roll    Right stick: throttle (up/down), rudder (left/right)    D-pad left / right: trim");
        vita2d_pgf_draw_text(pgf, 20, 56,  white, 0.85f, "Circle / Cross: flaps down / up    D-pad down: brakes    D-pad up: fast forward (free flight)    L / R: look left / right    Triangle: reset");
        vita2d_pgf_draw_text(pgf, 20, 82,  white, 0.85f, "Select: change camera (cockpit / chase)    L + R + Select: invert pitch    START: menu");
        if (spec->retract) vita2d_pgf_draw_text(pgf, 20, 108, white, 0.85f, "Square: landing gear up / down (raise it once you are climbing, lower it before you land).");
        else               vita2d_pgf_draw_text(pgf, 20, 108, white, 0.85f, "Takeoff: flaps 10, full throttle, hold right rudder, rotate at 55 kt, climb at 74 kt.");
    }
}

/* ------------------------------------------------------------------ */

