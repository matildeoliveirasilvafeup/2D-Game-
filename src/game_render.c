#include <lcom/lcf.h>
#include <stdlib.h>
#include <string.h>
#include "game_internal.h"
#include "graphics.h"

#define COLOR_FLOOR 0x000000
#define COLOR_EXIT_OFF 0xB8B800
#define COLOR_EXAM 0xF2F2F2
#define COLOR_WALL_L1_A 0x444444
#define COLOR_WALL_L1_B 0x222222

#define DRAW_BTN_MENU(BX, BY, BW, BH, LABEL, ACTIVE)                                                                 \
	do {                                                                                                              \
		bool _mh = (g->mouse_x >= (BX) && g->mouse_x < (BX) + (BW) && g->mouse_y >= (BY) && g->mouse_y < (BY) + (BH)); \
		uint32_t _bg = (ACTIVE) ? C_BTN_SEL : (_mh ? C_BTN_HOVER : C_BTN_IDLE);                                        \
		uint32_t _fg = ((ACTIVE) || _mh) ? C_TXT_SEL : C_TXT_IDLE;                                                     \
		draw_rectangle((BX), (BY), (BW), (BH), _bg);                                                                   \
		draw_rectangle((BX), (BY), (BW), 2, C_BTN_BORD);                                                               \
		draw_rectangle((BX), (BY) + (BH) - 2, (BW), 2, C_BTN_BORD);                                                    \
		draw_rectangle((BX), (BY), 2, (BH), C_BTN_BORD);                                                               \
		draw_rectangle((BX) + (BW) - 2, (BY), 2, (BH), C_BTN_BORD);                                                    \
		if (ACTIVE)                                                                                                    \
			draw_rectangle((BX), (BY), 3, (BH), 0x4AB0E6);                                                              \
		draw_text_centred((BX), (BW), (BY) + ((BH) - 7 * 2) / 2, (LABEL), _fg, 2);                                     \
	} while (0)

static void render_leaderboard(const game_t* g) {
	draw_rectangle(0, 0, 800, 600, 0x000000);
	int ox = 100, oy = 60, ow = 600, oh = 480;
	draw_rectangle(ox, oy, ow, oh, 0x0A0F14);
	draw_rectangle(ox, oy, ow, 3, 0x8FB8FF);
	draw_rectangle(ox, oy + oh - 3, ow, 3, 0x8FB8FF);
	draw_rectangle(ox, oy, 3, oh, 0x8FB8FF);
	draw_rectangle(ox + ow - 3, oy, 3, oh, 0x8FB8FF);

	draw_text_centred(ox, ow, oy + 14, "HIGH SCORES", 0xFFFFFF, 3);
	int div_y = oy + 52;
	draw_rectangle(ox, div_y, ow, 2, 0x8FB8FF);

	int hdy = div_y + 14;
	draw_text(ox + 30, hdy, "RANK", 0x8FB8FF, 2);
	draw_text(ox + 120, hdy, "SCORE", 0x8FB8FF, 2);
	draw_text(ox + 240, hdy, "DATE", 0x8FB8FF, 2);
	draw_text(ox + 380, hdy, "TIME", 0x8FB8FF, 2);
	draw_rectangle(ox, hdy + 20, ow, 1, 0x334455);

	static const uint32_t rank_colors[3] = {0xFFD700, 0xC0C0C0, 0xCD7F32};
	static const char* rank_labels[3] = {"1ST", "2ND", "3RD"};

	for (int i = 0; i < 3; i++) {
		int ry = hdy + 30 + i * 70;
		draw_rectangle(ox + 3, ry - 4, ow - 6, 60, (i % 2 == 0) ? 0x0D1825 : 0x091018);
		draw_text(ox + 30, ry + 10, rank_labels[i], rank_colors[i], 2);
		if (!g->high_scores[i].valid) {
			draw_text(ox + 120, ry + 10, "---", 0x444444, 2);
		} else {
			const high_score_t* hs = &g->high_scores[i];
			draw_int_text(ox + 120, ry + 10, hs->score, 0xFFFFFF, 2);
			draw_int_text(ox + 240, ry + 10, (int)hs->day, 0xCCCCCC, 2);
			draw_text(ox + 258, ry + 10, "/", 0xCCCCCC, 2);
			draw_int_text(ox + 270, ry + 10, (int)hs->month, 0xCCCCCC, 2);
			draw_text(ox + 288, ry + 10, "/", 0xCCCCCC, 2);
			draw_int_text(ox + 300, ry + 10, 2000 + (int)hs->year, 0xCCCCCC, 2);
			draw_int_text(ox + 370, ry + 10, (int)hs->hour, 0xCCCCCC, 2);
			draw_text(ox + 388, ry + 10, ":", 0xCCCCCC, 2);
			draw_int_text(ox + 400, ry + 10, (int)hs->minute, 0xCCCCCC, 2);
			draw_text(ox + 418, ry + 10, ":", 0xCCCCCC, 2);
			draw_int_text(ox + 430, ry + 10, (int)hs->second, 0xCCCCCC, 2);
		}
		draw_rectangle(ox, ry + 56, ow, 1, 0x1A2535);
	}
	draw_text_centred(ox, ow, oy + oh - 30, "PRESS ENTER OR CLICK TO GO BACK", 0x556677, 1);

	/* Cross cursor */
	draw_rectangle(g->mouse_x - 1, g->mouse_y - 9, 2, 18, 0xFFFFFF);
	draw_rectangle(g->mouse_x - 9, g->mouse_y - 1, 18, 2, 0xFFFFFF);
}

