#include "game_internal.h"
#include <lcom/lcf.h>
#include <stdlib.h>
#include <string.h>
#include "serial.h"
#include "rtc.h"
#include "mouse.h"
void game_handle_key(game_t *g, uint8_t sc) {
    if (g->state == STATE_QUIZ) {
        const quiz_entry_t *qe = current_question(g);
        if (qe->type == QUIZ_TYPE_DRAW) {
            /* Escape clears the canvas during the drawing step */
            if (sc == 0x01) {
                memset(g->draw_canvas, 0, sizeof(g->draw_canvas));
                g->draw_recognized = -1;
                g->draw_submitted  = false;
            }
            return;
        }
        /* Number keys 1-4 select an MCQ answer */
        int sel = -1;
        switch (sc) {
            case 0x02: sel = 0; break;
            case 0x03: sel = 1; break;
            case 0x04: sel = 2; break;
            case 0x05: sel = 3; break;
        }
        if (sel >= 0 && g->quiz_feedback == 0) {
            g->quiz_last_selected = sel;
            if (sel == qe->correct) {
                g->quiz_feedback       = 1;
                g->quiz_feedback_ticks = 50;
            } else {
                g->quiz_feedback       = 2;
                g->quiz_feedback_ticks = 50;
                g->quiz_lives--;
            }
        }
        return;
    }

    if (g->state == STATE_GUIDE) {
        if      (sc == 0x48 || sc == 0x11) { if (g->menu_selected > 0) g->menu_selected--; }
        else if (sc == 0x50 || sc == 0x1F) { if (g->menu_selected < 1) g->menu_selected++; }
        else if (sc == 0x1C) {
            if (g->menu_selected == 0) { game_init(g); g->state = STATE_PLAYING; }
            else { g->state = STATE_MENU; g->menu_selected = 0; }
        } else if (sc == 0x01) { g->state = STATE_MENU; g->menu_selected = 0; }
        return;
    }

    if (g->state == STATE_LEADERBOARD) {
        if (sc == 0x01 || sc == 0x1C) g->state = STATE_MENU;
        return;
    }

	if (g->state == STATE_CONTROLS) {
		if (sc == 0x01 || sc == 0x1C) g->state = g->prev_state;
		return;
	}

    if (g->state == STATE_MENU) {
        bool has_progress = (g->elapsed_ticks > 0 || g->ticks > 0);
        if      (sc == 0x48 || sc == 0x11) { if (g->menu_selected > 0) g->menu_selected--; }
        else if (sc == 0x50 || sc == 0x1F) { if (g->menu_selected < 5) g->menu_selected++; }
        else if (sc == 0x1C) {
            switch (g->menu_selected) {
                case 0: game_init(g); g->state = STATE_PLAYING; break;
                case 1: if (has_progress) g->state = STATE_PLAYING; break;
                case 2: g->state = STATE_LEADERBOARD; g->menu_selected = 0; break;
                case 3: g->prev_state = STATE_MENU; g->state = STATE_CONTROLS; g->menu_selected = 0; break;
				case 4: g->state = STATE_GUIDE; g->menu_selected = 0; break;
                case 5: request_quit = true; break;
            }
        }
        return;
    }

    if (g->state == STATE_WIN || g->state == STATE_LOSE) {
        if (sc == 0x1C || sc == 0x13) game_init(g);
        return;
    }

    if (g->state == STATE_PAUSED) {
        if      (sc == 0x48 || sc == 0x11) { if (g->menu_selected > 0) g->menu_selected--; }
        else if (sc == 0x50 || sc == 0x1F) { if (g->menu_selected < 3) g->menu_selected++; }
        else if (sc == 0x1C) {
            switch (g->menu_selected) {
                case 0: g->state = STATE_PLAYING; break;
                case 1: game_init(g); g->state = STATE_PLAYING; break;
				case 2: g->prev_state = STATE_PAUSED; g->state = STATE_CONTROLS; g->menu_selected = 0; break;
                case 3: g->state = STATE_MENU; g->menu_selected = 0; break;
            }
        }
        return;
    }

    /* P key toggles pause from gameplay */
    if (sc == 0x19) {
        if      (g->state == STATE_PLAYING) { g->state = STATE_PAUSED;  g->menu_selected = 0; }
        else if (g->state == STATE_PAUSED)   g->state = STATE_PLAYING;
        return;
    }

    if (g->state != STATE_PLAYING) return;

    /* Movement ΓÇö shared arrow / WASD layout */
    int nc = g->player.col, nr = g->player.row;
    switch (sc) {
        case 0x48: case 0x11: nr--; break;
        case 0x50: case 0x1F: nr++; break;
        case 0x4B: case 0x1E: nc--; break;
        case 0x4D: case 0x20: nc++; break;
        default: return;
    }

    if (g->player.in_car) {
        /* Car movement: can drive over any non-wall tile including exits */
        if (nc < 0 || nc >= MAP_COLS || nr < 0 || nr >= MAP_ROWS) return;
        tile_type_t dest = g->map[nr][nc];
        if (dest == TILE_WALL) return;
        g->map[g->player.row][g->player.col] = TILE_FLOOR;
        g->player.col = nc;
        g->player.row = nr;
        if (dest == TILE_EXIT) {
            g->score += g->countdown_sec * 10;
            g->state  = STATE_WIN;
            rtc_date date; rtc_time time;
            uint8_t d = 0, mo = 0, y = 0, h = 0, mi = 0, s = 0;
            if (rtc_read_date(&date) == 0) { d = date.day; mo = date.month; y = date.year; }
            if (rtc_read_time(&time) == 0) { h = time.hour; mi = time.minute; s = time.second; }
            leaderboard_try_insert(g, g->score, g->current_level + 1, d, mo, y, h, mi, s);
            serial_send_string("WIN CAR\n");
        }
    } else {
        if (tile_walkable(g, nc, nr)) {
            g->player.col = nc;
            g->player.row = nr;
        }
    }
}

