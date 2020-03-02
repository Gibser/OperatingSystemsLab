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
#include <signal.h>
#include "login.h"
#include "gameLib.h"
#define MAX 1000
#define SA struct sockaddr 
#define MAX_THREADS 8
#define TIMER 20
/**spawnPlayer gets a clientSD and a pointer to a struct player type as parameters.
 * It spawns a player in a cell (using some criteria) and initializes its associated struct.
 */
void spawnPlayer(int clientsd,struct player *info_player,char *username);
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
/*quicksort algorithm to sort a game session's scoreboard*/
void quicksort(struct player* a[MAX_USERS], int first, int last);
void writeLog(char *msg,int flag);
int mutexInitialization();
char * getUTCString();
struct player *getWinner(int dim);
void writeLog_ItemDelivered(struct player *info);
void writeLog_NewConnection(char *ip);
void writeLog_GameOver(struct player *winner);
void writeLog_JoinGame(char *user);
void writeLog_QuitGame(char *user);
void writeLog_serverAbort();
void dropItem(struct player *info);
void initCell(int i, int j);
void selectionSort(int n);
int partiziona(struct player* a[], int low, int high);
void dropInPosition(struct player *info, int add_x,int add_y);
void printScoreboard();
void buildLoggedUsersString(char *loggedUsersString);
int findMax(int i);
int setScoreboardArr();
void checkPort(int argc,char **args);
void waitSeconds(int seconds);
void clientAbort();

pthread_mutex_t signup_mutex;
pthread_mutex_t login;
pthread_mutex_t editMatrix; 
pthread_mutex_t editMapPlayers;
pthread_mutex_t mapGen;
pthread_mutex_t notifyMaxItems;
pthread_cond_t mapGen_cond_var = PTHREAD_COND_INITIALIZER;
pthread_mutex_t gameLog;
pthread_mutex_t loggedUsersCountMutex;

int PORT=49153;
struct mapObjects info_map;    //info numero oggetti sulla mappa
struct cell **map;
int rows, cols;
int mapPlayers[MAX_USERS];
struct player* scoreboard[MAX_USERS];
int gameStarted = 0;
int gameTime = TIMER;
int MAX_ITEMS;
int maxItemsReached=0;
char scoreboardString[500]="";
int loggedUsersCount = 0;
struct player **arr;


void waitSeconds(int seconds){
  struct timeval now;
  struct timeval current;
  gettimeofday(&now,NULL);
  current = now;
  while(current.tv_sec < now.tv_sec + seconds)
    gettimeofday(&current,NULL);

  return;
}


void *mapGenerator(void* args){
    int i=0,j=0;
    char *timeString;
    struct player *winner;
    char msg[100];

   

    while(1){
      maxItemsReached = 0;
      memset(msg,'\0',sizeof(msg));
      timeString=getUTCString();
      sprintf(msg,"[%s]Starting new game session...\n",timeString);
      writeLog(msg,1);
      gameStarted = 0;
      while(!loggedUsersCount);
      //pthread_mutex_lock(&editMatrix);
      rows = randNumb();
      cols = randNumb();
      printf("Rows: %d\nCols: %d\n", rows, cols);
      initGame();
      createMap(&info_map, rows, cols, map);

      //condizione di vittoria
      if(loggedUsersCount != 0)
        MAX_ITEMS = rand()%(info_map.n_items-MAX_USERS)+(MAX_USERS/loggedUsersCount);
      else
        MAX_ITEMS = rand()%(info_map.n_items-MAX_USERS)+(MAX_USERS/2);
      
      printf("Numero massimo di pacchi: %d\n", MAX_ITEMS);
      //waitSeconds(5);
      sleep(5);
      printf("Sblocco i threads...\n");
      //pthread_mutex_unlock(&editMatrix);
      pthread_cond_broadcast(&mapGen_cond_var);
      gameStarted = 1;
      while(gameTime-- > 0){
        if(maxItemsReached==1)
          break;
        sleep(1);
      }
      printf("\nCreo scoreboard...\n");
      createScoreboard();
      printf("\n\n--------------------------\n\n");
      printf("\n\nScoreboard creata\n\n");
      free(arr);
      printf("\n\nLog aggiornato\n\n");
      printf("\n\n---------------------------\n\n");
      gameTime = TIMER;
      printf("Fine sleep, genero mappa...\n");
    }
}


