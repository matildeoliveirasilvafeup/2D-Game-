#include "game_internal.h"
#include <lcom/lcf.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"

bool request_quit = false;

const quiz_entry_t quiz_bank[9] = {
    { QUIZ_TYPE_MCQ,
      "WHAT IS THE SCANCODE WHEN A KEY IS RELEASED",
      { "MAKE CODE", "BREAK CODE", "OVERLAP CODE", "SERIAL CODE" }, 1 },

    { QUIZ_TYPE_MCQ,
      "PURPOSE OF SYNCING FRAME BUFFER SWAP\nWITH VERTICAL RETRACE",
      { "ELIMINATE SCREEN TEARING", "COMPRESS VRAM DATA",
        "INCREASE REFRESH RATE",    "CONVERT TO VECTOR" }, 0 },

    { QUIZ_TYPE_MCQ,
      "WHICH REGISTER SELECTS THE VBE VIDEO MODE",
      { "AX", "BX", "CX", "DX" }, 0 },

    { QUIZ_TYPE_MCQ,
      "WHAT DOES RTC STAND FOR",
      { "REAL TIME CLOCK", "RAPID TRANSFER CACHE",
        "REGISTER TRANSFER CYCLE", "RANDOM TIMING CIRCUIT" }, 0 },

    { QUIZ_TYPE_MCQ,
      "WHICH IRQ LINE DOES THE KEYBOARD USE",
      { "IRQ 0", "IRQ 1", "IRQ 2", "IRQ 12" }, 1 },

    { QUIZ_TYPE_MCQ,
      "WHICH IRQ LINE DOES THE MOUSE USE",
      { "IRQ 1", "IRQ 4", "IRQ 12", "IRQ 14" }, 2 },

    { QUIZ_TYPE_MCQ,
      "WHAT IS THE PURPOSE OF DOUBLE BUFFERING",
      { "INCREASE VRAM SIZE", "PREVENT SCREEN TEARING",
        "SPEED UP THE CPU CLOCK", "REDUCE MOUSE LATENCY" }, 1 },

    { QUIZ_TYPE_MCQ,
      "HOW MANY BYTES DOES A MOUSE PACKET HAVE",
      { "1", "2", "3", "4" }, 2 },

    { QUIZ_TYPE_MCQ,
      "WHICH MINIX FUNCTION SUBSCRIBES TIMER INT",
      { "SYS IRQSETPOLICY", "TIMER SUBSCRIBE INT",
        "KBD SUBSCRIBE INT", "VBE SET MODE" }, 1 },
};

const quiz_entry_t quiz_draw = {
    QUIZ_TYPE_DRAW,
    "DRAW THE TIMER FREQUENCY IN HZ\nDIVIDED BY 10",
    { NULL, NULL, NULL, NULL },
    6
};

const int quiz_btn_y[4] = { 190, 250, 310, 370 };

/* Sprite handles — loaded once in game_init and freed in game_cleanup */
uint8_t     *player_pixmap        = NULL;
xpm_image_t  player_img;
uint8_t     *guard_calm_pixmap    = NULL;
xpm_image_t  guard_calm_img;
uint8_t     *guard_alert_pixmap   = NULL;
xpm_image_t  guard_alert_img;
uint8_t     *exam_pixmap          = NULL;
xpm_image_t  exam_img;
uint8_t     *guard_calm_pixmap2   = NULL;
xpm_image_t  guard_calm_img2;
uint8_t     *guard_alert_pixmap2  = NULL;
xpm_image_t  guard_alert_img2;
uint8_t     *exam_pixmap2         = NULL;
xpm_image_t  exam_img2;
uint8_t     *player_pixmap2       = NULL;
xpm_image_t  player_img2;
uint8_t     *player_final_pixmap  = NULL;
xpm_image_t  player_final_img;
uint8_t     *car_pixmap1          = NULL;
xpm_image_t  car_img1;
uint8_t     *car_pixmap2          = NULL;
xpm_image_t  car_img2;

bool guard_frame      = false;
bool exam_frame       = false;
bool guards_panicking = false;

const tile_type_t map_level1[MAP_ROWS][MAP_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1},
    {1,0,1,1,1,1,1,1,0,0,1,1,1,0,0,1,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,1},
    {1,1,1,0,1,0,0,1,1,1,0,0,1,0,1,1,0,1,0,1},
    {1,0,0,0,1,0,0,0,0,1,4,0,0,0,0,1,0,0,0,1},
    {1,0,1,1,1,0,1,0,0,1,1,1,0,1,0,1,1,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,4,3,1},
    {1,0,1,0,1,1,1,1,0,0,0,1,0,1,0,1,1,0,0,1},
    {1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,1},
    {1,0,1,1,1,0,1,0,0,1,1,1,0,1,1,0,1,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

const tile_type_t map_level2[MAP_ROWS][MAP_COLS] = {
	 {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	 {2, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1},
	 {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1},
	 {1, 4, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
	 {1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1},
	 {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1},
	 {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	 {1, 0, 4, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, 0, 0, 1},
	 {1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
	 {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 4, 0, 1, 0, 1, 0, 0, 0, 0, 1},
	 {1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1},
	 {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
	 {1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1},
	 {1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1},
	 {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

const tile_type_t (*all_maps[NUM_LEVELS])[MAP_COLS] = {
    map_level1,
    map_level2,
};
