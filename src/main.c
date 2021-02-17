//#include <stdio.h>
#include <stdlib.h>

#include "levels.h"
#include "gfx.h"
#include "baba.h"

unsigned char current_level = 0;

void game_loop(void);

int main (void)
{
	game_loop();
	return EXIT_SUCCESS;
}

void game_loop(void) {
	load_level(current_level);
	init_screen();
	draw_playfield();
	play_loop();
}


