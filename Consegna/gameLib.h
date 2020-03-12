#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <unistd.h>


struct cell{
  int playerSD; //Socket descriptor del giocatore
  int isWareHouse; //indica se la cella possiede un magazzino
  int isObstacle; //indica se la cella possiede un ostacolo
  char object; //carattere che indica visivamente un tipo di oggetto
  void* pointer; 
  
};

struct items{
  int warehouse; //indica l'ID del magazzino di destinazione
  char object; //carattere visivo dell'item
};

struct warehouse{
  int id; //id del magazzino
};

struct obstacles{
  int id; //id dell'ostacolo
};

struct mapObjects{
    int n_obstacles; //indica il numero di ostacoli sulla mappa
    int n_items; //indica il numero di items sulla mappa
    int n_warehouses; //indica il numero di magazzini sulla mappa
};

struct player{
  char username[50]; //username del giocatore
  int x; //coordinata x della posizione corrente
  int y; //coordinata y della posizione corrente
  int hasItem; //indica se il giocatore possiede o meno un oggetto da consegnare
  int itemsDelivered;//indica quanti items ha consegnato il giocatore
  struct items *pack; //puntatore a struttura items, non null se hasItem=1
  int *obstacles; //array di interi che conserva l'informazione sugli ostacoli incontrati o non
  int clientsd; //socket descriptor del giocatore
};

/*Genera un numero casuale da 16 a 24 */
int randNumb(); 
/*Si occupa di popolare la stringa con un determinato
numero di items,magazzini e ostacoli secondo alcuni criteri.*/
void createMap(struct mapObjects* info,int rows,int cols,struct cell **map);
/*Funzione che stampa la matrice di tipo struct cell*/
void printMatrix();
/*Restituisce il più piccolo tra rows e cols*/
int min(int rows,int cols);
