#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#define MAX_USERS 8

/*Apre o crea il file fileName ed esegue il comando con system(cmd). Il comando deve ridirezionare l'output sul file appena creato*/
int tmpCommand(char* cmd, char* fileName);
/*Prende in ingresso una stringa buffer che contiene username e password separati da \n e scrive il primo su username*/
void extractUsername(char *buffer,char* username);
/*Prende in ingresso una stringa buffer che contiene username e password separati da \n e scrive la seconda su password*/
void extractPassword(char *buffer, char* password);
/*Verifica la presenza di username nel file users. Restituisce 0 se è presente, 1 altrimenti*/
int usernameCheck(char* username);
/*Scrive il contenuto del file puntato da fd su string*/
void copyStringFromFile(char* string, int fd);
/*Se username è presente in users e password corrisponde alla stringa dopo lo spazio che segue username in users, allora restituisce 1, altrimenti 0. Restituisce -1 se il client non riesce a leggere dal socket*/
int loginF(char* username, char* password, int clientsd, pthread_mutex_t login);
/*Scrive "username password" su users se username non è presente nel file e restituisce 1, 0 altrimenti. Restituisce -1 se il client non riesce a leggere dal socket*/
int regF(char* username, char* password, int clientsd, pthread_mutex_t lock);
/*Avvia la funzionalità di login o registrazione in base alla scelta dell'utente*/
int loginMain(int clientsd, pthread_mutex_t lock, pthread_mutex_t login, char *username);
/*Scrive "username clientsd" su logged_users. La procedura di login termina con questa funzione, quindi l'utente è loggato.*/
void logUser(char* username, int clientsd, pthread_mutex_t login);
/*Verifica la presenza di username nel file logged_users. Ritorna 1 se presente, 0 altrimenti.*/
int loggedUser(char* username);
/*Elimina la riga che contiene clientsd dal file logged_users con un comando sed. L'utente non è più loggato.*/
void logout(int clientsd);
/*Invia al server prima strlen(msg) e poi msg*/
void sendSignal(int clientsd, char *msg);
/*Ritorna 1 se il numero di utenti loggati, quindi presenti in logged_users, è maggiore o uguale a MAX_USERS, 0 altrimenti.*/
int maxUsers();