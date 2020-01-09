/*Prova Server*/

#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>
#include <pthread.h>
#define MAX 80 
#define PORT 5000
#define SA struct sockaddr 

// Function designed for chat between client and server. 
void *func(void *sockfd) 
{ 
    char buff[MAX]; 
    int n;
    int clientsd=*(int*)sockfd; 
    // infinite loop for chat 
    for (;;) { 
        memset(buff, '\0', MAX);
        //write(*(int*)sockfd,"Benvenuto!")
        // read the message from client and copy it in buffer 
        read(clientsd, buff, sizeof(buff)); 
        // print buffer which contains the client contents 
        printf("From client: %s\t To client : ", buff); 

        memset(buff, '\0', MAX);
        n = 0; 
        // copy server message in the buffer 
        while ((buff[n++] = getchar()) != '\n') 
            ; 
  
        // and send that buffer to client 
        write(*(int*)sockfd, buff, sizeof(buff)); 
  
        // if msg contains "Exit" then server exit and chat ended. 
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    } 
    close(*(int*)sockfd);
    pthread_exit(NULL);
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
            pthread_create(&tid, NULL, func, (void *) thread_sd);
    
        }
        
        // After chatting close the socket 
        close(sockfd); 
    }
} 


