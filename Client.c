#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h> 
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#define MAX 1000 
extern int errno;
int epfd;
struct epoll_event event;
struct epoll_event events[64];

struct config{
    unsigned int port;
    char ip[28];
    char hostname[100];
};

void chooseServer(struct sockaddr_in *serverConfig);
void game(int server_sd);
int main() 
{ 

    int sockfd, connfd; 
    pthread_t tid;
    struct sockaddr_in serverConfig;
    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
   
    if (sockfd == -1){ 
        printf("C'è stato un problema: chiusura imminente.\n"); 
        exit(0); 
    } 
    else{
        chooseServer(&serverConfig);
        // connect the client socket to server socket 
        if (connect(sockfd, (struct sockaddr*)&serverConfig, sizeof(serverConfig)) != 0) { 
            //printf("Connessione con il server fallita...\n"); 
            printf("Errore: %s\nHai inserito bene i parametri?\n",strerror(errno));
            exit(0); 
        } 
        else
            printf("Connesso al server!\n");
            //fcntl(sockfd, F_SETFL, O_NONBLOCK); /* Change the socket into non-blocking state*/	
            game(sockfd);
        }
} 

void chooseServer(struct sockaddr_in *serverConfig){
    int chooseOption;
    unsigned int port;
    char ip[28];
    serverConfig->sin_family = AF_INET; 
    printf("Benvenuto!\n");
    printf("Specificare come vuoi selezionare il server:\n(1)Tramite IP\n(2)Tramite Hostname\n");
    scanf("%d",&chooseOption);
    switch (chooseOption)
    {
    case 1:
        printf("Inserisci IP:\n");
        scanf("%s",ip);
        if(inet_aton(ip, &serverConfig->sin_addr)==0){
            printf("Ip non valido\n");
            exit(1);
        }   
        break;
    case 2:
        printf("Inserisci hostname\n");
        break;
    
    default:
        printf("Operazione annullata. Chiusura imminente.\n");
        exit(0);
        break;
    }
    printf("Inserisci porta\n");
    scanf("%u",&port);
    serverConfig->sin_port=htons(port);
}

void game(int server_sd){
    char buffer[1000];
    int n,num_ready,i;
    n=read(server_sd, buffer, 1000); 
    write(STDOUT_FILENO,buffer,n);
    memset(buffer,'\0',sizeof(buffer));
    n=0;
    while(1){
        printf("Scegli:\n");
        scanf("%s",buffer);
        if(strlen(buffer)>0){
            write(server_sd,buffer,strlen(buffer));
            memset(buffer,'\0',sizeof(buffer));
            system("clear");
            /*while(n=read(server_sd,buffer,1)>0)
                write(STDOUT_FILENO,buffer,1);*/
            n=read(server_sd,buffer,1000);
            write(STDOUT_FILENO,buffer,n);
        }
        n=0;
        memset(buffer,'\0',sizeof(buffer));
    }
}