static void render_state_overlay(const game_t* g) {
	if (g->state == STATE_PLAYING || g->state == STATE_QUIZ)
		return;

	draw_rectangle(0, 0, 800, 600, 0x000000);
	int ox = 140, oy = 90, ow = 520, oh = 420;
	draw_rectangle(ox, oy, ow, oh, 0x0A0F14);
	draw_rectangle(ox, oy, ow, 3, 0x8FB8FF);
	draw_rectangle(ox, oy + oh - 3, ow, 3, 0x8FB8FF);
	draw_rectangle(ox, oy, 3, oh, 0x8FB8FF);
	draw_rectangle(ox + ow - 3, oy, 3, oh, 0x8FB8FF);

	int btn_w = 260, btn_h = 44, btn_gap = 10;
	int btn_x = ox + (ow - btn_w) / 2;

	uint32_t C_BTN_IDLE = 0x111111, C_BTN_SEL = 0x1C2E50, C_BTN_HOVER = 0x223A60;
	uint32_t C_BTN_BORD = 0x8FB8FF, C_TXT_IDLE = 0xCCCCCC, C_TXT_SEL = 0xFFFFFF;

	if (g->state == STATE_MENU) {
		draw_text_centred(ox, ow, oy + 14, "FEUP HEIST", 0xFFFFFF, 3);
		int div_y = oy + 62;
		draw_rectangle(ox, div_y, ow, 2, 0x8FB8FF);

		bool has_progress = (g->elapsed_ticks > 0 || g->ticks > 0);
		int b1y = div_y + 16, b2y = b1y + btn_h + btn_gap, b3y = b2y + btn_h + btn_gap, b4y = b3y + btn_h + btn_gap, b5y = b4y + btn_h + btn_gap, b6y = b5y + btn_h + btn_gap;

		DRAW_BTN_MENU(btn_x, b1y, btn_w, btn_h, "START NEW GAME", g->menu_selected == 0);

		if (has_progress) {
			DRAW_BTN_MENU(btn_x, b2y, btn_w, btn_h, "CONTINUE", g->menu_selected == 1);
		} else {
			/* Greyed-out continue when no session is in progress */
			draw_rectangle(btn_x, b2y, btn_w, btn_h, 0x0A0A0A);
			draw_rectangle(btn_x, b2y, btn_w, 2, 0x333333);
			draw_rectangle(btn_x, b2y + btn_h - 2, btn_w, 2, 0x333333);
			draw_rectangle(btn_x, b2y, 2, btn_h, 0x333333);
			draw_rectangle(btn_x + btn_w - 2, b2y, 2, btn_h, 0x333333);
			draw_text_centred(btn_x, btn_w, b2y + (btn_h - 7 * 2) / 2, "CONTINUE", 0x444444, 2);
		}

		/* High Scores button*/
		{
			bool mh = (g->mouse_x >= btn_x && g->mouse_x < btn_x + btn_w && g->mouse_y >= b3y && g->mouse_y < b3y + btn_h);
			bool act = (g->menu_selected == 2);
			draw_rectangle(btn_x, b3y, btn_w, btn_h, act ? 0x0F2A40 : (mh ? 0x0A1F30 : 0x111111));
			draw_rectangle(btn_x, b3y, btn_w, 2, 0xFFD700);
			draw_rectangle(btn_x, b3y + btn_h - 2, btn_w, 2, 0xFFD700);
			draw_rectangle(btn_x, b3y, 2, btn_h, 0xFFD700);
			draw_rectangle(btn_x + btn_w - 2, b3y, 2, btn_h, 0xFFD700);
			if (act)
				draw_rectangle(btn_x, b3y, 3, btn_h, 0xFFE44A);
			draw_text_centred(btn_x, btn_w, b3y + (btn_h - 7 * 2) / 2, "HIGH SCORES", act || mh ? 0xFFD700 : 0xAA8800, 2);
		}

		{
			bool mh = (g->mouse_x >= btn_x && g->mouse_x < btn_x + btn_w && g->mouse_y >= b4y && g->mouse_y < b4y + btn_h);
			bool act = (g->menu_selected == 3);
			draw_rectangle(btn_x, b4y, btn_w, btn_h, act ? 0x0A1F3A : (mh ? 0x071528 : 0x111111));
			draw_rectangle(btn_x, b4y, btn_w, 2, 0x4AB0E6);
			draw_rectangle(btn_x, b4y + btn_h - 2, btn_w, 2, 0x4AB0E6);
			draw_rectangle(btn_x, b4y, 2, btn_h, 0x4AB0E6);
			draw_rectangle(btn_x + btn_w - 2, b4y, 2, btn_h, 0x4AB0E6);
			if (act)
				draw_rectangle(btn_x, b4y, 3, btn_h, 0x77CCFF);
			draw_text_centred(btn_x, btn_w, b4y + (btn_h - 7 * 2) / 2, "CONTROLS", act || mh ? 0xAADDFF : 0x4AB0E6, 2);
		}

		/* Guide button*/
		{
			bool mh = (g->mouse_x >= btn_x && g->mouse_x < btn_x + btn_w && g->mouse_y >= b5y && g->mouse_y < b5y + btn_h);
			bool act = (g->menu_selected == 4);
			draw_rectangle(btn_x, b5y, btn_w, btn_h, act ? 0x0A2A20 : (mh ? 0x072015 : 0x111111));
			draw_rectangle(btn_x, b5y, btn_w, 2, 0x44CC88);
			draw_rectangle(btn_x, b5y + btn_h - 2, btn_w, 2, 0x44CC88);
			draw_rectangle(btn_x, b5y, 2, btn_h, 0x44CC88);
			draw_rectangle(btn_x + btn_w - 2, b5y, 2, btn_h, 0x44CC88);
			if (act)
				draw_rectangle(btn_x, b5y, 3, btn_h, 0x66EEAA);
			draw_text_centred(btn_x, btn_w, b5y + (btn_h - 7 * 2) / 2, "GUIDE", act || mh ? 0x88FFBB : 0x44CC88, 2);
		}

		/* Quit button*/
		{
			bool mh = (g->mouse_x >= btn_x && g->mouse_x < btn_x + btn_w && g->mouse_y >= b6y && g->mouse_y < b6y + btn_h);
			bool act = (g->menu_selected == 5);
			draw_rectangle(btn_x, b6y, btn_w, btn_h, act ? 0x3A0808 : (mh ? 0x2A0505 : 0x111111));
			draw_rectangle(btn_x, b6y, btn_w, 2, 0xFF4444);
			draw_rectangle(btn_x, b6y + btn_h - 2, btn_w, 2, 0xFF4444);
			draw_rectangle(btn_x, b6y, 2, btn_h, 0xFF4444);
			draw_rectangle(btn_x + btn_w - 2, b6y, 2, btn_h, 0xFF4444);
			if (act)
				draw_rectangle(btn_x, b6y, 3, btn_h, 0xFF6666);
			draw_text_centred(btn_x, btn_w, b6y + (btn_h - 7 * 2) / 2, "QUIT GAME", act || mh ? 0xFFAAAA : 0xCC6666, 2);
		}

	} else if (g->state == STATE_PAUSED) {
		draw_text_centred(ox, ow, oy + 14, "PAUSED", 0xFFFFFF, 3);
		draw_text_centred(ox, ow, oy + 48, "GAME IS PAUSED", 0x8FB8FF, 2);
		int div_y = oy + 68;
		draw_rectangle(ox, div_y, ow, 2, 0x8FB8FF);
		int b1y = div_y + 16, b2y = b1y + btn_h + btn_gap, b3y = b2y + btn_h + btn_gap, b4y = b3y + btn_h + btn_gap;
		DRAW_BTN_MENU(btn_x, b1y, btn_w, btn_h, "CONTINUE", g->menu_selected == 0);
		DRAW_BTN_MENU(btn_x, b2y, btn_w, btn_h, "NEW GAME", g->menu_selected == 1);
		{
			bool mh = (g->mouse_x >= btn_x && g->mouse_x < btn_x + btn_w && g->mouse_y >= b3y && g->mouse_y < b3y + btn_h);
			bool act = (g->menu_selected == 2);
			draw_rectangle(btn_x, b3y, btn_w, btn_h, act ? 0x0A1F3A : (mh ? 0x071528 : 0x111111));
			draw_rectangle(btn_x, b3y, btn_w, 2, 0x4AB0E6);
			draw_rectangle(btn_x, b3y + btn_h - 2, btn_w, 2, 0x4AB0E6);
			draw_rectangle(btn_x, b3y, 2, btn_h, 0x4AB0E6);
			draw_rectangle(btn_x + btn_w - 2, b3y, 2, btn_h, 0x4AB0E6);
			if (act)
				draw_rectangle(btn_x, b3y, 3, btn_h, 0x77CCFF);
			draw_text_centred(btn_x, btn_w, b3y + (btn_h - 7 * 2) / 2, "CONTROLS", act || mh ? 0xAADDFF : 0x4AB0E6, 2);
		}
		{
			bool mh = (g->mouse_x >= btn_x && g->mouse_x < btn_x + btn_w && g->mouse_y >= b4y && g->mouse_y < b4y + btn_h);
			bool act = (g->menu_selected == 3);
			draw_rectangle(btn_x, b4y, btn_w, btn_h, act ? 0x3A0808 : (mh ? 0x2A0505 : 0x111111));
			draw_rectangle(btn_x, b4y, btn_w, 2, 0xFF4444);
			draw_rectangle(btn_x, b4y + btn_h - 2, btn_w, 2, 0xFF4444);
			draw_rectangle(btn_x, b4y, 2, btn_h, 0xFF4444);
			draw_rectangle(btn_x + btn_w - 2, b4y, 2, btn_h, 0xFF4444);
			if (act)
				draw_rectangle(btn_x, b4y, 3, btn_h, 0xFF6666);
			draw_text_centred(btn_x, btn_w, b4y + (btn_h - 7 * 2) / 2, "QUIT", act || mh ? 0xFFAAAA : 0xCC6666, 2);
		}

	} else if (g->state == STATE_GUIDE) {
		draw_text_centred(ox, ow, oy + 14, "GUIDE", 0x8FB8FF, 3);
		int div_y = oy + 62;
		draw_rectangle(ox, div_y, ow, 2, 0x8FB8FF);

		static const char* guide_lines[] = {
			 "YOU ARE A FEUP STUDENT WHO HAS AN INCOMING LCOM",
			 "TEST AND YOU HAVENT STUDIED FOR IT",
			 "",
			 "YOU THOUGHT OF A GENIAL PLAN",
			 "STEAL THE TEST ANSWERS FROM THE TEACHERS SAFE AND",
			 "GO HOME BEFORE THE TEACHING STAFF GETS A HOLD OF YOU",
			 "",
			 "TO OPEN THE SAFE YOU WILL HAVE TO",
			 "ANSWER A SERIES OF LCOM RELATED QUESTIONS",
			 "RUN OUTSIDE THE UNIVERSITY GROUNDS",
			 "DODGE THE REMAINING TEACHERS",
			 "GET IN YOUR CAR AND DRIVE TO SAFETY"};

		int nlines = (int)(sizeof(guide_lines) / sizeof(guide_lines[0]));
		int ty = div_y + 16;

		for (int li = 0; li < nlines; li++) {
			/* Skip empty lines used as vertical spacing */
			if (guide_lines[li][0] == '\0') {
				ty += 12;
				continue;
			}

			/* --- Exam sheet icon on line 8: "ANSWER A SERIES OF LCOM RELATED QUESTIONS" ---
			 * Renders a small document with a cyan border and black text lines,
			 */
			if (li == 8) {
				/* Calculate total block width: icon (14px) + gap (6px) + text */
				int total_w = 14 + 6 + text_width(guide_lines[li], 1);
				int start_x = ox + (ow - total_w) / 2; /* centre the icon+text block */
				int ix = start_x;
				int iy = ty - 1; /* icon y: aligned with the current text baseline */

				/* White sheet background */
				draw_rectangle(ix, iy, 14, 18, 0xFFFFFF);

				/* Cyan border, matching the in-game exam sprite border colour */
				draw_rectangle(ix, iy, 14, 2, 0x00FFFF);		 /* top edge    */
				draw_rectangle(ix, iy + 16, 14, 2, 0x00FFFF); /* bottom edge */
				draw_rectangle(ix, iy, 2, 18, 0x00FFFF);		 /* left edge   */
				draw_rectangle(ix + 12, iy, 2, 18, 0x00FFFF); /* right edge  */

				/* Black marks simulating printed text content on the sheet */
				draw_rectangle(ix + 3, iy + 3, 7, 2, 0x000000);	 /* text line 1, wide  */
				draw_rectangle(ix + 3, iy + 6, 5, 2, 0x000000);	 /* text line 2, short */
				draw_rectangle(ix + 3, iy + 9, 7, 2, 0x000000);	 /* text line 3, wide  */
				draw_rectangle(ix + 3, iy + 12, 4, 2, 0x000000); /* text line 4, short */

				/* Render guide text immediately to the right of the icon */
				draw_text(start_x + 14 + 6, ty, guide_lines[li], 0xCCCCCC, 1);
				ty += 20;
				continue;
			}

			/* --- Door icon on line 9: "RUN OUTSIDE THE UNIVERSITY GROUNDS" ---
			 * Renders a small wooden door with a dark frame and a gold door handle,
			 * matching the visual style of the exit door tiles used in the game levels.
			 */
			if (li == 9) {
				/* Calculate total block width: icon (12px) + gap (6px) + text */
				int total_w = 12 + 6 + text_width(guide_lines[li], 1);
				int start_x = ox + (ow - total_w) / 2; /* centre the icon+text block */
				int ix = start_x;
				int iy = ty - 1; /* icon y: aligned with the current text baseline */

				/* Outer door frame, dark brown */
				draw_rectangle(ix, iy, 12, 18, 0x3D1F00);

				/* Inner door panel, lighter brown */
				draw_rectangle(ix + 2, iy + 2, 8, 14, 0x8B5A2B);

				/* Top decorative panel inset */
				draw_rectangle(ix + 3, iy + 3, 6, 4, 0x6B3A1F);

				/* Bottom decorative panel inset */
				draw_rectangle(ix + 3, iy + 9, 6, 6, 0x6B3A1F);

				/* Gold door handle on the right side of the inner panel */
				draw_rectangle(ix + 8, iy + 10, 2, 2, 0xFFD700);

				/* Render guide text immediately to the right of the icon */
				draw_text(start_x + 12 + 6, ty, guide_lines[li], 0xCCCCCC, 1);
				ty += 20;
				continue;
			}

			/* Car icon on line 11: "GET IN YOUR CAR AND DRIVE TO SAFETY"
			 * Renders a small side-view car with a red body, cyan windows,
			 * and dark wheels, consistent with the car sprites used in level 2.
			 */
			if (li == 11) {
				/* Calculate total block width: icon (22px) + gap (6px) + text */
				int total_w = 22 + 6 + text_width(guide_lines[li], 1);
				int start_x = ox + (ow - total_w) / 2; /* centre the icon+text block */
				int ix = start_x;
				int iy = ty - 1; /* icon y: aligned with the current text baseline */

				/* Car roof, slightly lighter red */
				draw_rectangle(ix + 3, iy, 14, 5, 0xFF6666);

				/* Car body, main red */
				draw_rectangle(ix, iy + 4, 22, 6, 0xFF4444);

				/* Left window, cyan tint */
				draw_rectangle(ix + 5, iy + 1, 4, 3, 0x99CCFF);

				/* Right window, cyan tint */
				draw_rectangle(ix + 11, iy + 1, 4, 3, 0x99CCFF);

				/* Left wheel, dark rubber */
				draw_rectangle(ix + 2, iy + 9, 4, 4, 0x222222);

				/* Right wheel, dark rubber */
				draw_rectangle(ix + 16, iy + 9, 4, 4, 0x222222);

				/* Left wheel hub highlight */
				draw_rectangle(ix + 3, iy + 10, 2, 2, 0x666666);

				/* Right wheel hub highlight */
				draw_rectangle(ix + 17, iy + 10, 2, 2, 0x666666);

				/* Render guide text immediately to the right of the icon */
				draw_text(start_x + 22 + 6, ty, guide_lines[li], 0xCCCCCC, 1);
				ty += 20;
				continue;
			}

			/* All other lines are rendered centred with no icon */
			draw_text_centred(ox, ow, ty, guide_lines[li], 0xCCCCCC, 1);
			ty += 20;
		}

		int b1y = oy + oh - 60 - btn_h;
		int b2y = b1y + btn_h + btn_gap;
		DRAW_BTN_MENU(btn_x, b1y, btn_w, btn_h, "PLAY", g->menu_selected == 0);
		DRAW_BTN_MENU(btn_x, b2y, btn_w, btn_h, "BACK", g->menu_selected == 1);

	} else if (g->state == STATE_WIN) {
		draw_text_centred(ox, ow, oy + 14, "YOU WIN!", 0x44FF88, 3);
		if (g->current_level == NUM_LEVELS - 1)
			draw_text_centred(ox, ow, oy + 50, "YOU ESCAPED IN THE CAR", 0x8FB8FF, 2);
		else
			draw_text_centred(ox, ow, oy + 50, "LEVEL COMPLETE", 0x8FB8FF, 2);
		int div_y = oy + 72;
		draw_rectangle(ox, div_y, ow, 2, 0x8FB8FF);
		draw_text_centred(ox, ow, div_y + 20, "FINAL SCORE", 0x8FB8FF, 2);
		char sbuf[16];
		int sv = g->score, sp = 0;
		if (sv == 0) {
			sbuf[sp++] = '0';
		} else {
			char tmp[8];
			int n = 0;
			while (sv > 0) {
				tmp[n++] = (char)('0' + sv % 10);
				sv /= 10;
			}
			while (n > 0)
				sbuf[sp++] = tmp[--n];
		}
		sbuf[sp] = '\0';
		draw_text_centred(ox, ow, div_y + 46, sbuf, 0xFFFFFF, 4);
		draw_text_centred(ox, ow, div_y + 110, "PRESS ENTER OR CLICK TO RETRY", 0xB8B800, 2);

	} else if (g->state == STATE_LOSE) {
		draw_text_centred(ox, ow, oy + 14, "BUSTED!", 0xFF4444, 3);
		draw_text_centred(ox, ow, oy + 50,
								g->lose_timeout ? "TIME RAN OUT" : "CAUGHT BY A TEACHER", 0xFF6666, 2);
		int div_y = oy + 72;
		draw_rectangle(ox, div_y, ow, 2, 0xFF4444);
		draw_text_centred(ox, ow, div_y + 70, "PRESS ENTER OR CLICK TO RETRY", 0xB8B800, 2);
	}

#undef DRAW_BTN_MENU
}

