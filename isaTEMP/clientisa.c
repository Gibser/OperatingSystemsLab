#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 2048
#define ROWS 10
#define COLS 20

char game[ROWS][COLS]; //matrice che possiede il puntatore alla matrice di gioco creata nel server

struct package{ //struct per la stampa delle informazioni riguardante i pacchi

	int riga;
	int colonna;
	struct package *next; //lista di pacchi

};

/* Create the connection with server */
int SocketTCP(char *IP, int PORT){

	int sockfd=0, ret=0, nbytes=0;
	struct sockaddr_in Saddr, Caddr;
	char buffer[MAX];

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		printf("[-]Error socket.\n");
		exit(1);
	}
	else printf("[+]ClientSocket created.\n");

	memset(&Saddr,'\0',sizeof(Saddr));
	Saddr.sin_family=AF_INET;
	Saddr.sin_port=htons(PORT);
	Saddr.sin_addr.s_addr=inet_addr(IP);

	if((ret=connect(sockfd,(struct sockaddr*)&Saddr,sizeof(Saddr)))<0){
		printf("[-]Error connection.\n");
		exit(1);
	}
	else printf("[+]Connected to Server.\n");
	printf("\n\n");

	return sockfd;

}

/* Create the list of packages */
struct package *MakePackage(struct package *top, int riga, int colonna){

	if(top==NULL){
		top=(struct package *)malloc(sizeof(struct package));
		top->riga=riga;
		top->colonna=colonna;
		top->next=NULL;
	}
	else top->next=MakePackage(top->next,riga,colonna);

	return top;

}

/* Print the matrix game and information about packages left */
void PrintMatrix(int sockfd){

	int index1=0, index2=0, count=0;
	struct package *top=NULL;

	for(int i=0;i<ROWS;i++){
		for(int j=0;j<COLS;j++){
			if(game[i][j]=='+') top=MakePackage(top,i,j);
		}
	}
	for(int i=0;i<ROWS;i++){
		for(int j=0;j<COLS;j++){

			if( game[i][j]=='o' ) printf("[ ]");
			else printf("[%c]",game[i][j]);

			if(j==COLS-1&&i==0) printf("   *Information about packages left*");
			if(j==COLS-1&&i>0&&i<ROWS-1){
				if(top){
					printf("    Riga[%d] Colonna[%d]", top->riga, top->colonna);
					top=top->next;
				}
			}
			else if(j==COLS-1&&i==ROWS-1){
				while(top){
					if(count==0){
						printf("    Riga[%d] Colonna[%d]\n", top->riga, top->colonna);
						count++;
					}
					else{
						for(int k=0;k<COLS;k++){
							printf("   ");
							if(k==COLS-1)printf("    Riga[%d] Colonna[%d]\n", top->riga, top->colonna);
						}
					}
					top=top->next;
				}
			}
		}
		printf("\n");
	}

}

