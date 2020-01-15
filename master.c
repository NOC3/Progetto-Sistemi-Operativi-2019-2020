#define _GNU_SOURCE
#include "master_lib.h"

game_config *my_gc;
keys_storage *my_ks;
int *score_arr;
int *array_id_pawn;
int gc_id_shm, key_id_shm, fl_id_shm, sem_sync_round, sem_set_pl, msgq_id, ch_id_shm;
int num_round;
pid_t *players_pid;  
slot *chessboard;
clock_t start,stop;
double duration;

void handle_signal(int signum){
        int i, j;
        pawn_data *pawn_list;

        stop=clock();
        duration=(double)(stop-start);
        printf("GAME ENDED\nPrinting chessboard and statistics.\n\n");
        for(i=0; i<my_gc->num_player; i++){
                pawn_list=(pawn_data*)shmat(array_id_pawn[i],NULL,0);
                for(j=0; j<my_gc->num_pawn; j++){
                        kill(pawn_list[j].my_pid, SIGTERM);
                }
                shmctl(array_id_pawn[i], IPC_RMID, NULL);
                kill(players_pid[i], SIGKILL);
        }
        print_chessboard(chessboard, my_gc, score_arr, array_id_pawn, num_round);
        print_metrics(my_gc, score_arr, array_id_pawn, num_round, duration);
        /*rimozione semafori e code*/
        semctl(sem_sync_round, 0, IPC_RMID);
        semctl(sem_set_pl, 0, IPC_RMID);
        msgctl(msgq_id, IPC_RMID, 0);
        /*free dei puntatori*/
        free(array_id_pawn);
        free(players_pid);
        free(score_arr);
        /*pulizia semafori*/
        clean_sem_chessboard(my_gc->height, my_gc->width, chessboard);
        /*rimozione ipcs*/
        shmctl(ch_id_shm, IPC_RMID, NULL);
        shmctl(gc_id_shm, IPC_RMID, NULL);
        shmctl(key_id_shm, IPC_RMID, NULL);
        shmctl(fl_id_shm, IPC_RMID, NULL);
        
        if(signum==SIGALRM){
                /*fine gioco causa SIGALRM*/
                printf("The game ended because the alarm went off.\n");
                exit(EXIT_SUCCESS);
        } else {
                /*fine gioco SIGINT*/
                printf("The game ended because the SIGINT signal was received.\n");
                exit(EXIT_FAILURE);
        }
}

