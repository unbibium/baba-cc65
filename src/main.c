//#include <stdio.h>
#include <stdlib.h>

#include "levels.h"
#include "gfx.h"
#include "baba.h"

void game_loop(void);

int main (void)
{
	game_loop();
	return EXIT_SUCCESS;
}

void game_loop(void) {
	init_screen();
	play_loop();
}