static void render_vision_cones(const game_t* g) {
	static const int fdc[] = {0, 1, 0, -1};
	static const int fdr[] = {-1, 0, 1, 0};
	for (int i = 0; i < g->num_guards; i++) {
		const guard_t* guard = &g->guards[i];
		uint32_t cone_color = (guard->alert == ALERT_DETECTED)
										  ? (g->current_level == 1 ? 0x550000 : 0xAA0000)
										  : (g->current_level == 1 ? 0x555500 : 0x997700);
		int fd = guard->direction;
		for (int step = 1; step <= guard->vision_range; step++) {
			int cc = guard->col + fdc[fd] * step;
			int cr = guard->row + fdr[fd] * step;
			if (cc < 0 || cc >= MAP_COLS || cr < 0 || cr >= MAP_ROWS)
				break;
			if (g->map[cr][cc] == TILE_WALL)
				break;
			/* Do not draw cone over the blue car exit in level 2 */
			if (g->current_level == 1 && g->map[cr][cc] == TILE_EXIT && cc != 0)
				continue;
			draw_rectangle(cc * TILE_SIZE + 2, cr * TILE_SIZE + 2, TILE_SIZE - 4, TILE_SIZE - 4, cone_color);
		}
	}
}

static void render_quiz_mcq(const game_t* g) {
	const quiz_entry_t* qe = current_question(g);
	int px = 60, py = 25, pw = 620, ph = 510;
	draw_rectangle(px, py, pw, ph, 0x0D1117);
	draw_rectangle(px, py, pw, 4, 0x8FB8FF);
	draw_rectangle(px, py + ph - 4, pw, 4, 0x8FB8FF);
	draw_rectangle(px, py, 4, ph, 0x8FB8FF);
	draw_rectangle(px + pw - 4, py, 4, ph, 0x8FB8FF);

	draw_text(px + 20, py + 15, "ANSWER TO GET THE TEST", 0xFFFFFF, 2);
	int step = (g->quiz_question < 3) ? g->quiz_question + 1 : 3;
	draw_text(px + 20, py + 42, "STEP", 0x8FB8FF, 2);
	draw_int_text(px + 78, py + 42, step, 0xFFFFFF, 2);
	draw_text(px + 94, py + 42, "OF 3", 0x8FB8FF, 2);

	/* Lives indicator */
	for (int i = 0; i < 3; i++) {
		uint32_t c = (i < g->quiz_lives) ? 0xFF3333 : 0x333333;
		draw_rectangle(px + pw - 80 + i * 26, py + 16, 20, 20, c);
		draw_rectangle(px + pw - 80 + i * 26 + 3, py + 13, 6, 4, c);
		draw_rectangle(px + pw - 80 + i * 26 + 11, py + 13, 6, 4, c);
	}

	draw_rectangle(px, py + 68, pw, 2, 0x8FB8FF);
	draw_text(px + 20, py + 82, qe->question, 0xF0C040, 2);
	draw_text(px + 20, py + 120, "PRESS 1-4 OR CLICK TO ANSWER", 0x556677, 2);

	static const char* labels[4] = {"1", "2", "3", "4"};
	for (int i = 0; i < 4; i++) {
		uint32_t btn_bg, txt_color;
		if (g->quiz_feedback != 0 && i == g->quiz_last_selected) {
			btn_bg = (g->quiz_feedback == 1) ? 0x0F3020 : 0x3A0A0A;
			txt_color = (g->quiz_feedback == 1) ? 0x44FF88 : 0xFF4444;
		} else if (g->quiz_feedback == 0 && i == g->quiz_hover) {
			btn_bg = 0x1C2E50;
			txt_color = 0xFFFFFF;
		} else {
			btn_bg = 0x111827;
			txt_color = 0xBBBBBB;
		}
		draw_rectangle(QUIZ_BTN_X, quiz_btn_y[i], QUIZ_BTN_W, QUIZ_BTN_H, btn_bg);
		draw_rectangle(QUIZ_BTN_X, quiz_btn_y[i], QUIZ_BTN_W, 2, 0x8FB8FF);
		draw_rectangle(QUIZ_BTN_X, quiz_btn_y[i] + QUIZ_BTN_H - 2, QUIZ_BTN_W, 2, 0x8FB8FF);
		draw_rectangle(QUIZ_BTN_X, quiz_btn_y[i], 2, QUIZ_BTN_H, 0x8FB8FF);
		draw_rectangle(QUIZ_BTN_X + QUIZ_BTN_W - 2, quiz_btn_y[i], 2, QUIZ_BTN_H, 0x8FB8FF);
		draw_text(QUIZ_BTN_X + 10, quiz_btn_y[i] + 14, labels[i], 0x8FB8FF, 2);
		draw_rectangle(QUIZ_BTN_X + 30, quiz_btn_y[i] + 6, 2, QUIZ_BTN_H - 12, 0x8FB8FF);
		draw_text(QUIZ_BTN_X + 40, quiz_btn_y[i] + 14, qe->options[i], txt_color, 2);
	}

	if (g->quiz_feedback == 1)
		draw_text(px + pw / 2 - 60, py + 430, "CORRECT", 0x44FF88, 3);
	else if (g->quiz_feedback == 2) {
		draw_text(px + pw / 2 - 50, py + 430, "WRONG", 0xFF4444, 3);
		draw_text(px + pw / 2 - (g->quiz_lives <= 0 ? 90 : 30), py + 462,
					 g->quiz_lives <= 0 ? "GAME OVER" : "TRY AGAIN",
					 g->quiz_lives <= 0 ? 0xFF4444 : 0xE6A817, 2);
	}

	draw_rectangle(g->mouse_x - 1, g->mouse_y - 9, 2, 18, 0xFFFFFF);
	draw_rectangle(g->mouse_x - 9, g->mouse_y - 1, 18, 2, 0xFFFFFF);
}

