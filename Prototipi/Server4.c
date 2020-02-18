/*Prova Server*/

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
#include "login3.h"
#include "gameLib.h"
#define MAX 1000
#define PORT 5000
#define SA struct sockaddr 
#define MAX_THREADS 8


pthread_mutex_t signup_mutex;
pthread_mutex_t login;
//int threadStatus[MAX_THREADS]={0};
//int isAvailable(int slot);

struct mapObjects info_map;    //info numero oggetti sulla mappa
struct cell **map;
int rows, cols;
char items[3][10]={"sword","gold","food"};
int mapPlayers[MAX_USERS]={0};


void *mapGenerator(void* args){
    rows = randNumb();
    cols = randNumb();
    initializeMatrix(rows, cols, map);
    createMap(&info_map, rows, cols, map);
    pthread_exit(NULL);
}

// Game Function
void game(int clientsd){
    char msg[30];
    spawnPlayer(map, mapPlayers, rows, cols);
    while(1){
        memset(msg,'\0',sizeof(msg));
        if(read(clientsd,msg,sizeof(msg))>0){
            printf("%s\n", msg);
        }
        else{
            logout(clientsd);
            break;
        }
    }
}


void *clientThread(void *sockfd) 
{ 
    int clientsd=*(int*)sockfd;
    int log = 0;
    log = loginMain(clientsd, signup_mutex, login);
    if(log == 1){
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