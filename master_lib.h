#ifndef _M_LIB_
#define _M_LIB_
#include "scacchiera.h"
#define PLAYER "player"

typedef struct _pawn_data{
        pid_t my_pid;
        int owner;      /*proprietario della pedina*/
        int pos;        /*posizione all'interno dell'array pawn_list del giocatore*/
        int moves;      /*mosse rimanenti della pedina*/
        int x;          /*posizione sulla scacchiera*/
        int y;
        int target;     /*indice della banderina da conquistare*/
} pawn_data;

typedef struct _keys_storage{
        int ks_shm_id;          /*id della struct keys storage*/
        int conf_id;            /*id della configurazione del gioco*/
        int chess_id;           /*id della scacchiera*/
        int msgq_id;            /*id della coda di messaggi*/
        int round_flag_id;      /*id del punatore a bandierine*/
        int sem_sync_round;     /*id del secondo semaforo di sincronizzazione player-master-pawn*/
        int sem_set_pl;         /*id del set di semafori circolari per la sincronizzazione tra giocatori*/
} keys_storage;

struct message{
        long type;
        int msgc[2]; 
};

/*restituisce un char* che corrisponde all'int passato come argomento*/
char* integer_to_string_arg(int);

/*riempimento della struct delle chiavi*/
keys_storage* fill_storage_shm(int, int, int, int, int, int);

/*restituisce un int tra i due interi passati*/
int get_rand_fl(int, int);

/*crea un array di bandierine con valore totale definito dalla configurazione*/
int* randomize_flags(int, int, game_config*, slot*);

/*stampa metriche richieste*/
void print_metrics(game_config*,int*,int*,int, double);

#endif