static void render_quiz_draw(const game_t* g) {
	int px = 60, py = 25, pw = 620, ph = 510;
	draw_rectangle(px, py, pw, ph, 0x0D1117);
	draw_rectangle(px, py, pw, 4, 0x8FB8FF);
	draw_rectangle(px, py + ph - 4, pw, 4, 0x8FB8FF);
	draw_rectangle(px, py, 4, ph, 0x8FB8FF);
	draw_rectangle(px + pw - 4, py, 4, ph, 0x8FB8FF);

	draw_text(px + 20, py + 15, "DRAW TO GET THE TEST", 0xFFFFFF, 2);
	draw_text(px + 20, py + 42, "STEP 3 OF 3", 0x8FB8FF, 2);

	for (int i = 0; i < 3; i++) {
		uint32_t c = (i < g->quiz_lives) ? 0xFF3333 : 0x333333;
		draw_rectangle(px + pw - 80 + i * 26, py + 16, 20, 20, c);
		draw_rectangle(px + pw - 80 + i * 26 + 3, py + 13, 6, 4, c);
		draw_rectangle(px + pw - 80 + i * 26 + 11, py + 13, 6, 4, c);
	}

	draw_rectangle(px, py + 68, pw, 2, 0x8FB8FF);
	draw_text(px + 20, py + 82, quiz_draw.question, 0xF0C040, 2);

	/* Canvas border and background */
	draw_rectangle(DRAW_CANVAS_X - 2, DRAW_CANVAS_Y - 2, DRAW_CANVAS_W + 4, DRAW_CANVAS_H + 4, 0x8FB8FF);
	draw_rectangle(DRAW_CANVAS_X, DRAW_CANVAS_Y, DRAW_CANVAS_W, DRAW_CANVAS_H, 0x111111);

	if (g->draw_ready_ticks > 0) {
		/* Instruction overlay shown at the start of the drawing step */
		/* text_width = num_chars * 6 * scale; DRAW_CANVAS_W = 340 */
		draw_text(DRAW_CANVAS_X + (340 - 8 * 6 * 3) / 2, DRAW_CANVAS_Y + 80, "DRAW BIG", 0xFF9900, 3);
		draw_text(DRAW_CANVAS_X + (340 - 12 * 6 * 3) / 2, DRAW_CANVAS_Y + 130, "AND CENTERED", 0xFF9900, 3);
		draw_text(DRAW_CANVAS_X + (340 - 10 * 6 * 3) / 2, DRAW_CANVAS_Y + 180, "IN THE BOX", 0xFF9900, 3);
	} else {
		for (int r = 0; r < DRAW_ROWS; r++)
			for (int c = 0; c < DRAW_COLS; c++)
				if (g->draw_canvas[r][c])
					draw_rectangle(DRAW_CANVAS_X + c * DRAW_CELL_SIZE,
										DRAW_CANVAS_Y + r * DRAW_CELL_SIZE,
										DRAW_CELL_SIZE, DRAW_CELL_SIZE, 0xFFFFFF);
	}

	/* Clear button */
	uint32_t clr_bg = g->draw_hover_clear ? 0x3A1010 : 0x1A0808;
	draw_rectangle(DRAW_BTN_CLEAR_X, DRAW_BTN_CLEAR_Y, DRAW_BTN_W, DRAW_BTN_H, clr_bg);
	draw_rectangle(DRAW_BTN_CLEAR_X, DRAW_BTN_CLEAR_Y, DRAW_BTN_W, 2, 0xFF4444);
	draw_rectangle(DRAW_BTN_CLEAR_X, DRAW_BTN_CLEAR_Y + DRAW_BTN_H - 2, DRAW_BTN_W, 2, 0xFF4444);
	draw_rectangle(DRAW_BTN_CLEAR_X, DRAW_BTN_CLEAR_Y, 2, DRAW_BTN_H, 0xFF4444);
	draw_rectangle(DRAW_BTN_CLEAR_X + DRAW_BTN_W - 2, DRAW_BTN_CLEAR_Y, 2, DRAW_BTN_H, 0xFF4444);
	draw_text(DRAW_BTN_CLEAR_X + 44, DRAW_BTN_CLEAR_Y + 13, "CLEAR", 0xFF4444, 2);

	/* Check button */
	uint32_t chk_bg = g->draw_hover_check ? 0x103A10 : 0x081A08;
	draw_rectangle(DRAW_BTN_CHECK_X, DRAW_BTN_CHECK_Y, DRAW_BTN_W, DRAW_BTN_H, chk_bg);
	draw_rectangle(DRAW_BTN_CHECK_X, DRAW_BTN_CHECK_Y, DRAW_BTN_W, 2, 0x44FF88);
	draw_rectangle(DRAW_BTN_CHECK_X, DRAW_BTN_CHECK_Y + DRAW_BTN_H - 2, DRAW_BTN_W, 2, 0x44FF88);
	draw_rectangle(DRAW_BTN_CHECK_X, DRAW_BTN_CHECK_Y, 2, DRAW_BTN_H, 0x44FF88);
	draw_rectangle(DRAW_BTN_CHECK_X + DRAW_BTN_W - 2, DRAW_BTN_CHECK_Y, 2, DRAW_BTN_H, 0x44FF88);
	draw_text(DRAW_BTN_CHECK_X + 44, DRAW_BTN_CHECK_Y + 13, "CHECK", 0x44FF88, 2);

	if (g->draw_submitted) {
		if (g->quiz_feedback == 1)
			draw_text(DRAW_CANVAS_X + DRAW_CANVAS_W / 2 - 60,
						 DRAW_CANVAS_Y + DRAW_CANVAS_H / 2 - 10, "CORRECT", 0x44FF88, 3);
		else if (g->quiz_feedback == 2) {
			draw_text(DRAW_CANVAS_X + DRAW_CANVAS_W / 2 - 50,
						 DRAW_CANVAS_Y + DRAW_CANVAS_H / 2 - 10, "WRONG", 0xFF4444, 3);
			draw_text(DRAW_CANVAS_X + DRAW_CANVAS_W / 2 - (g->quiz_lives > 0 ? 30 : 90),
						 DRAW_CANVAS_Y + DRAW_CANVAS_H / 2 + 20,
						 g->quiz_lives > 0 ? "TRY AGAIN" : "GAME OVER",
						 g->quiz_lives > 0 ? 0xE6A817 : 0xFF4444, 2);
		}
	}

	draw_rectangle(g->mouse_x - 1, g->mouse_y - 9, 2, 18, 0xFFFFFF);
	draw_rectangle(g->mouse_x - 9, g->mouse_y - 1, 18, 2, 0xFFFFFF);
}

