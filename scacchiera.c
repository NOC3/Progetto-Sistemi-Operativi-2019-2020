#include "scacchiera.h"
#include "pawn_lib.h"

slot* create_chessboard(int height, int width, int sc_id){
        int i, j, sem_id;
        slot *chessboard;

        if((chessboard=(slot*)shmat(sc_id, NULL, 0))== (void *) -1){
                TEST_ERROR
        }
        
        for(i=0; i<height; i++){
                for(j=0; j<width; j++){
                        if((sem_id=semget(IPC_PRIVATE, 1, IPC_CREAT|0666))==-1){
                                TEST_ERROR
                        }
                        if((init_sem_to_val(sem_id, 0, 1))==-1){
                                TEST_ERROR
                        }
                        chessboard[i*width+j].c_sem_id=sem_id;
                        chessboard[i*width+j].g_flag_value=0;
                        chessboard[i*width+j].num_owner=0;
                        chessboard[i*width+j].x=i;
                        chessboard[i*width+j].y=j;
                }
        }
        return chessboard;
}

void clean_sem_chessboard(int height, int width, slot* chessboard){
        int i, j;

        for(i=0; i<height*width; i++){
                if((semctl(chessboard[i].c_sem_id, 0, IPC_RMID))==-1){
                        TEST_ERROR
                }
        }
}

void print_chessboard(slot* chessboard, game_config* my_gc, int* scores, int* array_id_moves_res, int num_round){
        pawn_data* pawn_list;
        int i, j, y, part, u;
        int sem_c;
        int *sup;

        /*stampa punteggi giocatori*/
        printf("Players scores:\n");
        for(j=0; j<=my_gc->width; j++){
                printf("=");           
        }
        printf("\n");
        printf("NUM PLAYER\tSCORE\n");
        for(i=0; i<my_gc->num_player; i++){
                printf("%d\t\t%d\n", i+1, scores[i]);
        }
        for(j=0; j<=my_gc->width; j++){
                printf("=");               
        }
        printf("\n\n");

        /*stampa chessboard*/
        printf("Chessboard:\n ");
        
        for(j=0; j<=my_gc->width-1; j++){
                printf("_");
        }
        printf("\n|");
        u=0;
        for(i=0;i<my_gc->height; i++){
                for(j=0; j<my_gc->width; j++){
                        if((sem_c=semctl(chessboard[i*my_gc->width+j].c_sem_id, 0, GETVAL))==-1){
                                TEST_ERROR
                        }
                        if(sem_c!=0 && chessboard[i*my_gc->width+j].g_flag_value==0){
                                printf(" ");          
                        } else if (sem_c!=0&&chessboard[i*my_gc->width+j].g_flag_value!=0){
                                printf("X");
                                u+=1;
                        } else {
                                switch(chessboard[i*my_gc->width+j].num_owner+1){
                                        case 1:
                                        color(RED);
                                        break;
                                        case 2:
                                        color(GRN);
                                        break;
                                        case 3:
                                        color(BYEL);
                                        break;
                                        case 4:
                                        color(BLU);
                                        break;
                                        case 5:
                                        color(MAG);
                                        break;
                                        case 6:
                                        color(CYN);
                                        break;                                        
                                        case 7:
                                        color(BRED);
                                        break;  
                                        case 8:
                                        color(BGRN);
                                        case 9:
                                        color(YEL);
                                        break;
                                        case 10:
                                        color(BBLU);
                                        break;                                        
                                        case 11:
                                        color(BMAG);
                                        break;  
                                        case 12:
                                        color(BCYN);
                                        break;  
                                        default:
                                        break;
                                }
                                if(chessboard[i*my_gc->width+j].num_owner+1<10){
                                        printf("%d", chessboard[i*my_gc->width+j].num_owner+1);
                                } else {
                                        printf("%c", (char)(55+(chessboard[i*my_gc->width+j].num_owner+1)));
                                }
                                reset();
                        }
                        if(j==my_gc->width-1){
                                printf("|\n|");
                        }
                }
        }
        for(j=0; j<=my_gc->width-1; j++){
                printf("_");
        }
        printf("|\n");
        /*stampa moves*/
        printf("\n\nRemaining moves of pawns:\n");
        part=(int)my_gc->pawn_moves/10;
        sup=malloc(sizeof(int)*11);
        for(j=0; j<=my_gc->width; j++){
                printf("=");           
        }
        printf("\n");

        for(i=0;i<my_gc->num_player;i++){
                if(i+1<10){
                        printf("PLAYER %d\n", i+1);
                } else {
                       printf("PLAYER %c\n", (char)((55+(i+1))));
                }
                if((pawn_list=(pawn_data*)shmat(array_id_moves_res[i],NULL,0))== (void *) -1){
                        TEST_ERROR
                }
                for(j=0;j<my_gc->num_pawn;j++){
                        sup[(int)(pawn_list[j].moves/part)]+=1;
                }
                for(y=0; y<10; y++){
                        printf("Moves in range [%d]-[%d]=%d\n", part*y, part*(y+1), sup[y]);
                        sup[y]=0;
                }
                printf("%d pawns not moved.\n\n", sup[10]);
                sup[10]=0;
        }

        for(j=0; j<=my_gc->width; j++){
                printf("=");
        }
        printf("\nPlayed rounds:%d, remaining flags %d.\n", num_round, u);
        for(j=0; j<=my_gc->width; j++){
                printf("=");
        }
        printf("\n\n");
        free(sup);
}

void color(char* my_color){
        printf("\033%s", my_color);
}

void reset () {
        printf("\033[0m");
}
