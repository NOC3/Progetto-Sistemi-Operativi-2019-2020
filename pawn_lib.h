#ifndef _PA_LIB_
#define _PA_LIB_
#include "scacchiera.h"
#include "master_lib.h"

/*alloca la pedina sulla scacchiera*/
void allocate_pawn(int, int, int, slot*, int);

/*restituisce l'indice della bandierina non nulla piu' vicina alla pedina*/
int calculate_target(pawn_data , int*, slot*);

#endif 