/* Custom pointer cursor drawn directly into the framebuffer */
static void render_cursor(const game_t* g) {
	uint16_t x = (uint16_t)g->mouse_x, y = (uint16_t)g->mouse_y;
	/* Outline */
	draw_hline(x, y, 1, 0x000000);
	draw_hline(x, (uint16_t)(y + 1), 2, 0x000000);
	draw_hline(x, (uint16_t)(y + 2), 3, 0x000000);
	draw_hline(x, (uint16_t)(y + 3), 4, 0x000000);
	draw_hline(x, (uint16_t)(y + 4), 5, 0x000000);
	draw_hline(x, (uint16_t)(y + 5), 6, 0x000000);
	draw_hline(x, (uint16_t)(y + 6), 7, 0x000000);
	draw_hline(x, (uint16_t)(y + 7), 8, 0x000000);
	draw_hline(x, (uint16_t)(y + 8), 9, 0x000000);
	draw_hline(x, (uint16_t)(y + 9), 10, 0x000000);
	draw_hline(x, (uint16_t)(y + 10), 7, 0x000000);
	draw_hline((uint16_t)(x + 3), (uint16_t)(y + 11), 6, 0x000000);
	draw_hline((uint16_t)(x + 3), (uint16_t)(y + 12), 6, 0x000000);
	draw_hline((uint16_t)(x + 6), (uint16_t)(y + 13), 3, 0x000000);
	draw_hline((uint16_t)(x + 6), (uint16_t)(y + 14), 3, 0x000000);
	/* Fill */
	draw_hline((uint16_t)(x + 1), (uint16_t)(y + 2), 1, 0xFFFFFF);
	draw_hline((uint16_t)(x + 1), (uint16_t)(y + 3), 2, 0xFFFFFF);
	draw_hline((uint16_t)(x + 1), (uint16_t)(y + 4), 3, 0xFFFFFF);
	draw_hline((uint16_t)(x + 1), (uint16_t)(y + 5), 4, 0xFFFFFF);
	draw_hline((uint16_t)(x + 1), (uint16_t)(y + 6), 5, 0xFFFFFF);
	draw_hline((uint16_t)(x + 1), (uint16_t)(y + 7), 6, 0xFFFFFF);
	draw_hline((uint16_t)(x + 1), (uint16_t)(y + 8), 7, 0xFFFFFF);
	draw_hline((uint16_t)(x + 1), (uint16_t)(y + 9), 5, 0xFFFFFF);
	draw_hline((uint16_t)(x + 1), (uint16_t)(y + 10), 5, 0xFFFFFF);
	draw_hline((uint16_t)(x + 4), (uint16_t)(y + 11), 4, 0xFFFFFF);
	draw_hline((uint16_t)(x + 4), (uint16_t)(y + 12), 4, 0xFFFFFF);
	draw_hline((uint16_t)(x + 7), (uint16_t)(y + 13), 1, 0xFFFFFF);
	draw_hline((uint16_t)(x + 7), (uint16_t)(y + 14), 1, 0xFFFFFF);
}

