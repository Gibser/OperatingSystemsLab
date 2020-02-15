/*Matrix punz by cappadavide punz*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

struct cell{
  int playerSD; //Socket descriptor
  int isWareHouse;
  int isObstacle;
  char object[10];
  
};
void initializeMatrix(int rows,int cols);
void createMap(int rows,int cols);
void printMatrix(int rows,int cols);
time_t t;

char items[3][10]={"sword","gold","food"};
struct cell **map;

/*  int **arr = (int **)malloc(r * sizeof(int *)); 
    for (i=0; i<r; i++) 
         arr[i] = (int *)malloc(c * sizeof(int)); */

int main(){
   int rows, cols;
   
   
   srand((unsigned) time(&t));
   rows=rand()%9+8;  //da 0 a 8 -> da 8 a 16
   cols=rand()%9+8; 
   printf("rows %d cols %d\n",rows,cols);
   initializeMatrix(rows,cols);
   createMap(rows,cols);
   printf("Map created.\n");
   printMatrix(rows,cols);
   return 0;
}

void initializeMatrix(int rows,int cols){
  int i=0,j=0;
  map=(struct cell**)malloc(rows * sizeof(struct cell *));
  for(i=0;i<rows;i++){
    map[i]=(struct cell*)malloc(cols*sizeof(struct cell));
  }
  for(i=0;i<rows;i++){
    for(j=0;j<cols;j++){
      map[i][j].isObstacle=0;
      map[i][j].isWareHouse=0;
      map[i][j].playerSD=-1; //Un socket descriptor ha valori tra 0 e 1024
      memset(map[i][j].object,'\0',sizeof(map[i][j].object));
    }
  } 

}

void createMap(int rows,int cols){
  int wareHouses, obstacles=-1, n_items;
  int i;
  int r,c,index;
  srand((unsigned) time(&t));
  printf("Choosing number of warehouses...\n");
  wareHouses=rand()%4+1; 
  printf("Number of Warehouses: %d\n",wareHouses);
  printf("Choosing number of obstacles...\n");
  while(obstacles<0||(obstacles>=rows&&obstacles>=cols)){
    obstacles=rand()%16+5; //Almeno ostacoli fino a un massimo di 16 (righe e colonne permettendo) 
    //Si deve evitare la possibilità che un utente spawni in un punto chiuso, laddove esista
  }
  printf("Number of obstacles: %d\n",obstacles);
  printf("Choosing number of items...\n");
  n_items=rand()%5+8; //Da 8 a 12 items sparsi nella mappa
  printf("Number of items: %d\n",n_items);
  printf("Generating map...\n");
  i=0;
  while(i<wareHouses){
    r=rand()%rows;
    c=rand()%cols;
    if(map[r][c].isWareHouse==0){
      map[r][c].isWareHouse=1;
      i++;
    }
  }
  i=0;
  while(i<obstacles){
    r=rand()%rows;
    c=rand()%cols;
    if(map[r][c].isWareHouse==0&&map[r][c].isObstacle==0){
      map[r][c].isObstacle=1;
      i++;
    }
  }
  i=0;
  while(i<n_items){
    r=rand()%rows;
    c=rand()%cols;
    index=rand()%3;
    if(map[r][c].isWareHouse==0&&map[r][c].isObstacle==0&&strlen(map[r][c].object)==0){
      strcpy(map[r][c].object,items[index]);
      i++;
    }

  }

}

void printMatrix(int rows,int cols){
  int i,j;
  printf("  ");
  for(i=0;i<cols;i++){
    printf("_ ");
  }
  printf(" \n");
  for(i=0;i<rows;i++){
    printf("| ");
    for(j=0;j<cols;j++){
      if(map[i][j].isWareHouse==1){
        printf("M ");
      }
      else if(map[i][j].isObstacle==1){//Dovrà essere nascosto in un primo momento al client
        printf("x ");
      }
      else if(strlen(map[i][j].object)>0){
        if(strcmp(map[i][j].object,"gold")==0){
          printf("G ");
        }
        else if(strcmp(map[i][j].object,"sword")==0){
          printf("S ");
        }
        else if(strcmp(map[i][j].object,"food")==0){
          printf("F ");
        }
      }
      else{
        printf("  ");
      }
    }
    printf("|");
    printf("\n");
  }
  printf("  ");
  for(i=0;i<cols;i++){
    printf("─ ");
  }
  printf("\n");
}