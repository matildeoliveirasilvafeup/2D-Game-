#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup game Game
 * @brief Core game structures and functions
 * @{
 */

#define MAP_ROWS 15	/**< @brief Number of tile rows in the game map */
#define MAP_COLS 20	/**< @brief Number of tile columns in the game map */
#define TILE_SIZE 37 /**< @brief Width and height of one tile in pixels */

/**
 * @brief Tile types that can appear in a level map.
 */
typedef enum {
	TILE_FLOOR = 0, /**< @brief Walkable floor tile */
	TILE_WALL = 1,	 /**< @brief Impassable wall tile */
	TILE_EXIT = 2,	 /**< @brief Level exit tile (car escape) */
	TILE_EXAM = 3,	 /**< @brief Tile containing the exam the player must collect */
	TILE_SPAWN = 4	 /**< @brief Player spawn position tile */
} tile_type_t;

/**
 * @brief High-level states that the game finite-state machine can be in.
 */
typedef enum {
	STATE_MENU,			 /**< @brief Main menu */
	STATE_PLAYING,		 /**< @brief Active gameplay */
	STATE_PAUSED,		 /**< @brief Game paused */
	STATE_WIN,			 /**< @brief Victory screen */
	STATE_LOSE,			 /**< @brief Game-over screen */
	STATE_QUIZ,			 /**< @brief Mid-level quiz / drawing challenge */
	STATE_LEADERBOARD, /**< @brief High scores screen */
	STATE_GUIDE,		 /**< @brief Instructions / story screen */
	STATE_CONTROLS		 /**< @brief Controls reference screen */
} game_state_t;

/**
 * @brief Alert level of a guard towards the player.
 */
typedef enum {
	ALERT_NONE,			/**< @brief Guard is unaware of the player */
	ALERT_SUSPICIOUS, /**< @brief Guard noticed something and is investigating */
	ALERT_DETECTED		/**< @brief Guard has fully detected the player */
} alert_t;

/**
 * @brief State of the player character.
 */
typedef struct {
	int col;			/**< @brief Current map column of the player */
	int row;			/**< @brief Current map row of the player */
	bool detected; /**< @brief True when the player has been caught by a guard */
	bool has_exam; /**< @brief True when the player is carrying the exam */
	bool in_car;	/**< @brief True when the player has reached the exit car */
} player_t;

/**
 * @brief State of one guard (professor) on patrol.
 */
typedef struct {
	int col;				/**< @brief Current map column of the guard */
	int row;				/**< @brief Current map row of the guard */
	int direction;		/**< @brief Facing direction: 0=up 1=right 2=down 3=left */
	int patrol_len;	/**< @brief Number of waypoints in the patrol route */
	int patrol_idx;	/**< @brief Index of the current patrol waypoint */
	int vision_range; /**< @brief Number of tiles the guard can see ahead */
	alert_t alert;		/**< @brief Current alert level towards the player */
	int alert_ticks;	/**< @brief Remaining ticks before the alert level decays */
} guard_t;

/**
 * @brief One entry in the high-score leaderboard.
 */
typedef struct {
	int score;							/**< @brief Final score achieved */
	uint8_t day, month, year;		/**< @brief Date the score was recorded (RTC BCD values) */
	uint8_t hour, minute, second; /**< @brief Time the score was recorded (RTC BCD values) */
	int level;							/**< @brief Level number in which the player won */
	bool valid;							/**< @brief False when this slot is empty */
} high_score_t;

/**
 * @brief Master game state structure passed throughout the game.
 *
 * Holds the complete runtime state: map, entities, timers, UI sub-states,
 * quiz data, drawing canvas, and the leaderboard.
 */
typedef struct {
	tile_type_t map[MAP_ROWS][MAP_COLS]; /**< @brief Tile grid of the current level */
	player_t player;							 /**< @brief Player character state */
	guard_t guards[8];						 /**< @brief Array of guards in the current level */
	int num_guards;							 /**< @brief Number of active guards */
	uint32_t ticks;							 /**< @brief Total timer ticks since the game started */
	uint32_t elapsed_ticks;					 /**< @brief Timer ticks elapsed in the current level */
	int countdown_sec;						 /**< @brief Remaining seconds on the countdown timer */
	int score;									 /**< @brief Current player score */

	int rtc_day, rtc_month, rtc_year;	  /**< @brief RTC date snapshot (BCD, updated each frame) */
	int rtc_hour, rtc_minute, rtc_second; /**< @brief RTC time snapshot (BCD, updated each frame) */

	int mouse_x, mouse_y; /**< @brief Current mouse cursor position in screen pixels */
	bool mouse_left_down; /**< @brief True while the left mouse button is held */

	game_state_t state;		 /**< @brief Current FSM state */
	game_state_t prev_state; /**< @brief Previous FSM state (used when returning from sub-screens) */
	bool lose_timeout;		 /**< @brief True when the player lost due to time running out */

	int current_level; /**< @brief Zero-based index of the active level (0 = map 1, 1 = map 2) */

	/* Quiz sub-state */
	int quiz_question;				/**< @brief Index of the current question within the round (0–2) */
	int quiz_lives;					/**< @brief Remaining quiz lives (wrong answers decrement this) */
	int quiz_hover;					/**< @brief Index of the answer option under the mouse cursor (-1 = none) */
	int quiz_last_selected;			/**< @brief Index of the last answer option clicked */
	int quiz_feedback;				/**< @brief Feedback code: 1 = correct, -1 = wrong, 0 = none */
	uint32_t quiz_feedback_ticks; /**< @brief Tick count at which the current feedback was triggered */
	int selected_questions[2];		/**< @brief Indices into quiz_bank drawn for this MCQ round */

	/* Drawing canvas sub-state */
	uint8_t draw_canvas[70][85]; /**< @brief Pixel grid for the digit-drawing challenge (rows × cols) */
	bool draw_is_drawing;		  /**< @brief True while the player is pressing the mouse to draw */
	int draw_recognized;			  /**< @brief Digit recognised by the classifier (-1 = none yet) */
	bool draw_submitted;			  /**< @brief True after the player confirmed their drawing */
	bool draw_hover_clear;		  /**< @brief True when the mouse is hovering over the Clear button */
	bool draw_hover_check;		  /**< @brief True when the mouse is hovering over the Check button */
	int draw_last_x;				  /**< @brief Canvas column of the last drawn pixel */
	int draw_last_y;				  /**< @brief Canvas row of the last drawn pixel */
	int draw_ready_ticks;		  /**< @brief Tick count when the drawing canvas became ready */

	/* Menu sub-state */
	int menu_selected; /**< @brief Index of the currently highlighted main-menu item */

	/* Leaderboard */
	high_score_t high_scores[3]; /**< @brief Top-3 entries, sorted by score descending */
	int car_col;					  /**< @brief Map column of the animated escape car */
	int car_row;					  /**< @brief Map row of the animated escape car */
} game_t;

/**
 * @brief Advances the game by one tick (called every timer interrupt).
 * @param g Pointer to the game state.
 */
void game_update(game_t* g);

/**
 * @brief Processes a keyboard scancode and updates the game state accordingly.
 * @param g  Pointer to the game state.
 * @param sc Raw scancode byte received from the KBC.
 */
void game_handle_key(game_t* g, uint8_t sc);

/**
 * @brief Processes the current mouse state (position and buttons).
 * @param g Pointer to the game state.
 */
void game_handle_mouse(game_t* g);

/**
 * @brief Renders the current frame to the back buffer and flips to screen.
 * @param g Pointer to the (const) game state.
 */
void game_render(const game_t* g);

/**
 * @brief Initialises all game sub-systems and loads the first level.
 * @param g Pointer to the game state structure to initialise.
 * @return 0 on success, non-zero on failure.
 */
int game_init(game_t* g);

/**
 * @brief Releases all resources allocated by game_init().
 * @param g Pointer to the game state.
 */
void game_cleanup(game_t* g);

/**
 * @brief Renders the controls-reference overlay screen.
 * @param g Pointer to the (const) game state.
 */
void game_render_controls(const game_t* g);

/** @brief Default number of tiles a guard can see ahead. */
#define GUARD_VISION_RANGE 5

/** @brief Set to true by any subsystem that wants the main loop to terminate. */
extern bool request_quit;

/** @} */

#endif
