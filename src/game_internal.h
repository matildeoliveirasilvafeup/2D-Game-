#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "game.h"
#include "graphics.h"

/**
 * @defgroup game_internal Game Internal
 * @brief Internal game constants, structures and helper functions
 * @{
 */

/* ── Timing & viewport ─────────────────────────────────────────────────── */

#define GAME_TIME_SEC 120								/**< @brief Total level time limit in seconds */
#define TICKS_PER_SEC 60								/**< @brief Timer interrupts per second (frame rate) */
#define GUARD_MOVE_TICKS 20							/**< @brief Timer ticks between guard movement steps */
#define ALERT_DECAY_TICKS 90							/**< @brief Ticks before a guard's alert level decreases */
#define VIEW_WIDTH (MAP_COLS * TILE_SIZE)			/**< @brief Viewport width in pixels */
#define VIEW_HEIGHT (MAP_ROWS * TILE_SIZE + 32) /**< @brief Viewport height in pixels (map + HUD strip) */
#define SPRITE_SCALE 2									/**< @brief Integer scale factor applied to all sprites */
#define MOUSE_SENSITIVITY 8							/**< @brief Mouse delta divisor for cursor movement */
#define NUM_LEVELS 2										/**< @brief Total number of playable levels */

/* ── Quiz types ─────────────────────────────────────────────────────────── */

/**
 * @brief Distinguishes the two kinds of quiz challenge.
 */
typedef enum {
	QUIZ_TYPE_MCQ, /**< @brief Multiple-choice question (four text options) */
	QUIZ_TYPE_DRAW /**< @brief Digit-drawing challenge (mouse canvas) */
} quiz_type_t;

/**
 * @brief Describes one question in the quiz bank.
 */
typedef struct {
	quiz_type_t type;			/**< @brief Whether this is an MCQ or a drawing challenge */
	const char* question;	/**< @brief Question text shown to the player */
	const char* options[4]; /**< @brief Answer options (only used for MCQ entries) */
	int correct;				/**< @brief Zero-based index of the correct option (MCQ) or expected digit (draw) */
} quiz_entry_t;

/* ── Drawing canvas geometry ────────────────────────────────────────────── */

#define DRAW_CANVAS_X 200									/**< @brief Screen X of the drawing canvas top-left corner */
#define DRAW_CANVAS_Y 160									/**< @brief Screen Y of the drawing canvas top-left corner */
#define DRAW_CANVAS_W 340									/**< @brief Drawing canvas width in pixels */
#define DRAW_CANVAS_H 280									/**< @brief Drawing canvas height in pixels */
#define DRAW_CELL_SIZE 4									/**< @brief Pixel size of one canvas grid cell */
#define DRAW_COLS (DRAW_CANVAS_W / DRAW_CELL_SIZE) /**< @brief Number of canvas columns */
#define DRAW_ROWS (DRAW_CANVAS_H / DRAW_CELL_SIZE) /**< @brief Number of canvas rows */

#define DRAW_BTN_CLEAR_X 200 /**< @brief Screen X of the Clear button */
#define DRAW_BTN_CLEAR_Y 455 /**< @brief Screen Y of the Clear button */
#define DRAW_BTN_W 160		  /**< @brief Width of canvas action buttons */
#define DRAW_BTN_H 40		  /**< @brief Height of canvas action buttons */
#define DRAW_BTN_CHECK_X 380 /**< @brief Screen X of the Check / Submit button */
#define DRAW_BTN_CHECK_Y 455 /**< @brief Screen Y of the Check / Submit button */

/* ── MCQ button layout ──────────────────────────────────────────────────── */

#define QUIZ_BTN_X 110 /**< @brief Screen X of MCQ answer buttons */
#define QUIZ_BTN_W 520 /**< @brief Width of MCQ answer buttons */
#define QUIZ_BTN_H 48  /**< @brief Height of MCQ answer buttons */

/* ── Sprite assets (loaded during game_init) ────────────────────────────── */

