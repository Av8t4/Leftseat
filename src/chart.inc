// Left Seat: the chart you see while holding Select - both airports, the mountains, towns, river, your track and your position.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

// North is up. The chart is a fixed window of about 51 x 42 km: both airports, the western and southern mountains, everything.
static void draw_chart(vita2d_pgf *pgf) {
    const float PX = 100.0f, PY = 14.0f, PW = 760.0f, PH = 350.0f;               // the card (kept inside the windscreen opening: clear of the posts, roof, sill and glareshield)
    const float MX = 116.0f, MY = 28.0f, MW = 400.0f, MH = 324.0f;               // the map inside it
    const float S = MH / 42100.0f;                                               // pixels per metre
    const float WX0 = -1700.0f, WZ0 = 10050.0f;                                   // world point at the middle of the map
    const unsigned int gold = RGBA8(255, 206, 130, 255), white = RGBA8(255, 255, 255, 255), dim = RGBA8(170, 184, 214, 255);
    #define CH_SX(x) (MX + MW * 0.5f + ((x) - WX0) * S)
    #define CH_SY(z) (MY + MH * 0.5f - ((z) - WZ0) * S)

    vita2d_draw_rectangle(PX, PY, PW, PH, RGBA8(8, 18, 40, 238));
    vita2d_draw_rectangle(MX, MY, MW, MH, RGBA8(40, 74, 44, 255));                // farmland
    {                                                                              // the desert around Redrock (kept inside the map)
        const Airport *b = &AIRPORTS[AP_REDROCK];
        float bx = CH_SX(b->cx), by = CH_SY(b->cz), r = 6500.0f * S;
        P2 o[8];
        for (int i = 0; i < 8; i++) {
            float a = i * PI_F / 4.0f + 0.39f;
            o[i].x = clampf(bx + cosf(a) * r, MX, MX + MW);
            o[i].y = clampf(by + sinf(a) * r, MY, MY + MH);
        }
        for (int i = 0; i < 8; i++) pb_tri((P2){ bx, clampf(by, MY, MY + MH) }, o[i], o[(i + 1) & 7], RGBA8(112, 80, 58, 255));
        pb_flush();
    }
    for (int n = 0; n < NLAKES; n++) {                                                // lakes
        float lx = CH_SX(LAKES[n].x), ly = CH_SY(LAKES[n].z);
        if (ly > MY && ly < MY + MH && lx > MX && lx < MX + MW) vita2d_draw_fill_circle(lx, ly, 3.0f + LAKES[n].rx * S, RGBA8(52, 108, 172, 255));
    }
    for (int i = 0; i + 1 < nriver; i++) {                                         // river
        float x0 = CH_SX(river_x[i]), y0 = CH_SY(river_z[i]), x1 = CH_SX(river_x[i + 1]), y1 = CH_SY(river_z[i + 1]);
        if (y0 > MY && y0 < MY + MH && y1 > MY && y1 < MY + MH && x0 > MX && x0 < MX + MW && x1 > MX && x1 < MX + MW)
            pb_line(x0, y0, x1, y1, 2.0f, RGBA8(70, 130, 200, 255));
    }
    for (int i = 0; i + 1 < nroadp; i++) {                                         // country roads
        if (road_new[i + 1]) continue;
        float x0 = CH_SX(road_x[i]), y0 = CH_SY(road_z[i]), x1 = CH_SX(road_x[i + 1]), y1 = CH_SY(road_z[i + 1]);
        if (y0 > MY && y0 < MY + MH && y1 > MY && y1 < MY + MH && x0 > MX && x0 < MX + MW && x1 > MX && x1 < MX + MW)
            pb_line(x0, y0, x1, y1, 1.2f, road_dirt[i] ? RGBA8(176, 146, 100, 255) : RGBA8(168, 168, 174, 255));
    }
    {                                                                              // highway
        V3 e = ap_pt(&AIRPORTS[AP_REDROCK], 330.0f, 0.0f, -220.0f);
        pb_line(CH_SX(704.0f), CH_SY(-300.0f), CH_SX(e.x), CH_SY(e.z), 1.5f, RGBA8(150, 150, 156, 255));
    }
    for (int i = 0; i < nmounts; i++) {                                            // mountains and hills
        float x = CH_SX(mounts[i].x), y = CH_SY(mounts[i].z);
        if (y < MY + 6 || y > MY + MH - 2 || x < MX + 5 || x > MX + MW - 5) continue;
        bool hill = mounts[i].h < 220.0f;
        float s = hill ? 3.5f : 5.0f + mounts[i].hb * S * 0.45f;
        unsigned int c = hill ? RGBA8(70, 106, 58, 255) : RGBA8(150, 146, 140, 255);
        pb_tri((P2){ x - s, y + s * 0.7f }, (P2){ x + s, y + s * 0.7f }, (P2){ x, y - s }, c);
    }
    for (int i = 0; i < nmesas; i++) {
        float x = CH_SX(mesas[i].x), y = CH_SY(mesas[i].z);
        if (y > MY + 4 && y < MY + MH - 4 && x > MX + 5 && x < MX + MW - 5) pb_quad((P2){ x - 4, y - 3 }, (P2){ x + 4, y - 3 }, (P2){ x + 4, y + 3 }, (P2){ x - 4, y + 3 }, RGBA8(190, 100, 66, 255));
    }
    for (int i = 0; i < ntowns; i++) {
        float x = CH_SX(town_x[i]), y = CH_SY(town_z[i]);
        pb_quad((P2){ x - 3, y - 3 }, (P2){ x + 3, y - 3 }, (P2){ x + 3, y + 3 }, (P2){ x - 3, y + 3 }, RGBA8(240, 214, 120, 255));
    }
    for (int i = 0; i < ntrk; i++) {                                               // where you have been
        float x = CH_SX(trk_x[i]), y = CH_SY(trk_z[i]);
        if (x > MX && x < MX + MW && y > MY && y < MY + MH)
            pb_quad((P2){ x - 1, y - 1 }, (P2){ x + 1, y - 1 }, (P2){ x + 1, y + 1 }, (P2){ x - 1, y + 1 }, RGBA8(120, 220, 230, 255));
    }
    for (int k = 0; k < AP_COUNT; k++) {                                           // runways
        V3 a = ap_pt(&AIRPORTS[k], 0.0f, 0.0f, -AIRPORTS[k].hl), b = ap_pt(&AIRPORTS[k], 0.0f, 0.0f, AIRPORTS[k].hl);
        pb_line(CH_SX(a.x), CH_SY(a.z), CH_SX(b.x), CH_SY(b.z), 3.0f, white);
    }
    if (game_mode == MODE_LANDING && ch_trip()) {                                  // straight line to where you are going
        float x0 = CH_SX(pl.pos.x), y0 = CH_SY(pl.pos.z), x1 = CH_SX(AIRPORTS[ch_dest_ap()].cx), y1 = CH_SY(AIRPORTS[ch_dest_ap()].cz);
        float dx = x1 - x0, dy = y1 - y0, len = sqrtf(dx * dx + dy * dy);
        for (float t = 0.0f; t + 5.0f < len; t += 11.0f)
            pb_line(x0 + dx / len * t, y0 + dy / len * t, x0 + dx / len * (t + 5.0f), y0 + dy / len * (t + 5.0f), 1.5f, RGBA8(255, 120, 220, 255));
    }
    {                                                                              // you
        float x = CH_SX(pl.pos.x), y = CH_SY(pl.pos.z);
        x = clampf(x, MX + 6.0f, MX + MW - 6.0f); y = clampf(y, MY + 8.0f, MY + MH - 6.0f);
        float h = pl.hdg, sx = sinf(h), sy = -cosf(h);                              // screen direction of the nose (north is up)
        pb_tri((P2){ x + sx * 11.0f, y + sy * 11.0f }, (P2){ x - sx * 6.0f - sy * 6.0f, y - sy * 6.0f + sx * 6.0f },
               (P2){ x - sx * 6.0f + sy * 6.0f, y - sy * 6.0f - sx * 6.0f }, RGBA8(255, 220, 60, 255));
    }
    pb_flush();
    vita2d_draw_rectangle(MX, MY, MW, 1, gold); vita2d_draw_rectangle(MX, MY + MH - 1, MW, 1, gold);   // frame round the map
    vita2d_draw_rectangle(MX, MY, 1, MH, gold); vita2d_draw_rectangle(MX + MW - 1, MY, 1, MH, gold);
    for (int k = 0; k < AP_COUNT; k++) {                                           // labels
        float x = CH_SX(AIRPORTS[k].cx), y = CH_SY(AIRPORTS[k].cz);
        vita2d_pgf_draw_text(pgf, (int)x + 8, (int)y - 6, white, 0.8f, AIRPORTS[k].id);
    }
    vita2d_pgf_draw_text(pgf, MX + 8, MY + 26, dim, 0.7f, "N");
    vita2d_draw_rectangle(MX + 12, MY + MH - 14, 10000.0f * S, 2, white);          // 10 km scale
    vita2d_pgf_draw_text(pgf, MX + 18 + (int)(10000.0f * S), MY + MH - 8, white, 0.7f, "10 KM");

    // The side column.
    float gs = sqrtf(pl.vel.x * pl.vel.x + pl.vel.z * pl.vel.z);
    float x0 = MX + MW + 30.0f;
    vita2d_pgf_draw_text(pgf, x0, 52, gold, 1.2f, "CHART");
    vita2d_pgf_draw_textf(pgf, x0, 82, white, 0.8f, "HDG %03d    ALT %d FT    GS %d KT", ((int)(pl.hdg * 57.2958f + 0.5f)) % 360,
                          (int)(fmaxf(0.0f, pl.pos.y) * 3.281f), (int)(gs * 1.944f));
    for (int k = AP_COUNT - 1; k >= 0; k--) {
        float brg, dist;
        ap_bearing(&AIRPORTS[k], &brg, &dist);
        float y = 120.0f + (AP_COUNT - 1 - k) * 74.0f;
        vita2d_pgf_draw_textf(pgf, x0, y, gold, 0.95f, "%s  %s", AIRPORTS[k].id, AIRPORTS[k].name);
        vita2d_pgf_draw_textf(pgf, x0, y + 24, white, 0.85f, "BEARING %03d    %.1f KM  (%.1f NM)", (int)(brg + 0.5f) % 360, dist / 1000.0f, dist / 1852.0f);
        if (gs > 8.0f) vita2d_pgf_draw_textf(pgf, x0, y + 46, dim, 0.8f, "%d:%02d to go at this speed", (int)(dist / gs / 60.0f), (int)(dist / gs) % 60);
        else           vita2d_pgf_draw_textf(pgf, x0, y + 46, dim, 0.8f, "Runway %02d / %02d", ((int)(AIRPORTS[k].hdg * 57.2958f / 10.0f + 0.5f) + 36) % 36 == 0 ? 36 : ((int)(AIRPORTS[k].hdg * 57.2958f / 10.0f + 0.5f) + 36) % 36, ((int)(AIRPORTS[k].hdg * 57.2958f / 10.0f + 0.5f) + 18) % 36 == 0 ? 36 : ((int)(AIRPORTS[k].hdg * 57.2958f / 10.0f + 0.5f) + 18) % 36);
    }
    vita2d_pgf_draw_text(pgf, x0, 346, dim, 0.8f, "Release SELECT to close");
    #undef CH_SX
    #undef CH_SY
}

