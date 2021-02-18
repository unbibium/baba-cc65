/*
 *
 */
#include "constants.h"
#include <peekpoke.h>
#include <cbm.h>

extern void __fastcall__ print_gr(unsigned char);

void init_screen() {
	POKE(53272U,21);
	VIC.bgcolor0=COLOR_BLUE; 
	cbm_k_bsout(0x93); // CLR
}

void draw_playfield() {
	unsigned char i;
	cbm_k_bsout(0x13); // HOME
	for ( i=0; i < LEVEL_TILES ; ++i) {
		print_gr(PLAYFIELD[i] & 0x1F);
	}
}
