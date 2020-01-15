#include "master_lib.h"

char* integer_to_string_arg(int x){
        char *sup_x;
        sup_x=malloc(3*sizeof(x)+1);
        sprintf(sup_x, "%d", x);
        return sup_x;
}

keys_storage* fill_storage_shm(int idm, int idc, int ids, int idq, int idsemr, int idp){
        keys_storage* new_s;
        if((new_s=shmat(idm, NULL, 0))==((void *) -1)){
                TEST_ERROR
        }
        new_s->conf_id=idc;
        new_s->chess_id=ids;
        new_s->ks_shm_id=idm;
        new_s->msgq_id=idq;
        new_s->sem_sync_round=idsemr;
        new_s->sem_set_pl=idp;
        return new_s;
}

int get_rand_fl(int min, int max){
        srand(time(NULL));
        return min==max? min : (rand() % (max - min)) + min;
}

int* randomize_flags(int arr_id, int fl_num, game_config* my_gc, slot* chessboard){
        int* my_arr;
        int i,j,x,y,ok;
        int num_r, num_sem, tmp_index;

        if((my_arr=(int*)shmat(arr_id, NULL, 0))== (void *) -1){
                TEST_ERROR
        }
        srand(time(NULL));
        i=0;
        while(i<fl_num){
                x=rand()%(my_gc->height);
                y=rand()%(my_gc->width);
                tmp_index=x*my_gc->width+y;
                if(semctl(chessboard[tmp_index].c_sem_id, 0, GETVAL)){
                        ok=1;
                        for(j=0;j<i;j++){
                                if(my_arr[j]==tmp_index){
                                        ok=0;
                                }
                        }
                        if(ok==1){
                                my_arr[i]=tmp_index;
                                chessboard[my_arr[i]].g_flag_value=1;
                                i++;
                        }
                }
        }
        for(i=fl_num; i!=my_gc->tot_score_flag; i++){ 
                chessboard[my_arr[rand()%(fl_num)]].g_flag_value+=1;
        }
        /*chiudo l'array con valori fittizi*/
        my_arr[fl_num]=-1;
        return my_arr;
}

void print_metrics(game_config* my_gc,int* score_arr,int* array_id_pawn,int num_round, double duration){
        int i,j;
        pawn_data* pawn_list;
        float sup, tot_score, tot_moves;
        tot_moves=0;
        tot_score=0;
        printf("PRINTING METRICS:\n");
        for(j=0; j<=my_gc->width; j++){
                printf("=");               
        }
        printf("\nTotal played round: %d.\n",num_round);
        printf("Game time %f\n", duration);
        for(i=0;i<my_gc->num_player;i++){
                sup=0;
                if((pawn_list=(pawn_data*)shmat(array_id_pawn[i],NULL,0))== (void *) -1){
                        TEST_ERROR
                }
                for(j=0;j<my_gc->num_pawn;j++){
                        sup+=pawn_list[j].moves;
                }
                tot_moves+=sup;
                if(i+1<10){
                        printf("Ratio consumed/total moves player %d: %f.\n",i+1,((sup)/(my_gc->num_pawn*my_gc->pawn_moves)));
                } else {
                        printf("Ratio consumed/total moves player %c: %f.\n",(char)((55+(i+1))),((sup)/(my_gc->num_pawn*my_gc->pawn_moves)));
                }
        }
        for(i=0;i<my_gc->num_player;i++){
                tot_score+=score_arr[i];
        }
        printf("Ratio total score/consumed moves: %f.\n",((float)(tot_score/tot_moves)));
        printf("Ratio total score/time: %f.\n",((double)(tot_score/duration)));
        for(j=0; j<=my_gc->width; j++){
                printf("=");               
        }
        printf("\n");
}