// Game Function
void game(int clientsd,char *username){
    char info[350]="Benvenuto in partita, giovane avventuriero! Premi [H] per aiuto, [Q] per uscire.\nPremi un tasto qualsiasi per iniziare la partita.\n";
    char command;
    struct player infoplayer;
    int isLogged=1;
    char playerLetter;
    pthread_mutex_lock(&loggedUsersCountMutex);
    loggedUsersCount++;
    pthread_mutex_unlock(&loggedUsersCountMutex);
    while(isLogged){
      if(!gameStarted){
        //read(clientsd, &command, 1);
        pthread_mutex_lock(&editMatrix);
        pthread_cond_wait(&mapGen_cond_var, &editMatrix);
        pthread_mutex_unlock(&editMatrix);
        printf("Attesa terminata\n");
      }
      infoplayer.obstacles=(int *)calloc(info_map.n_obstacles,sizeof(int));
      pthread_mutex_lock(&editMatrix);
      spawnPlayer(clientsd,&infoplayer,username);
      pthread_mutex_unlock(&editMatrix);
      playerLetter = getLetter(clientsd);
      write(clientsd, &playerLetter, 1);
      while(1){
          if(infoplayer.itemsDelivered==-1)
            break;
          matrixToString(info, clientsd,infoplayer.obstacles);
          memset(info,'\0',sizeof(info));
          if(getLetter(clientsd) == '0') break;
          if(read(clientsd, &command, sizeof(command))>0){
            if(getLetter(clientsd) == '0') break;
            checkCommand(command, &infoplayer,info);
          }
          else{
              printf("\n\nClient disconnesso...\n\n");
              if(infoplayer.hasItem)
                dropItem(&infoplayer);
              else
                initCell(infoplayer.x, infoplayer.y);
              gameLogout(clientsd);
              isLogged=0;
              writeLog_QuitGame(username);
              pthread_mutex_lock(&loggedUsersCountMutex);
              loggedUsersCount--;
              pthread_mutex_unlock(&loggedUsersCountMutex);
              break;
          }
      }
      if(isLogged){
        if(infoplayer.itemsDelivered>=0){
          write(clientsd,&(int){0},sizeof(int));
          write(clientsd,&(int){0},sizeof(int));
          sendMessage(clientsd,scoreboardString);
          strcpy(info,"Premi un tasto qualsiasi per continuare\n");
        }
        if(read(clientsd,&command,1) <= 0){
          printf("\n\nClient disconnesso...\n\n");
          if(infoplayer.hasItem)
            dropItem(&infoplayer);
          else
            initCell(infoplayer.x, infoplayer.y);
          gameLogout(clientsd);
          isLogged=0;
          writeLog_QuitGame(username);
          pthread_mutex_lock(&loggedUsersCountMutex);
          loggedUsersCount--;
          pthread_mutex_unlock(&loggedUsersCountMutex);
        }
        else
          printf("Comando: %c\n", command);
        

      }
      
    }
}

void clientAbort(){
  
}

void *clientThread(void *sockfd) 
{ 
    signal(SIGPIPE, clientAbort);
    int clientsd=*(int*)sockfd;
    int log = 0;
    char username[100];
    char message[50];
    log = loginMain(clientsd, signup_mutex, login, username);
    if(log == 1){
        printf("Gestisco il client con socket descriptor %d\n\n", clientsd);
        writeLog_JoinGame(username);
        game(clientsd,username);
    }
	close(clientsd);
	pthread_exit(NULL);

}