/* Interface for the user with functions for the comunication with server */
void ClientMenu(int sockfd){

	char buffer[MAX], buffer2[MAX], message[MAX], *buf;
	int nbytes=0, flag=0;

	system("clear");
	printf("\t\t\t* * *C  L  I  E  N  T* * *\n\n");
	printf("TYPE:\n [1]Number of connected clients.\n [2]Number of disconnected clients.\n [3]Registration.\n");
	printf(" [4]Connected users.\n [5]Login.\n [6]Start Game.\n *To exit write [exit]*\n");

	//pulisco i buffer
	bzero(buffer,MAX);
	bzero(buffer2,MAX);
	bzero(message,MAX);

	while(1){

		printf("\n>: ");
		fgets(buffer,MAX,stdin);
		write(sockfd,buffer,strlen(buffer));

		if(strcmp(buffer, "exit\n")==0){

			printf(">: [+]Client disconnected.\n");
			break;
		}
		
		if(strcmp(buffer,"3\n")==0){ //registrazione
			nbytes=read(sockfd,buffer,MAX);
			buffer[nbytes]='\0';
			if(strcmp(buffer,"[+]You cannot register, you already logged in.\n")!=0){
				printf(">: %s",buffer);
				printf(">: ");
				fgets(buffer,MAX,stdin);
				write(sockfd,buffer,strlen(buffer)-1);
				nbytes=read(sockfd,buffer,MAX);
				buffer[nbytes]='\0';
				printf(">: %s",buffer);
				buf=getpass(">: ");
				nbytes=sprintf(buffer,"%s",buf);
				write(sockfd,buffer,strlen(buffer));
				nbytes=read(sockfd,buffer,MAX);
				buffer[nbytes]='\0';
				printf(">: %s",buffer);
			}
			else printf(">: %s",buffer);
		}	

		else if(strcmp(buffer,"5\n")==0){ //login
			nbytes=read(sockfd,buffer,MAX);
			buffer[nbytes]='\0';
			printf(">: %s",buffer);
			printf(">: ");
			fgets(buffer,MAX,stdin);
			write(sockfd,buffer,strlen(buffer)-1);
			nbytes=read(sockfd,buffer,MAX);
			buffer[nbytes]='\0';
			printf(">: %s",buffer);
			buf=getpass(">: ");
			nbytes=sprintf(buffer,"%s",buf);
			write(sockfd,buffer,strlen(buffer));
			nbytes=read(sockfd,buffer,MAX);
			buffer[nbytes]='\0';
			printf(">: %s",buffer);
		}

		else if(strcmp(buffer,"6\n")==0){ //se ha inizio la sessione di gioco

			if((read(sockfd,buffer,MAX))&& (strcmp(buffer,"OK")==0) ){

				nbytes=read(sockfd,game,sizeof(game)); //puntatore alla matrice di gioco inizializzata nel server
				nbytes=read(sockfd,buffer2,MAX);
				system("clear");
				printf("\n\t\t\t* * *G A M E* * *\n\n");
				strcpy(message,buffer2);
				printf(">: %s\n", message);
				PrintMatrix(sockfd);

				while(1){ //menu di gioco

					printf("\n[S]left\n[N]down\n[O]right\n[E]up\n*To exit write [exit]*\n");
					printf(">: ");
					fgets(buffer,MAX,stdin);
					write(sockfd,buffer,strlen(buffer));

					if(strcmp(buffer, "exit\n")==0){

						printf(">: [+]Disconnected from game.\n");
						break;
					}
					else if(strcmp(buffer,"\n")==0) printf(">: [!]Command not valid, retry.\n");

					else{

						nbytes=read(sockfd,buffer,MAX);
						buffer[nbytes]='\0';

						if(strcmp(buffer, "one")==0){ //caso 1: aggiorno solo la matrice

							nbytes=read(sockfd,buffer,MAX);
							nbytes=read(sockfd,game,sizeof(game));
							system("clear");
							printf("\n\t\t\t* * *G A M E* * *\n\n");
							printf(">: %s\n", message);
							PrintMatrix(sockfd);
							if(strcmp(buffer,"any_message")!=0)printf(">: %s",buffer);
						}

						else if(strcmp(buffer, "two")==0){ //caso 2: non aggiorno la matrice e stampo un messaggio

							nbytes=read(sockfd,buffer,MAX);
							system("clear");
							printf("\n\t\t\t* * *G A M E* * *\n\n");
							printf(">: %s\n", message);
							PrintMatrix(sockfd);
							printf(">: %s",buffer);
						}

						else printf(">: %s",buffer);

						if((read(sockfd,buffer,MAX))&&(strcmp(buffer,"noendgame")!=0)) break; //la sessione è terminata
					}
				}

				sleep(1);
				system("clear");
				if(strcmp(buffer,"exit\n")!=0)printf("\t\t* * *%s* * *",buffer); //stampo il vincitore solo se non sono uscito dal gioco

				printf("\n\n\t\t\t* * *C  L  I  E  N  T* * *\n\n");
				printf("TYPE:\n [1]Number of connected clients.\n [2]Number of disconnected clients.\n [3]Registration.\n");
				printf(" [4]Connected users.\n [5]Login.\n [6]Start Game.\n *To exit write [exit]*\n");
			}
			else if((strcmp(buffer,"NOK")==0))printf(">: [+]You have to login to game, retry.\n");
		}

		else{

			nbytes=read(sockfd,buffer,MAX);
			buffer[nbytes]='\0';
			printf(">: %s", buffer);

		}
	}

	close(sockfd);

}

/* Check that number of argument inserted by user on command line is correct */
int CheckNumberArguments(int argc){

	if(argc==3) return 1;
	else return 0;

}

int main(int argc, char **argv){

	int sockfd=0;

	if(CheckNumberArguments(argc)){
		sockfd=SocketTCP(argv[1],atoi(argv[2]));
		ClientMenu(sockfd);
	}
	else printf("[!]Please enter the correct number of arguments.\n");

	return 0;

}