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
        if (ch_rings()) {
            V3 rp = ring_pos(ch_ring_i < NRINGS ? ch_ring_i : NRINGS - 1);
            float dx = rp.x - pl.pos.x, dz = rp.z - pl.pos.z, b = atan2f(dx, dz) * 57.2958f;
            vita2d_pgf_draw_textf(pgf, 380, 28, gold, 0.8f, "%s    RING %d / %d   HIT %d   NEXT %03d  %.1f NM", CK_NAMES[ch_kind], ch_ring_i + 1 > NRINGS ? NRINGS : ch_ring_i + 1,
                                  NRINGS, ch_ring_hit, (int)(b < 0.0f ? b + 360.0f : b + 0.5f) % 360, sqrtf(dx * dx + dz * dz) / 1852.0f);
        }
        else if (ch_trip()) {
            float brg, dist;
            ap_bearing(&AIRPORTS[ch_dest_ap()], &brg, &dist);
            char place[24];
            snprintf(place, sizeof(place), "%s", AIRPORTS[ch_dest_ap()].place);
            for (char *c = place; *c; c++) if (*c >= 'a' && *c <= 'z') *c -= 32;
            vita2d_pgf_draw_textf(pgf, 480, 28, gold, 0.8f, "%s    %s %03d   %.1f NM", CK_NAMES[ch_kind], place, (int)(brg + 0.5f) % 360, dist / 1852.0f);
        }
        else if (to_thr > 0.0f) vita2d_pgf_draw_textf(pgf, 540, 28, gold, 0.8f, "%s CHALLENGE    RUNWAY %.1f NM", CK_NAMES[ch_kind], to_thr * (1000.0f / 1852.0f));
        else                    vita2d_pgf_draw_textf(pgf, 540, 28, gold, 0.8f, "%s CHALLENGE", CK_NAMES[ch_kind]);
        if (ch_state == CH_FLYING && ch_t < (ch_trip() ? 14.0f : 10.0f)) {
            static const char *const L1[CK_COUNT] = {
                "Land smoothly near the aiming-point markings, on the centre line.",
                "The engine has failed! Hold about 70 kt and glide to the runway.",
                "Strong crosswind: point the nose into the wind to track the runway.",
                "Take off and fly to Redrock Municipal, about 16 NM to the north-north-east.",
                "Take off and fly west-northwest to Pinecrest, a short grass strip 11 NM away.",
                "Fly through the gold rings over Lana Lake, one after another." };
            static const char *const L2[CK_COUNT] = {
                "Use the glide-slope lights beside the runway. Triangle restarts.",
                "Keep the flaps for the last part of the approach. Triangle restarts.",
                "Straighten up with rudder just before touchdown. Triangle restarts.",
                "Hold SELECT to see the chart. Land on the Redrock runway. Triangle restarts.",
                "Hold SELECT for the chart. The strip is short: touch down near the start. Triangle restarts.",
                "A ring you miss is skipped. Fly low and smooth. Triangle restarts." };
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
    if (!pl.crashed && trim_msg_t > 0) {                                 // D-pad left + right: "trimmed for this attitude"
        int tw = vita2d_pgf_text_width(pgf, 0.8f, "TRIMMED");
        vita2d_pgf_draw_text(pgf, (SCREEN_W - tw) / 2, 68, RGBA8(230, 190, 60, 255), 0.8f, "TRIMMED");
    }
    draw_shot_msg(pgf);
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

}

/* ------------------------------------------------------------------ */

