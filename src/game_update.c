#include "game_internal.h"
#include <lcom/lcf.h>
#include <stdlib.h>
#include <string.h>
#include "serial.h"
#include "rtc.h"

static bool guard_sees_player(const game_t *g, const guard_t *guard, const player_t *player) {
    static const int fdc[] = { 0, 1, 0, -1 };
    static const int fdr[] = { -1, 0, 1,  0 };
    int fd = guard->direction;
    for (int step = 1; step <= guard->vision_range; step++) {
        int cc = guard->col + fdc[fd] * step;
        int cr = guard->row + fdr[fd] * step;
        if (cc < 0 || cc >= MAP_COLS || cr < 0 || cr >= MAP_ROWS) break;
        if (g->map[cr][cc] == TILE_WALL) break;
        if (cc == player->col && cr == player->row) return true;
    }
    return (guard->col == player->col && guard->row == player->row);
}

static void bfs_next_step(const game_t *g, int sc, int sr, int tc, int tr,
                           int *out_col, int *out_row) {
    int8_t visited[MAP_ROWS][MAP_COLS];
    int8_t from_c[MAP_ROWS][MAP_COLS];
    int8_t from_r[MAP_ROWS][MAP_COLS];
    memset(visited, 0, sizeof(visited));
    memset(from_c, -1, sizeof(from_c));
    memset(from_r, -1, sizeof(from_r));

    int qc[MAP_ROWS * MAP_COLS], qr[MAP_ROWS * MAP_COLS];
    int head = 0, tail = 0;
    visited[sr][sc] = 1;
    qc[tail] = sc; qr[tail] = sr; tail++;

    static const int ndc[] = { 0, 1,  0, -1 };
    static const int ndr[] = { -1, 0, 1,  0 };
    bool found = false;

    while (head != tail) {
        int cc = qc[head], cr = qr[head]; head++;
        if (cc == tc && cr == tr) { found = true; break; }
        for (int i = 0; i < 4; i++) {
            int nc = cc + ndc[i], nr = cr + ndr[i];
            if (nr < 0 || nr >= MAP_ROWS || nc < 0 || nc >= MAP_COLS) continue;
            if (visited[nr][nc] || !tile_walkable(g, nc, nr)) continue;
            visited[nr][nc] = 1;
            from_c[nr][nc]  = (int8_t)cc;
            from_r[nr][nc]  = (int8_t)cr;
            qc[tail] = nc; qr[tail] = nr; tail++;
        }
    }

    if (!found) { *out_col = -1; *out_row = -1; return; }

    int cc = tc, cr = tr;
    while (from_c[cr][cc] != (int8_t)sc || from_r[cr][cc] != (int8_t)sr) {
        int pc = (int8_t)from_c[cr][cc], pr = (int8_t)from_r[cr][cc];
        if (from_c[cr][cc] == -1) { *out_col = -1; *out_row = -1; return; }
        cc = pc; cr = pr;
    }
    *out_col = cc; *out_row = cr;
}

static void guard_step(const game_t *g, guard_t *guard) {
    static const int fdc[] = { 0, 1,  0, -1 };
    static const int fdr[] = { -1, 0, 1,  0 };
    int nc, nr;
    bfs_next_step(g, guard->col, guard->row, g->player.col, g->player.row, &nc, &nr);
    if (nc == -1) return;
    for (int d = 0; d < 4; d++)
        if (guard->col + fdc[d] == nc && guard->row + fdr[d] == nr) {
            guard->direction = d;
            break;
        }
    guard->patrol_len = guard->col;
    guard->patrol_idx = guard->row;
    guard->col = nc;
    guard->row = nr;
}

static void dumb_guard_step(const game_t *g, guard_t *guard) {
    static const int dc[] = { 0, 1,  0, -1 };
    static const int dr[] = { -1, 0, 1,  0 };
    int options[4], count = 0, best_dist = 1000;
    int current_dist = abs(g->player.col - guard->col) + abs(g->player.row - guard->row);

    for (int d = 0; d < 4; d++) {
        int nc = guard->col + dc[d], nr = guard->row + dr[d];
        if (!tile_walkable(g, nc, nr)) continue;
        if (nc == guard->patrol_len && nr == guard->patrol_idx) continue;
        int dist = abs(g->player.col - nc) + abs(g->player.row - nr);
        if (dist >= current_dist) continue;
        if (dist < best_dist) { best_dist = dist; count = 0; }
        if (dist == best_dist) options[count++] = d;
    }

    if (count == 0) { guard_step(g, guard); return; }

    int dir = options[(g->ticks / GUARD_MOVE_TICKS + guard->col + guard->row) % count];
    guard->direction  = dir;
    guard->patrol_len = guard->col;
    guard->patrol_idx = guard->row;
    guard->col += dc[dir];
    guard->row += dr[dir];
}

