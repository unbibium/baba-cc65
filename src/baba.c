#include "constants.h"
#include "gfx.h"
#include "levels.h"
#include "undo.h"
#include <stdio.h>
#include <cbm.h>

extern void __fastcall__ print_gr(unsigned char);

unsigned int obj_properties[8];
unsigned char obj_transforms[8];

unsigned char current_level=0;
unsigned char current_turn=1;
unsigned char you_win=0;

void compile_rules(void);
void process_IS(unsigned char i, unsigned char d);
void noun_is_noun(unsigned char left_side, unsigned char right_side);
void noun_is_prop(unsigned char left_side, unsigned char right_side);
signed char __fastcall__ get_move(void);
void destroy_rule(unsigned char ck, int property);
void __fastcall__ move_YOU_tiles(signed char dx);
void __fastcall__ move_obj(unsigned char, signed char);
void load_next_level(void);
void process_rules(void);

void load_next_level(void) {
	if (++current_level > 5) {
		current_level = 0;
	}
	load_level(current_level);
	clear_undo_stack();
}

void play_loop(void) {
	signed char baba_move;
	load_level(current_level);
	compile_rules();
	while(1) {
		draw_playfield();
		printf("turn %d\n", current_turn);
		baba_move = get_move();
		if(baba_move) {
			move_YOU_tiles(baba_move);
			apply_deltas();
		}
		compile_rules();
		process_rules();
		if(you_win) {
			load_next_level();
		} else {
			apply_deltas();
			++current_turn;
		}
	}
}

signed char get_move(void) {
	char f;
	while(1) {
		f = cbm_k_getin();
		switch(f) {
			case CH_CURS_UP:
			case 'w':
				return -LEVEL_WIDTH;
			case CH_CURS_DOWN:
			case 's':
				return LEVEL_WIDTH;
			case CH_CURS_LEFT:
			case 'a':
				return -1;
			case CH_CURS_RIGHT:
			case 'd':
				return 1;
			case ' ': // wait
				return 0;
			case 'r':
				/* TODO: restart */
				load_level(current_level);
				/* invalidate all of undo stack */
				clear_undo_stack();
				compile_rules();
				draw_playfield();
				break;
			case 'z':
				perform_undo();
				compile_rules();
				draw_playfield();
				break;
			case 'n':
				load_next_level();
				compile_rules();
				draw_playfield();
				break;
			case 'x':
				init_screen();
				draw_xray();
				break;
			case 'c':
				init_screen();
				draw_playfield();
				break;
			default:
				break;
		}
	}
}

/*
 * process all the consequences of the current ruleset
 * after the player's last move
 */
void process_rules(void) {
	unsigned char ck, n;
	unsigned char you_tiles=0; 
	unsigned int r;
	you_win=0;
	for (ck=0; ck < LEVEL_TILES; ++ck) {
		/* 255 n=pf%(ck):ifn=0then295 */
		n=PLAYFIELD[ck];
		if(n == 0) {
			continue;
		}
		if(is_obj(foreground(n)) && obj_transforms[foreground(n)] ) {
			/* noun is noun foreground */
			push_undo(ck);
			PLAYFIELD[ck] = n = (n & 0xE0) | obj_transforms[foreground(n)];
		}
		/* 265 ifn>32theniftr%(n/32)theniftr%(n/32)<>int(n/32) then gosub720:rem bg */
		if(n > 31 && obj_transforms[background(n)]) {
			/* noun is noun background */
			push_undo(ck);
			PLAYFIELD[ck] = n = (n & 0x1F) | (obj_transforms[background(n)] << 5);
		}
		/* interactions */
		r = tile_props(ck);
		if((r & PROPS_OPEN_SHUT) == PROPS_OPEN_SHUT) {
			printf("%d open-shut %4x\n", ck, r);
			push_delta(ck, 0); /* destroys both */
		} else if ((r & PROP_SINK) && PLAYFIELD[ck] > 31) {
			printf("%d sink %4x\n", ck, r);
			push_delta(ck, 0); /* any object + sink = empty */
		} else if ((r & PROPS_HOT_MELT) == PROPS_HOT_MELT) {
			destroy_rule(ck, PROP_MELT);
		} else if ((r & PROPS_YOU_LOSE) == PROPS_YOU_LOSE) {
			destroy_rule(ck, PROP_YOU);
		} else if (r & PROP_YOU) {
			++you_tiles;
			// TODO: add to "you" tile list?
			if (r & PROP_WIN) {
				you_win=1;
			}
		}
	}	
	/* TODO: if no you_tiles then display warning */
}