int main(){
        int i, num_fl;
        char *args[4]={PLAYER};
        int* round_flags;
        struct message mexSnd;
        struct message mexRcv;
        struct sigaction sa;
        sigset_t my_mask;
        struct pawn_data *pawn_list;

        start=clock();
        /*mask di SIGINT durante la configurazione, sbloccato durante il gioco*/
        sigemptyset(&my_mask);                  
	sigaddset(&my_mask, SIGINT);           
	sigprocmask(SIG_BLOCK, &my_mask, NULL); 

        bzero(&sa, sizeof(sa));  
        sa.sa_handler = handle_signal; 
        sigaction(SIGALRM, &sa, NULL); 
        sigaction(SIGINT, &sa, NULL);

        /*allocazione e inizializzazione memoria condivisa della configurazione*/
        if((gc_id_shm=shmget(IPC_PRIVATE, sizeof(game_config), IPC_CREAT|0666))==-1){
                TEST_ERROR
        }
        my_gc=init_game_config(gc_id_shm); 
        
        /*alloco array necessari*/
        array_id_pawn=calloc(my_gc->num_player, sizeof(int));
        players_pid=calloc(my_gc->num_player, sizeof(pid_t));
        score_arr=calloc(my_gc->num_player, sizeof(int));

        /*azzero punteggi dei giocatori*/
        for(i=0; i<my_gc->num_player; i++){
                score_arr[i]=0;
        }

        /*alloco e inizializzo scacchiera*/
        if((ch_id_shm=shmget(IPC_PRIVATE, my_gc->height*my_gc->width*my_gc->height*sizeof(slot*), IPC_CREAT|0666))==-1){
                TEST_ERROR
        }
        chessboard=create_chessboard(my_gc->height, my_gc->width, ch_id_shm);

        /*coda di messaggio*/
        if((msgq_id=msgget(IPC_PRIVATE, IPC_CREAT|0666))==-1){
                TEST_ERROR
        }

        /*inizializzo semafori*/
        if((sem_sync_round=semget(IPC_PRIVATE, 4, IPC_CREAT|0666))==-1){
                TEST_ERROR
        }
        for(i=0; i<4; i++){
                switch(i){
                        case 0:
                        if((init_sem_to_val(sem_sync_round, i, my_gc->num_player+1))==-1){
                                TEST_ERROR 
                        }
                        break;

                        case WAITING:
                        if((init_sem_to_val(sem_sync_round, i, 1))==-1){
                                TEST_ERROR 
                        }
                        break;

                        case STARTING:
                        if((init_sem_to_val(sem_sync_round, i, 0))==-1){
                                TEST_ERROR 
                        }
                        break;

                        case ENDING:
                        if((init_sem_to_val(sem_sync_round, i, 0))==-1){
                                TEST_ERROR 
                        }
                }
        }

        if((sem_set_pl=semget(IPC_PRIVATE, my_gc->num_player, IPC_CREAT|0666))==-1){
                TEST_ERROR
        }
        for(i=0; i<my_gc->num_player; i++){
                if((init_sem_to_val(sem_set_pl, i, 0))==-1){
                        TEST_ERROR 
                }
        }

        /*ipcs insieme di chiavi*/
        if((key_id_shm=shmget(IPC_PRIVATE, sizeof(keys_storage), IPC_CREAT|0666))==-1){
                TEST_ERROR
        }
        my_ks=fill_storage_shm(key_id_shm, gc_id_shm, ch_id_shm, msgq_id, sem_sync_round, sem_set_pl);

        args[1]=integer_to_string_arg(key_id_shm); /*id dell'insieme delle chiavi*/
        /*args[2]= numero giocatore(gestito in seguito)*/
        args[3]=NULL;

        for(i=0; i<my_gc->num_player; i++){
                switch(players_pid[i]=fork()){
                        case -1:
                                TEST_ERROR
                                break;
                        case 0:
                                args[2]=integer_to_string_arg(i); /*numero giocatore*/
                                execve(PLAYER, args, NULL);
                                TEST_ERROR
                                break;
                        default:
                                break;
                }
        }
        wait_zero(sem_sync_round, 0);
        /*master aspetta i messaggi con gli indirizzi degli array delle pedine*/
        for(i=0; i<my_gc->num_player;i++){
                mexRcv.type=PLAYER_TO_MASTER;
                if((msgrcv(my_ks->msgq_id, &mexRcv, sizeof(mexRcv)-sizeof(long), mexRcv.type,0))==-1){
                        TEST_ERROR 
                }
                array_id_pawn[mexRcv.msgc[0]]=mexRcv.msgc[1];
        }
        
        sem_release(sem_set_pl, 0);
        increase_resource(sem_sync_round, 0, my_gc->num_player+1);
        wait_zero(sem_sync_round, 0);
        
        /*inizializzazione gioco finita, riabilito SIGINT*/
        sigaddset(&my_mask, SIGINT);           
	sigprocmask(SIG_UNBLOCK, &my_mask, NULL);

        mexRcv.type=PAWN_TO_MASTER;
        num_round=0;
        
        while(1){
                increase_resource(sem_sync_round, STARTING, ((my_gc->num_player)+(my_gc->num_player*my_gc->num_pawn)));
                /*inizio round*/
                num_round+=1;
                num_fl=get_rand_fl(my_gc->min_flag, my_gc->max_flag);
                if((fl_id_shm=shmget(IPC_PRIVATE, (num_fl+1)*sizeof(int), IPC_CREAT|0666))==-1){
                        TEST_ERROR
                }
                round_flags=randomize_flags(fl_id_shm, num_fl, my_gc, chessboard);
                my_ks->round_flag_id=fl_id_shm;
                print_chessboard(chessboard, my_gc, score_arr, array_id_pawn, num_round);
                sem_reserve(sem_sync_round, WAITING);
                check_zero(sem_sync_round, STARTING);
                alarm(my_gc->max_round_time);
                sem_release(sem_sync_round, WAITING);

                while(num_fl>0){
                        /*attendo messaggi conquiste*/
                        msgrcv(my_ks->msgq_id, &mexRcv, sizeof(mexRcv)-sizeof(long), mexRcv.type,0);
                        score_arr[mexRcv.msgc[0]]+=chessboard[mexRcv.msgc[1]].g_flag_value;
                        chessboard[mexRcv.msgc[1]].g_flag_value=0;
                        num_fl-=1;
                }
                /*disattivo allarme*/
                alarm(0);
                if((shmdt(round_flags))==-1){
                        TEST_ERROR
                }  
                /*fine round*/
                increase_resource(sem_sync_round, ENDING, ((my_gc->num_player)+(my_gc->num_player*my_gc->num_pawn)));
                print_chessboard(chessboard, my_gc, score_arr, array_id_pawn, num_round);
                /*aspetto che tutte le pedine e tutti i giocatori abbiano finito*/
                check_zero(sem_sync_round, ENDING);
                if((shmctl(fl_id_shm, IPC_RMID, NULL))==-1){
                        TEST_ERROR
                }
        }
}