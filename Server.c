/*Prova Server*/
#include <time.h>
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <sys/time.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include "login.h"
#include "gameLib.h"
#define MAX 1000
#define PORT 5000
#define SA struct sockaddr 
#define MAX_THREADS 8

void spawnPlayer(int clientsd,struct player *info_player);
int isCellGood(struct cell a,int index1,int index2);
int isCellFree(struct cell a);
int isCellNotSolid(struct cell a);
int isLeftFree(int index1,int index2);
int isRightFree(int index1,int index2);
int isUpFree(int index1,int index2);
int isDownFree(int index1,int index2);
void setLetter(int clientsd);
char getLetter(int clientsd);
void matrixToString(char *msg, int clientsd,int *obstacles);
char parsePlayer(int playerSD);
void initializeMatrix();
void checkMovement(char msg,struct player *info_player);
void checkCommand(char msg, struct player *info_player);
void goUp(struct player *info_player);
void goLeft(struct player *info_player);
void goRight(struct player *info_player);
void goDown(struct player *info_player);
void changeCoordinates(struct player *info_player, int add_x, int add_y);
void movement(struct player *info_player, int add_x, int add_y);
int isWarehouseHere(struct player *a);
int checkWarehouse(struct player *info_player, int add_x, int add_y);
int noBoundaryCheck(struct player *a,int add_x,int add_y);

pthread_mutex_t signup_mutex;
pthread_mutex_t login;
pthread_mutex_t editMatrix;
pthread_mutex_t editMapPlayers;
pthread_mutex_t mapGen;
pthread_cond_t mapGen_cond_var;

//int threadStatus[MAX_THREADS]={0};
//int isAvailable(int slot);

struct mapObjects info_map;    //info numero oggetti sulla mappa
struct cell **map;
int rows, cols;
int mapPlayers[MAX_USERS];
int gameStarted = 0;
int gameTime = 0;

void *mapGenerator(void* args){
    int i=0,j=0;
    while(1){
      gameStarted = 0;
      pthread_mutex_lock(&editMatrix);
  
      rows = randNumb();
      cols = randNumb();
      for(i=0;i<MAX_USERS;i++) //MAPPLAYERS INITIALIZATION
        mapPlayers[i]=-1;
      printf("%d %d\n", rows, cols);
      initializeMatrix();
      //printMatrix(rows, cols, map);
      createMap(&info_map, rows, cols, map);
      pthread_cond_broadcast(&mapGen_cond_var);
      pthread_mutex_unlock(&editMatrix);
      gameStarted = 1;
      while(gameTime++ < 120)
        sleep(1);
      gameTime = 0;
      printf("Fine sleep, genero mappa...\n");
      //pthread_exit(NULL);
      
    }
}

// Game Function
void game(int clientsd){
    char msg[16];
    char command;
    struct player infoplayer;
    while(1){
      if(!gameStarted){
        pthread_mutex_lock(&editMatrix);
        pthread_cond_wait(&mapGen_cond_var, &editMatrix);
        pthread_mutex_unlock(&editMatrix);
      }
      infoplayer.obstacles=(int *)calloc(info_map.n_obstacles,sizeof(int));
      spawnPlayer(clientsd, &infoplayer);
      printf("Giocatore %c\n", parsePlayer(clientsd));
      printf("Fine Spawn\n");
      printf("Coordinate\nx: %d\ny: %d\n", infoplayer.x, infoplayer.y);
      while(1){
          printf("Valore gameStarted: %d\n", gameStarted);
          //if(!gameStarted) break;
          if(getLetter(clientsd) == '0') break;
          memset(msg,'\0',sizeof(msg));
          matrixToString(msg, clientsd,infoplayer.obstacles);
          if(read(clientsd, &command, sizeof(command))>0){
            if(getLetter(clientsd) == '0') break;
              checkCommand(command, &infoplayer);
          }
          else{
              logout(clientsd);
              break;
          }
      }
    }
}


void *clientThread(void *sockfd) 
{ 
    int clientsd=*(int*)sockfd;
    int log = 0;
    log = loginMain(clientsd, signup_mutex, login);
    if(log == 1){
        printf("Gestisco il client...\n");
        game(clientsd);
    }
	close(clientsd);
	pthread_exit(NULL);

}