extern uint8_t* player_pixmap;		 /**< @brief Decoded pixel data for the player sprite (frame 1) */
extern xpm_image_t player_img;		 /**< @brief Image descriptor for the player sprite (frame 1) */
extern uint8_t* guard_calm_pixmap;	 /**< @brief Decoded pixel data for the calm guard sprite (frame 1) */
extern xpm_image_t guard_calm_img;	 /**< @brief Image descriptor for the calm guard sprite (frame 1) */
extern uint8_t* guard_alert_pixmap;	 /**< @brief Decoded pixel data for the alert guard sprite (frame 1) */
extern xpm_image_t guard_alert_img;	 /**< @brief Image descriptor for the alert guard sprite (frame 1) */
extern uint8_t* exam_pixmap;			 /**< @brief Decoded pixel data for the exam item sprite (frame 1) */
extern xpm_image_t exam_img;			 /**< @brief Image descriptor for the exam item sprite (frame 1) */
extern uint8_t* guard_calm_pixmap2;	 /**< @brief Decoded pixel data for the calm guard sprite (frame 2) */
extern xpm_image_t guard_calm_img2;	 /**< @brief Image descriptor for the calm guard sprite (frame 2) */
extern uint8_t* guard_alert_pixmap2; /**< @brief Decoded pixel data for the alert guard sprite (frame 2) */
extern xpm_image_t guard_alert_img2; /**< @brief Image descriptor for the alert guard sprite (frame 2) */
extern uint8_t* exam_pixmap2;			 /**< @brief Decoded pixel data for the exam item sprite (frame 2) */
extern xpm_image_t exam_img2;			 /**< @brief Image descriptor for the exam item sprite (frame 2) */
extern uint8_t* player_pixmap2;		 /**< @brief Decoded pixel data for the player sprite (frame 2) */
extern xpm_image_t player_img2;		 /**< @brief Image descriptor for the player sprite (frame 2) */
extern uint8_t* player_final_pixmap; /**< @brief Decoded pixel data for the player victory sprite */
extern xpm_image_t player_final_img; /**< @brief Image descriptor for the player victory sprite */
extern uint8_t* car_pixmap1;			 /**< @brief Decoded pixel data for the escape car (frame 1) */
extern xpm_image_t car_img1;			 /**< @brief Image descriptor for the escape car (frame 1) */
extern uint8_t* car_pixmap2;			 /**< @brief Decoded pixel data for the escape car (frame 2) */
extern xpm_image_t car_img2;			 /**< @brief Image descriptor for the escape car (frame 2) */

/* ── Animation state ────────────────────────────────────────────────────── */

extern bool guard_frame;		/**< @brief Flip-flop selecting between guard animation frames */
extern bool exam_frame;			/**< @brief Flip-flop selecting between exam animation frames */
extern bool guards_panicking; /**< @brief True when guards should display the alert animation */

/* ── Quiz data ──────────────────────────────────────────────────────────── */

extern const quiz_entry_t quiz_bank[9]; /**< @brief Pool of all MCQ questions available in the game */
extern const quiz_entry_t quiz_draw;	 /**< @brief The single digit-drawing challenge entry */
extern const int quiz_btn_y[4];			 /**< @brief Screen Y positions of the four MCQ answer buttons */

/* ── Level maps ─────────────────────────────────────────────────────────── */

extern const tile_type_t map_level1[MAP_ROWS][MAP_COLS];		/**< @brief Tile data for level 1 */
extern const tile_type_t map_level2[MAP_ROWS][MAP_COLS];		/**< @brief Tile data for level 2 */
extern const tile_type_t (*all_maps[NUM_LEVELS])[MAP_COLS]; /**< @brief Array of pointers to all level maps */

/* ── game_init.c ────────────────────────────────────────────────────────── */

/**
 * @brief Loads level @p level into the game state (map, guards, player spawn).
 * @param g     Pointer to the game state.
 * @param level Zero-based level index.
 */
void load_level(game_t* g, int level);

/* ── game_update.c ──────────────────────────────────────────────────────── */

