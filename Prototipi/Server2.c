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
#define MAX 5000
#define PORT 5000
#define SA struct sockaddr 
void gameMenu(int clientsd);
int isClientConnClose(int clientsd,char buffer[]);


// Game Function
void *gameThread(void *sockfd) 
{ 
    int clientsd=*(int*)sockfd;
    gameMenu(clientsd);

    /*close(clientsd);
    pthread_exit(NULL);*/

}


// Driver function 
int main() 
{ 
   
    int sockfd, connfd, len,i=0; 
    struct sockaddr_in servaddr, cli; 
    pthread_t tid;
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
    printf("Socket successfully created..\n"); 
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
    while(1){

        // Accept the data packet from client and verification 
        connfd = accept(sockfd, (SA*)&cli, &len); 
        if(connfd>0){
            i++;
            int *thread_sd = (int*) malloc(sizeof(int));
            *thread_sd =  connfd;
            printf("server: new connection from %d %s\n",connfd,inet_ntoa(cli.sin_addr));
            pthread_create(&tid, NULL, gameThread, (void *) thread_sd);
    
        }
        

    }
    close(sockfd); 
} 
void gameMenu(int clientsd){
    char gameHome[]="\n----PROGETTO LSO-GIOCO----\nBenvenuto,cosa vuoi fare?\n(1)Login\n(2)Registrati\n(3)Aiuto\n(4)Informazioni sul gioco\n";
    char buffer[MAX];
    int n,nb,fd,nread;
    nb=sizeof(gameHome);
    write(clientsd,&nb,sizeof(int));//Tell to client how many bytes is going to receive
    write(clientsd,gameHome,sizeof(gameHome));//Send menu interface

    while(1){
        if(read(clientsd,&nb,sizeof(int))!=-1){//Read how many bytes client is going to send me
            read(clientsd, buffer, nb); //Read data
            if(strlen(buffer)>0){
                if(isClientConnClose(clientsd,buffer)){
                    printf("Closing connection...\n");
                    close(clientsd);
                    pthread_exit(NULL); 
                }
                if(buffer[0]=='1'){
                }
                else if(buffer[0]=='2'){

                }
                else if (buffer[0]=='3'){
                    memset(buffer,'\0',MAX);
                    fd=open("GameGuide.txt",O_RDONLY);
                    if(fd<0){
                        perror("Qualcosa è andato storto");
                    }
                    else{
                        n=0;
                        while((nread=read(fd,buffer,5))>0){//Reading file in little chunks of bytes
                            n+=nread;
                        }
                        printf("n %d\n",n);
                        strcat(buffer,gameHome);
                        nb=n+sizeof(gameHome);//Send Guide + Menu
                        write(clientsd,&nb,sizeof(int));//Tell to client I'm going to send him nb bytes
                        write(clientsd,buffer,nb);//Send data
                        close(fd);
                    }
                }
                else if(buffer[0]=='4'){
                    memset(buffer,'\0',MAX);
                    fd=open("GameInfo.txt",O_RDONLY);
                    if(fd<0){
                        perror("Qualcosa è andato storto");
                    }
                    else{
                        n=0;
                        while((nread=read(fd,buffer,5))>0){//Reading file in little chunks of bytes
                            n+=nread;
                        }
                        printf("n %d\n",n);
                        strcat(buffer,gameHome);
                        nb=n+sizeof(gameHome);//Send Info + Menu
                        write(clientsd,&nb,sizeof(int));//Tell to client I'm going to send him nb bytes
                        write(clientsd,buffer,nb);//Send data
                        close(fd);
                    }
                }
                else{
                    write(clientsd,"Per favore, immettere una scelta valida, altrimenti exit per uscire",50);
                }
            }   
        }
        n=0;
        memset(buffer,'\0',MAX);
    }

}
int isClientConnClose(int clientsd,char buffer[]){
    if(strlen(buffer)==4){
        if(memcmp(buffer,"exit",4))
            return 1;
    }
    return 0;

}