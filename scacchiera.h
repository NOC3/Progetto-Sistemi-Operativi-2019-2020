#ifndef _SC_LIB_
#define _SC_LIB_
/*colori ANSI supportati fino ad un massimo di 12 giocatori*/
#define RED "[0;31m" 
#define GRN "[0;32m"
#define YEL "[0;33m"
#define BLU "[0;34m"
#define MAG "[0;35m"
#define CYN "[0;36m"
#define BRED "[1;31m"
#define BGRN "[1;32m"
#define BYEL "[1;33m"
#define BBLU "[1;34m"
#define BMAG "[1;35m"
#define BCYN "[1;36m"
#include "config.h"
#include "sem_lib.h"
#include <sys/shm.h>
#include <stdlib.h>

typedef struct _slot{
        key_t c_sem_id;         /*id del semaforo associato alla casella*/
        int g_flag_value;       /*valore della bandierina se presente, 0 altrimenti*/
        int num_owner;          /*numero del player se pedina presente, 0 altrimenti*/
        int x;
        int y;
} slot;

/*alloca in memoria condivisa un array di struct slot*/
slot* create_chessboard(int, int, int);

/*pulizia semafori della scacchiera*/
void clean_sem_chessboard(int, int, slot*);

/*stampa numero round giocati, scacchiera e messo residue delle pedine del giocatore*/
void print_chessboard(slot*, game_config*, int*, int*, int);

/*funzioni di colore del codice*/
void color(char*);
void reset();

#endif