bool tile_walkable(const game_t *g, int col, int row) {
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return false;
    return g->map[row][col] != TILE_WALL;
}

void game_update(game_t *g) {
    if (g->state == STATE_QUIZ) {
        if (g->draw_ready_ticks > 0) g->draw_ready_ticks--;
        if (g->quiz_feedback_ticks > 0) {
            g->quiz_feedback_ticks--;
				if (g->quiz_feedback_ticks == 0) {
					if (g->quiz_feedback == 1) {
						g->quiz_question++;
						if (g->quiz_question >= 3) {
							g->player.has_exam = true;
							g->score += 500;
							g->state = STATE_PLAYING;
							serial_send_string("EXAM COLLECTED\n");
						} else if (g->quiz_question == 2) {
							g->draw_ready_ticks = 180;
						}
					} else if (g->quiz_feedback == 2 && g->quiz_lives <= 0) {
						g->state = STATE_LOSE;
						g->lose_timeout = false;
						serial_send_string("LOSE QUIZ\n");
					}
					g->quiz_feedback = 0;
				}
		  }
        return;
    }

    if (g->state != STATE_PLAYING) return;

    g->ticks++;
    g->elapsed_ticks++;

    if (g->ticks % TICKS_PER_SEC == 0) {
        exam_frame = !exam_frame;
        if (g->countdown_sec > 0) {
            g->countdown_sec--;
        } else {
            g->state = STATE_LOSE;
            g->lose_timeout = true;
            serial_send_string("LOSE TIMEOUT\n");
            return;
        }
        rtc_date date;
        if (rtc_read_date(&date) == 0) {
            g->rtc_day   = date.day;
            g->rtc_month = date.month;
            g->rtc_year  = date.year;
        }
        rtc_time time;
        if (rtc_read_time(&time) == 0) {
            g->rtc_hour   = time.hour;
            g->rtc_minute = time.minute;
            g->rtc_second = time.second;
        }
    }

    int move_ticks = (g->current_level == 1) ? (GUARD_MOVE_TICKS * 2) : GUARD_MOVE_TICKS;
    if (g->ticks % move_ticks == 0) {
        guard_frame = !guard_frame;
        for (int i = 0; i < g->num_guards; i++) {
            guard_t *guard = &g->guards[i];
            if (guards_panicking) continue;

            if (guard->alert > ALERT_NONE) {
                guard->alert_ticks--;
                if (guard->alert_ticks <= 0) {
						guard->alert = ALERT_NONE;
						guard->alert_ticks = ALERT_DECAY_TICKS;
                }
            }

            if (guard_sees_player(g, guard, &g->player)) {
                guard->alert       = ALERT_DETECTED;
                guard->alert_ticks = ALERT_DECAY_TICKS;
            }

            if (!g->player.in_car && guard->col == g->player.col && guard->row == g->player.row) {
                g->player.detected = true;
                g->state           = STATE_LOSE;
                g->lose_timeout    = false;
                serial_send_string("LOSE CAUGHT\n");
                return;
            }

				int next_c = -1, next_r = -1;
				int old_c = guard->col, old_r = guard->row;

				if ((i % 2) == 0)
					guard_step(g, guard);
				else
					dumb_guard_step(g, guard);

				for (int j = 0; j < i; j++) {
					if (g->guards[j].col == guard->col && g->guards[j].row == guard->row) {
						guard->col = old_c;
						guard->row = old_r;
						break;
					}
				}

				/* Check if another guard already occupies this cell */
				bool collision = false;
				for (int j = 0; j < g->num_guards; j++) {
					if (j == i)
						continue;
					if (g->guards[j].col == next_c && g->guards[j].row == next_r) {
						collision = true;
						break;
					}
				}
				if (collision) {
					guard->col = guard->patrol_len;
					guard->row = guard->patrol_idx;
				}

				if (!g->player.in_car && guard->col == g->player.col && guard->row == g->player.row) {
                g->player.detected = true;
                g->state           = STATE_LOSE;
                g->lose_timeout    = false;
                serial_send_string("LOSE CAUGHT\n");
                return;
            }
        }
    }

    if (g->map[g->player.row][g->player.col] == TILE_EXAM) {
        g->map[g->player.row][g->player.col] = TILE_FLOOR;

        if (g->current_level == 1) {
            g->player.in_car = true;
            guards_panicking = true;
            serial_send_string("PLAYER IN CAR\n");
            if (g->num_guards > 0) {
                int best = 0, best_dist = 9999;
                for (int i = 0; i < g->num_guards; i++) {
                    int d = abs(g->guards[i].col - 0) + abs(g->guards[i].row - 1);
                    if (d < best_dist) { best_dist = d; best = i; }
                }
                g->guards[best].alert       = ALERT_DETECTED;
                g->guards[best].alert_ticks = 9999;
            }
        } else {
            g->quiz_question       = 0;
            g->quiz_lives          = 3;
            g->quiz_hover          = -1;
            g->quiz_last_selected  = -1;
            g->quiz_feedback       = 0;
            g->quiz_feedback_ticks = 0;
            quiz_pick_questions(g);
            memset(g->draw_canvas, 0, sizeof(g->draw_canvas));
            g->draw_is_drawing  = false;
            g->draw_recognized  = -1;
            g->draw_submitted   = false;
            g->draw_hover_clear = false;
            g->draw_hover_check = false;
            g->draw_last_x      = -1;
            g->draw_last_y      = -1;
            g->mouse_x          = DRAW_CANVAS_X + DRAW_CANVAS_W / 2;
            g->mouse_y          = DRAW_CANVAS_Y + DRAW_CANVAS_H / 2;
            g->draw_ready_ticks = 180;
            g->state = STATE_QUIZ;
        }
        return;
    }

    if (g->current_level == 1 && g->player.in_car
        && g->map[g->player.row][g->player.col] == TILE_EXIT) {
        g->score += g->countdown_sec * 10;
        g->state  = STATE_WIN;
        rtc_date date; rtc_time time;
        uint8_t d = 0, mo = 0, y = 0, h = 0, mi = 0, s = 0;
        if (rtc_read_date(&date) == 0) { d = date.day; mo = date.month; y = date.year; }
        if (rtc_read_time(&time) == 0) { h = time.hour; mi = time.minute; s = time.second; }
        leaderboard_try_insert(g, g->score, g->current_level + 1, d, mo, y, h, mi, s);
        serial_send_string("WIN CAR\n");
        return;
    }

    if (g->current_level == 0
        && g->map[g->player.row][g->player.col] == TILE_EXIT
        && g->player.has_exam) {

        g->score += g->countdown_sec * 10;

        if (g->current_level < NUM_LEVELS - 1) {
            int          saved_score = g->score;
            high_score_t saved_hs[3];
            memcpy(saved_hs, g->high_scores, sizeof(saved_hs));

            load_level(g, g->current_level + 1);
            g->score           = saved_score;
            g->player.has_exam = true;
            memcpy(g->high_scores, saved_hs, sizeof(saved_hs));

            g->ticks              = 0;
            g->elapsed_ticks      = 1;
            g->quiz_question      = 0;
            g->quiz_lives         = 3;
            g->quiz_hover         = -1;
            g->quiz_last_selected = -1;
            g->quiz_feedback      = 0;
            g->quiz_feedback_ticks = 0;
            quiz_pick_questions(g);
            memset(g->draw_canvas, 0, sizeof(g->draw_canvas));
            g->draw_is_drawing  = false;
            g->draw_recognized  = -1;
            g->draw_submitted   = false;
            g->draw_hover_clear = false;
            g->draw_hover_check = false;
            g->draw_last_x      = -1;
            g->draw_last_y      = -1;
            g->mouse_left_down  = false;
            g->lose_timeout     = false;
            g->state = STATE_PLAYING;
            serial_send_string("LEVEL 2\n");
            return;
        }

        g->state = STATE_WIN;
        rtc_date date; rtc_time time;
        uint8_t d = 0, mo = 0, y = 0, h = 0, mi = 0, s = 0;
        if (rtc_read_date(&date) == 0) { d = date.day; mo = date.month; y = date.year; }
        if (rtc_read_time(&time) == 0) { h = time.hour; mi = time.minute; s = time.second; }
        leaderboard_try_insert(g, g->score, g->current_level + 1, d, mo, y, h, mi, s);

        char buf[64]; int pos = 0;
        const char prefix[] = "WIN SCORE:";
        for (int i = 0; prefix[i]; i++) buf[pos++] = prefix[i];
        int sv = g->score;
        if (sv == 0) { buf[pos++] = '0'; }
        else {
            char tmp[8]; int n = 0;
            while (sv > 0) { tmp[n++] = (char)('0' + sv % 10); sv /= 10; }
            while (n > 0)  buf[pos++] = tmp[--n];
        }
        buf[pos++] = ' ';
        const char tprefix[] = "TIME:";
        for (int i = 0; tprefix[i]; i++) buf[pos++] = tprefix[i];
        int secs = (int)g->countdown_sec;
        if (secs == 0) { buf[pos++] = '0'; }
        else {
            char tmp[8]; int n = 0;
            while (secs > 0) { tmp[n++] = (char)('0' + secs % 10); secs /= 10; }
            while (n > 0)    buf[pos++] = tmp[--n];
        }
        buf[pos++] = '\n'; buf[pos] = '\0';
        serial_send_string(buf);
    }
}