void serverAbort(){
  printf("\nChiusura server...\n");
  system("rm logged_users");
  writeLog_serverAbort();
  exit(0);
}

// Driver function 
int main(int argc, char **args) 
{   

    checkPort(argc,args);
    system("touch logged_users");
    signal(SIGINT, serverAbort);
    int sockfd, connfd, len,i=0; 
    struct sockaddr_in servaddr, cli; 
    void *result;
    pthread_t tid,gameThread;
    char msg[200];
    char *timeString;
    timeString=getUTCString(timeString);
    printf("Turning on the server..\nListening on port %d..\n",PORT);
    sprintf(msg,"[%s]Turning on the server...  Listening on port %d...\n",timeString,PORT);
    writeLog(msg,0);
    memset(msg,'\0',sizeof(msg));
    srand(time(NULL));
    if(mutexInitialization()){
      strcpy(msg,"\t-Mutex initialization FAILED\n");
      writeLog(msg,0);
      return 1;
    }
    strcpy(msg,"\t-Mutex initialized\n");
    writeLog(msg,0);
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
    if ((listen(sockfd,128)) != 0) { 
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

    pthread_create(&gameThread, NULL, mapGenerator, NULL);
    while(1){
        // Accept the data packet from client and verification 
        connfd = accept(sockfd, (SA*)&cli, &len); 
        if(connfd>0){
            memset(msg,'\0',sizeof(msg));
            i++;
            int *thread_sd = (int*) malloc(sizeof(int));
            *thread_sd =  connfd;
            writeLog_NewConnection(inet_ntoa(cli.sin_addr));
            printf("server: new connection from %d %s\n",connfd,inet_ntoa(cli.sin_addr));
            pthread_create(&tid, NULL, clientThread, (void *) thread_sd);
        }
            
        

    }
    close(sockfd); 
} 

void setLetter(int clientsd){
  int i;
  for(i=0;i<MAX_USERS;i++){
    if(mapPlayers[i]==-1){
      pthread_mutex_lock(&editMapPlayers);
      mapPlayers[i]=clientsd;
      pthread_mutex_unlock(&editMapPlayers);
      break;
    }
  }
}


void logoutStructs(int clientsd){
  int i;
  for(i=0;i<MAX_USERS;i++){
    if(mapPlayers[i]==clientsd){
      mapPlayers[i]=-1;
    }
    if(scoreboard[i] != NULL)
      if(scoreboard[i]->clientsd == clientsd){
        scoreboard[i]=NULL;
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



void spawnPlayer(int clientsd,struct player *info_player,char *username){
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
  strcpy(info_player->username,username);
  printf("Spawn %s\n",info_player->username);
  info_player->x=index1;
  info_player->y=index2;
  info_player->hasItem=0;
  info_player->itemsDelivered=0; //AGGIUNTO LOCK QUANDO SI FA LO SPAWN (cappadavide)
  info_player->pack=NULL;
  info_player->clientsd = clientsd;

  map[index1][index2].playerSD=clientsd;

}

void setScorePlayer(struct player *info_player){
  int i=0;
  while(scoreboard[i] != NULL){
    i++;
  }
  scoreboard[i]=info_player;

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
  char msg[30];
  struct obstacles *a;
  write(clientsd, &rows, sizeof(int));
  write(clientsd, &cols, sizeof(int));
  while(i < rows){
    memset(msg,'\0',sizeof(msg));
    while(j < cols){
      if(map[i][j].playerSD >=0){
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
    write(clientsd, msg, cols);
    j = 0;
    i++;
  }
  sendMessage(clientsd,info);

}

void initGame(){
  int i=0,j=0;
  for(i=0;i<MAX_USERS;i++){ 
        mapPlayers[i]=-1;
        scoreboard[i]=NULL;
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
      if(isCellNotSolid(map[(info_player->x)+add_x][info_player->y+add_y])){
        changeCoordinates(info_player, add_x, add_y);
      }
      else if(map[(info_player->x)+add_x][info_player->y+add_y].isObstacle){
        a=(struct obstacles*)map[(info_player->x)+add_x][info_player->y+add_y].pointer;
        id=a->id;
        info_player->obstacles[id]=1; 
      }
    
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
  int n;
  char obj;
  char loggedUsers[300];
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
      strcpy(info,"Inventario pieno.\n");
    }
  }
  else if(msg=='e'||msg=='E'){
    if(info_player->hasItem && isWarehouseHere(info_player)){
      info_player->hasItem=0;
      info_player->itemsDelivered++;
      writeLog_ItemDelivered(info_player);
      info_player->pack=NULL;
      strcpy(info,"Oggetto consegnato.\n");
      if(info_player->itemsDelivered >= MAX_ITEMS){
        pthread_mutex_lock(&notifyMaxItems);
        maxItemsReached = 1;
        pthread_mutex_unlock(&notifyMaxItems);
      }
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
    snprintf(info,256,"---LISTA COMANDI---\n[W]Muoversi sopra\n[A]Muoversi a sinistra\n[S]Muoversi giù\n[D]Muoversi a destra\n[I]Informazioni giocatore\n[P]Prendere oggetti\n[E]Consegnare oggetti\n[T]Tempo rimanente\n(Sono ammesse anche le lettere minuscole)\n[U]Utenti connessi\n[Q]Esci\n");
  }
  else if(msg=='q'||msg=='Q'){
      strcpy(info,"Sei uscito dal gioco.\n");
      write(info_player->clientsd, &(int){0}, sizeof(int));
      write(info_player->clientsd, &(int){0}, sizeof(int));
      sendMessage(info_player->clientsd,info);
      close(info_player->clientsd);
      info_player->itemsDelivered=-1;
  }
  else if(msg=='u'||msg=='U'){
    buildLoggedUsersString(loggedUsers);
    sprintf(info, "---UTENTI CONNESSI---\n%s", loggedUsers);
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
    /*if (pthread_cond_init(&mapGen_cond_var, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    */
    if (pthread_mutex_init(&loggedUsersCountMutex, NULL) != 0)
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

int setScoreboardArr(){
  int j = 0;
  for(int i = 0; i < MAX_USERS; i++){
    if(scoreboard[i] != NULL)
      arr[j++] = scoreboard[i];
  }
  return j;
}

void createScoreboard(){
  int i; //= 0;
  int n;
  char buffer[100];
  arr = (struct player **)malloc(loggedUsersCount*sizeof(struct player *));
  n = setScoreboardArr();
  printf("n = %d\n", n);
  memset(scoreboardString,'\0',sizeof(scoreboardString)); //cappadavide
  strcpy(scoreboardString,"    ---PARTITA FINITA---\n   Classifica avventurieri:\nGIOCATORI\t\t\tOGGETTI\n");
  selectionSort(n);
  i = n - 1;
  while(i >= 0){
    memset(buffer,'\0',sizeof(buffer));
    if(arr[i]->clientsd >= 0){
      sprintf(buffer,"%s\t\t\t\t%d\n", arr[i]->username, arr[i]->itemsDelivered);
      strcat(scoreboardString, buffer);
    }
    i--;
  }
  struct player *winner;
  if(n == 0)
    winner = NULL;
  else
    winner = getWinner(n);
  writeLog_GameOver(winner);
}

struct player *getWinner(int dim){
  int max = arr[dim-1]->itemsDelivered;
  int i = 0;
  while(i < dim-1){
    if(arr[i]->itemsDelivered == max)
      return NULL;
    i++;
  }
  return arr[dim-1];
}

void copyStruct(struct player *a, struct player *temp){
  memset(temp->username,'\0',sizeof(temp->username));
  temp->x = a->x;
  temp->y = a->y;
  temp->hasItem = a->hasItem;
  temp->itemsDelivered = a->itemsDelivered;
  temp->pack = a->pack;
  temp->obstacles = a->obstacles;
  temp->clientsd = a->clientsd;
  strcpy(temp->username,a->username);
}

void swapStructAddr(struct player *a, struct player *b){
  struct player *temp;
  temp = a;
  a = b;
  b = temp;
}

void swapStruct(struct player *a, struct player *b){
  struct player *temp = (struct player *)malloc(sizeof(struct player));
  copyStruct(a, temp);
  copyStruct(b, a);
  copyStruct(temp, b);
  free(temp);
}

/*
int partiziona(struct player* a[], int low, int high){
  int pivot = a[high]->itemsDelivered;
  int i = (low - 1);
  for(int j = low; j <= high- 1; j++){
    if (a[j]->itemsDelivered < pivot)
        {
            i++;    // increment index of smaller element
            swapStructAddr(a[i], a[j]);
        }
    }
    swapStructAddr(a[i + 1], a[high]);
    return (i + 1);
  }
*/

void selectionSort(int n){
  int max;
  for(int i=n-1;i>0;i--){
    max=findMax(i);
    swapStruct(arr[max], arr[i]);
  }
}

int findMax(int i){
  int max = 0;
  for(int j=1; j<=i; j++){
    if(arr[max]->itemsDelivered < arr[j]->itemsDelivered)
      max=j;
  }
  return max;
}

void printScoreboard(){
  
}

/*
void quicksort(struct player* a[MAX_USERS], int low, int high){
    int pi;
    if (low < high)
    {
      
        pi = partiziona(a, low, high);

        quicksort(a, low, pi - 1);  // Before pi
        quicksort(a, pi + 1, high); // After pi
    }
}
*/


void writeLog(char *msg,int flag){
  int fd;
  if(flag){
    pthread_mutex_lock(&gameLog);
    fd=open("./gameLog",O_RDWR | O_APPEND|O_CREAT, 0666);
    if(fd<0){
      perror("Errore creazione file gameLog");
      exit(1);
    }
    write(fd,msg,strlen(msg));
    close(fd);
    pthread_mutex_unlock(&gameLog);
  }
  else{
    fd=open("./gameLog",O_RDWR | O_APPEND|O_CREAT, 0666);
    if(fd<0){
      perror("Errore creazione file gameLog");
      exit(1); 
    }
    write(fd,msg,strlen(msg));
    close(fd);
  }
}
void writeLog_GameOver(struct player *winner){
  char str[30];
  char msg[200];
  time_t connTime;
  struct tm *infoTime;
  memset(str,'\0',sizeof(str));
  time(&connTime);
  infoTime=gmtime(&connTime);
  strftime(str,sizeof(str),"%c",infoTime);
  if(winner!=NULL){
    sprintf(msg,"\t-[%s] GAME OVER: Winner is %s with %d items delivered.\n",str,winner->username,winner->itemsDelivered);
  }
  else
    sprintf(msg,"\t-[%s] GAME OVER: No winner.\n",str);
  writeLog(msg,1);
}

void writeLog_serverAbort(){
  char str[30];
  char msg[200];
  time_t connTime;
  struct tm *infoTime;
  memset(str,'\0',sizeof(str));
  time(&connTime);
  infoTime=gmtime(&connTime);
  strftime(str,sizeof(str),"%c",infoTime);
  
  sprintf(msg,"[%s]Server closed.\n",str);
  writeLog(msg,1);
}


void writeLog_ItemDelivered(struct player *info){
  char str[30];
  char msg[200];
  time_t connTime;
  struct tm *infoTime;
  memset(str,'\0',sizeof(str));
  time(&connTime);
  infoTime=gmtime(&connTime);
  strftime(str,sizeof(str),"%c",infoTime);
  sprintf(msg,"\t-[%s] %s delivered an item to warehouse n.%d\n",str,info->username,info->pack->warehouse);
  writeLog(msg,1);
}


void writeLog_NewConnection(char *ip){
  char str[30];
  char msg[200];
  time_t connTime;
  struct tm *infoTime;
  memset(str,'\0',sizeof(str));
  time(&connTime);
  infoTime=gmtime(&connTime);
  strftime(str,sizeof(str),"%c",infoTime);
  sprintf(msg,"\t-[%s] New connection from %s\n",str,ip);
  writeLog(msg,1);
}

void writeLog_JoinGame(char *user){
  char str[30];
  char msg[200];
  time_t connTime;
  struct tm *infoTime;
  memset(str,'\0',sizeof(str));
  time(&connTime);
  infoTime=gmtime(&connTime);
  strftime(str,sizeof(str),"%c",infoTime);
  sprintf(msg,"\t-[%s] %s has joined the game!\n",str,user);
  writeLog(msg,1);
}

void writeLog_QuitGame(char *user){
  char str[30];
  char msg[200];
  time_t connTime;
  struct tm *infoTime;
  memset(str,'\0',sizeof(str));
  time(&connTime);
  infoTime=gmtime(&connTime);
  strftime(str,sizeof(str),"%c",infoTime);
  sprintf(msg,"\t-[%s] %s has left the game.\n",str,user);
  writeLog(msg,1);
}

char * getUTCString(){
  static char str[30];
  time_t connTime=time(NULL);
  struct tm *infoTime;
  memset(str,'\0',sizeof(str));
  time(&connTime);
  infoTime=gmtime(&connTime);
  strftime(str,sizeof(str),"%c",infoTime);
  return str;

}
void dropItem(struct player *info){
  if(map[info->x][info->y].object==' ')
    dropInPosition(info,0,0);
  else if(isLeftFree(info->x,info->y))
    dropInPosition(info,0,-1);
  else if(isRightFree(info->x,info->y))
    dropInPosition(info,0,1);
  else if(isUpFree(info->x,info->y))
    dropInPosition(info,-1,0);
  else if(isDownFree(info->x,info->y))
    dropInPosition(info,1,0);
  map[info->x][info->y].playerSD=-1;
}


void initCell(int i, int j){
  map[i][j].isObstacle=0;
  map[i][j].isWareHouse=0;
  map[i][j].playerSD=-1; 
  map[i][j].object=' ';
  map[i][j].pointer=NULL;
}

void dropInPosition(struct player *info, int add_x,int add_y){
  pthread_mutex_lock(&editMatrix);
  map[info->x+add_x][info->y+add_y].object=info->pack->object;
  map[info->x+add_x][info->y+add_y].pointer=(void*)info->pack;
  pthread_mutex_unlock(&editMatrix);
}

void buildLoggedUsersString(char *loggedUsersString){
  int fd;
  int i = 0;
  if((fd = open("logged_users", O_RDONLY)) < 0){
    perror("Errore apertura file logged_users.");
    exit(1);
  }

  char buff;
  while(read(fd, &buff, 1) == 1){
    if(buff != ' ')
      loggedUsersString[i++] = buff;
    else{

      loggedUsersString[i++] = ' ';
      loggedUsersString[i++] = '-';
      loggedUsersString[i++] = ' ';

      while(read(fd, &buff, 1) == 1){
        if(buff == '\n')
          break;
      }
    }
  }
  i = i-3;
  loggedUsersString[i++] = '\n';
  loggedUsersString[i] = '\0';
  close(fd);
}

void checkPort(int argc,char **args){
  if(argc>2){
      printf("Numero di argomenti non valido. Utilizzo: %s <PORTA> (facoltativo)\n",args[0]);
      exit(1);
    }
    else if(argc==2){
      PORT=atoi(args[1]);
      if(PORT <= 0){
        printf("Numero di porta non valido.\n");
        exit(1);
      }
  }
}