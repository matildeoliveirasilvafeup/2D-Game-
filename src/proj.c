#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>
#include "game.h"
#include "graphics.h"
#include "i8042.h"
#include "kbc.h"
#include "mouse.h"
#include "serial.h"
#include "timer.h"

#define VIDEO_MODE 0x115

	 static game_t game;

extern int timer_counter;
extern uint8_t packet_idx;
extern uint8_t packet_bytes[3];

int(proj_main_loop)(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	/* Local variables */
	uint8_t timer_bit = 0, kbd_bit = 0, mouse_bit = 0;
	uint32_t timer_irq = 0, kbd_irq = 0, mouse_irq = 0;
	bool running = true;
	bool extended = false;
	message msg;
	int ipc_status;

	/* Initialize graphics and memory */
	if (map_vram(VIDEO_MODE) != 0) {
		printf("[proj] map_vram failed\n");
		return 1;
	}
	if (set_graphic_mode(VIDEO_MODE) != 0) {
		printf("[proj] set_graphic_mode failed\n");
		return 1;
	}
	if (timer_set_frequency(0, 60) != 0) {
		printf("[proj] timer_set_frequency failed\n");
		return 1;
	}

	if (game_init(&game) != 0) {
		printf("[proj] game_init failed\n");
		free_secondary_buffer();
		vg_exit();
		return 1;
	}

	if (serial_init() == 0)
		serial_send_string("STEALTH TEST READY\n");

	/* Initial render */
	game_render(&game);
	flip_buffer();

	/* Subscribe hardware interrupts */
	if (timer_subscribe_int(&timer_bit) != 0)
		return 1;
	if (kbd_subscribe_int(&kbd_bit) != 0)
		return 1;
	if (mouse_subscribe_int(&mouse_bit) != 0)
		return 1;
	if (mouse_write_cmd(ENABLE_DATA_REP) != 0)
		return 1;
	if (mouse_write_cmd(0xF3) != 0) /* Set Sample Rate */
		return 1;
	if (mouse_write_cmd(100) != 0) /* 100 samples/sec */
		return 1;

	/* Compute interrupt bitmasks */
	timer_irq = BIT(timer_bit);
	kbd_irq = BIT(kbd_bit);
	mouse_irq = BIT(mouse_bit);

	packet_idx = 0;

	/* Main game loop */
	while (running) {
		if (request_quit) {
			running = false;
			break;
		}
		if (driver_receive(ANY, &msg, &ipc_status) != 0)
			continue;
		if (!is_ipc_notify(ipc_status))
			continue;
		if (_ENDPOINT_P(msg.m_source) != HARDWARE)
			continue;

		uint32_t intr = msg.m_notify.interrupts;

		/* Timer interrupt: update game state and render */
		if (intr & timer_irq) {
			timer_int_handler();
			game_update(&game);

			/* Serial port: receive external commands.
			 * Supported commands (send via terminal to /dev/tty00):
			 *   'P' - pause / resume
			 *   'R' - restart
			 *   'Q' - quit */
			uint8_t serial_cmd;
			if (serial_read_byte(&serial_cmd) == 0) {
				switch (serial_cmd) {
					case 'P':
						game_handle_key(&game, 0x19);
						break;
					case 'R':
						game_init(&game);
						break;
					case 'Q':
						running = false;
						break;
					default:
						break;
				}
			}

			game_render(&game);
			flip_buffer();
		}

		/* Keyboard interrupt */
		if (intr & kbd_irq) {
			kbc_ih();

			if (scancode == 0xE0) {
				extended = true;
				continue;
			}

			if (scancode == 0)
				continue;

			if (!extended && scancode == ESC_BREAK_CODE) {
				running = false;
				continue;
			}

			if (!(scancode & 0x80))
				game_handle_key(&game, scancode);

			extended = false;
		}

		/* Mouse interrupt: accumulate 3-byte packet */
		if (intr & mouse_irq) {
			mouse_ih();
			if (error_found)
				continue;

			if (packet_idx == 0 && !(mouse_byte & BIT(3)))
				continue;

			packet_bytes[packet_idx++] = mouse_byte;

			if (packet_idx == 3) {
				mouse_parse_packet();
				packet_idx = 0;
				game_handle_mouse(&game);
				mouse_pkt.delta_x = 0;
				mouse_pkt.delta_y = 0;
				mouse_pkt.lb = false;
				mouse_pkt.rb = false;
				mouse_pkt.mb = false;
				if (game.state == STATE_QUIZ) {
					game_render(&game);
					flip_buffer();
				}
			}
		}
	}

	/* Cleanup and shutdown */
	mouse_write_cmd(DISABLE_DATA_REP);
	timer_unsubscribe_int();
	kbd_unsubscribe_int();
	mouse_unsubscribe_int();

	free_secondary_buffer();
	vg_exit();

	return 0;
}

int main(int argc, char* argv[]) {
	lcf_start(argc, argv);
	return 0;
}