void game_handle_mouse(game_t *g) {
    if (!mouse_pkt.x_ov) {
        int nx = g->mouse_x + mouse_pkt.delta_x;
        g->mouse_x = (nx < 0) ? 0 : (nx >= VIEW_WIDTH  ? VIEW_WIDTH  - 1 : nx);
    }
    if (!mouse_pkt.y_ov) {
        int ny = g->mouse_y - mouse_pkt.delta_y;
        g->mouse_y = (ny < 0) ? 0 : (ny >= VIEW_HEIGHT ? VIEW_HEIGHT - 1 : ny);
    }

    int mx = g->mouse_x, my = g->mouse_y;

    /* Update hover state for whichever panel is visible */
    if (g->state == STATE_QUIZ) {
        const quiz_entry_t *qe = current_question(g);
        if (qe->type == QUIZ_TYPE_MCQ) {
            g->quiz_hover = -1;
            for (int i = 0; i < 4; i++)
                if (mx >= QUIZ_BTN_X && mx < QUIZ_BTN_X + QUIZ_BTN_W
                    && my >= quiz_btn_y[i] && my < quiz_btn_y[i] + QUIZ_BTN_H) {
                    g->quiz_hover = i;
                    break;
                }
        } else {
            g->draw_hover_clear = (mx >= DRAW_BTN_CLEAR_X && mx < DRAW_BTN_CLEAR_X + DRAW_BTN_W
                                   && my >= DRAW_BTN_CLEAR_Y && my < DRAW_BTN_CLEAR_Y + DRAW_BTN_H);
            g->draw_hover_check = (mx >= DRAW_BTN_CHECK_X && mx < DRAW_BTN_CHECK_X + DRAW_BTN_W
                                   && my >= DRAW_BTN_CHECK_Y && my < DRAW_BTN_CHECK_Y + DRAW_BTN_H);
        }
    }

    /* Paint onto the drawing canvas while the left button is held */
    if (g->state == STATE_QUIZ) {
        const quiz_entry_t *qe = current_question(g);
        if (qe->type == QUIZ_TYPE_DRAW && mouse_pkt.lb
            && !g->draw_submitted && g->draw_ready_ticks == 0) {

            int cx = (mx - DRAW_CANVAS_X) / DRAW_CELL_SIZE;
            int cy = (my - DRAW_CANVAS_Y) / DRAW_CELL_SIZE;
            int x0 = (g->draw_last_x >= 0) ? g->draw_last_x : cx;
            int y0 = (g->draw_last_y >= 0) ? g->draw_last_y : cy;
            int x1 = cx, y1 = cy;

            /* Bresenham line between the last and current cell to avoid gaps */
            int dx = (x1 - x0 < 0) ? -(x1 - x0) : (x1 - x0);
            int dy = (y1 - y0 < 0) ? -(y1 - y0) : (y1 - y0);
            int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
            int err = dx - dy;
            while (1) {
                for (int oy = -1; oy <= 2; oy++)
                    for (int ox = -1; ox <= 2; ox++)
                        if (x0+ox >= 0 && x0+ox < DRAW_COLS && y0+oy >= 0 && y0+oy < DRAW_ROWS)
                            g->draw_canvas[y0+oy][x0+ox] = 1;
                if (x0 == x1 && y0 == y1) break;
                int e2 = 2 * err;
                if (e2 > -dy) { err -= dy; x0 += sx; }
                if (e2 <  dx) { err += dx; y0 += sy; }
            }
            g->draw_last_x = cx;
            g->draw_last_y = cy;
        } else if (!mouse_pkt.lb) {
            g->draw_last_x = g->draw_last_y = -1;
        }
    }

    if (!mouse_pkt.lb) { g->mouse_left_down = false; return; }
    if (g->mouse_left_down) return;
    g->mouse_left_down = true;

    /* Sync keyboard-highlighted menu item with mouse position */
    if (g->state == STATE_MENU || g->state == STATE_PAUSED || g->state == STATE_GUIDE) {
        int ox = 140, ow = 520, btn_w = 260, btn_h = 44, btn_gap = 10;
        int btn_x = ox + (ow - btn_w) / 2;
        int items, base_y;
        if (g->state == STATE_MENU)          { items = 6; base_y = 90 + 62 + 16; }
        else if (g->state == STATE_GUIDE)    { items = 2; base_y = 90 + 420 - 60 - btn_h; }
        else if (g -> state == STATE_PAUSED) { items = 4; base_y = 90 + 68 + 16; }
		else                               { items = 3; base_y = 90 + 68 + 16; }
        if (mx >= btn_x && mx < btn_x + btn_w)
            for (int i = 0; i < items; i++) {
                int by = base_y + i * (btn_h + btn_gap);
                if (my >= by && my < by + btn_h) { g->menu_selected = i; break; }
            }
    }

    /* Handle click actions per state */
    switch (g->state) {
        case STATE_MENU: {
            int ox = 140, ow = 520, btn_w = 260, btn_h = 44, btn_gap = 10;
            int btn_x = ox + (ow - btn_w) / 2;
            int base_y = 90 + 62 + 16;
            bool has_progress = (g->elapsed_ticks > 0 || g->ticks > 0);
			int by[6];
			for (int i = 0; i < 6; i++) by[i] = base_y + i * (btn_h + btn_gap);
			bool hit[6];
			for (int i = 0; i < 6; i++)
				hit[i] = (mx >= btn_x && mx < btn_x + btn_w && my >= by[i] && my < by[i] + btn_h);
            if      (hit[0]) { game_init(g); g->state = STATE_PLAYING; }
            else if (hit[1] && has_progress) g->state = STATE_PLAYING;
            else if (hit[2]) { g->state = STATE_LEADERBOARD; g->menu_selected = 0; }
			else if (hit[3]) { g->prev_state = STATE_MENU; g->state = STATE_CONTROLS; g->menu_selected = 0; }
            else if (hit[4]) { g->state = STATE_GUIDE; g->menu_selected = 0; }
            else if (hit[5]) request_quit = true;
            break;
        }
        case STATE_GUIDE: {
            int ox = 140, ow = 520, btn_w = 260, btn_h = 44, btn_gap = 10;
            int btn_x = ox + (ow - btn_w) / 2;
            int base_y = 90 + 420 - 60 - btn_h;
            if (mx >= btn_x && mx < btn_x + btn_w) {
                if (my >= base_y && my < base_y + btn_h)
                    { game_init(g); g->state = STATE_PLAYING; }
                else if (my >= base_y+btn_h+btn_gap && my < base_y+2*(btn_h+btn_gap))
                    { g->state = STATE_MENU; g->menu_selected = 0; }
            }
            break;
        }
        case STATE_LEADERBOARD:
            g->state = STATE_MENU;
            break;
        case STATE_PAUSED: {
            int ox = 140, ow = 520, btn_w = 260, btn_h = 44, btn_gap = 10;
            int btn_x = ox + (ow - btn_w) / 2;
            int base_y = 90 + 68 + 16;
			int by[4];
			for (int i = 0; i < 4; i++) by[i] = base_y + i * (btn_h + btn_gap);
			bool hit[4];
			for (int i = 0; i < 4; i++)
				hit[i] = (mx >= btn_x && mx < btn_x + btn_w && my >= by[i] && my < by[i] + btn_h);
            if      (hit[0]) g->state = STATE_PLAYING;
            else if (hit[1]) { game_init(g); g->state = STATE_PLAYING; }
			else if (hit[2]) { g->prev_state = STATE_PAUSED; g->state = STATE_CONTROLS; g->menu_selected = 0; }
            else if (hit[3]) { g->state = STATE_MENU; g->menu_selected = 0; }
            break;
        }
        case STATE_WIN:
        case STATE_LOSE:
            game_init(g);
            break;
        case STATE_QUIZ: {
            const quiz_entry_t *qe = current_question(g);
            if (qe->type == QUIZ_TYPE_MCQ) {
                if (g->quiz_feedback == 0 && g->quiz_hover >= 0) {
                    int sel = g->quiz_hover;
                    g->quiz_last_selected = sel;
                    if (sel == qe->correct) {
                        g->quiz_feedback       = 1;
                        g->quiz_feedback_ticks = 50;
                    } else {
                        g->quiz_feedback       = 2;
                        g->quiz_feedback_ticks = 50;
                        g->quiz_lives--;
                    }
                }
            } else {
                if (g->draw_hover_clear) {
                    memset(g->draw_canvas, 0, sizeof(g->draw_canvas));
                    g->draw_recognized = -1;
                    g->draw_submitted  = false;
                }
                if (g->draw_hover_check && !g->draw_submitted) {
                    g->draw_recognized = recognize_digit(g->draw_canvas);
                    g->draw_submitted  = true;
                    if (g->draw_recognized == quiz_draw.correct) {
                        g->quiz_feedback       = 1;
                        g->quiz_feedback_ticks = 80;
                    } else {
                        g->quiz_feedback       = 2;
                        g->quiz_feedback_ticks = 80;
                        g->quiz_lives--;
                    }
                }
            }
            break;
        }
		case STATE_CONTROLS:
			g->state = g->prev_state;
			g->menu_selected = 0;
			break;
        default: break;
    }
}
