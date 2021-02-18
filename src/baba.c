#include "constants.h"
#include "gfx.h"
#include "levels.h"
#include "undo.h"
#include <stdio.h>
#include <cbm.h>

extern void __fastcall__ print_gr(unsigned char);
extern unsigned char current_level;

unsigned int obj_properties[8];
unsigned char obj_transforms[8];

unsigned char current_turn=1;
unsigned char you_win=0;

void compile_rules(void);
void process_IS(unsigned char i, unsigned char d);
void noun_is_noun(unsigned char left_side, unsigned char right_side);
void noun_is_prop(unsigned char left_side, unsigned char right_side);
signed char __fastcall__ get_move(void);
void destroy_rule(unsigned char ck, int property);
void move_YOU_tiles(signed char dx);
void move_obj(unsigned char, signed char);

void play_loop(void) {
	signed char baba_move;
	while(!you_win) {
		compile_rules();
		draw_playfield();
		/*
		printf("52: %02x, %02x, %02x, %04x\n",
				PLAYFIELD[52],
				pf_fg(52),
				pf_bg(52),
				tile_props(52) );
		printf("pr:%04x,%04x,%04x,%04x\n",
				obj_properties[1],
				obj_properties[2],
				obj_properties[3],
				obj_properties[4]);
				*/
		printf("turn %d\n", current_turn);
		baba_move = get_move();
		if(baba_move) {
			move_YOU_tiles(baba_move);
		} else {
		}
		apply_deltas();
		++current_turn;
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
			case 'z': // wait
				return 0;
			case 'r':
				/* TODO: restart */
				load_level(current_level);
				/* invalidate all of undo stack */
				clear_undo_stack();
				draw_playfield();
			case 'u':
				perform_undo();
				draw_playfield();
			default:
				break;
		}
	}
}

/*
 * process all the consequences of the current ruleset
 * after the player's last move
 */
void process_rules() {
	/* 250 u=0:win=0:for ck=0tomx */
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
		/* 260 if(nand24)=0theniftr%(nand7)theniftr%(nand7)<>(nand7) then gosub700:rem fg */
		if(is_obj(foreground(n)) && obj_transforms[foreground(n)] ) {
			/* noun is noun foreground */
			/* 705 gosub 740 */
			push_undo(ck);
			/* 710 n=nand224ortr%(nand7):pf%(ck)=n */
			PLAYFIELD[ck] = n = (n & 0xE0) | obj_transforms[foreground(n)];
			
		}
		/* 265 ifn>32theniftr%(n/32)theniftr%(n/32)<>int(n/32) then gosub720:rem bg */
		if(n > 31 && obj_transforms[background(n)]) {
			/* noun is noun background */
			/* 725 gosub 740 */
			push_undo(ck);
			/* 730 n=nand31or(tr%(n/32)*32):pf%(ck)=n */
			PLAYFIELD[ck] = n = (n & 0x1F) | (obj_transforms[background(n)] << 5);
		}
		/* 270 r=ru%(nand31)orru%(n/32) */
		/* interactions */
		r = obj_properties[foreground(n)] | obj_properties[background(n)];
		/* 275 if(rand48)=48 then np=0:gosub 765:goto295:rem open-shut destroys both */
		if((r & PROPS_OPEN_SHUT) == PROPS_OPEN_SHUT) {
			push_delta(ck, 0); /* destroys both */
		/* 280 if(rand64)=64thenif pf%(ck)>32 thennp=0:gosub765:rem sink */
		} else if ((r & PROP_SINK) && PLAYFIELD[ck] > 32) {
			push_delta(ck, 0); /* any object + sink = empty */
		/* 285 if(rand768)=768then dr=512:gosub 785:goto295:rem hot/melt */
		} else if ((r & PROPS_HOT_MELT) == PROPS_HOT_MELT) {
			destroy_rule(ck, PROP_MELT);
		/* 286 if(rand129)=129 then dr=1:gosub 785:goto295:rem you/lose */
		} else if ((r & PROPS_YOU_LOSE) == PROPS_YOU_LOSE) {
			destroy_rule(ck, PROP_YOU);
		/* 290 ifrand1thenu%(u)=ck:u=u+1:ifrand2thenwin=1 */
		} else if (r & PROP_YOU) {
			++you_tiles;
			// TODO: add to "you" tile list?
			if (r & PROP_WIN) {
				you_win=1;
			}
		}
	}	
	/* TODO: if no you_tiles then display warning */
	// TODO: gosub 500
	// TODO: poke 53280,14
}