/* HUD strip rendered below the map tile area */
static void render_hud(const game_t* g) {
	int hy = MAP_ROWS * TILE_SIZE + 4;
	draw_rectangle(0, MAP_ROWS * TILE_SIZE, 800, 40, 0x000000);

	draw_text(4, hy, "TIME", 0x8FB8FF, 2);
	draw_number(64, hy, (int)g->countdown_sec, 0xFFFFFF);

	/* Show animated exam sprite in the HUD when the player carries the exam */
	if (exam_pixmap && g->player.has_exam) {
		uint8_t* ep = exam_frame && exam_pixmap2 ? exam_pixmap2 : exam_pixmap;
		xpm_image_t ei = exam_frame && exam_pixmap2 ? exam_img2 : exam_img;
		draw_xpm_scaled(ep, ei, 130, MAP_ROWS * TILE_SIZE, SPRITE_SCALE);
	} else {
		draw_rectangle(130, hy, 14, 14, g->player.has_exam ? COLOR_EXAM : 0x333333);
	}

	draw_text(170, hy, "SCORE", 0x8FB8FF, 2);
	draw_int_text(250, hy, g->score, 0xFFFFFF, 2);

	draw_text(340, hy, "LVL", 0x8FB8FF, 2);
	draw_int_text(382, hy, g->current_level + 1, 0xFFFFFF, 2);

	/* RTC date and time */
	draw_text(410, hy, "DATE", 0x8FB8FF, 2);
	draw_int_text(460, hy, g->rtc_day, 0xFFFFFF, 2);
	draw_text(478, hy, "/", 0xFFFFFF, 2);
	draw_int_text(490, hy, g->rtc_month, 0xFFFFFF, 2);
	draw_text(508, hy, "/", 0xFFFFFF, 2);
	draw_int_text(520, hy, 2000 + g->rtc_year, 0xFFFFFF, 2);
	draw_int_text(580, hy, g->rtc_hour, 0xFFFFFF, 2);
	draw_text(598, hy, ":", 0xFFFFFF, 2);
	draw_int_text(610, hy, g->rtc_minute, 0xFFFFFF, 2);
	draw_text(628, hy, ":", 0xFFFFFF, 2);
	draw_int_text(640, hy, g->rtc_second, 0xFFFFFF, 2);
}

/* Tinted overlay used to signal the lose state */
static void render_overlay(uint32_t color) {
	int ox = MAP_COLS * TILE_SIZE / 4;
	int oy = MAP_ROWS * TILE_SIZE / 4;
	int ow = MAP_COLS * TILE_SIZE / 2;
	int oh = MAP_ROWS * TILE_SIZE / 2;
	draw_rectangle(ox, oy, ow, oh, color);
}

static void draw_key(int kx, int ky, int kw, int kh, const char* label, uint32_t bg, uint32_t fg, int scale) {
	draw_rectangle(kx + 2, ky + 2, kw, kh, 0x000000);
	draw_rectangle(kx, ky, kw, kh, bg);
	draw_rectangle(kx, ky, kw, 2, 0x555555);
	draw_rectangle(kx, ky, kw, 1, 0x888888);
	draw_rectangle(kx, ky + kh - 1, kw, 1, 0x333333);
	draw_rectangle(kx, ky, 1, kh, 0x888888);
	draw_rectangle(kx + kw - 1, ky, 1, kh, 0x333333);
	int lw = text_width(label, scale), lh = 7 * scale;
	draw_text(kx + (kw - lw) / 2, ky + (kh - lh) / 2, label, fg, scale);
}

static void draw_arrow_key(int kx, int ky, int kw, int kh, int dir) {
	draw_rectangle(kx + 2, ky + 2, kw, kh, 0x000000);
	draw_rectangle(kx, ky, kw, kh, 0x333333);
	draw_rectangle(kx, ky, kw, 2, 0x555555);
	draw_rectangle(kx, ky, kw, 1, 0x888888);
	draw_rectangle(kx, ky + kh - 1, kw, 1, 0x333333);
	draw_rectangle(kx, ky, 1, kh, 0x888888);
	draw_rectangle(kx + kw - 1, ky, 1, kh, 0x333333);
	int cx = kx + kw / 2, cy = ky + kh / 2;
	if (dir == 0)
		for (int i = 0; i < 5; i++)
			draw_rectangle(cx - i, cy - 4 + i, i * 2 + 1, 2, 0xFFFFFF);
	else if (dir == 1)
		for (int i = 0; i < 5; i++)
			draw_rectangle(cx - (4 - i), cy + i, (4 - i) * 2 + 1, 2, 0xFFFFFF);
	else if (dir == 2)
		for (int i = 0; i < 5; i++)
			draw_rectangle(cx - 4 + i, cy - i, 2, i * 2 + 1, 0xFFFFFF);
	else
		for (int i = 0; i < 5; i++)
			draw_rectangle(cx + i, cy - (4 - i), 2, (4 - i) * 2 + 1, 0xFFFFFF);
}