/**
 * @brief Returns whether the tile at (@p col, @p row) can be walked on.
 * @param g   Pointer to the (const) game state.
 * @param col Map column to test.
 * @param row Map row to test.
 * @return true if the tile is walkable (floor, exit, exam, spawn), false otherwise.
 */
bool tile_walkable(const game_t* g, int col, int row);

/* ── game_text.c ────────────────────────────────────────────────────────── */

/**
 * @brief Renders a single decimal digit sprite at the given screen position.
 * @param x     Screen X coordinate.
 * @param y     Screen Y coordinate.
 * @param digit Digit to draw (0–9).
 * @param color 32-bit RGB colour.
 */
void draw_digit(int x, int y, int digit, uint32_t color);

/**
 * @brief Renders a non-negative integer as a sequence of digit sprites.
 * @param x Screen X coordinate of the leftmost digit.
 * @param y Screen Y coordinate.
 * @param n Integer value to render.
 * @param color 32-bit RGB colour.
 */
void draw_number(int x, int y, int n, uint32_t color);

/**
 * @brief Renders a null-terminated string using the built-in bitmap font.
 * @param x     Screen X coordinate of the first character.
 * @param y     Screen Y coordinate.
 * @param text  String to render.
 * @param color 32-bit RGB colour.
 * @param scale Integer scale factor (1 = original size).
 */
void draw_text(int x, int y, const char* text, uint32_t color, int scale);

/**
 * @brief Renders an integer value as text using the built-in bitmap font.
 * @param x     Screen X coordinate.
 * @param y     Screen Y coordinate.
 * @param value Integer value to render.
 * @param color 32-bit RGB colour.
 * @param scale Integer scale factor.
 */
void draw_int_text(int x, int y, int value, uint32_t color, int scale);

/**
 * @brief Returns the rendered pixel width of a string at the given scale.
 * @param s     Null-terminated string.
 * @param scale Integer scale factor.
 * @return Width in pixels.
 */
int text_width(const char* s, int scale);

/**
 * @brief Renders text horizontally centred within a region.
 * @param x     Left edge of the region.
 * @param w     Width of the region.
 * @param y     Screen Y coordinate.
 * @param t     Null-terminated string to render.
 * @param c     32-bit RGB colour.
 * @param scale Integer scale factor.
 */
void draw_text_centred(int x, int w, int y, const char* t, uint32_t c, int scale);

/* ── game_quiz.c ────────────────────────────────────────────────────────── */

/**
 * @brief Classifies a hand-drawn digit from the drawing canvas.
 * @param canvas 2-D array of pixel values (non-zero = drawn).
 * @return Recognised digit (0–9), or -1 if recognition failed.
 */
int recognize_digit(const uint8_t canvas[DRAW_ROWS][DRAW_COLS]);

/**
 * @brief Attempts to insert a new score into the top-3 leaderboard.
 *
 * The entry is inserted only if it is better than the lowest current entry.
 * The list is kept sorted by score descending.
 *
 * @param g      Pointer to the game state.
 * @param score  Final score to insert.
 * @param level  Level number in which the player won.
 * @param day    RTC day (BCD).
 * @param month  RTC month (BCD).
 * @param year   RTC year (BCD).
 * @param hour   RTC hour (BCD).
 * @param minute RTC minute (BCD).
 * @param second RTC second (BCD).
 */
void leaderboard_try_insert(game_t* g, int score, int level, uint8_t day, uint8_t month, uint8_t year, uint8_t hour, uint8_t minute, uint8_t second);

/**
 * @brief Randomly selects two MCQ questions from quiz_bank for this round.
 * @param g Pointer to the game state (updates g->selected_questions).
 */
void quiz_pick_questions(game_t* g);

/**
 * @brief Returns a pointer to the quiz_entry_t for the current question.
 * @param g Pointer to the (const) game state.
 * @return Pointer to the active quiz entry, or NULL if the round is complete.
 */
const quiz_entry_t* current_question(const game_t* g);

/** @} */

#endif
