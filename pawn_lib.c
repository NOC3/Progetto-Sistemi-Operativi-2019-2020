#include "pawn_lib.h"

void allocate_pawn(int x, int y, int pl, slot* chessboard, int width){
        chessboard[x*width+y].num_owner=pl;
        init_sem_to_val(chessboard[x*width+y].c_sem_id, 0, 0); 
}

int calculate_target(pawn_data pawn, int* arr, slot* chessboard){
        int i,j,best,x,y,dist;
        x=pawn.x;
        y=pawn.y;

        best=__INT_MAX__;

        for(i=0;arr[i]!=-1;i++){
                if(chessboard[arr[i]].g_flag_value!=0){
                        dist=abs(chessboard[arr[i]].x-x)+abs(chessboard[arr[i]].y-y);
                        if(dist<pawn.moves&&dist<best){
                                best=dist;
                                j=i;
                        }
                }
        }
        return best==__INT_MAX__? -1:j;
}