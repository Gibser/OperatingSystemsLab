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
struct config{
    unsigned int port;
    char ip[28];
    char hostname[100];
};
void clientAbort(int signalvalue);
void chooseServer(struct sockaddr_in *serverConfig);
void homeClient(int server_sd);//aveva server_sd
char loginCred(int server_sd);//aveva server_sd
void regCred(int server_sd);//aveva server_sd
void printGuide();
void printInfo();
int checkLoginStatus(char *msg);
int combineStr(char *creds,char *username, char *password);
void printRow(char *buff);
void printMap(int server_sd);

void clientAbort(int signalvalue){
    printf("\nRitorna presto!\n");
    exit(0);
}


void printRow(char *buff){
    printf("| ");
    for(int i = 0; i < strlen(buff); i++){
        printf("%c ", buff[i]);
    }
    printf("|");
    printf("\n");
}

void printMap(int server_sd){
    char row[16];
    int rows;
    int cols;
    read(server_sd, &rows, sizeof(int));
    read(server_sd, &cols, sizeof(int));
    //printf("Righe: %d Colonne: %d\n",rows,cols);
    printf("  ");
    for(int i = 0; i < cols; i++)
        printf("_ ");
    printf("\n");
    for(int i = 0; i < rows; i++){
        read(server_sd, row, cols);
        printRow(row);
    }

    printf("  ");
    for(int i = 0; i < cols; i++)
        printf("─ ");

    printf("\n");
}

void game(int server_sd){
    char msg;
    char row[16];
    int rows;
    int cols;
    while(1){
        printMap(server_sd);
        printf("Comando: ");
        scanf(" %c", &msg);
        write(server_sd, &msg, 1);
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
            homeClient(sockfd);
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

int checkLoginStatus(char *msg){
    if(strcmp(msg,"~OKLOGIN")==0){
        printf("Login effettuato!\n");
        return 1;
    }
    else if(strcmp(msg,"~USRNOTEXISTS")==0){
        printf("L'utente non esiste\n");
    }
    else if(strcmp(msg, "~USRLOGGED") == 0){
                printf("L'utente è già loggato\n");
    }
    else if(strcmp(msg,"~NOVALIDPW")==0){
        printf("La password inserita non è corretta\n");
    }
    else if(strcmp(msg,"~SERVERISFULL")==0){
        printf("Il server è pieno\n");
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
char loginCred(int server_sd){
    char username[100];
    char password[100];
    char creds[200];
    char msg[20];
    int n;
    userStatus=1;
    write(server_sd,"~USRLOGIN",9); //Notifying server about new login
    memset(creds,'\0',sizeof(creds));
    memset(msg,'\0',sizeof(msg));
    printf("Inserisci nome utente: ");
    getchar(); //scarico il buffer
    scanf("%[^\n]", username); 
    if(strstr(username," ")==NULL){ //Check if username contains space character
        printf("Per favore inserire password: ");
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
            n=-1;//Notifies server that an error occurred
            write(server_sd,&n,sizeof(int));
        }
    }
    else{
        printf("Il nome utente non può contenere spazi.\n");
        n=-1;//Notifies server that an error occurred
        write(server_sd,&n,sizeof(int));
    }
    return '0';
    
}

//Gestione della registrazione nel client
void regCred(int server_sd){
    char username[100];
    char password[100];
    char creds[200];
    char msg[20];
    int n;
    userStatus=2;                
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
            n=-1;//Notifies server that an error occurred
            write(server_sd,&n,sizeof(int));
        }
    }
    else{
        printf("Il nome utente non può contenere spazi.\n");
        n=-1;//Notifies server that an error occurred
        write(server_sd,&n,sizeof(int));
    }
}

void printGuide(){
    char buffer[10];
    int nread;
    int fd=open("GameGuide.txt",O_RDONLY);
    if(fd<0){
        perror("Qualcosa è andato storto");
    }
    else{
        system("clear");
        while((nread=read(fd,buffer,5))>0){
            write(STDOUT_FILENO,buffer,nread);
        }
        close(fd);
    }
}

void printInfo(){
    char buffer[10];
    int nread;
    int fd=open("GameInfo.txt",O_RDONLY);
    if(fd<0){
        perror("Qualcosa è andato storto");
    }
    else{
        system("clear");
        while((nread=read(fd,buffer,3))>0){
            write(STDOUT_FILENO,buffer,nread);
        }
        close(fd);
    }
}

void homeClient(int server_sd){
    char scelta[30];
    char home[]="\n----PROGETTO LSO-GIOCO----\nBenvenuto,cosa vuoi fare?\n(1)Login\n(2)Registrati\n(3)Aiuto\n(4)Informazioni sul gioco\n(5)Esci\n\nScelta:";
    char log = '0';
    while(log != '1'){
        userStatus=0;
        write(STDOUT_FILENO,home,sizeof(home));
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
                    printInfo();
                    break;
                case '5':
                    printf("Uscita...\n");
                    write(server_sd,"~USREXIT",8);
                    exit(0);
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
        game(server_sd);
}