
#include "constants.h"
#include <stdio.h>

unsigned char ud_pos[MAX_DELTAS]; /* ud%(x,0) */
unsigned char ud_tile[MAX_DELTAS]; /* ud%(x,1) */
unsigned char ud_turn[MAX_DELTAS]; /* ud%(x,2) */
unsigned char undo_index=0;
unsigned char delta_index=0;
unsigned char td=0;   /* not sure */
unsigned char f_undo=0;  /* BASIC fu variable */

extern unsigned char current_turn;

void push_undo(unsigned char pos);
void push_delta(unsigned char pos, unsigned char tile) ;
void apply_deltas(void);

/*
  740 rem push to undo stack after delta loop already ran
  745 ud%(ud,0)=ck:ud%(ud,1)=pf%(ck):ud%(ud,2)=tu:td=1
  750 ud=ud+1:if ud>mu then ud=0
  755 dl=ud
  760 return
*/
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
  765 rem push to delta-undo stack
  770 ud%(dl,0)=ck:ud%(dl,1)=np:ud%(dl,2)=tu
  775 dl=dl+1:ifdl>mu then dl=0
  780 return
*/
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

/*
 *   500 rem apply deltas and convert to undo
  505 if ud=dl then return
  506 td=1
  510 rem turn delta into undo
  515 x=ud%(ud,0)
  520 np=ud%(ud,1):ud%(ud,1)=pf%(x)
  521 if(npand31)=int(np/32) then np=npand31:rem absorb doubles
  522 pf%(x)=np
  525 ud=ud+1:ifud>muthen ud=0
  530 if ud<>dl then 515
  535 rem todo: signal whether stack has changed
  540 fu=ud%(ud,2)
  545 return:rem end of applying deltas
 */
void apply_deltas(void) {
	unsigned char incoming_tile;
	printf("moving: ud=%d\n", undo_index);
	while (delta_index != undo_index) {
		incoming_tile = ud_tile[undo_index];
		if ( background(incoming_tile) == foreground(incoming_tile) ) {
			incoming_tile = foreground(incoming_tile);
		}
		PLAYFIELD[ud_pos[undo_index]] = incoming_tile;
		printf("%d_%02x ", ud_pos[undo_index], incoming_tile);
		undo_index = (undo_index + 1) % MAX_DELTAS;
	}
	printf("\n");
	f_undo = ud_turn[undo_index];
}