// Driver function 
int main() 
{ 
   
    int sockfd, connfd, len,i=0; 
    struct sockaddr_in servaddr, cli; 
    void *result;
    pthread_t tid,gameThread;
    /*pthread_t gameThread;
    pthread_t playerThreads[MAX_THREADS];*/
    srand(time(NULL));
    /*for(i=0;i<MAX_USERS;i++)
      printf("mp %d",mapPlayers[i]);*/
    if (pthread_mutex_init(&signup_mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    if (pthread_mutex_init(&login, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
     if (pthread_mutex_init(&editMatrix, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    if (pthread_mutex_init(&editMapPlayers, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    if (pthread_mutex_init(&mapGen, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    if (pthread_cond_init(&mapGen_cond_var, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
    printf("Socket successfully created..\n"); 
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
        perror("setsockopt(SO_REUSEPORT) failed");

    //set timeout for socket input/output
    struct timeval timeout;      
    timeout.tv_sec = 180;
    timeout.tv_usec = 0;

    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        perror("setsockopt failed\n");

    if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        perror("setsockopt failed\n");

    //------------------------------------------------------------------------------------------------------------------
    memset(&servaddr, '\0', sizeof(servaddr));
    
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 


    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 10)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    len = sizeof(cli); 

    /*METTERE QUI THREAD DEL GIOCO PRINCIPALEPUNZ*/ 
    pthread_create(&gameThread, NULL, mapGenerator, NULL);
    while(1){
        // Accept the data packet from client and verification 
        connfd = accept(sockfd, (SA*)&cli, &len); 
        if(connfd>0){
            i++;
            int *thread_sd = (int*) malloc(sizeof(int));
            *thread_sd =  connfd;
            printf("server: new connection from %d %s\n",connfd,inet_ntoa(cli.sin_addr));
            pthread_create(&tid, NULL, clientThread, (void *) thread_sd);
            /*i=0;
            while(!isAvailable(threadStatus[i])){
                i++;
            }
            pthread_create(&playerThreads[i], NULL, clientThread, (void *) thread_sd);*/
        }
            
        

    }
    close(sockfd); 
} 

/*int isAvailable(int slot){
    return slot==0;
}*/

void setLetter(int clientsd){
  int i;
  for(i=0;i<MAX_USERS;i++){
    if(mapPlayers[i]==-1){
      pthread_mutex_lock(&editMapPlayers); //cappadavide  //NUOVO MUTEX
      mapPlayers[i]=clientsd;
      pthread_mutex_unlock(&editMapPlayers);//cappadavide
      break;
    }
  }
}

char getLetter(int clientsd){
  int i;
  char c = '0';
  for(i=0;i<MAX_USERS;i++){
    if(mapPlayers[i]==clientsd){
      c=(char)(i+65);
      break;
    }
  }
  return c;
}



int isLeftFree(int index1,int index2){
  if(index2-1>=0){
    return isCellNotSolid(map[index1][index2-1]);
  }
  return 0;
}
int isRightFree(int index1,int index2){
  if(index2+1<=cols-1){
    return isCellNotSolid(map[index1][index2-1]);
  }
  return 0;
}
int isUpFree(int index1,int index2){
  if(index1-1>=0){
    return isCellNotSolid(map[index1-1][index2]);
  }
  return 0;
}
int isDownFree(int index1,int index2){
  if(index1+1<=rows-1){
    return isCellNotSolid(map[index1+1][index2]);
  }
  return 0;
}
int isCellFree(struct cell a){
  //printf("Ostacolo: %d  Magazzino: %d  Giocatore: %d Oggetto: %c\n",a.isObstacle,a.isWareHouse,a.playerSD,a.object);
  if(a.isObstacle==0&&a.isWareHouse==0&&a.playerSD==-1&&a.object==' ')
    return 1;
  return 0;
}
int isCellNotSolid(struct cell a){
  if(a.isObstacle==0&&a.isWareHouse==0&&a.playerSD==-1)
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



void spawnPlayer(int clientsd,struct player *info_player){
  char c;
  int index1,index2; //Potrebbero trovarsi all'esterno, quindi magari devono essere puntatori a quegli indici
  setLetter(clientsd);
  while(1){
    index1=rand()%rows;
    index2=rand()%cols; //Cerca indici buoni finché non otteniamo una cella libera e non scomoda
    if(isCellGood(map[index1][index2],index1,index2)){
      break;
    }
  }
  info_player->x=index1;
  info_player->y=index2;
  info_player->hasItem=0;
  info_player->itemsDelivered=0; //AGGIUNTO LOCK QUANDO SI FA LO SPAWN (cappadavide)
  pthread_mutex_lock(&editMatrix); //cappadavide
  map[index1][index2].playerSD=clientsd;
  pthread_mutex_unlock(&editMatrix);//cappadavide
}


char parsePlayer(int playerSD){
  for(int i = 0; i < MAX_USERS; i++){
    if(mapPlayers[i] == playerSD)
      return ((char)i+65);
  }
}

void matrixToString(char *msg, int clientsd,int *obstacles){
  int i = 0;
  int j = 0;
  struct obstacles *a;
  write(clientsd, &rows, sizeof(int));
  write(clientsd, &cols, sizeof(int));
  while(i < rows){
    memset(msg,'\0',16);
    while(j < cols){
      if(map[i][j].playerSD >=0){
        printf("parsing %c\n", parsePlayer(map[i][j].playerSD));
        msg[j] = parsePlayer(map[i][j].playerSD);
      }
      else if(map[i][j].isObstacle){
        a=(struct obstacles *)map[i][j].pointer;
        if(obstacles[a->id]==1){
          msg[j]='x';
        }
        else
          msg[j]=' ';
      }
        
      else
          msg[j] = map[i][j].object;
      
      j++;
    }
    msg[j] = '\0';
    //printf("Riga inviata al Client: %s\n",msg);
    write(clientsd, msg, cols);
    j = 0;
    i++;
  }
  write(clientsd,&(int){0},sizeof(int));//cappadavide


}

void initializeMatrix(){
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
      map[i][j].object=' '; //prima era 0
      map[i][j].pointer=NULL;
    }
  }
} 

void movement(struct player *info_player, int add_x, int add_y){
      int id;
      struct obstacles *a;
      printf("x: %d y: %d\n", info_player->x, info_player->y);
      if(isCellNotSolid(map[(info_player->x)+add_x][info_player->y+add_y])){
        changeCoordinates(info_player, add_x, add_y);
        printf("x: %d y: %d\n", info_player->x, info_player->y);
      }
      
      else if(map[(info_player->x)+add_x][info_player->y+add_y].isWareHouse){
        if(info_player->hasItem){
          info_player->itemsDelivered+=1;
          info_player->hasItem=0;
        }
      }
      else if(map[(info_player->x)+add_x][info_player->y+add_y].isObstacle){
        a=(struct obstacles*)map[(info_player->x)+add_x][info_player->y+add_y].pointer;
        id=a->id;
        info_player->obstacles[id]=1; 
      }
      /*else if(map[(info_player->x)+add_x][info_player->y+add_y].object!=' ' && map[(info_player->x)+add_x][info_player->y+add_y].isObstacle == 0){
          changeCoordinates(info_player, add_x, add_y);
      }*/
    
}

void sendMessage(int clientsd, char *msg){
  char buff[200];
  int n;
  strcpy(buff, msg);
  n = strlen(buff);
  write(clientsd, &n, sizeof(int));
  write(clientsd, buff, n);
}



void checkCommand(char msg, struct player *info_player){
  int clientsd=map[info_player->x][info_player->y].playerSD;
  char buff[200];
  int n;
  if(msg == 't' || msg == 'T'){
    sprintf(buff, "%d", gameTime);
    n = strlen(buff);
    write(clientsd, &n, sizeof(int));
    write(clientsd, buff, n);
  }
  else if(msg=='p'||msg=='P'){
    if(!info_player->hasItem){
      if(map[info_player->x][info_player->y].pointer!=NULL){
        info_player->pack=(struct items*)map[info_player->x][info_player->y].pointer;
        map[info_player->x][info_player->y].object=' ';
        map[info_player->x][info_player->y].pointer=NULL;
        info_player->hasItem=1;
        sendMessage(clientsd, "Oggetto raccolto.");
      }
    }
    else
    {
      sendMessage(clientsd, "Inventario pieno.");
    }
    
  }
  else if(msg=='e'||msg=='E'){
    if(info_player->hasItem && isWarehouseHere(info_player)){
      //printf("Il giocatore possiede un pacco con id %d\n",info_player->pack->warehouse);
      info_player->hasItem=0;
      info_player->itemsDelivered++;
      info_player->pack=NULL;
      sendMessage(clientsd, "Oggetto consegnato.");
    }
    else if(!info_player->hasItem && isWarehouseHere(info_player))
        sendMessage(clientsd, "Non hai oggetti nell'inventario.");
    else if(info_player->hasItem && !isWarehouseHere(info_player))
        sendMessage(clientsd, "Non ci sono magazzini nelle vicinanze.");
    else
        sendMessage(clientsd, "Per depositare un oggetto hai bisogno di un oggetto e di un magazzino nelle vicinanze!");
    
  }
  else
    checkMovement(msg, info_player);

}



void checkMovement(char msg, struct player *info_player){
  int clientsd=map[info_player->x][info_player->y].playerSD;
  int n;
  if(msg=='w'||msg=='W'){
    if(info_player->x-1>=0){
      movement(info_player, -1, 0);
    }
  }
  else if(msg=='a'||msg=='A'){
    if(info_player->y-1 >= 0){
      movement(info_player, 0, -1);
    }
  }
  else if(msg=='s'||msg=='S'){
    if(info_player->x+1 < rows)
      movement(info_player, 1, 0);
  }
  else if(msg=='d'||msg=='D'){
    if(info_player->y+1 < cols)
      movement(info_player, 0, 1);
  }
  else{
    n = 0;
    write(clientsd, &n, sizeof(int));
  }
  
  

}

void changeCoordinates(struct player *info_player, int add_x, int add_y){
  int clientsd;
  pthread_mutex_lock(&editMatrix);
  clientsd=map[info_player->x][info_player->y].playerSD;
  map[info_player->x][info_player->y].playerSD=-1;
  map[info_player->x+add_x][info_player->y+add_y].playerSD=clientsd;
  pthread_mutex_unlock(&editMatrix);
  info_player->x+=add_x;
  info_player->y+=add_y;
}

void goUp(struct player *info_player){
  int clientsd;
  pthread_mutex_lock(&editMatrix);
  clientsd=map[info_player->x][info_player->y].playerSD;
  map[info_player->x][info_player->y].playerSD=-1;
  map[info_player->x+1][info_player->y].playerSD=clientsd;
  pthread_mutex_unlock(&editMatrix);
  info_player->x+=1;
}

void goLeft(struct player *info_player){
  int clientsd;
  getLetter(map[info_player->x][info_player->y-1].playerSD);
  pthread_mutex_lock(&editMatrix);
  clientsd=map[info_player->x][info_player->y].playerSD;
  map[info_player->x][info_player->y].playerSD=-1;
  map[info_player->x][info_player->y-1].playerSD=clientsd;
  pthread_mutex_unlock(&editMatrix);
  info_player->y-=1;
}

int isWarehouseHere(struct player *a){
  int exp;
  exp=checkWarehouse(a,-1,0)+checkWarehouse(a,1,0)+checkWarehouse(a,0,-1)+checkWarehouse(a,0,1);
  return exp>0;
}

int checkWarehouse(struct player *a, int add_x,int add_y){
  struct warehouse *b;
  if(noBoundaryCheck(a,add_x,add_y)){
    if(map[a->x+add_x][a->y+add_y].isWareHouse==1){
      b=(struct warehouse*)map[a->x+add_x][a->y+add_y].pointer;
      printf("Trovato deposito con id %d\n",b->id);
      return a->pack->warehouse==b->id;
    }
  }
  return 0;
}

int noBoundaryCheck(struct player *a,int add_x,int add_y){
  int r=a->x+add_x;
  int c=a->y+add_y;
  if(r>=0 && r<rows && c>=0 && c<cols)
    return 1;
  return 0;
}