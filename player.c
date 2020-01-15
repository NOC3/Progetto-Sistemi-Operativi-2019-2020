#define _GNU_SOURCE
#include "player_lib.h"

/*player manda segnale sigint al master*/
void handle_signal(){
        kill(getppid(), SIGINT);
}

int main(int argc, char * argv[]) {
        keys_storage *my_ks;
        slot *chessboard;
        game_config *my_gc;
        int* round_flags;
        pawn_data *pawn_list;
        struct sembuf sem;
        struct message mexRcv;
        struct message mexSnd;
        struct sigaction sa;
        char* args[6]={PAWN};
        int keys_id, num_p, pawn_list_id;
        int i;

        bzero(&sa, sizeof(sa));  
        sa.sa_handler = handle_signal; 
        sigaction(SIGINT, &sa, NULL);

        /*numero giocatore*/
        num_p=atoi(argv[2]);

        /*attach memoria condivisa*/
        keys_id=atoi(argv[1]);
        if((my_ks=shmat(keys_id, NULL, 0))== (void *) -1){
                TEST_ERROR
        } 

        if((my_gc=shmat(my_ks->conf_id, NULL, 0))== (void *) -1){
                TEST_ERROR
        }
        if((chessboard=shmat(my_ks->chess_id, NULL, 0))== (void *) -1){
                TEST_ERROR
        }
        /*allocazione array lista pedine*/
        if((pawn_list_id=shmget(IPC_PRIVATE,sizeof(pawn_data)*my_gc->num_pawn, IPC_CREAT|0666))== -1){
                TEST_ERROR
        }
        if((pawn_list=shmat(pawn_list_id, NULL, 0))== (void *) -1){
                TEST_ERROR
        }
                        
        args[1]=argv[1];/*id insieme di chiavi*/
        args[2]=argv[2];/*numero giocatore*/
        args[3]=integer_to_string_arg(pawn_list_id);/*id lista pedine*/
        args[5]=NULL;

        wait_zero(my_ks->sem_sync_round, 0);
        mexSnd.type=PLAYER_TO_MASTER;
        mexSnd.msgc[0]=atoi(argv[2]);/*numero giocatore*/
        mexSnd.msgc[1]=pawn_list_id;/*comunico al master id array lista pedine*/
        msgsnd(my_ks->msgq_id, &mexSnd, sizeof(mexSnd)-sizeof(long), 0);

        for(i=0;i<my_gc->num_pawn; i++){
                sem_reserve(my_ks->sem_set_pl, num_p);
                randomize_coordinate_pawn(pawn_list,i,my_gc->width, my_gc->height, chessboard);
                switch (fork()){
                case -1:
                        TEST_ERROR
                        break;
                case 0: 
                        args[4]=integer_to_string_arg(i);/*numero pedina attuale*/
                        execve(PAWN, args, NULL);
                        TEST_ERROR
                default:
                        break;
                }               
                sem_reserve(my_ks->sem_set_pl, num_p);
                /*sblocco prossimo player*/
                sem_release(my_ks->sem_set_pl, (atoi(argv[2])+1)%(my_gc->num_player));
        }
        /*tutti i giocatori hanno messo le pedine*/
        wait_zero(my_ks->sem_sync_round,0);
        
        while(1){
                /*inizio round*/
                check_zero(my_ks->sem_sync_round, WAITING);
                
                if((round_flags=(int*)shmat(my_ks->round_flag_id, NULL, 0))== (void *) -1){
                        TEST_ERROR
                }
                /*giocatore calcola target e lo setta nelle pedine*/
                compute_targets(pawn_list,round_flags, my_gc->num_pawn, chessboard);
                wait_zero(my_ks->sem_sync_round, STARTING);

                if((shmdt(round_flags))==-1){
                        TEST_ERROR
                }
                /*attesa fine round*/
                wait_zero(my_ks->sem_sync_round, ENDING);
                /*fine round*/
        }
}