/*
 * for destroying
 */
void destroy_rule(unsigned char ck, int property) {
	/* TODO: push to undo stack */
	unsigned char tile = PLAYFIELD[ck];
	if(obj_properties[foreground(tile)] & property) {
		tile = background(tile);
		PLAYFIELD[ck] = tile;
	} else if(obj_properties[background(tile)] & property) {
		tile = foreground(tile);
		PLAYFIELD[ck] = tile;
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
		printf("\005n @ %d+%d ", i, d);
		noun_is_noun(left_side,right_side);
	} else if( is_prop(right_side) ) {
		noun_is_prop(left_side, right_side);
	}
}

void noun_is_noun(unsigned char left_side, unsigned char right_side) {
	print_gr(left_side);
	if(left_side == right_side) {
		printf(" is self\n");
		obj_properties[left_side & 7] |= PROP_SELF;
	} else {
		printf(" is %d\n", right_side);
		obj_transforms[left_side & 7] = right_side & 7;
	}
}

void noun_is_prop(unsigned char left_side, unsigned char right_side) {
	/* printf("rule %d:%04x->", left_side & 7, obj_properties[left_side & 7]); */
	obj_properties[left_side & 7] |= 1 << (right_side & 0x0F);
	/* printf("%04x\n",  obj_properties[left_side & 7]); */
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
	printf("searching\n");
	for(i=0; i<LEVEL_TILES; ++i) {
		r = tile_props(i);
		if(r & PROP_YOU) {
			printf("\034you\005@%d (%02x:%04x)\n", i, PLAYFIELD[i], tile_props(i));
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
		/*
		1040 ifck<0orck>mxthenreturn:rem stop at edge
		1045 iffnpp(ck)and8 then ck=ck+dx:goto 1040:rem push property
		1050 ifpf%(ck)and24 then ck=ck+dx:goto 1040:rem push text
		1055 iffnpp(ck)and4then return:rem stop
		 */
		if(ck > LEVEL_TILES || (tile_props(ck) & PROP_STOP)) {
			/* movement is blocked entirely */
			printf("s@ %d. ", ck);
			return;
		}
		if( (tile_props(ck) & PROP_PUSH) || is_text(PLAYFIELD[ck])) {
			printf("p@ %d. ", ck);
			ck = ck + dx;
		} else {
			printf("[%d]", ck);
			break;
		}
	} while(1);
	/* 1060 if(fnpp(ck)and64)=0 then 1065 */
	/* line 1060 had no effect but checked PROP_SINK */
	/* move tiles head-first */
	do {
		/* 1070 bg=pf%(ck)and224
		1075 ifpf%(ck)<8thenif(fnpp(ck)and44)=0thenbg=pf%(ck)*32
		1080 np=(pf%(ck-dx)and31)or bg:gosub765
		1085 if ck<>ds then ck=ck-dx:goto 1070
		*/
		if( PLAYFIELD[ck] < 8 && 
				!(tile_props(ck) & PROPS_STOP_PUSH_OPEN) ) {
			/* move non-displaced object to background */
			bg = PLAYFIELD[ck] << 5;
		} else {
			bg = background(PLAYFIELD[ck]);
		}
		push_delta( ck, PLAYFIELD[ck-dx] | bg );
		if( ck != ds) {
			ck -= dx;
		} else {
			break;
		};
	} while (1);

	/*1090 ck=ck-dx:np=pf%(x)/32:gosub 765:rem restore bg */
	push_delta(ck-dx, background(PLAYFIELD[i]));

	/* 1095 return */
}
