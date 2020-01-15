CFLAGS = -std=c89 -pedantic

all: master player pawn

master: init.conf master.c player.c master_lib.c master_lib.h scacchiera.c scacchiera.h config.h config.c sem_lib.h sem_lib.c player_lib.h player_lib.c pawn_lib.c pawn_lib.h pawn.c makefile 
	gcc $(CFLAGS) master.c master_lib.c config.c scacchiera.c sem_lib.c  -o master

player: init.conf master.c player.c master_lib.c master_lib.h scacchiera.c scacchiera.h config.h config.c sem_lib.h sem_lib.c player_lib.h player_lib.c pawn_lib.c pawn_lib.h pawn.c makefile
	gcc $(CFLAGS) player.c master_lib.c config.c scacchiera.c sem_lib.c player_lib.c -lm pawn_lib.c -o player

pawn: init.conf master.c player.c master_lib.c master_lib.h scacchiera.c scacchiera.h config.h config.c sem_lib.h sem_lib.c player_lib.h player_lib.c pawn_lib.c pawn_lib.h pawn.c makefile
	gcc $(CFLAGS) pawn.c master_lib.c config.c scacchiera.c sem_lib.c player_lib.c -lm pawn_lib.c -o pawn

run:  all
	./master