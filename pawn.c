#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
#include "pawn_lib.h"

/*pawn manda segnale sigint al player che lo mandera' al master*/
void handle_signal(){
        kill(getppid(), SIGINT);
}

int main(int argc, char *argv[]){
        slot* chessboard;
        keys_storage* my_ks;
        game_config *my_gc;
        pawn_data *pawn_list;
        int* arr_target;
        int index, targ_x, targ_y, my_x, my_y, ex_x, ex_y, my_pos;
        struct message mexRcv;
        struct message mexSnd;
        struct sembuf sops;
        struct timespec tim;
        struct sigaction sa;

        bzero(&sa, sizeof(sa));  
        sa.sa_handler = handle_signal; 
        sigaction(SIGINT, &sa, NULL);
        
        /*attach memoria condivisa*/
        if((my_ks=shmat(atoi(argv[1]), NULL,0))==(void *)-1){
                TEST_ERROR
        }
        if((my_gc=(game_config*)shmat(my_ks->conf_id, NULL, 0))==(void*)-1){
                TEST_ERROR
        }
        if((chessboard=shmat(my_ks->chess_id, NULL, 0))==(void*)-1){
                TEST_ERROR
        }      
        if((pawn_list=shmat(atoi(argv[3]), NULL, 0))==(void *)-1){
                TEST_ERROR
        }
        
        /*FINE CONFIGURAZIONE*/

        /*inizializzo la struct della mia pedina*/
        my_pos=atoi(argv[4]);
        pawn_list[my_pos].owner=atoi(argv[2]); 
        pawn_list[my_pos].moves= my_gc->pawn_moves;
        pawn_list[my_pos].my_pid=getpid(); 
        pawn_list[my_pos].pos=my_pos;

        /*alloco la pedina e sblocco il semaforo del mio giocatore*/
        allocate_pawn(pawn_list[my_pos].x,pawn_list[my_pos].y,atoi(argv[2]),chessboard, my_gc->width); 
        sem_release(my_ks->sem_set_pl, (atoi(argv[2])));
        
        /*preparazione inizio gioco*/
        sops.sem_num = 0;
        sops.sem_op = -1;
        sops.sem_flg = IPC_NOWAIT;

        mexSnd.type=PAWN_TO_MASTER;
        mexSnd.msgc[0]=pawn_list[my_pos].owner; /*giocatore che possiede la pedina*/

        tim.tv_sec=0;
        tim.tv_nsec=(long)my_gc->min_sec_hold_pawn;
        
        my_x=pawn_list[my_pos].x;
        my_y=pawn_list[my_pos].y;
        ex_x=my_x;
        ex_y=my_y;

        while(1){
                /*inizio gioco*/
                check_zero(my_ks->sem_sync_round, WAITING);
                if((arr_target=(int*)shmat(my_ks->round_flag_id,NULL,0))==(void*)-1){
                        TEST_ERROR
                }
                wait_zero(my_ks->sem_sync_round, STARTING);

                /*pedina procede con il movimento se il player le ha fornito un target*/
                if(pawn_list[my_pos].target!=-1){
                        /*attach a arr_targetay bandierine*/
                        index=pawn_list[my_pos].target;
                        targ_x=chessboard[arr_target[index]].x;
                        targ_y=chessboard[arr_target[index]].y;
                        while(index!=-1&&pawn_list[my_pos].moves>0){
                                /*la bandierina è stata conquistata*/
                                if(chessboard[arr_target[index]].g_flag_value==0){
                                        index=calculate_target(pawn_list[my_pos], arr_target, chessboard);
                                        if(index==-1){
                                                break;
                                        }
                                        targ_x=chessboard[arr_target[index]].x;
                                        targ_y=chessboard[arr_target[index]].y;
                                }
                                /*provo a muovermi sulle x*/
                                if(pawn_list[my_pos].moves>0){ 
                                        if(my_x<targ_x){
                                                if(semop(chessboard[(my_x+1)*my_gc->width+my_y].c_sem_id, &sops, 1)!=-1){
                                                        if(chessboard[(my_x+1)*my_gc->width+my_y].g_flag_value!=0){
                                                                mexSnd.msgc[1]=(my_x+1)*my_gc->width+my_y;  
                                                                msgsnd(my_ks->msgq_id, &mexSnd, sizeof(mexSnd)-sizeof(long), 0); 
                                                        }
                                                        chessboard[my_x*my_gc->width+my_y].num_owner=0;
                                                        sem_release(chessboard[my_x*my_gc->width+my_y].c_sem_id, 0);
                                                        chessboard[(my_x+1)*my_gc->width+my_y].num_owner=pawn_list[my_pos].owner;
                                                        my_x+=1;
                                                        pawn_list[my_pos].moves-=1;
                                                        nanosleep(&tim, NULL);
                                                }
                                        } else if(my_x>targ_x){
                                                if(semop(chessboard[(my_x-1)*my_gc->width+my_y].c_sem_id, &sops, 1)!=-1){
                                                        if(chessboard[(my_x-1)*my_gc->width+my_y].g_flag_value!=0){
                                                                mexSnd.msgc[1]=(my_x-1)*my_gc->width+my_y;        
                                                                msgsnd(my_ks->msgq_id, &mexSnd, sizeof(mexSnd)-sizeof(long), 0);
                                                        }
                                                        chessboard[my_x*my_gc->width+my_y].num_owner=0;
                                                        sem_release(chessboard[my_x*my_gc->width+my_y].c_sem_id, 0);
                                                        chessboard[(my_x-1)*my_gc->width+my_y].num_owner=pawn_list[my_pos].owner;
                                                        my_x-=1;
                                                        pawn_list[my_pos].moves-=1;
                                                        nanosleep(&tim, NULL);
                                                }
                                        }
                                }
                                /*provo a muovermi sulle y*/
                                if(pawn_list[my_pos].moves>0){
                                        if(my_y<targ_y){
                                                if(semop(chessboard[my_x*my_gc->width+my_y+1].c_sem_id, &sops, 1)!=-1){
                                                        if(chessboard[my_x*my_gc->width+my_y+1].g_flag_value!=0){
                                                                mexSnd.msgc[1]=my_x*my_gc->width+(my_y+1);                                       
                                                                msgsnd(my_ks->msgq_id, &mexSnd, sizeof(mexSnd)-sizeof(long), 0); 
                                                        }
                                                        chessboard[my_x*my_gc->width+my_y].num_owner=0;
                                                        sem_release(chessboard[my_x*my_gc->width+my_y].c_sem_id, 0);
                                                        chessboard[my_x*my_gc->width+my_y+1].num_owner=pawn_list[my_pos].owner;
                                                        my_y+=1;
                                                        pawn_list[my_pos].moves-=1;
                                                        nanosleep(&tim, NULL);
                                                }
                                        } else if(my_y>targ_y){
                                                if(semop(chessboard[my_x*my_gc->width+my_y-1].c_sem_id, &sops, 1)!=-1){
                                                        if(chessboard[my_x*my_gc->width+my_y-1].g_flag_value!=0){
                                                                mexSnd.msgc[1]=my_x*my_gc->width+(my_y-1);                                     
                                                                msgsnd(my_ks->msgq_id, &mexSnd, sizeof(mexSnd)-sizeof(long), 0); 
                                                        }
                                                        chessboard[my_x*my_gc->width+my_y].num_owner=0;
                                                        sem_release(chessboard[my_x*my_gc->width+my_y].c_sem_id, 0);
                                                        chessboard[my_x*my_gc->width+my_y-1].num_owner=pawn_list[my_pos].owner;
                                                        my_y-=1;
                                                        pawn_list[my_pos].moves-=1;
                                                        nanosleep(&tim, NULL);
                                                }
                                        }
                                }
                                /*controllo deadlock*/
                                if(my_x==ex_x&&my_y==ex_y){
                                        break;
                                }
                                ex_x=my_x;
                                ex_y=my_y;
                        }
                        /*aggiorno le mie coordinate nella memoria condivisa*/
                        pawn_list[my_pos].x=my_x;
                        pawn_list[my_pos].y=my_y;
                        
                } 
                if(shmdt(arr_target)==-1){
                        TEST_ERROR
                }
                wait_zero(my_ks->sem_sync_round, ENDING);
        }
}