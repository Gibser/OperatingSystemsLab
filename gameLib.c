/*Matrix punz by cappadavide punz*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <unistd.h>
#include "gameLib.h"
#define MAX_USERS 8




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

int randNumb(){
  return rand()%9+8; 
}

void createMap(struct mapObjects* info,int rows,int cols,struct cell **map){
  int wareHouses, obstacles=-1, n_items;
  int i;
  int r,c,index;
  char items[3]={'$','@','s'};
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
      map[r][c].object = 'w';
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
      map[r][c].object = 'x';
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

  int j;
  for(i = 0; i < rows; i++){
    for(j = 0; j < cols; j++){
      if(map[i][j].isWareHouse == 0 && map[i][j].isObstacle == 0 && map[i][j].object == '0')
        map[i][j].object = ' ';
    }
  }
}


/*void printMatrix(){
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
}*/


int min(int rows,int cols){
  if(rows<=cols)
    return rows;
  else
    return cols;
  
}
