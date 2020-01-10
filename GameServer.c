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

// Login Function
void *login(void *sockfd) 
{ 
    char choice[10],buffer[100]="----PROGETTO LSO-GIOCO----\nBenvenuto\n(1)Login\n(2)Registrati\n(3)Aiuto"; 
    int n,connected=1,clientsd=*(int*)sockfd; 
    write(clientsd,"----PROGETTO LSO-GIOCO----\nBenvenuto\n(1)Login\n(2)Registrati\n(3)Aiuto",sizeof(buffer));
    // read the message from client and copy it in buffer 
    //memset(choice, '\0', 10);
    while(1){
        printf("Listening..\n");
        read(clientsd, choice, sizeof(choice)); 
        printf("Lettera: %c\n",choice[0]);
        if(choice[0]=='1'){
        }
        else if(choice[0]=='2'){

        }
        else if (choice[0]=='3'){
            write(clientsd,"Studente: Davide Somma\nMatricola:N86002618\n",100);

        }
        else if (strncmp("exit", choice, 4) == 0) { 
                printf("Server Exit...\n"); 
                close(*(int*)sockfd);
                pthread_exit(NULL);
        } 
        else{
            write(clientsd,"Per favore, immettere una scelta valida, altrimenti exit per uscire",50);
        }
        memset(choice, '\0', MAX);
    }

   /* n = 0; 
        // copy server message in the buffer 
        while ((buff[n++] = getchar()) != '\n') 
            ; 
  */
        // and send that buffer to client 
        //write(*(int*)sockfd, buff, sizeof(buff)); 
  
        // if msg contains "Exit" then server exit and chat ended. 
      
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
            pthread_create(&tid, NULL, login, (void *) thread_sd);
    
        }
        

    }
    // After chatting close the socket 
    close(sockfd); 
} 


