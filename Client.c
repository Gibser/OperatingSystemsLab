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
#include <signal.h>
#define MAX 1000 
extern int errno;
int userStatus=0; //0 Menu 1 login 2 sign up
int server_sd;
struct config{
    unsigned int port;
    char ip[28];
    char hostname[100];
};
void clientAbort(int signalvalue);
void chooseServer(struct sockaddr_in *serverConfig);
//void game(int server_sd);
//void receiveMessage(int server_sd);
//int isExit(char buffer[]);
void homeClient();//aveva server_sd
char loginCred();//aveva server_sd
void regCred();//aveva server_sd
void printGuide();
int checkLoginStatus(char *msg);
int combineStr(char *creds,char *username, char *password);

void clientAbort(int signalvalue){
    printf("Sono il gestore dei segnali\n");
    if(userStatus==0){
        write(server_sd,"~USREXIT",8);
    }
}


int main() 
{ 
    signal(SIGINT,clientAbort);
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
            printf("Errore: %s\nHai inserito bene i parametri?\n",strerror(errno));
            exit(0); 
        } 
        else
            printf("Connesso al server!\n");
            server_sd=sockfd;
            homeClient();
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
/*
void receiveMessage(int server_sd){
    int i=0,nread,nbytes;
    char buffer[5000];
    read(server_sd,&nbytes,sizeof(int));//Read how many bytes server is going to send me
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
*/
int checkLoginStatus(char *msg){
    if(strcmp(msg,"~OKLOGIN")==0){
        printf("Login effettuato!\n");
        return 1;
    }
    else if(strcmp(msg,"~USRNOTEXISTS")==0){
        printf("L'utente non esiste\n");
    }
    else if(strcmp(msg,"~NOVALIDPW")==0){
        printf("La password inserita non è corretta\n");
    }
    else{
        printf("Qualcosa è andato storto\n");
    }
    return 0;
}

int combineStr(char *creds,char *username,char *password){
    strcpy(creds,username);
    strcat(creds,"\n");
    strcat(creds,password);
    return strlen(creds);
}

//Gestione del login nel client
char loginCred(){
    char username[100];
    char password[100];
    char creds[200];
    char msg[20];
    int n;
    write(server_sd,"~USRLOGIN",9); //Notifying server about new login
    memset(creds,'\0',sizeof(creds));
    memset(msg,'\0',sizeof(msg));
    printf("Inserisci nome utente: ");
    getchar(); //scarico il buffer
    scanf("%[^\n]", username); 
    if(strstr(username," ")==NULL){ //Check if username contains space character
        printf("Per favore inserire password ");
        getchar(); //scarico il buffer
        scanf("%[^\n]", password);
        if(strstr(password," ")==NULL){
            n=combineStr(creds,username,password);
            write(server_sd,&n,sizeof(int));//Tell to server how many bytes I'm going to send him
            write(server_sd,creds,strlen(creds));//Then I send data
            read(server_sd,msg,sizeof(msg));//Receive msg about login status
            if(checkLoginStatus(msg))
                return '1';
            else
                return '0';
            
        }
        else{
            printf("La password non può contenere spazi.\n");
            n=0;
            write(server_sd,&n,sizeof(int));
        }
    }
    else{
        printf("Il nome utente non può contenere spazi.\n");
        n=0;
        write(server_sd,&n,sizeof(int));
    }
    
}

//Gestione della registrazione nel client
void regCred(){
    char username[100];
    char password[100];
    char creds[200];
    char msg[20];
    int n;                
    write(server_sd,"~USRSIGNUP",10); //Notifying server about new registration
    memset(creds,'\0',sizeof(creds));
    memset(msg,'\0',sizeof(msg));
    printf("Inserisci nome utente: ");
    getchar(); //scarico il buffer
    scanf("%[^\n]", username); 
    if(strstr(username," ")==NULL){ //Check if username contains space character
        printf("Per favore inserire password ");
        getchar(); //scarico il buffer
        scanf("%[^\n]", password);
        if(strstr(password," ")==NULL){
            n=combineStr(creds,username,password);
            write(server_sd,&n,sizeof(int));//Tell to server how many bytes I'm going to send him
            write(server_sd,creds,strlen(creds));//Then I send data
            read(server_sd,msg,sizeof(msg));
            if(strcmp(msg,"~SIGNUPOK")==0){
                printf("Registrazione effettuata con successo!\n");
            }
            else{
                printf("Utente già registrato.\n");
            }
        }
        else{
            printf("La password non può contenere spazi.\n");
            n=0;
            write(server_sd,&n,sizeof(int));
        }
    }
    else{
        printf("Il nome utente non può contenere spazi.\n");
        n=0;
        write(server_sd,&n,sizeof(int));
    }
}

void printGuide(){
    /*char buffer[1200];
    int fd=open("GameGuide.txt",O_RDONLY);
    if(fd<0){
        perror("Qualcosa è andato storto");
    }
    else{
        
        close(fd);
    }*/
}

void homeClient(){
    char scelta[30];
    char log = '0';
    while(log != '1'){
        printf("Benvenuto!\n\n1 - Login\n2 - Registrazione\n3 - Guida\n4 - Informazioni\n5 - Esci\n\nScelta: ");
        scanf("%s", scelta);
        if(strlen(scelta)==1){
            switch(scelta[0]){
                case '1':
                    log=loginCred(server_sd);
                    break;
                case '2':
                    regCred(server_sd);
                    break;
                case '3':
                    printGuide();
                    break;
                case '4':
                    //printInfo();
                    break;
                case '5':
                    printf("Uscita...\n");
                    break;
                default:
                    printf("Scelta non valida, riprovare.\n");
                    break;
                
            }            
        }
        else{
            printf("Scelta non valida, riprovare.\n");
        }
    
        
    }

    printf("Gioco...\n");
}