#include "constants.h"
#include "gfx.h"

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
#define RULE_YOU   0x0001
#define RULE_WIN   0x0002
#define RULE_STOP  0x0004
#define RULE_PUSH  0x0008
#define RULE_SHUT  0x0010
#define RULE_OPEN  0x0020
#define RULE_SINK  0x0040
#define RULE_LOSE  0x0080
#define RULE_HOT   0x0100
#define RULE_MELT  0x0200
#define RULE_26    0x0400
#define RULE_27    0x0800
#define RULE_28    0x1000
#define RULE_29    0x2000
#define RULE_30    0x4000
#define RULE_31    0x8000
#define is_noun(x) ((x & 0x18) == 0x08)
#define is_prop(x) ((x & 0x18) == 0x10 && x < 0x1E)
#define is_obj(x)  ((x & 0x18) == 0)

#define foreground(x) (x & 0x1F)
#define background(x) ((unsigned char) x >> 5)

extern void __fastcall__ print_gr(unsigned char);

unsigned int obj_properties[8];
unsigned char obj_transforms[8];

void compile_rules(unsigned char d);
void process_IS(unsigned char i, unsigned char d);
void noun_is_noun(unsigned char left_side, unsigned char right_side);
void noun_is_prop(unsigned char left_side, unsigned char right_side);

void play_loop(void) {
	compile_rules(1);
	compile_rules(LEVEL_WIDTH);
}

void compile_rules(unsigned char d) {
	unsigned char i;
	unsigned char max = LEVEL_TILES;
	max -= d;
	for(i=d; i < max; i++) {
		if( foreground(PLAYFIELD[i]) == TEXT_IS) {
			process_IS(i, d);
		}
	}
}

void process_IS(unsigned char i, unsigned char d) {
	unsigned char left_side = PLAYFIELD[i-d];
	unsigned char right_side;
	if( ! is_noun(left_side) ) {
		return;
	}
	right_side = PLAYFIELD[i+d];
	if( is_noun(right_side) ) {
		noun_is_noun(left_side,right_side);
	} else if( is_prop(right_side) ) {
		noun_is_prop(left_side, right_side);
	}
}

void noun_is_noun(unsigned char left_side, unsigned char right_side) {
	print_gr(left_side);
	print_gr(31);
	print_gr(right_side);
	print_gr(1);
	print_gr(0);
}

void noun_is_prop(unsigned char left_side, unsigned char right_side) {
	print_gr(left_side);
	print_gr(31);
	print_gr(right_side);
	print_gr(2);
	print_gr(0);
}

