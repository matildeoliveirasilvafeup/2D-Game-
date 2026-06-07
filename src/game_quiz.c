#include "game_internal.h"
#include <lcom/lcf.h>
#include <stdlib.h>
#include <string.h>
/* Count filled cells inside the rectangle [r0,r1] x [c0,c1] of the canvas */
int count_pixels_in_region(const uint8_t canvas[DRAW_ROWS][DRAW_COLS],
                                   int r0, int r1, int c0, int c1) {
    int cnt = 0;
    for (int r = r0; r <= r1 && r < DRAW_ROWS; r++)
        for (int c = c0; c <= c1 && c < DRAW_COLS; c++)
            if (canvas[r][c]) cnt++;
    return cnt;
}

/* Return 6 if the drawing looks like a 6, -1 otherwise */
int recognize_digit(const uint8_t canvas[DRAW_ROWS][DRAW_COLS]) {
	/* Find the bounding box of the drawing */
	int min_r = DRAW_ROWS, max_r = 0, min_c = DRAW_COLS, max_c = 0;
	int total = 0;
	for (int r = 0; r < DRAW_ROWS; r++) {
		for (int c = 0; c < DRAW_COLS; c++) {
			if (canvas[r][c]) {
				total++;
				if (r < min_r) min_r = r;
				if (r > max_r) max_r = r;
				if (c < min_c) min_c = c;
				if (c > max_c) max_c = c;
			}
		}
	}

	if (total < 15) return -1;
	if (max_r - min_r < 4 || max_c - min_c < 4) return -1;

	/* Divide the bounding box into 4 adaptive quadrants */
	int r_mid = (min_r + max_r) / 2;
	int c_mid = (min_c + max_c) / 2;

	int q_tl = count_pixels_in_region(canvas, min_r, r_mid,     min_c, c_mid);
	int q_tr = count_pixels_in_region(canvas, min_r, r_mid,     c_mid + 1, max_c);
	int q_bl = count_pixels_in_region(canvas, r_mid + 1, max_r, min_c, c_mid);
	int q_br = count_pixels_in_region(canvas, r_mid + 1, max_r, c_mid + 1, max_c);

	/* Crossbar band — around the vertical midpoint */
	int band_h = (max_r - min_r) / 5;
	if (band_h < 2) band_h = 2;
	int b_top = r_mid - band_h;
	int b_bot = r_mid + band_h;
	if (b_top < min_r) b_top = min_r;
	if (b_bot > max_r) b_bot = max_r;
	int cross_l = count_pixels_in_region(canvas, b_top, b_bot, min_c, c_mid);
	int cross_r = count_pixels_in_region(canvas, b_top, b_bot, c_mid + 1, max_c);

	/* Relative left-vs-right split in the top half (key 6 vs 8 discriminator) */
	int top_total = q_tl + q_tr;


	if (top_total > 0 && q_tl * 100 / top_total >= 60 &&
	    cross_l > 1 && cross_r > 1 &&
	    q_bl > 2 && q_br > 2 &&
	    q_bl + q_br > q_tl + q_tr) {
		return 6;
	}

	return -1;
}

/* Insert a new score into the top-3 table, shifting entries down as needed */
void leaderboard_try_insert(game_t *g, int score, int level,
                                   uint8_t day, uint8_t month, uint8_t year,
                                   uint8_t hour, uint8_t minute, uint8_t second) {
    int pos = -1;
    for (int i = 0; i < 3; i++) {
        if (!g->high_scores[i].valid || score > g->high_scores[i].score) {
            pos = i;
            break;
        }
    }
    if (pos < 0) return;

    for (int i = 2; i > pos; i--)
        g->high_scores[i] = g->high_scores[i - 1];

    g->high_scores[pos] = (high_score_t){
        .score  = score,  .level  = level,
        .day    = day,    .month  = month, .year   = year,
        .hour   = hour,   .minute = minute,.second = second,
        .valid  = true
    };
}

void quiz_pick_questions(game_t *g) {
    int idx[9];
    for (int i = 0; i < 9; i++) idx[i] = i;

    uint32_t seed = g->ticks
                  ^ (uint32_t)(g->score       *    31)
                  ^ (uint32_t)(g->rtc_second  *  7919)
                  ^ (uint32_t)(g->rtc_minute  *  1031)
                  ^ (uint32_t)(g->rtc_hour    *   137);

    for (int i = 8; i > 0; i--) {
        seed = seed * 1664525u + 1013904223u;
        int j = (int)(seed >> 16) % (i + 1);
        int tmp = idx[i]; idx[i] = idx[j]; idx[j] = tmp;
    }

    g->selected_questions[0] = idx[0];
    g->selected_questions[1] = idx[1];
}

/* Return the quiz entry for the current step of the round (0,1 = MCQ; 2 = draw) */
const quiz_entry_t *current_question(const game_t *g) {
    return (g->quiz_question >= 2) ? &quiz_draw : &quiz_bank[g->selected_questions[g->quiz_question]];
}

