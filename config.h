#ifndef _CONF_LIB_
#define _CONF_LIB_
#define MASTER_TO_PLAYER 1
#define PLAYER_TO_MASTER 2
#define PAWN_TO_MASTER 3
#define WAITING 1
#define STARTING 2
#define ENDING 3

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>

#define TEST_ERROR      printf("%s:%d: PID=%5d: Error %d (%s)\n",__FILE__,__LINE__,getpid(),errno,strerror(errno));\
                        printf("exit:EXIT_FAILURE\n");\
                        raise(SIGINT);

typedef struct _game_config{
        int num_player;
        int num_pawn;
        int max_round_time;
        int height;
        int width;
        int min_flag;
        int max_flag;
        int tot_score_flag;
        int pawn_moves;
        int min_sec_hold_pawn;
} game_config;

/*riempie configurazione*/
game_config* init_game_config();

/*ritorna il valore intero di una stringa di caratteri del file .conf*/
int find_value(char *);

#endif