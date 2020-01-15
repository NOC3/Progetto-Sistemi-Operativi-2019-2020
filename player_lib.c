#include "player_lib.h"
#include <math.h>

void randomize_coordinate_pawn(pawn_data* pawn_list, int my_pos, int width, int height, slot* chessboard){
        int i, ok, sem, x, y;
        srand(time(NULL));
        ok=0;
        while(!ok){
                x=rand()%height;
                y=rand()%width;
                sem=chessboard[x*width+y].c_sem_id;
                if((semctl(sem, 0, GETVAL))){
                        ok=1;
                }
        }
        pawn_list[my_pos].x=x;
        pawn_list[my_pos].y=y;
}

void compute_targets(pawn_data *pawn, int* arr, int num_pawn, slot* chessboard){
        int i ,j, x, y, k, h, best, dist, num_act;
        
        for(i=0;i<num_pawn;i++){
                pawn[i].target=-1;
        }
        
        num_act=(int)log(num_pawn);

        for(i=0;arr[i]!=-1;i++){
                best=__INT_MAX__;
                j=__INT_MAX__;
                for(k=0;k<num_pawn;k++){
                        if(pawn[k].target==-1){
                                x=pawn[k].x;
                                y=pawn[k].y;
                                dist=abs(chessboard[arr[i]].x-x)+abs(chessboard[arr[i]].y-y);
                                if(dist<=pawn[k].moves && dist<best){
                                        best=dist;
                                        j=k;
                                }
                        }
                }
                if(j!=__INT_MAX__){
                        pawn[j].target=i;
                }
        }
        for(i=0;arr[i]!=-1;i++){
                for(h=0;h<num_act;h++){
                        best=__INT_MAX__;
                        j=__INT_MAX__;
                        for(k=0;k<num_pawn;k++){
                                if(pawn[i].target!=-1){
                                        x=pawn[k].x;
                                        y=pawn[k].y;
                                        dist=abs(chessboard[arr[i]].x-x)+abs(chessboard[arr[i]].y-y);
                                        if(dist<pawn[k].moves && dist<best){
                                                best=dist;
                                                j=k;
                                        }
                                }
                        } 
                        if(j!=__INT_MAX__){
                                pawn[j].target=i;
                        }
                }
        }
}