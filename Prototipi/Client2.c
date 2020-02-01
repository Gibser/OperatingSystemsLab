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
#define clear() printf("\033[H\033[J")
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
void receiveMessage(int server_sd);
int isExit(char buffer[]);

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
            game(sockfd);
        }
} 

void chooseServer(struct sockaddr_in *serverConfig){
    int chooseOption;
    unsigned int port;
    char ip[28];
    char hostname[100];
    serverConfig->sin_family = AF_INET; 
    struct hostent *p;
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
        //Da testare
        printf("Inserisci hostname\n");
        scanf("%s",hostname);
        p=gethostbyname(hostname);
        if(!p){
            herror("gethostbyname");
            exit(1);
        }
        serverConfig->sin_addr.s_addr=*(uint32_t*)(p->h_addr_list[0]);
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

void receiveMessage(int server_sd){
    int i=0,nread,nbytes;
    char buffer[5000];
    read(server_sd,&nbytes,sizeof(int));//Read how many bytes server is going to send me
    printf("Il server mi manda %d caratteri\n",nbytes);
    while(i<nbytes){//Ready to read message and write on STDOUT
        nread=read(server_sd,buffer,100);//reading small chunks of bytes to avoid lost data 
        i+=nread;
        write(STDOUT_FILENO,buffer,nread);
    }
}

int isExit(char buffer[]){
    if(strlen(buffer)==4){
        if(memcmp(buffer,"exit",4)==0)
            return 1;
    }
    return 0;
}


void game(int server_sd){
    char buffer[5000];
    int n,num_ready,i,nread;
    receiveMessage(server_sd); 
    memset(buffer,'\0',sizeof(buffer));
    while(1){
        printf("Scegli:\n");
        scanf("%s",buffer);
        if(strlen(buffer)>0){
            n=strlen(buffer);
            write(server_sd,&n,sizeof(int));//Tell to server how many bytes I'm going to send him
            write(server_sd,buffer,strlen(buffer));//Then I send data
            if(isExit(buffer)){
                close(server_sd);
                printf("Disconnesso.\n");
                break;
            }
            memset(buffer,'\0',sizeof(buffer));//Clear buffer
            system("clear");//Clear shell for a better readability
            receiveMessage(server_sd);
  
        }
        memset(buffer,'\0',sizeof(buffer));
    }
}
