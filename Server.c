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

/**spawnPlayer gets a clientSD and a pointer to a struct player type as parameters.
 * It spawns a player in a cell (using some criteria) and initializes its associated struct.
 */
void spawnPlayer(int clientsd,struct player *info_player);
/*isCellGood function determines if a cell object is good for a player's spawn*/
int isCellGood(struct cell a,int index1,int index2);
/*isCellFree function tells if a cell is free (no warehouses/players/objects/obstacles)*/
int isCellFree(struct cell a);
/*isCellNotSolid function tells if a cell is "not Solid", that is, a cell on which a player can move*/
int isCellNotSolid(struct cell a);
/**isLeftFree function checks whether the left cell (in relation of current position) is free or not.
 * It requires a struct cell matrix and current position
 */
int isLeftFree(int index1,int index2);
/**isRightFree function checks whether the right cell (in relation of current position) is free or not.
 * It requires a struct cell matrix and current position
 */
int isRightFree(int index1,int index2);
/**isUpFree function checks whether the top cell (in relation of current position) is free or not.
 * It requires a struct cell matrix and current position
 */
int isUpFree(int index1,int index2);
/**isDownFree function checks whether the bottom cell (in relation of current position) is free or not.
 * It requires a struct cell matrix and current position
 */
int isDownFree(int index1,int index2);
/*setLetter function assigns a letter to a new Player*/
void setLetter(int clientsd);
/*getLetter function returns the letter used by a specified player*/
char getLetter(int clientsd);
/**matrixToString sends game map to a specific player in order to display it on its Game's Client.
 * It can append an informative message.
*/
void matrixToString(char *msg, int clientsd,int *obstacles);
//char parsePlayer(int playerSD);
/**initGame initializes everything that's necessary to run the game correctly 
 * (mapPlayers,map,scoreboard).
 */
void initGame();

void checkMovement(char msg,struct player *info_player,char *info);
void checkCommand(char msg, struct player *info_player,char *info);
/*changeCoordinates modifies game map to update a player's position*/
void changeCoordinates(struct player *info_player, int add_x, int add_y);
void movement(struct player *info_player, int add_x, int add_y);
/*isWarehouseHere tells if there's at least a warehouse around the player*/
int isWarehouseHere(struct player *a);
/*checkWarehouse tells whether a warehouse is on a specified cell*/
int checkWarehouse(struct player *info_player, int add_x, int add_y);
int noBoundaryCheck(struct player *a,int add_x,int add_y);
/*sendMessage sends a message to a specified socket. It sends the length of the message it's going to send before sending it*/
void sendMessage(int clientsd, char *msg);
/*gameLogout does everything it's necessary when a player leaves game*/
void gameLogout(int clientsd);
/*logoutStructs removes every information about a player that left a game session*/
void logoutStructs(int clientsd);
/*setScorePlayer inserts a new player into scoreboard*/
void setScorePlayer(struct player *info_player);
/*createScoreboard creates a message containing scoreboard when a game session is over*/
void createScoreboard();
void initNullStruct();
/*quicksort algorithm to sort a game session's scoreboard*/
void quicksort(struct player* a[MAX_USERS], int first, int last);
void writeLog(char *msg,int flag);
int mutexInitialization();
void getUTCString(char *str);

pthread_mutex_t signup_mutex;
pthread_mutex_t login;
pthread_mutex_t editMatrix; 
pthread_mutex_t editMapPlayers;
pthread_mutex_t mapGen;
pthread_mutex_t notifyMaxItems;
pthread_cond_t mapGen_cond_var;
pthread_mutex_t gameLog;



//int threadStatus[MAX_THREADS]={0};
//int isAvailable(int slot);

struct mapObjects info_map;    //info numero oggetti sulla mappa
struct cell **map;
int rows, cols;
int mapPlayers[MAX_USERS];
struct player* scoreboard[MAX_USERS];
int gameStarted = 0;
int gameTime = 60; //era zero
int MAX_ITEMS;
int maxItemReached=0;
char scoreboardString[500]="";
struct player *nullStruct;


