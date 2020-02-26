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

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"
#define RST "\e[0m"

extern int errno;
int userStatus=0; //0 Menu 1 login 2 sign up
struct config{
    unsigned int port;
    char ip[28];
    char hostname[100];
};

char playerLetter;
int gameFinished = 0;

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
void receiveMessage(int server_sd);

void clientAbort(int signalvalue){
    printf("\nRitorna presto!\n");
    exit(0);
}


void printRow(char *buff){
    //printf("| ");
    printf(RED"|"RST" ");
    for(int i = 0; i < strlen(buff); i++){
        if(buff[i] == playerLetter)
            printf(ANSI_COLOR_YELLOW"%c"ANSI_COLOR_RESET" ", buff[i]);
        else if(buff[i]>='A'&&buff[i]<='H')
            printf(BLU"%c"RST" ",buff[i]);
        else
            printf("%c ", buff[i]);
    }
    //printf("|");
    printf(RED"|"RST);
    printf("\n");
}

void receiveMessage(int server_sd){
    char buffer[250];
    int n_bytes;
    read(server_sd, &n_bytes, sizeof(int));
    //printf("Messaggio di lunghezza %d\n",n_bytes);
    if(n_bytes > 0){
        read(server_sd, buffer, n_bytes);
        buffer[n_bytes] = '\0';
        //write(STDOUT_FILENO, buffer, n_bytes);
        printf(GRN"%s"RST,buffer);
    }

}

void printMap(int server_sd){
    char row[16];
    int rows;
    int cols;
    read(server_sd, &rows, sizeof(int));
    read(server_sd, &cols, sizeof(int));
    if(rows>0&&cols>0){
        //printf("Righe: %d Colonne: %d\n",rows,cols);
        printf("Lettera giocatore: %c\n\n", playerLetter);
        printf("  ");
        for(int i = 0; i < cols; i++)
            printf(ANSI_COLOR_RED"_"ANSI_COLOR_RESET" ");
            //printf("_ ");
        printf("\n");
        for(int i = 0; i < rows; i++){
            read(server_sd, row, cols);
            row[cols]='\0';
            printRow(row);
        }

        printf("  ");
        for(int i = 0; i < cols; i++)
            printf(ANSI_COLOR_RED"─"ANSI_COLOR_RESET" ");
            //printf("─ ");

        printf("\n\n");
    }
    else{
        gameFinished = 1;
    }
    
}

char firstChar(char *buffer){
    return buffer[0];
}

void clearBuffer(){
    char c;
    while ((c = getchar()) != '\n' && c != EOF) { };
}

void game(int server_sd){
    char msg;
    char buffer[200];
    char row[16];
    int rows;
    int cols;
    read(server_sd, &playerLetter, 1);
    while(1){
        system("clear");
        //receiveMessage(server_sd);//Scoreboard
        printMap(server_sd);
        receiveMessage(server_sd);//Info
        printf("Comando: ");
        //scanf(" %c",&msg);
        //fgets(buffer, sizeof(buffer), stdin);  //PEPPE NON ASPETTA INPUTPUNZ
        clearBuffer();
        scanf("%s",buffer);//con questo sembra andare
        msg = firstChar(buffer);
        write(server_sd, &msg, 1);
        if(gameFinished){
            read(server_sd, &playerLetter, 1);
            gameFinished = 0;
        }
            
        
       
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