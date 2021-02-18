
#include "constants.h"

/*
 * some variables from BASIC where i'm not sure if they should be global
 */

void push_undo(unsigned char pos);
void push_delta(unsigned char pos, unsigned char tile) ;
void apply_deltas(void);

