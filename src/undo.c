
#include "constants.h"
#include <stdio.h>

unsigned char ud_pos[MAX_DELTAS]; /* ud%(x,0) */
unsigned char ud_tile[MAX_DELTAS]; /* ud%(x,1) */
unsigned char ud_turn[MAX_DELTAS]; /* ud%(x,2) */
unsigned char undo_index=0;
unsigned char delta_index=0;
unsigned char td=0;   /* not sure */
unsigned char f_undo=0;  /* first invalid undo */

extern unsigned char current_turn;

void set_with_undo(unsigned char pos, unsigned char tile);
void push_undo(unsigned char pos);
void push_delta(unsigned char pos, unsigned char tile) ;
void apply_deltas(void);
void perform_undo(void);

void set_with_undo(unsigned char pos, unsigned char tile) {
	push_undo(pos);
	PLAYFIELD[pos] = tile;
}
/*
 * Push tile at given position to "undo" stack after the
 * current turn's deltas have already been applied.
 * Based on gosub 740 in BASIC version.
 */
void push_undo(unsigned char pos) {
	ud_pos[undo_index] = pos;
	ud_tile[undo_index] = PLAYFIELD[pos];
	ud_turn[undo_index] = current_turn;
	delta_index = (undo_index = (undo_index+1) % MAX_DELTAS);
	// TODO: figure out what td=1 did
	td=1;
}
/*
 * Push a tile change to the delta stack.
 * This change will be applied and converted into an "undo"
 * before current_turn is incremented.
 * Based on gosub 765 in BASIC vversion.
 */
void push_delta(unsigned char pos, unsigned char tile) {
	ud_pos[delta_index] = pos;
	ud_tile[delta_index] = tile;
	ud_turn[delta_index] = current_turn;
	delta_index = (delta_index+1) % MAX_DELTAS;
}

void apply_deltas(void) {
	unsigned char incoming_tile;
	unsigned char current_pos;
	printf("applying %d deltas\n", delta_index - undo_index);
	while (delta_index != undo_index) {
		incoming_tile = ud_tile[undo_index];
		if ( background(incoming_tile) == foreground(incoming_tile) ) {
			incoming_tile = foreground(incoming_tile);
		}
		current_pos = ud_pos[undo_index];
		/*replace incoming tile with outgoing tile for undo later */
		ud_tile[undo_index] = PLAYFIELD[current_pos];
		PLAYFIELD[current_pos] = incoming_tile;
		printf("%d\x5f%02x ", ud_pos[undo_index], incoming_tile);
		undo_index = (undo_index + 1) % MAX_DELTAS;
	}
	printf("\n");
	f_undo = ud_turn[undo_index];
}

void perform_undo(void) {
	--current_turn;
	td=0;
	if(current_turn <= f_undo) {
		current_turn = f_undo+1;
		return;
	}
	printf("\x9fundo turn #%d\n", current_turn);
	while(1) {
		if (undo_index == 0) {
			undo_index = MAX_DELTAS-1;
		} else {
			--undo_index;
		}
		if(ud_turn[undo_index] != current_turn) {
			break;
		}
		ud_turn[undo_index] = f_undo;
		PLAYFIELD[ud_pos[undo_index]] = ud_tile[undo_index];
		printf("%02x=%02x ", ud_pos[undo_index], ud_tile[undo_index]);
	}
	printf("\n");

	undo_index = (undo_index + 1) % MAX_DELTAS;
	delta_index = undo_index;
}

void clear_undo_stack(void) {
	unsigned char i;
	current_turn = 1;
	f_undo = 0;
	for(i=0;i<MAX_DELTAS;i++) {
		ud_turn[undo_index] = 0;
	}
}
