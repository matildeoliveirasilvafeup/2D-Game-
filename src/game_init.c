#include "game_internal.h"
#include <lcom/lcf.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "rtc.h"
#include "../Sprites/XPMs/Car_1.xpm"
#include "../Sprites/XPMs/Car_2.xpm"
#include "../Sprites/XPMs/Test.xpm"
#include "../Sprites/XPMs/Test_2.xpm"
#include "../Sprites/XPMs/Dumb_Teacher.xpm"
#include "../Sprites/XPMs/Dumb_Teacher_2.xpm"
#include "../Sprites/XPMs/Smart_Teacher.xpm"
#include "../Sprites/XPMs/Smart_Teacher_2.xpm"
#include "../Sprites/XPMs/Student.xpm"
#include "../Sprites/XPMs/StudentFinal.xpm"
#include "../Sprites/XPMs/Student_2.xpm"

void load_level(game_t *g, int level) {
    if (level >= NUM_LEVELS) level = NUM_LEVELS - 1;
    g->current_level = level;
    memcpy(g->map, all_maps[level], sizeof(g->map));

    g->countdown_sec = GAME_TIME_SEC - level * 20;

    g->player.col      = 1;
    g->player.row      = 1;
    g->player.detected = false;
    g->player.has_exam = false;

    g->num_guards = 0;
    for (int r = 0; r < MAP_ROWS && g->num_guards < 8; r++)
        for (int c = 0; c < MAP_COLS && g->num_guards < 8; c++)
            if (g->map[r][c] == TILE_SPAWN) {
					int init_dir;
					if (r <= 2)
						init_dir = 2;
					else if (r >= MAP_ROWS - 3)
						init_dir = 0;
					else if (c <= 2)
						init_dir = 1;
					else if (c >= MAP_COLS - 3)
						init_dir = 3;
					else
						init_dir = (g->num_guards % 4);

					g->guards[g->num_guards] = (guard_t){
						 .col = c,
						 .row = r,
						 .direction = init_dir,
						 .patrol_len = -1,
						 .patrol_idx = -1,
						 .vision_range = GUARD_VISION_RANGE,
						 .alert = ALERT_NONE,
						 .alert_ticks = 0};
					g->num_guards++;
            }
}

int game_init(game_t *g) {
    high_score_t saved[3];
    memcpy(saved, g->high_scores, sizeof(saved));
    memset(g, 0, sizeof(*g));
    memcpy(g->high_scores, saved, sizeof(saved));

    g->menu_selected   = 0;
    g->ticks           = 0;
    g->elapsed_ticks   = 0;
    g->score           = 0;
    g->mouse_x         = 20;
    g->mouse_y         = VIEW_HEIGHT / 2;
    g->mouse_left_down = false;
    g->lose_timeout    = false;

    load_level(g, 0);
    g->player.in_car = false;

#define LOAD_XPM(ptr, img, src) \
    if (!(ptr)) (ptr) = xpm_load((xpm_map_t)(src), XPM_8_8_8_8, &(img))

    LOAD_XPM(player_pixmap,       player_img,       student_xpm);
    LOAD_XPM(guard_calm_pixmap,   guard_calm_img,   smart_teacher_xpm);
    LOAD_XPM(guard_alert_pixmap,  guard_alert_img,  dumb_teacher_xpm);
    LOAD_XPM(exam_pixmap,         exam_img,         test_xpm);
    LOAD_XPM(guard_calm_pixmap2,  guard_calm_img2,  smart_teacher_2_xpm);
    LOAD_XPM(guard_alert_pixmap2, guard_alert_img2, dumb_teacher_2_xpm);
    LOAD_XPM(exam_pixmap2,        exam_img2,        test_2_xpm);
    LOAD_XPM(player_pixmap2,      player_img2,      student_2_xpm);
    LOAD_XPM(player_final_pixmap, player_final_img, student_final_xpm);
    LOAD_XPM(car_pixmap1,         car_img1,         car_1_xpm);
    LOAD_XPM(car_pixmap2,         car_img2,         car_2_xpm);

#undef LOAD_XPM

#define FALLBACK(ptr, img, fb_ptr, fb_img) \
    if (!(ptr)) { (ptr) = (fb_ptr); (img) = (fb_img); }

    FALLBACK(guard_calm_pixmap2,  guard_calm_img2,  player_pixmap, player_img)
    FALLBACK(guard_alert_pixmap2, guard_alert_img2, player_pixmap, player_img)
    FALLBACK(exam_pixmap2,        exam_img2,        player_pixmap, player_img)
    FALLBACK(player_pixmap2,      player_img2,      player_pixmap, player_img)
    FALLBACK(player_final_pixmap, player_final_img, player_pixmap, player_img)

#undef FALLBACK

    guards_panicking = false;
    guard_frame      = false;
    exam_frame       = false;

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

    g->state = STATE_MENU;
    return 0;
}

void game_cleanup(game_t *g) {
    (void)g;
    if (player_pixmap)       { free(player_pixmap);       player_pixmap       = NULL; }
    if (guard_calm_pixmap)   { free(guard_calm_pixmap);   guard_calm_pixmap   = NULL; }
    if (guard_alert_pixmap)  { free(guard_alert_pixmap);  guard_alert_pixmap  = NULL; }
    if (exam_pixmap)         { free(exam_pixmap);         exam_pixmap         = NULL; }
    if (guard_calm_pixmap2)  { free(guard_calm_pixmap2);  guard_calm_pixmap2  = NULL; }
    if (guard_alert_pixmap2) { free(guard_alert_pixmap2); guard_alert_pixmap2 = NULL; }
    if (exam_pixmap2)        { free(exam_pixmap2);        exam_pixmap2        = NULL; }
    if (player_pixmap2)      { free(player_pixmap2);      player_pixmap2      = NULL; }
    if (player_final_pixmap && player_final_pixmap != player_pixmap) {
        free(player_final_pixmap);
        player_final_pixmap = NULL;
    }
    if (car_pixmap1) { free(car_pixmap1); car_pixmap1 = NULL; }
    if (car_pixmap2) { free(car_pixmap2); car_pixmap2 = NULL; }
}