void game_render_controls(const game_t* g) {
	draw_rectangle(0, 0, 800, 600, 0x000000);
	int ox = 140, oy = 90, ow = 520, oh = 420;
	draw_rectangle(ox, oy, ow, oh, 0x0A0F14);
	draw_rectangle(ox, oy, ow, 3, 0x8FB8FF);
	draw_rectangle(ox, oy + oh - 3, ow, 3, 0x8FB8FF);
	draw_rectangle(ox, oy, 3, oh, 0x8FB8FF);
	draw_rectangle(ox + ow - 3, oy, 3, oh, 0x8FB8FF);

	draw_text_centred(ox, ow, oy + 14, "CONTROLS", 0xFFFFFF, 3);
	int div_y = oy + 62;
	draw_rectangle(ox, div_y, ow, 2, 0x8FB8FF);

	/* MOVE PLAYER */
	int sec_y = div_y + 18;
	draw_text_centred(ox, ow, sec_y, "MOVE PLAYER", 0x8FB8FF, 2);
	sec_y += 24;

	int kw = 32, kh = 32, kg = 4;

	int cluster_w = kw * 3 + kg * 2;

	int total_w = cluster_w + 30 + cluster_w;
	int left_start = ox + (ow - total_w) / 2;
	int right_start = left_start + cluster_w + 30;

	draw_key(left_start + kw + kg, sec_y, kw, kh, "W", 0x1C2E50, 0xFFFFFF, 2);
	draw_key(left_start, sec_y + kh + kg, kw, kh, "A", 0x1C2E50, 0xFFFFFF, 2);
	draw_key(left_start + kw + kg, sec_y + kh + kg, kw, kh, "S", 0x1C2E50, 0xFFFFFF, 2);
	draw_key(left_start + 2 * (kw + kg), sec_y + kh + kg, kw, kh, "D", 0x1C2E50, 0xFFFFFF, 2);

	int or_x = left_start + cluster_w + (30 - text_width("OR", 2)) / 2;
	draw_text(or_x, sec_y + kh / 2, "OR", 0x556677, 2);

	draw_arrow_key(right_start + kw + kg, sec_y, kw, kh, 0);						 /* up    */
	draw_arrow_key(right_start, sec_y + kh + kg, kw, kh, 2);						 /* left  */
	draw_arrow_key(right_start + kw + kg, sec_y + kh + kg, kw, kh, 1);		 /* down  */
	draw_arrow_key(right_start + 2 * (kw + kg), sec_y + kh + kg, kw, kh, 3); /* right */

	/* OTHERS */
	int act_y = sec_y + kh * 2 + kg + 24;
	draw_rectangle(ox + 20, act_y - 8, ow - 40, 1, 0x223344);
	act_y += 6;
	draw_text_centred(ox, ow, act_y, "OTHERS", 0x8FB8FF, 2);
	act_y += 22;

	static const struct {
		const char* key;
		const char* desc;
	} actions[] = {
		 {"P", "PAUSE / UNPAUSE"},
		 {"ENTER", "CONFIRM / SELECT"},
		 {"ESC", "CLEAR DRAWING"},
		 {"1-4", "QUIZ ANSWERS"},
	};
	int col_x[2] = {ox + 30, ox + ow / 2 + 10};
	for (int i = 0; i < 4; i++) {
		int col = i % 2, row = i / 2;
		int kx2 = col_x[col], ky2 = act_y + row * 44;
		int ekw = (actions[i].key[1] && actions[i].key[1] != ' ') ? 52 : 32;
		draw_key(kx2, ky2, ekw, 28, actions[i].key, 0x1C2E50, 0xFFFFFF, 1);
		draw_text(kx2 + ekw + 10, ky2 + 8, actions[i].desc, 0xCCCCCC, 1);
	}

	int btn_w = 260, btn_h = 44;
	int btn_x = ox + (ow - btn_w) / 2;
	int btn_y = oy + oh - btn_h - 18;

	uint32_t C_BTN_IDLE = 0x111111, C_BTN_SEL = 0x1C2E50, C_BTN_HOVER = 0x223A60;
	uint32_t C_BTN_BORD = 0x8FB8FF, C_TXT_IDLE = 0xCCCCCC, C_TXT_SEL = 0xFFFFFF;

	bool mh = (g->mouse_x >= btn_x && g->mouse_x < btn_x + btn_w && g->mouse_y >= btn_y && g->mouse_y < btn_y + btn_h);
	bool act = (g->menu_selected == 0);
	uint32_t _bg = act ? C_BTN_SEL : (mh ? C_BTN_HOVER : C_BTN_IDLE);
	uint32_t _fg = (act || mh) ? C_TXT_SEL : C_TXT_IDLE;
	draw_rectangle(btn_x, btn_y, btn_w, btn_h, _bg);
	draw_rectangle(btn_x, btn_y, btn_w, 2, C_BTN_BORD);
	draw_rectangle(btn_x, btn_y + btn_h - 2, btn_w, 2, C_BTN_BORD);
	draw_rectangle(btn_x, btn_y, 2, btn_h, C_BTN_BORD);
	draw_rectangle(btn_x + btn_w - 2, btn_y, 2, btn_h, C_BTN_BORD);
	if (act)
		draw_rectangle(btn_x, btn_y, 3, btn_h, 0x4AB0E6);
	draw_text_centred(btn_x, btn_w, btn_y + (btn_h - 7 * 2) / 2, "BACK", _fg, 2);

	draw_rectangle(g->mouse_x - 1, g->mouse_y - 9, 2, 18, 0xFFFFFF);
	draw_rectangle(g->mouse_x - 9, g->mouse_y - 1, 18, 2, 0xFFFFFF);
}

