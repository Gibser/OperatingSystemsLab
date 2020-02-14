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


struct config{
    unsigned int port;
    char ip[28];
    char hostname[100];
};

void chooseServer(struct sockaddr_in *serverConfig);
void game(int server_sd);
void receiveMessage(int server_sd);
int isExit(char buffer[]);
void loginClient(int server_sd);
char loginCred(int server_sd);
void regCred(int server_sd);

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
            printf("Errore: %s\nHai inserito bene i parametri?\n",strerror(errno));
            exit(0); 
        } 
        else
            printf("Connesso al server!\n");
            loginClient(sockfd);
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

//Gestione del login nel client
char loginCred(int server_sd){
    char username[200];
    char password[200];
    char flag;
    int n;

    printf("Nome utente: ");
    getchar(); //scarico il buffer
    scanf("%[^\n]", username); //posso inserire anche spazi
    n = strlen(username);
    //write(server_sd, &n, sizeof(int));
    write(server_sd, username, sizeof(username));

    read(server_sd, &flag, 1);
    if(flag == '2'){
        printf("Il nome utente non può contenere spazi!\n");
        return '0';
    }

    printf("Password: ");
    getchar(); //scarico il buffer
    scanf("%[^\n]", password); //posso inserire anche spazi
    n = strlen(password);
    //write(server_sd, &n, sizeof(int));
    write(server_sd, password, sizeof(password));

    read(server_sd, &flag, 1);
    if(flag == '2'){
        printf("La password non può contenere spazi!\n");
        return '0';
    }

    //Credenziali inserite, verifica utente nel file

    read(server_sd, &flag, 1);

    switch(flag){
        case('1'):
            printf("Login effettuato!\n");
            return '1';
        case('2'):
            printf("Password non valida.\n");
            return '0';
        case('3'):
            printf("Username non esistente.\n");
            return '0';
        default:
            printf("Qualcosa non ha funzionato...\n");
            return '0';
    }
}

//Gestione della registrazione nel client
void regCred(int server_sd){
    char username[200];
    char password[200];
    char flag;
    int n;

    printf("Nome utente: ");
    getchar(); //scarico il buffer
    scanf("%[^\n]", username); //posso inserire anche spazi
    n = strlen(username);
    n = htonl(n);
    //printf("username: %s\n", username);
    //write(server_sd, &n, sizeof(n));
    write(server_sd, username, sizeof(username));

    read(server_sd, &flag, 1);
    if(flag == '2'){
        printf("Il nome utente non può contenere spazi!\n");
        return;
    }

    printf("Password: ");
    getchar(); //scarico il buffer
    scanf("%[^\n]", password); //posso inserire anche spazi
    getchar();
    n = strlen(password);
    //printf("password: %s\n", password);
    //write(server_sd, &n, sizeof(int));
    write(server_sd, password, sizeof(password));

    read(server_sd, &flag, 1);
    if(flag == '2'){
        printf("La password non può contenere spazi!\n");
        return;
    }
    //printf("%c\n", flag);
    //Credenziali inserite, verifica utente nel file
    read(server_sd, &flag, 1);
    printf("%c\n", flag);
    if(flag == '3')
        printf("Username già esistente!\n");
    else if(flag == '1')
        printf("Registrazione effettuata!\n");
}


void loginClient(int server_sd){
    char buffer[5000];
    char scelta;
    char cred[200];
    memset(buffer, '\0', sizeof(buffer));
    char log = '0';
    int dim;
    while(log != '1'){
        printf("Benvenuto!\n\n1 - Login\n2 - Registrazione\n3 - Guida\n4 - Esci\n\nScelta: ");
        scanf(" %c", &scelta);
        dim = htonl(1);
        write(server_sd, &dim, sizeof(dim));
        write(server_sd, &scelta, 1);
        
        if(scelta != '1' && scelta != '2' && scelta != '3' && scelta != '4')
            printf("Scelta non valida.\n\n");
        
        switch(scelta){
            case '1':
                log = loginCred(server_sd);
                break;
            case '2':
                regCred(server_sd);
                break;
            case '3':
                printf("Inserire guida...\n");
                break;
            case '4':
                printf("Uscita...\n");
                return;
        }
        
    }

    printf("Gioco...\n");
}