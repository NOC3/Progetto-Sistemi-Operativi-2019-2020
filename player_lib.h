#ifndef _PL_LIB_
#define _PL_LIB_
#define PAWN "pawn"
#include "master_lib.h"

/*randomizzazione coordinate pedina*/
void randomize_coordinate_pawn(pawn_data*, int, int, int, slot*);

/*calcolo target per ogni bandierina*/
void compute_targets(pawn_data*, int*, int, slot*);

#endif