# Progetto-SO-2019-2020

Federico Serra, matricola 898925
Alessandro Nocera, matricola 886732

RELAZIONE PROGETTO SISTEMI OPERATIVI 2019-2020

Compilazione: make
Esecuzione(inclusa compilazione): make run

Inizializzazione:  
Il gioco inizia con il master che alloca in memoria condivisa una struct game_config che contiene i vari parametri della configurazione del gioco estraendoli dal file "init.conf". 
Successivamente alloca in memoria condivisa la scacchiera(chessboard) come un array di dimensioni baseXaltezza di caselle (slot), alloca una coda di messaggi, utilizzata da tutti i processi per la comunicazione tra essi grazie a un tipo di messaggio specifico per tipo di comunicazione differente (MASTER_TO_PLAYER, PLAYER_TO_MASTER,PAWN_TO_MASTER).
Per la sincronizzazione vengono utilizzati 2 set di semafori contatori(escludendo i semafori già presenti in ogni casella della scacchiera): "sem_sync_round" per la sincronizzazione tra players e master durante la configurazione, e tra players, pawns e master nello svolgimento del gioco; "sem_set_pl" set di semafori di dimensione [numero giocatori] utilizzato come semaforo circolare per i giocatori, permettendogli di allocare le pedine in mutua esclusione a turno.
Tutti gli id precedentemente ottenuti con le chiamate per la generazione di ipcs sono inseriti in una struct keys_storage (escluso "round_flag_id" gestito successivamente)in memoria condivisa in modo da essere disponibili a tutti i processi in ogni momento del gioco.
Terminata l'inizializzazione, il master fa fork() e esegue una execve() per generare i processi player. Terminata  la generazione da parte del master, i player fanno attach alla memoria condivisa grazie agli id presenti in keys_storage (la cui attach è consentita dall'id "key_id_shm" passato ai player come argomento args nell'execve), successivamente allocano in memoria condivisa un'array di dimensione [numero pedine] (di nome "pawn_list") e comunicano l'id per l'attach al master tramite una msgsnd(), per poi fare fork() ed execve() a turno e uno alla volta generando così le pedine, assegnandogli una x e una y calcolata randomicamente.
Ogni singola pedina, una volta generata, fa anch'essa attach grazie alla keys_storage(passata dal player alle pedine con lo stesso metodo con cui è stata passata dal master ai player) e si alloca sulla scacchiera con le coordinate precedentemente rese disponibili, nella propria posizione in pawn_list. Successivamente la pedina aspetta di essere sbloccata per l'inizio del primo round.
Il master ricevuti i messaggi mandati dai giocatori (contenenti l'id di ogni pawn_list per consentire l'attach) li inserisce nell'array_id_pawn, ottenendo così un'array di dimensione [numero_giocatori] contenente l'id per l'attach a ogni array pawn_list, contenente a sua volta [numero_pawn] struct pawn_data con all'interno i dati di ogni pedina.
A questo punto sono sincronizzati master e players e pawns per l'inizio del round sul semaforo WAITING del set sem_sync_round.

Inizia il round:
mentre tutti i processi attendono di essere sbloccati, il master genera un numero randomico(entro i limiti della configurazione) di bandierine e gli assegna un valore anch'esso randomico (entro i limiti della configurazione), le alloca sulla scacchiera e crea un array di int (contenente gli indirizzi delle bandierine sulla scacchiera) in memoria condivisa inserendone in keys_storage l'id, procede stampando la scacchiera, sblocca WAITING e attende su STARTING. I players, una volta fatta l'attach all'id dell'array di bandierine, calcolano per ogni bandierina un determinato numero (uguale al logaritmo naturale del numero di pedine per giocatore) di pedine (le più vicine e con un numero di mosse sufficienti)a cui assegnarla come target, e ne inseriscono l'indice nella struct pawn_list nel campo corretto. Fatto cio' sbloccano STARTING del set "sem_sync_round". A questo punto il master setta l'allarme, e attende di ricevere messaggi della conquista dalle pedine, i players sono invece bloccati sul semaforo di fine round (ENDING) e le pedine pronte a verificare di avere un target, in caso non fosse (o non fosse piu') valido si bloccano anch'esse su ENDING.
Le pedine, se hanno un target, provano a raggiungerlo muovendosi alternatamente sulle x e sulle y finchè la bandierina è valida(ha valore diverso da 0); se lo raggiungono mandando un messaggio al master, bloccato dalla msgrcv, che provvede ad aggiornare lo score_arr(array dei punteggi), azzerare il valore della bandierina sulla scacchiera e ridurre il contatore delle bandierine. Una volta conquistate tutte le bandierine, il round termina, l'allarme viene disattivato dal master e viene eseguita la stampa della scacchiera, infine attende che tutti i processi siano sincronizzati a fine round(sul semaforo ENDING). 
Il round ricomincia dalla generazione da parte del master delle bandierine, i vari round continuano finchè non scatta l'alarm durante l'esecuzione, facendo terminare il gioco.

NOTE
-strategia: 
La strategia utilizzata è piuttosto semplice: le pedine provano a fare -1 sul semaforo della prossima casella, se riescono verificano se sono su una bandierina (in tal caso lo comunicano al master) e se non riescono tentano di muoversi sull'altra coordinata. Abbiamo quindi deciso di adottare una strategia non complicata, che includesse ad esempio una strategia comune di squadra, tuttavia la nostra scelta ci è sembrata un buon compromesso tra semplicità ed efficienza.
-handler:
Abbiamo bloccato il segnale SIGINT prima che la configurazione fosse terminata. In caso di errore nei player o nei pawn viene sollevato il segnale SIGINT che grazie all'handler viene comunicato: nel caso del pawn al player che lo comunica al master, direttamente al master nel caso del player. Il master ricevuto un qualsiasi SIGINT(successivo alla disabilitazione del blocco) procede con la stampa dello stato, la rimozione delle ipcs, la kill di player e pawn e una exit(EXIT_FAILURE). La stessa cosa accade allo scadere dell'alarm() nel round, con la differenza che l'exit è exit(EXIT_SUCCESS).
