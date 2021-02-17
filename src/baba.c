#include "constants.h"
#include "gfx.h"
#include <stdio.h>
#include <cbm.h>

#define TEXT_YOU   0x10
#define TEXT_WIN   0x11
#define TEXT_STOP  0x12
#define TEXT_PUSH  0x13
#define TEXT_SHUT  0x14
#define TEXT_OPEN  0x15
#define TEXT_SINK  0x16
#define TEXT_LOSE  0x17
#define TEXT_HOT   0x18
#define TEXT_MELT  0x19
#define TEXT_26    0x1A
#define TEXT_27    0x1B
#define TEXT_28    0x1C
#define TEXT_29    0x1D
#define TEXT_HAS   0x1E
#define TEXT_IS    0x1F
#define PROP_YOU   0x0001
#define PROP_WIN   0x0002
#define PROPS_YOU_WIN 0x0003
#define PROPS_YOU_LOSE 0x0081
#define PROP_STOP  0x0004
#define PROP_PUSH  0x0008
#define PROP_SHUT  0x0010
#define PROP_OPEN  0x0020
#define PROPS_OPEN_SHUT  0x0030
#define PROP_SINK  0x0040
#define PROP_LOSE  0x0080
#define PROP_HOT   0x0100
#define PROP_MELT  0x0200
#define PROPS_HOT_MELT  0x0300
#define PROP_26    0x0400
#define PROP_27    0x0800
#define PROP_28    0x1000
#define PROP_29    0x2000
#define PROP_30    0x4000
#define PROP_SELF  0x8000 /* set when SAME IS SAME blocks transforms */
#define is_noun(x) ((x & 0x18) == 0x08)
#define is_prop(x) ((x & 0x18) == 0x10 && x < 0x1E)
#define is_obj(x)  ((x & 0x18) == 0)

#define foreground(x) (x & 0x1F)
#define background(x) ((unsigned char) x >> 5)

extern void __fastcall__ print_gr(unsigned char);

unsigned int obj_properties[8];
unsigned char obj_transforms[8];

unsigned char you_win;

void compile_rules(void);
void process_IS(unsigned char i, unsigned char d);
void noun_is_noun(unsigned char left_side, unsigned char right_side);
void noun_is_prop(unsigned char left_side, unsigned char right_side);
signed char move(void);
void destroy_rule(unsigned char ck, int property);
void gosub765(unsigned char ck, unsigned char new_value) ;

void play_loop(void) {
	signed char baba_move;
	compile_rules();
	baba_move = move();
}

signed char move(void) {
	char f;
	while(1) {
		f = cbm_k_getin();
		switch(f) {
			case CH_CURS_UP:
				return -LEVEL_WIDTH;
			case CH_CURS_DOWN:
				return LEVEL_WIDTH;
			case CH_CURS_LEFT:
				return -1;
			case CH_CURS_RIGHT:
				return 1;
			case 'Z': // wait
				return 0;
			case 'U':
				break;
		}
	}
}

/*
 *   245 ru%(0)=0
  255 n=pf%(ck):ifn=0then295
  260 if(nand24)=0theniftr%(nand7)theniftr%(nand7)<>(nand7) then gosub700:rem fg
  265 ifn>32theniftr%(n/32)theniftr%(n/32)<>int(n/32) then gosub720:rem bg
  270 r=ru%(nand31)orru%(n/32)
  275 if(rand48)=48 then np=0:gosub 765:goto295:rem open-shut destroys both
  280 if(rand64)=64thenif pf%(ck)>32 thennp=0:gosub765:rem sink
  285 if(rand768)=768then dr=512:gosub 785:goto295:rem hot/melt
  286 if(rand129)=129 then dr=1:gosub 785:goto295:rem you/lose
  290 ifrand1thenu%(u)=ck:u=u+1:ifrand2thenwin=1
  295 nextck:gosub500:poke 53280,14

 */
void herp() {
	/* 250 u=0:win=0:for ck=0tomx */
	unsigned char ck, n;
	unsigned char you_tiles=0; 
	int r;
	you_win=0;
	for (ck=0; ck < LEVEL_TILES; ++ck) {
		/* 255 n=pf%(ck):ifn=0then295 */
		n=PLAYFIELD[ck];
		if(n == 0) {
			continue;
		}
		/* 260 if(nand24)=0theniftr%(nand7)theniftr%(nand7)<>(nand7) then gosub700:rem fg */
		if(is_obj(foreground(n)) && obj_transforms[foreground(n)] ) {
			/* TODO gosub 700 */
		}
		/* 265 ifn>32theniftr%(n/32)theniftr%(n/32)<>int(n/32) then gosub720:rem bg */
		if(n > 31 && obj_transforms[background(n)]) {
			/* TODO: gosub 720 */
		}
		/* 270 r=ru%(nand31)orru%(n/32) */
		/* interactions */
		r = obj_properties[foreground(n)] | obj_properties[background(n)];
		/* 275 if(rand48)=48 then np=0:gosub 765:goto295:rem open-shut destroys both */
		if((r & PROPS_OPEN_SHUT) == PROPS_OPEN_SHUT) {
			gosub765(ck, 0); /* destroys both */
		/* 280 if(rand64)=64thenif pf%(ck)>32 thennp=0:gosub765:rem sink */
		} else if ((r & PROP_SINK) && PLAYFIELD[ck] > 32) {
			gosub765(ck, 0); /* any object + sink = empty */
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

void gosub765(unsigned char ck, unsigned char new_value) {
	/* TODO: push to undo stack */
	PLAYFIELD[ck] = new_value;
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
	unsigned char max = LEVEL_TILES;
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
	if(left_side == right_side) {
		obj_properties[left_side & 7] |= PROP_SELF;
	} else {
		obj_transforms[left_side & 7] = right_side & 7;
	}
}

void noun_is_prop(unsigned char left_side, unsigned char right_side) {
	obj_properties[left_side & 7] |= 1 << (right_side & 0x0F);
}

