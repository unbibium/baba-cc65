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
	cbm_k_bsout(0x13); // HOME
	for (i=0; i<LEVEL_HEIGHT; ++i) {
		cbm_k_bsout(0x11); // CRSR-DOWN
	}
}
void draw_xray(void) {
	unsigned char i;
	for(i=0;i<LEVEL_TILES;i++) {
		cbm_k_bsout(32); // SPACE
		POKE(646,4);
		if((PLAYFIELD[i] & 224) != 0 ) {
			cbm_k_bsout( 64 + (PLAYFIELD[i] >> 5) );
		} else {
			cbm_k_bsout(32);
		}
		if( is_text(PLAYFIELD[i]) ) {
			POKE(646,15);
		} else {
			POKE(646,3);
		}
		if((PLAYFIELD[i] & 31) != 0) {
			cbm_k_bsout( 64 + (PLAYFIELD[i] & 31) );
		} else {
			cbm_k_bsout( 46 );
		}
		cbm_k_bsout(32); // SPACE
	}
}
