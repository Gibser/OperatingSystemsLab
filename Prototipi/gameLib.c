/*Matrix punz by cappadavide punz*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <unistd.h>
#define MAX_USERS 8

struct cell{
  int playerSD; //Socket descriptor
  int isWareHouse;
  int isObstacle;
  char object;
  
};
struct mapObjects{
    int n_obstacles;
    int n_items;
    int n_warehouses;
};

/*
void initializeMatrix();
void createMap();
void printMatrix();
void spawnPlayer();
int isCellGood(struct cell a,int index1,int index2);
int isCellFree(struct cell a);
int isLeftFree(int index1,int index2);
int isRightFree(int index1,int index2);
int isUpFree(int index1,int index2);
int isDownFree(int index1,int index2);
char getLetter();
*/

time_t t;

char items[3][10]={"sword","gold","food"};
/*int mapPlayers[MAX_USERS]={0};
int rows,cols;*/

struct cell **map;

/*  int **arr = (int **)malloc(r * sizeof(int *)); 
    for (i=0; i<r; i++) 
         arr[i] = (int *)malloc(c * sizeof(int)); */

int main(){
   //int rows, cols;
   
   
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

void initializeMatrix(struct cell **map,int rows,int cols){
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
      map[i][j].object='0';
    }
  } 

}


void createMap(struct mapObjects* info,int rows,int cols,struct cell **map){
  int wareHouses, obstacles=-1, n_items;
  int i;
  int r,c,index;
  char items[3]={'$','@','ł'};
  printf("Choosing number of warehouses...\n");
  info->n_warehouses=rand()%4+1; 
  printf("Number of Warehouses: %d\n",info->n_warehouses);
  printf("Choosing number of obstacles...\n");
  info->n_obstacles= (rand()%min(rows,cols))+5;
  printf("Number of Obstacles: %d\n", info->n_obstacles);
  printf("Choosing number of items...\n");
  info->n_items=rand()%5+8; //Da 8 a 12 items sparsi nella mappa
  printf("Number of items: %d\n",info->n_items);
  printf("Generating map...\n");
  i=0;
  while(i<info->n_warehouses){
    printf("rows: %d cols: %d\n", rows, cols);
    r=rand()%rows;
    c=rand()%cols;
    if(map[r][c].isWareHouse==0){
      map[r][c].isWareHouse=1;
      i++;
    }
  }
  printf("warehouses done\n");
  i=0;
  while(i<info->n_obstacles){
    r=rand()%rows;
    c=rand()%cols;
    if(map[r][c].isWareHouse==0&&map[r][c].isObstacle==0){
      map[r][c].isObstacle=1;
      i++;
    }
  }
  printf("obstacles done\n");
  i=0;
  while(i<info->n_items){
    r=rand()%rows;
    c=rand()%cols;
    index=rand()%3;
    if(map[r][c].isWareHouse==0&&map[r][c].isObstacle==0&&map[r][c].object=='0'){
      map[r][c].object=items[index];
      i++;
    }

  }
  printf("items done\n");


}

/*
void createMap(){
  int wareHouses, obstacles=-1, n_items;
  int i;
  int r,c,index;
  printf("Choosing number of warehouses...\n");
  wareHouses=rand()%4+1; 
  printf("Number of Warehouses: %d\n",wareHouses);
  printf("Choosing number of obstacles...\n");
  obstacles = (rand()%12)+5;
  printf("Number of Obstacles: %d\n", obstacles);
  printf("Choosing number of items...\n");
  n_items=rand()%5+8; //Da 8 a 12 items sparsi nella mappa
  printf("Number of items: %d\n",n_items);
  printf("Generating map...\n");
  i=0;
  while(i<wareHouses){
    printf("rows: %d cols: %d\n", rows, cols);
    r=rand()%rows;
    c=rand()%cols;
    if(map[r][c].isWareHouse==0){
      map[r][c].isWareHouse=1;
      i++;
    }
  }
  printf("warehouses done\n");
  i=0;
  while(i<obstacles){
    r=rand()%rows;
    c=rand()%cols;
    if(map[r][c].isWareHouse==0&&map[r][c].isObstacle==0){
      map[r][c].isObstacle=1;
      i++;
    }
  }
  printf("obstacles done\n");
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
  printf("items done\n");


}
*/

void printMatrix(){
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
        printf("ħ ");
      }
      else if(map[i][j].isObstacle==1){//Dovrà essere nascosto in un primo momento al client
        printf("x ");
      }
      else if(strlen(map[i][j].object)>0){
        if(strcmp(map[i][j].object,"gold")==0){
          printf("$ ");
        }
        else if(strcmp(map[i][j].object,"sword")==0){
          printf("ł ");
        }
        else if(strcmp(map[i][j].object,"food")==0){
          printf("@ ");
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


char getLetter(int* mapPlayers){
  int i;
  char c;
  for(i=0;i<MAX_USERS;i++){
    if(mapPlayers[i]==0){
      mapPlayers[i]=1;//Inserire al posto di 1 mettiamo socketdescriptor
      c=(char)(i+65);
      break;
    }
  }
  return c;
}


int isLeftFree(int index1,int index2){
  if(index2-1>=0){
    return isCellFree(map[index1][index2-1]);
  }
  return 0;
}
int isRightFree(int index1,int index2){
  if(index2+1<=cols-1){
    return isCellFree(map[index1][index2-1]);
  }
  return 0;
}
int isUpFree(int index1,int index2){
  if(index1-1>=0){
    return isCellFree(map[index1-1][index2]);
  }
  return 0;
}
int isDownFree(int index1,int index2){
  if(index1+1<=rows-1){
    return isCellFree(map[index1+1][index2]);
  }
  return 0;
}
int isCellFree(struct cell a){
  if(a.isObstacle==0&&a.isWareHouse==0&&a.playerSD==-1&&strlen(a.object)==0)
    return 1;
  return 0;
}

int isCellGood(struct cell a,int index1,int index2){
  int exp;
  if(isCellFree(a)){
    exp=isLeftFree(index1,index2)+isRightFree(index1,index2)+isUpFree(index1,index2)+isDownFree(index1,index2);
    if(exp>0)//Se almeno una cella è libera attorno al giocatore okay
      return 1;
    return 0;
  }
}



void spawnPlayer(struct cell **map, int *mapPlayers, int rows, int cols){
  char c;
  int index1,index2; //Potrebbero trovarsi all'esterno, quindi magari devono essere puntatori a quegli indici
  c=getLetter();
  while(1){
    index1=rand()%rows;
    index2=rand()%cols; //Cerca indici buoni finché non otteniamo una cella libera e non scomoda
    if(isCellGood(map[index1][index2],index1,index2)){
      break;
    }
  }
  //map[index1][index2].playerSD=SOCKETDESCRIPTOR
}

int min(int rows,int cols){
  if(rows<=cols)
    return rows;
  else
    return cols;
  
}