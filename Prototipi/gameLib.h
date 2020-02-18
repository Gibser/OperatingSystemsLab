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


void randNumb();
void initializeMatrix(struct cell **map,int rows,int cols);
void createMap(struct mapObjects* info,int rows,int cols,struct cell **map);
void printMatrix();
void spawnPlayer(struct cell **map, int *mapPlayers, int rows, int cols, int clientsd);
int isCellGood(struct cell a,int index1,int index2);
int isCellFree(struct cell a);
int isLeftFree(int index1,int index2);
int isRightFree(int index1,int index2);
int isUpFree(int index1,int index2);
int isDownFree(int index1,int index2);
char getLetter(int *mapPlayers);
