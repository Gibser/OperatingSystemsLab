#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <unistd.h>

struct cell{
  int playerSD; //Socket descriptor
  int isWareHouse;
  int isObstacle;
  char object;
  void* pointer;
  
};
struct items{
  int warehouse;
};

struct warehouse{
  int id;
};

struct obstacles{
  int id;
};

struct mapObjects{
    int n_obstacles;
    int n_items;
    int n_warehouses;
};

struct player{
  int x;
  int y;
  int hasItem;
  int itemsDelivered;
  struct items *pack;
  int *obstacles;
  int clientsd;
};

int randNumb();
//void initializeMatrix(struct cell **map,int rows,int cols);
void createMap(struct mapObjects* info,int rows,int cols,struct cell **map);
void printMatrix();
int min(int rows,int cols);