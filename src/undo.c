
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

void push_undo(unsigned char pos);
void push_delta(unsigned char pos, unsigned char tile) ;
void apply_deltas(void);
void perform_undo(void);

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
	unsigned char current_pos;
	printf("moving: ud=%d\n", undo_index);
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

/*
 * 2000 rem "z" for undo
 2005 rem@ \fastfor:\fastarray
*/
void perform_undo(void) {
	/* 2010 tu=tu-1:td=0
	2015 if tu<=fu then tu=fu+1:goto2050 */
	--current_turn;
	td=0;
	if(current_turn <= f_undo) {
		current_turn = f_undo+1;
		return;
	}
	printf("\x9fundo turn #%d\n", current_turn);
	while(1) {
		/* 2020 ud=ud-1:if ud<0 then ud=mu */
		if (undo_index == 0) {
			undo_index = MAX_DELTAS-1;
		} else {
			--undo_index;
		}
		/* 2025 ifud%(ud,2)<>tu then 2040 */
		if(ud_turn[undo_index] != current_turn) {
			break;
		}
		/* 2030 ud%(ud,2)=fu:pf%(ud%(ud,0))=ud%(ud,1) */
		ud_turn[undo_index] = f_undo;
		PLAYFIELD[ud_pos[undo_index]] = ud_tile[undo_index];
		printf("%02x=%02x ", ud_pos[undo_index], ud_tile[undo_index]);
		/* 2035 goto 2020 */
	}
	printf("\n");

	/* 2040 ud=ud+1:if ud>mu then ud=0 */
	undo_index = (undo_index + 1) % MAX_DELTAS;
	/* 2045 dl=ud */
	delta_index = undo_index;
	/* 2050 goto 215 */
}

void clear_undo_stack(void) {
	f_undo = current_turn - 1;
}
