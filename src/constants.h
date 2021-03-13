
/* w=10:h=18:mx=h*w-1:sm=49152:is=31 */

#define LEVEL_WIDTH   16
#define LEVEL_HEIGHT  10
#define LEVEL_TILES   160

#define PLAYFIELD    ((unsigned char*) 0xC000)

#define MAX_DELTAS   128

/*
 * Text tile values for properties and verbs.
 */
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

/*
 * Bit values of properties stored in obj_properties[]
 */
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
#define PROPS_STOP_PUSH_OPEN 0x002C /* used in BASIC line 1075 */
#define PROP_26    0x0400
#define PROP_27    0x0800
#define PROP_28    0x1000
#define PROP_29    0x2000
#define PROP_30    0x4000 /* will probably be HAS */
/* Property bit that would otherwise go unused because of IS block */
#define PROP_SELF  0x8000 /* TODO: set when SAME IS SAME blocks transforms */


#define is_noun(x) ((x & 0x18) == 0x08)
#define is_prop(x) ((x & 0x18) == 0x10 && x < 0x1E)
#define is_obj(x)  ((x & 0x18) == 0)
#define is_text(x)  ((x & 0x18) != 0)


#define foreground(x) (x & 0x1F)
#define background(x) ((unsigned char) x >> 5)

#define pf_fg(x) (foreground(PLAYFIELD[x]))
#define pf_bg(x) (background(PLAYFIELD[x]))

/* collect property bits of all overlapping objects */
#define tile_props(x) ((is_obj(pf_fg(x)) ? obj_properties[pf_fg(x)] : PROP_PUSH) | \
		obj_properties[pf_bg(x)])