void game_render(const game_t* g) {
	if (g->state == STATE_LEADERBOARD) {
		render_leaderboard(g);
		return;
	}
	if (g->state == STATE_CONTROLS) {
		game_render_controls(g);
		return;
	}

	/* Clear side and bottom margins */
	draw_rectangle(740, 0, 60, 600, 0x000000);
	draw_rectangle(0, 587, 800, 13, 0x000000);

	/* Tile layer */
	for (int r = 0; r < MAP_ROWS; r++) {
		for (int c = 0; c < MAP_COLS; c++) {
			int wx = c * TILE_SIZE, wy = r * TILE_SIZE, ts = TILE_SIZE;

			if (g->current_level == 0) {
				/* Indoor dungeon style: grey brick walls */
				if (g->map[r][c] == TILE_WALL) {
					draw_rectangle(wx, wy, ts, ts, 0x444444);
					draw_rectangle(wx, wy, ts, 1, 0x222222);
					draw_rectangle(wx, wy + ts / 2, ts, 1, 0x222222);
					draw_rectangle(wx, wy + ts - 1, ts, 1, 0x222222);
					if (r % 2 == 0) {
						draw_rectangle(wx + ts / 2, wy, 1, ts / 2, 0x222222);
						draw_rectangle(wx, wy + ts / 2, 1, ts / 2, 0x222222);
						draw_rectangle(wx + ts - 1, wy + ts / 2, 1, ts / 2, 0x222222);
					} else {
						draw_rectangle(wx, wy, 1, ts / 2, 0x222222);
						draw_rectangle(wx + ts - 1, wy, 1, ts / 2, 0x222222);
						draw_rectangle(wx + ts / 2, wy + ts / 2, 1, ts / 2, 0x222222);
					}
				} else if (g->map[r][c] == TILE_EXIT) {
					draw_rectangle(wx, wy, ts, ts, COLOR_EXIT_OFF);
				} else {
					draw_rectangle(wx, wy, ts, ts, COLOR_FLOOR);
				}
			} else {
				/* Outdoor style: red-brick walls, sky-blue floor with grass strip */
				if (g->map[r][c] == TILE_WALL) {
					uint32_t base = (r % 2 == 0) ? 0x8B4513 : 0x9A4E1A;
					draw_rectangle(wx, wy, ts, ts, base);
					draw_rectangle(wx, wy, ts, 2, 0x5C2E0A);
					draw_rectangle(wx, wy + ts - 2, ts, 2, 0x5C2E0A);
					if (r % 2 == 0) {
						draw_rectangle(wx + ts / 2, wy, 2, ts, 0x5C2E0A);
					} else {
						if (c % 2 == 0)
							draw_rectangle(wx, wy, 2, ts, 0x5C2E0A);
						draw_rectangle(wx + ts - 2, wy, 2, ts, 0x5C2E0A);
					}
					draw_rectangle(wx + 2, wy + 2, ts - 4, 2, 0xBB6633);
				} else if (g->map[r][c] == TILE_EXIT) {
					if (c == 0) {
						/* Left entry ΓÇö brown door */
						draw_rectangle(wx, wy, ts, ts, 0x6B3A1F);
						draw_rectangle(wx, wy, ts, 3, 0x3D1F00);
						draw_rectangle(wx, wy + ts - 3, ts, 3, 0x3D1F00);
						draw_rectangle(wx, wy, 3, ts, 0x3D1F00);
						draw_rectangle(wx + ts - 3, wy, 3, ts, 0x3D1F00);
						draw_rectangle(wx + 5, wy + 5, ts - 10, ts - 10, 0x8B5A2B);
						draw_rectangle(wx + ts - 10, wy + ts / 2 - 3, 5, 6, 0xFFD700);
					} else {
						/* Right car exit ΓÇö blue door */
						draw_rectangle(wx, wy, ts, ts, 0x1A3A6A);
						draw_rectangle(wx, wy, ts, 2, 0x4A7ACA);
						draw_rectangle(wx, wy + ts - 2, ts, 2, 0x4A7ACA);
						draw_rectangle(wx, wy, 2, ts, 0x4A7ACA);
						draw_rectangle(wx + ts - 2, wy, 2, ts, 0x4A7ACA);
						uint32_t hcol = exam_frame ? 0xFFFFFF : 0xFFFF44;
						draw_rectangle(wx + ts - 8, wy + ts / 2 - 5, 6, 4, hcol);
						draw_rectangle(wx + ts - 8, wy + ts / 2 + 1, 6, 4, hcol);
					}
				} else {
					draw_rectangle(wx, wy, ts, ts, 0x87CEEB);
					if (r == MAP_ROWS - 2) {
						draw_rectangle(wx, wy + ts - 4, ts, 4, 0x2E7D32);
						draw_rectangle(wx, wy + ts - 6, ts, 2, 0x388E3C);
					}
					if ((r + c) % 4 == 0)
						draw_rectangle(wx + 4, wy + 4, ts - 8, 1, 0xA8D8EA);
					if ((r + c) % 5 == 0)
						draw_rectangle(wx + 6, wy + 8, ts - 12, 1, 0xA8D8EA);
				}
			}
		}
	}

	/* Briefly mark guard spawn locations at the start of each level */
	if (g->elapsed_ticks < 2 * TICKS_PER_SEC)
		for (int r = 0; r < MAP_ROWS; r++)
			for (int c = 0; c < MAP_COLS; c++)
				if (g->map[r][c] == TILE_SPAWN) {
					draw_rectangle(c * TILE_SIZE + 4, r * TILE_SIZE + TILE_SIZE / 2 - 2, TILE_SIZE - 8, 4, 0x880000);
					draw_rectangle(c * TILE_SIZE + TILE_SIZE / 2 - 2, r * TILE_SIZE + 4, 4, TILE_SIZE - 8, 0x880000);
				}

	render_vision_cones(g);

	/* Entry door at top-left */
	if (g->current_level == 0) {
		int dx = 0, dy = TILE_SIZE, ts = TILE_SIZE;
		draw_rectangle(dx, dy, ts, ts, 0x6B3A1F);
		draw_rectangle(dx, dy, ts, 3, 0x3D1F00);
		draw_rectangle(dx, dy + ts - 3, ts, 3, 0x3D1F00);
		draw_rectangle(dx, dy, 3, ts, 0x3D1F00);
		draw_rectangle(dx + ts - 3, dy, 3, ts, 0x3D1F00);
		draw_rectangle(dx + 5, dy + 5, ts - 10, ts - 10, 0x8B5A2B);
		draw_rectangle(dx + ts - 10, dy + ts / 2 - 3, 5, 6, 0xFFD700);
	}

	/* Exam / car sprites on TILE_EXAM tiles */
	for (int r = 0; r < MAP_ROWS; r++)
		for (int c = 0; c < MAP_COLS; c++)
			if (g->map[r][c] == TILE_EXAM) {
				int wx = c * TILE_SIZE, wy = r * TILE_SIZE, ts = TILE_SIZE;
				if (g->current_level == 1) {
					draw_rectangle(wx, wy, ts, ts, 0x87CEEB);
					draw_rectangle(wx, wy + ts - 4, ts, 4, 0x2E7D32);
					uint8_t* cp = exam_frame && car_pixmap2 ? car_pixmap2 : car_pixmap1;
					xpm_image_t ci = exam_frame && car_pixmap2 ? car_img2 : car_img1;
					if (cp) {
						draw_xpm_scaled(cp, ci,
											 (uint16_t)(wx + (ts - ci.width * SPRITE_SCALE) / 2),
											 (uint16_t)(wy + ts - ci.height * SPRITE_SCALE), SPRITE_SCALE);
					} else {
						draw_rectangle(wx + 2, wy + ts / 2, ts - 4, ts / 2 - 4, 0xCC2200);
					}
				} else {
					uint8_t* ep = exam_frame && exam_pixmap2 ? exam_pixmap2 : exam_pixmap;
					xpm_image_t ei = exam_frame && exam_pixmap2 ? exam_img2 : exam_img;
					if (ep)
						draw_xpm_scaled(ep, ei,
											 (uint16_t)(wx + (ts - ei.width * SPRITE_SCALE) / 2),
											 (uint16_t)(wy + (ts - ei.height * SPRITE_SCALE) / 2), SPRITE_SCALE);
					else
						draw_rectangle(wx + 2, wy + 2, ts - 4, ts - 4, COLOR_EXAM);
				}
			}

	/* Render the car at the player's position when driving */
	if (g->player.in_car && g->current_level == 1) {
		int wx = g->player.col * TILE_SIZE, wy = g->player.row * TILE_SIZE, ts = TILE_SIZE;
		draw_rectangle(wx, wy, ts, ts, 0x87CEEB);
		draw_rectangle(wx, wy + ts - 6, ts, 6, 0x2E7D32);
		uint8_t* cp = exam_frame && car_pixmap2 ? car_pixmap2 : car_pixmap1;
		xpm_image_t ci = exam_frame && car_pixmap2 ? car_img2 : car_img1;
		if (cp)
			draw_xpm_scaled(cp, ci,
								 (uint16_t)(wx + (ts - ci.width * SPRITE_SCALE) / 2),
								 (uint16_t)(wy + ts - ci.height * SPRITE_SCALE), SPRITE_SCALE);
		if (exam_frame)
			draw_text(wx - 10, wy - 14, "DRIVE TO EXIT", 0xFFFF44, 1);
	}

	/* Guard sprites */
	for (int i = 0; i < g->num_guards; i++) {
		bool use_dumb = (i % 2) == 1;
		xpm_image_t ref_img = use_dumb ? guard_alert_img : guard_calm_img;
		uint16_t gx = (uint16_t)(g->guards[i].col * TILE_SIZE + (TILE_SIZE - ref_img.width * SPRITE_SCALE) / 2);
		uint16_t gy = (uint16_t)(g->guards[i].row * TILE_SIZE + (TILE_SIZE - ref_img.height * SPRITE_SCALE) / 2);
		if (use_dumb) {
			uint8_t* ap = guard_frame && guard_alert_pixmap2 ? guard_alert_pixmap2 : guard_alert_pixmap;
			xpm_image_t ai = guard_frame && guard_alert_pixmap2 ? guard_alert_img2 : guard_alert_img;
			if (ap)
				draw_xpm_scaled(ap, ai, gx, gy, SPRITE_SCALE);
			else
				draw_rectangle(gx + 4, gy + 4, TILE_SIZE - 8, TILE_SIZE - 8, 0xCC2222);
		} else {
			uint8_t* cp = guard_frame && guard_calm_pixmap2 ? guard_calm_pixmap2 : guard_calm_pixmap;
			xpm_image_t ci = guard_frame && guard_calm_pixmap2 ? guard_calm_img2 : guard_calm_img;
			if (cp)
				draw_xpm_scaled(cp, ci, gx, gy, SPRITE_SCALE);
			else
				draw_rectangle(gx + 4, gy + 4, TILE_SIZE - 8, TILE_SIZE - 8, 0x3366CC);
		}
		if (guards_panicking)
			draw_text(gx - 4, gy - 14, "OH NO!", 0xFF4444, 1);
	}

	/* Student sprite (hidden when inside the car) */
	if (!g->player.in_car) {
		uint8_t* pp;
		xpm_image_t pi;
		if (g->player.has_exam && player_final_pixmap) {
			pp = player_final_pixmap;
			pi = player_final_img;
		} else {
			pp = guard_frame && player_pixmap2 ? player_pixmap2 : player_pixmap;
			pi = guard_frame && player_pixmap2 ? player_img2 : player_img;
		}
		if (pp) {
			uint16_t sx = (uint16_t)(g->player.col * TILE_SIZE + (TILE_SIZE - pi.width * SPRITE_SCALE) / 2);
			uint16_t sy = (uint16_t)(g->player.row * TILE_SIZE + (TILE_SIZE - pi.height * SPRITE_SCALE) / 2);
			draw_xpm_scaled(pp, pi, sx, sy, SPRITE_SCALE);
		}
	}

	render_hud(g);

	if (g->state == STATE_LOSE)
		render_overlay(0xAA2222);

	render_state_overlay(g);

	if (g->state == STATE_QUIZ) {
		const quiz_entry_t* qe = current_question(g);
		if (qe->type == QUIZ_TYPE_DRAW)
			render_quiz_draw(g);
		else
			render_quiz_mcq(g);
	}

	render_cursor(g);
}