/*
 * Given tile located at "ck", destroy whichever object
 * on that tile has the property of "property".
 */
void destroy_rule(unsigned char ck, int property) {
	unsigned char tile = PLAYFIELD[ck];
	if(obj_properties[foreground(tile)] & property) {
		set_with_undo(ck, background(tile));
	} else if(obj_properties[background(tile)] & property) {
		set_with_undo(ck, foreground(tile));
	} 
}

void compile_rules(void) {
	unsigned char i;
	/* Clear rules */
	for (i=0; i < 8; ++i) {
		obj_properties[i]=0;
		obj_transforms[i]=0;
	}
	for(i=1; i < LEVEL_TILES-1 ; i++) {
		if( foreground(PLAYFIELD[i]) == TEXT_IS) {
			process_IS(i, 1);
			if( i >= LEVEL_WIDTH && i < LEVEL_TILES - LEVEL_WIDTH) {
				process_IS(i, LEVEL_WIDTH);
			}
		}
	}
}

void process_IS(unsigned char i, unsigned char d) {
	unsigned char left_side = PLAYFIELD[i-d];
	unsigned char right_side;
	if( ! is_noun(left_side) ) {
		return;
	}
	left_side = foreground(left_side);
	right_side = foreground(PLAYFIELD[i+d]);
	if( is_noun(right_side) ) {
		noun_is_noun(left_side,right_side);
	} else if( is_prop(right_side) ) {
		noun_is_prop(left_side, right_side);
	}
}

void noun_is_noun(unsigned char left_side, unsigned char right_side) {
	print_gr(left_side);
	if(left_side == right_side) {
		printf(" is self\n");
		/* this rule isn't implemented yet */
		obj_properties[left_side & 7] |= PROP_SELF;
	} else {
		obj_transforms[left_side & 7] = right_side & 7;
	}
}

void noun_is_prop(unsigned char left_side, unsigned char right_side) {
	obj_properties[left_side & 7] |= 1 << (right_side & 0x0F);
}

/*
 * attempt to move all YOU tiles in the given direction
 */
void move_YOU_tiles(signed char dx) {
	unsigned char i;
	unsigned int r;
	if(dx == 0) {
		return;
	}
	for(i=0; i<LEVEL_TILES; ++i) {
		r = tile_props(i);
		if(r & PROP_YOU) {
			move_obj(i, dx);
		}
	}
}

/*
 * obj at x wants to move in direction dx
 */
void move_obj(unsigned char i, signed char dx) {
/*
 1030 rem obj at x wants to move dx
 */
 	/* 1035 ds=x+dx:ck=ds */
	unsigned char ds = i+dx;
	unsigned char ck = ds;
	unsigned char bg;
	/* build a train of tiles */
	do {
		if(ck > LEVEL_TILES || (tile_props(ck) & PROP_STOP)) {
			/* movement is blocked entirely */
			return;
		}
		if( (tile_props(ck) & PROP_PUSH) || is_text(PLAYFIELD[ck])) {
			ck = ck + dx;
		} else {
			break;
		}
	} while(1);
	/* 1060 if(fnpp(ck)and64)=0 then 1065 */
	/* line 1060 had no effect but checked PROP_SINK */
	/* move tiles head-first */
	do {
		if( PLAYFIELD[ck] < 8 && 
				!(tile_props(ck) & PROPS_STOP_PUSH_OPEN) ) {
			/* move non-displaced object to background */
			bg = PLAYFIELD[ck] << 5;
		} else if( PLAYFIELD[ck] < 8) {
			bg = PLAYFIELD[ck] & 224;
		} else {
			bg = PLAYFIELD[ck] & 224;
		}
		push_delta( ck, foreground(PLAYFIELD[ck-dx]) | bg );
		if( ck != ds) {
			ck -= dx;
		} else {
			break;
		};
	} while (1);

	push_delta(ck-dx, background(PLAYFIELD[i]));

}