void *mapGenerator(void* args){
    int i=0,j=0;
    while(1){
      gameStarted = 0;
      pthread_mutex_lock(&editMatrix);
      rows = randNumb();
      cols = randNumb();
      printf("%d %d\n", rows, cols);
      initGame();
      printf("\n\nClassifica live\n");
      for(i= 0; i < MAX_USERS; i++){
        if(scoreboard[i]->clientsd >= 0)
          printf("Giocatore: %d\n", scoreboard[i]->clientsd);
      }
      printf("Valore clientsd nullStruct: %d\n", nullStruct->clientsd);
      printf("\n\n");
      //printMatrix(rows, cols, map);
      createMap(&info_map, rows, cols, map);
      MAX_ITEMS=info_map.n_items/2;
      pthread_cond_broadcast(&mapGen_cond_var);
      pthread_mutex_unlock(&editMatrix);
      gameStarted = 1;
      while(gameTime-- > 0){ //era gameTime++ < 60
        if(maxItemReached==1)
          break;
        sleep(1);
      }
      createScoreboard();
      gameTime = 60; //Era 0
      printf("Fine sleep, genero mappa...\n");
      //pthread_exit(NULL);
      
    }
}

// Game Function
void game(int clientsd){
    char info[250]="Benvenuto in partita, giovane avventuriero! Premi [H] per aiuto.\n";
    char command;
    struct player infoplayer;
    int isLogged=1;
    while(isLogged){
      if(!gameStarted){
        pthread_mutex_lock(&editMatrix);
        pthread_cond_wait(&mapGen_cond_var, &editMatrix);
        pthread_mutex_unlock(&editMatrix);
      }
      infoplayer.obstacles=(int *)calloc(info_map.n_obstacles,sizeof(int));

      pthread_mutex_lock(&editMatrix);
      spawnPlayer(clientsd, &infoplayer);
      pthread_mutex_unlock(&editMatrix);

      printf("Giocatore %c\n", getLetter(clientsd));
      printf("Fine Spawn\n");
      printf("Coordinate\nx: %d\ny: %d\n", infoplayer.x, infoplayer.y);
      while(1){
          matrixToString(info, clientsd,infoplayer.obstacles);
          memset(info,'\0',sizeof(info));
          printf("Valore gameStarted: %d\n", gameStarted);
          //if(!gameStarted) break;
          if(getLetter(clientsd) == '0') break;
          if(read(clientsd, &command, sizeof(command))>0){
            if(getLetter(clientsd) == '0') break;
              checkCommand(command, &infoplayer,info);
          }
          else{
              gameLogout(clientsd);
              isLogged=0;
              break;
          }
      }
      if(isLogged){
        printf("classifica \n%s\n",scoreboardString);
        write(clientsd,&(int){0},sizeof(int));
        write(clientsd,&(int){0},sizeof(int));
        sendMessage(clientsd,scoreboardString);
        read(clientsd,&command,1);
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
    char msg[200];
    char timeString[30];
    getUTCString(timeString);
    printf("Tempo %s",timeString);
    sprintf(msg,"[%s]Turning on the server...\n",timeString);
    writeLog(msg,0);
    memset(msg,'\0',sizeof(msg));
    memset(timeString,'\0',sizeof(timeString));
    srand(time(NULL));
    if(mutexInitialization()){
      strcpy(msg,"\t-Mutex initialization FAILED\n");
      writeLog(msg,0);
      return 1;
    }
    strcpy(msg,"\t-Mutex initialized\n");
    writeLog(msg,0);
    initNullStruct();
    // socket create and verification 
    memset(msg,'\0',sizeof(msg));
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        strcpy(msg,"\t-Socket creation FAILED\n");
        writeLog(msg,0);
        exit(0); 
    }
    strcpy(msg,"\t-Socket successfully created\n");
    writeLog(msg,0);
    memset(msg,'\0',sizeof(msg));
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
        strcpy(msg,"\t-Socket bind FAILED\n");
        writeLog(msg,0);
        exit(0); 
    }
    strcpy(msg,"\t-Socket successfully binded\n");
    writeLog(msg,0); 
    memset(msg,'\0',sizeof(msg));
    printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 10)) != 0) { 
        printf("Listen failed...\n"); 
        strcpy(msg,"\t-Socket listen FAILED\n");
        writeLog(msg,0); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    strcpy(msg,"\t-Server Listening...\n");
    writeLog(msg,0); 
    memset(msg,'\0',sizeof(msg));
    len = sizeof(cli); 

    /*METTERE QUI THREAD DEL GIOCO PRINCIPALEPUNZ*/ 
    pthread_create(&gameThread, NULL, mapGenerator, NULL);
    while(1){
        // Accept the data packet from client and verification 
        connfd = accept(sockfd, (SA*)&cli, &len); 
        if(connfd>0){
            memset(msg,'\0',sizeof(msg));
            memset(timeString,'\0',sizeof(timeString));
            i++;
            int *thread_sd = (int*) malloc(sizeof(int));
            *thread_sd =  connfd;
            /////////
            sprintf(msg,"\t[%s] New connection from %s\n",timeString,inet_ntoa(cli.sin_addr));
            writeLog(msg,1);
            ////////
            printf("server: new connection from %d %s\n",connfd,inet_ntoa(cli.sin_addr));
            pthread_create(&tid, NULL, clientThread, (void *) thread_sd);
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

/*char getLetter(int clientsd){
  int i;
  char c = '0';
  for(i=0;i<MAX_USERS;i++){
    if(mapPlayers[i]==clientsd){
      c=(char)(i+65);
      break;
    }
  }
  return c;
} VECCHIO GETLETTER RIMPIAZZATO DA PARSEPLAYER*/

void logoutStructs(int clientsd){
  int i;
  for(i=0;i<MAX_USERS;i++){
    if(mapPlayers[i]==clientsd){
      //pthread_mutex_lock(&editMapPlayers); //cappadavide 
      mapPlayers[i]=-1;
      //pthread_mutex_unlock(&editMapPlayers);//cappadavide
    }
    if(scoreboard[i]->clientsd == clientsd){
      scoreboard[i]=nullStruct;
    }
  }
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
  setScorePlayer(info_player);
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
  info_player->pack=NULL;
  printf("clientsd nullStruct prima di dio: %d\n", nullStruct->clientsd);
  info_player->clientsd = clientsd;
  printf("clientsd nullStruct dopo dio: %d\n", nullStruct->clientsd);

  map[index1][index2].playerSD=clientsd;

}

void setScorePlayer(struct player *info_player){
  int i=0;
  while(scoreboard[i]!=nullStruct){
    i++;
  }
  scoreboard[i]=info_player;
  printf("Nuovo valore giocatore: %p\n", scoreboard[i]);

}

char getLetter(int playerSD){
  char c='0';
  for(int i = 0; i < MAX_USERS; i++){
    if(mapPlayers[i] == playerSD)
      return ((char)i+65);
  }
  return c;
}

void matrixToString(char *info, int clientsd,int *obstacles){
  int i = 0;
  int j = 0;
  char msg[16];
  struct obstacles *a;
  write(clientsd, &rows, sizeof(int));
  write(clientsd, &cols, sizeof(int));
  while(i < rows){
    memset(msg,'\0',16);
    while(j < cols){
      if(map[i][j].playerSD >=0){
        printf("parsing %c\n", getLetter(map[i][j].playerSD));
        msg[j] = getLetter(map[i][j].playerSD);
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
  sendMessage(clientsd,info);


}

void initGame(){
  int i=0,j=0;
  initNullStruct();
  for(i=0;i<MAX_USERS;i++){ 
        mapPlayers[i]=-1;
        scoreboard[i]=nullStruct;
        printf("Init: %d\n", scoreboard[i]->clientsd);
  }
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
  char buff[250];
  int n;
  strcpy(buff, msg);
  n = strlen(buff);
  write(clientsd, &n, sizeof(int));
  if(n>0)
    write(clientsd, buff, n);
}



void checkCommand(char msg, struct player *info_player,char *info){
  //int clientsd=map[info_player->x][info_player->y].playerSD;
  int n;
  char obj;
  if(msg == 't' || msg == 'T'){
    sprintf(info, "Tempo rimanente: %d secondi\n", gameTime);
  }
  else if(msg=='p'||msg=='P'){
    if(!info_player->hasItem){
      if(map[info_player->x][info_player->y].pointer!=NULL){
        info_player->pack=(struct items*)map[info_player->x][info_player->y].pointer;
        obj=map[info_player->x][info_player->y].object;
        map[info_player->x][info_player->y].object=' ';
        map[info_player->x][info_player->y].pointer=NULL;
        info_player->hasItem=1;
        if(obj=='$')
          sprintf(info, "Raccolto il seguente oggetto: Oro. Consegnalo al magazzino numero %d\n",info_player->pack->warehouse);
        else if(obj=='@')
          sprintf(info, "Raccolto il seguente oggetto: Cibo. Consegnalo al magazzino numero %d\n",info_player->pack->warehouse);
        else
          sprintf(info, "Raccolto il seguente oggetto: Spada. Consegnalo al magazzino numero %d\n",info_player->pack->warehouse);
      }
    }
    else
    {
      strcpy(info,"Inventario pieno.");
    }
  }
  else if(msg=='e'||msg=='E'){
    if(info_player->hasItem && isWarehouseHere(info_player)){
      //printf("Il giocatore possiede un pacco con id %d\n",info_player->pack->warehouse);
      info_player->hasItem=0;
      info_player->itemsDelivered++;
      info_player->pack=NULL;
      strcpy(info,"Oggetto consegnato.\n");
    }
    else if(info_player->hasItem==0 && isWarehouseHere(info_player))
        strcpy(info,"Non hai oggetti nell'inventario.\n");
    else if(info_player->hasItem && !isWarehouseHere(info_player))
        strcpy(info,"Non ci sono magazzini nelle vicinanze.\n");
    else
        strcpy(info,"Per depositare un oggetto hai bisogno di un oggetto e di un magazzino nelle vicinanze!\n");
    
  }
  else if(msg=='i'||msg=='I'){
    if(info_player->hasItem){
      sprintf(info,"Sei il giocatore %c\nOggetti consegnati:%d\nHai un oggetto da consegnare al magazzino numero %d\n",getLetter(info_player->clientsd),info_player->itemsDelivered,info_player->pack->warehouse);
    }
    else
      sprintf(info,"Sei il giocatore %c\nOggetti consegnati:%d\nNon hai oggetti da consegnare\n",getLetter(info_player->clientsd),info_player->itemsDelivered);
    
  }
  else if(msg=='h'||msg=='H'){
    snprintf(info,238,"---LISTA COMANDI---\n[W]Muoversi sopra\n[A]Muoversi a sinistra\n[S]Muoversi giù\n[D]Muoversi a destra\n[I]Informazioni giocatore\n[P]Prendere oggetti\n[E]Consegnare oggetti\n[T]Tempo rimanente\n(Sono ammesse anche le lettere minuscole)\n");
  }
  else
    checkMovement(msg, info_player,info);

}



void checkMovement(char msg, struct player *info_player,char *info){
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
  strcpy(info,"");
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


int isWarehouseHere(struct player *a){
  int exp;
  exp=checkWarehouse(a,-1,0)+checkWarehouse(a,1,0)+checkWarehouse(a,0,-1)+checkWarehouse(a,0,1);
  return exp>0;
}

int checkWarehouse(struct player *a, int add_x,int add_y){
  struct warehouse *b;
  if(noBoundaryCheck(a,add_x,add_y) && a->pack!=NULL){
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

int mutexInitialization(){
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
    if (pthread_mutex_init(&notifyMaxItems, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    if (pthread_cond_init(&mapGen_cond_var, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    return 0;
}

void gameLogout(int clientsd){
  logout(clientsd);
  logoutStructs(clientsd);
}

void createScoreboard(){
  int i = 0;
  char buff[50];
  char cmd[200] = ""; 
  char num[10];
  int fd;
  int n;
  memset(scoreboardString,'\0',sizeof(scoreboardString)); //cappadavide
  strcpy(scoreboardString,"---PARTITA FINITA---\nClassifica avventurieri:\n");
  quicksort(scoreboard, 0, MAX_USERS-1);
  while(i < MAX_USERS){
    if(scoreboard[i]->clientsd >= 0){
      sprintf(num, "%d", scoreboard[i]->clientsd);
      strcpy(cmd, "echo $(cat logged_users | sed -n 's/\\(.*\\) ");
      strcat(cmd, num);
      strcat(cmd, "$/\\1/p') > tmp2");
      printf("Comando: %s\n", cmd);

      if((fd = open("./tmp2", O_RDWR | O_CREAT | O_TRUNC, 0777)) < 0){
        perror("Errore creazione file tmp2");
        exit(1);
      }

      system(cmd);
      n = read(fd, buff, 50);
      system("rm tmp2");
      buff[n-1] = '\0';
      strcat(scoreboardString, buff);
      strcat(scoreboardString, " ");
      sprintf(num, "%d", scoreboard[i]->itemsDelivered);
      strcat(scoreboardString, num);
      strcat(scoreboardString, "\n");
      printf("Operazione su %d %s\n",scoreboard[i]->clientsd,scoreboardString);
    }

    i++;
  }
}

void copyStruct(struct player *a, struct player *temp){
  temp->x = a->x;
  temp->y = a->y;
  temp->hasItem = a->hasItem;
  temp->itemsDelivered = a->itemsDelivered;
  temp->pack = a->pack;
  temp->obstacles = a->obstacles;
  temp->clientsd = a->clientsd;
}


void swapStruct(struct player *a, struct player *b){
  struct player *temp = (struct player *)malloc(sizeof(struct player));
  copyStruct(a, temp);
  copyStruct(b, a);
  copyStruct(temp, b);
  free(temp);
}

void quicksort(struct player* a[MAX_USERS], int first, int last){
   int i, j, pivot, temp;
/*pivot -- inizialmente il pivot è il first elemento
first e last sono le due variabili che servono per scorrere l'array
*/
   if(first<last){
      pivot=first;
      i=first;
      j=last;     
      
      while(i<j){
         while(a[i]->itemsDelivered <= a[pivot]->itemsDelivered && i < last)
            i++;
         while(a[j]->itemsDelivered > a[pivot]->itemsDelivered)
            j--;
         if(i<j){   
            swapStruct(a[i], a[j]);
         }
      }

      swapStruct(a[pivot], a[j]);
      quicksort(a, first, j-1);
      quicksort(a, j+1, last);
   }
}

void initNullStruct(){
  nullStruct = (struct player *)malloc(sizeof(struct player));
  nullStruct->clientsd = -1;
  nullStruct->itemsDelivered = 0;
}

void writeLog(char *msg,int flag){
  int fd;
  if(flag){
    pthread_mutex_lock(&gameLog);
    fd=open("./gameLog",O_RDWR | O_APPEND|O_CREAT, 0666);
    if(fd<0){
      perror("Errore creazione file gameLog");
      exit(1); //da gestire meglio
    }
    write(fd,msg,strlen(msg));
    close(fd);
    pthread_mutex_unlock(&gameLog);
  }
  else{
    fd=open("./gameLog",O_RDWR | O_APPEND|O_CREAT, 0666);
    if(fd<0){
      perror("Errore creazione file gameLog");
      exit(1); //da gestire meglio
    }
    write(fd,msg,strlen(msg));
    close(fd);
  }
}

void getUTCString(char *str){
  time_t connTime;
  struct tm *infoTime;
  time(&connTime);
  infoTime=gmtime(&connTime);
  strftime(str,sizeof(str),"%c",infoTime);
}