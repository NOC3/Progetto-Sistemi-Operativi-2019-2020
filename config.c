#include "config.h"

game_config *init_game_config(int shm_id){
        FILE *my_file;
        game_config *my_gc;
        char *line=NULL;
        ssize_t length=0;

        my_file=fopen("init.conf", "r");
        if((my_gc=(game_config*)shmat(shm_id, NULL, 0))== (void *) -1){
                TEST_ERROR
        }
        line=NULL;

        if(my_file==NULL){
                TEST_ERROR
        }

        while(getline(&line, &length, my_file)!=EOF){ 
                if(strstr(line, "SO_NUM_G")!=NULL)
                        my_gc->num_player=find_value(line);
                else if(strstr(line, "SO_NUM_P")!=NULL)
                        my_gc->num_pawn=find_value(line);
                else if(strstr(line, "SO_MAX_TIME")!=NULL)
                        my_gc->max_round_time=find_value(line);
                else if(strstr(line, "SO_BASE")!=NULL)
                        my_gc->width=find_value(line);
                else if(strstr(line, "SO_ALTEZZA")!=NULL)
                        my_gc->height=find_value(line);
                else if(strstr(line, "SO_FLAG_MI")!=NULL)
                        my_gc->min_flag=find_value(line);
                else if(strstr(line, "SO_FLAG_MAX")!=NULL)
                        my_gc->max_flag=find_value(line);
                else if(strstr(line, "SO_ROUND_SCORE")!=NULL)
                        my_gc->tot_score_flag=find_value(line);
                else if(strstr(line, "SO_N_MOVES")!=NULL)
                        my_gc->pawn_moves=find_value(line);
                else if(strstr(line, "SO_MIN_HOLD_NSEC")!=NULL)
                        my_gc->min_sec_hold_pawn=find_value(line);
                else {
                       TEST_ERROR
                }
        }

        if((fclose(my_file))!=0){
                TEST_ERROR
        }
        return my_gc;
}                                       

int find_value (char * line){
        strtok(line, "=");
        return atoi(strtok(NULL, "="));
}