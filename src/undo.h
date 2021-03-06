
#include "constants.h"

void set_with_undo(unsigned char pos, unsigned char tile);
void push_undo(unsigned char pos);
void push_delta(unsigned char pos, unsigned char tile) ;
void apply_deltas(void);
void perform_undo(void);
void clear_undo_stack(void);


