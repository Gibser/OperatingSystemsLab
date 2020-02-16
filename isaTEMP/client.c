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
#include <signal.h>

#define MAX 300
#define ROWS 10
#define COLS 20

int sockfd=0;
char player[MAX];
char game[ROWS][COLS];


//STRUTTURE


struct package{ //struct per la stampa delle informazioni riguardante i pacchi

	int riga;
	int colonna;
	struct package *next; //lista di pacchi

};


//FUNZIONI


/* When an user executes Ctrl-C */
void ServerAbort(int signNum){

	printf("\n[+]Server aborted.\n");
	close(sockfd);
	exit(1);

}

char *IP(char **argv){

	char *ip;
	struct hostent *hp;

	hp=gethostbyname(argv[1]);
	if(!hp) exit(1);

	return inet_ntoa(*(struct in_addr *)hp->h_addr_list[0]);
}

/* Create the connection with server */
void SocketTCP(char **argv){

	int ret=0, nbytes=0;
	struct sockaddr_in Caddr;
	char buffer[MAX], *serverAddr;

	uint16_t PORT= strtoul(argv[2],NULL,10);
	serverAddr=IP(argv);

	Caddr.sin_family=AF_INET;
	Caddr.sin_port=htons(PORT);
	inet_aton(serverAddr,&Caddr.sin_addr);

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		printf("[-]Error socket.\n");
		exit(1);
	}
	else printf("[+]ClientSocket created.\n");

	if((ret=connect(sockfd,(struct sockaddr*)&Caddr,sizeof(Caddr)))<0){
		printf("[-]Error connection.\n");
		exit(1);
	}
	else printf("[+]Connected to Server.\n");
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
void PrintMatrix(){

	int index1=0, index2=0, count=0;
	struct package *top=NULL;

	for(int i=0;i<ROWS;i++){
		for(int j=0;j<COLS;j++){
			if(game[i][j]=='+') top=MakePackage(top,i,j);
		}
	}
	for(int i=0;i<ROWS;i++){
		for(int j=0;j<COLS;j++){
			if(game[i][j]=='o' || game[i][j]==' ') printf("[ ]");
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

/* Insert credentials for registration */
void Registration(){

	char *buf, buffer[MAX], buffer2[MAX], abort[MAX];
	int nbytes=0;

	do{
		bzero(buffer,MAX);
		bzero(buffer2,MAX);

		printf(">: [!]Insert username:\n>: ");
		fgets(buffer,MAX,stdin);
		printf(">: [!]Insert password:\n>: ");
		fgets(buffer2,MAX,stdin);

		if(strcmp(buffer," \n")==0 || strcmp(buffer,"\n")==0 || strcmp(buffer2," ")==0 || strcmp(buffer2,"\n")==0){
				printf(">: [+]Credentials not valid, retry.\n");
		}

	} while(strcmp(buffer," \n")==0 || strcmp(buffer,"\n")==0 || strcmp(buffer2," ")==0 || strcmp(buffer2,"\n")==0);

	write(sockfd,buffer,strlen(buffer)-1);
	write(sockfd,buffer2,strlen(buffer2)-1);

	nbytes=read(sockfd,buffer,MAX);
	if(nbytes==0){
		printf("\n[+]Server aborted.\n");
		exit(1);
	}
	buffer[nbytes]='\0';
	printf(">: %s",buffer);

}

/* Insert credentials for login */
void Login(){

	char *buf, buffer[MAX], buffer2[MAX];
	int nbytes=0;

	printf(">: [!]Insert username:\n>: ");
	fgets(buffer,MAX,stdin);
	printf(">: [!]Insert password:\n>: ");
	fgets(buffer2,MAX,stdin);

	write(sockfd,buffer,strlen(buffer)-1);
	sleep(1);
	write(sockfd,buffer2,strlen(buffer2)-1);

	nbytes=read(sockfd,buffer,MAX);
	if(nbytes==0){
		printf("\n[+]Server aborted.\n");
		exit(1);
	}
	buffer[nbytes]='\0';
	printf(">: %s",buffer);
}

void PrintMenu(){

	printf("\n\n\t\t\t* * *C  L  I  E  N  T* * *\n\n");
	printf("TYPE:\n [1]Number of connected clients.\n [2]Number of disconnected clients.\n [3]Registration.\n");
	printf(" [4]Connected users.\n [5]Login.\n [6]Start Game.\n*To exit write [exit]*\n");
}

void MatrixMenu(){

	system("clear");
	printf("\n\t\t\t* * *G A M E* * *\n\n");
	printf(">: %s",player);
	PrintMatrix();
}

void Play(){

	char buffer[MAX], message[MAX], quit[MAX], mex[MAX];
	int nbytes=0;

	nbytes=read(sockfd,game,sizeof(game));
	nbytes=read(sockfd,message,MAX);
	message[nbytes]='\0';
	strcpy(player,message);
	MatrixMenu();

	while(1){ //menu di gioco

		printf("\n[S]left\n[N]down\n[O]right\n[E]up\n*To exit write [exit]*\n");
		printf(">: ");
		fgets(buffer,MAX,stdin);
		write(sockfd,buffer,strlen(buffer));

		if(strcmp(buffer,"exit\n")==0){
			printf(">: [+]Disconnected from game.\n");
			break;
		}
		else if((strcmp(buffer,"\n")==0)||(strcmp(buffer,"s\n")!=0&&strcmp(buffer,"o\n")!=0
				&&strcmp(buffer,"e\n")!=0&&strcmp(buffer,"n\n")!=0&&strcmp(buffer,"S\n")!=0
					&&strcmp(buffer,"O\n")!=0&&strcmp(buffer,"E\n")!=0&&strcmp(buffer,"N\n")!=0))
			printf(">: [!]Command not valid, retry.\n");

		else{
			nbytes=read(sockfd,mex,MAX);
			if(nbytes==0){
				printf("[+]Server aborted.\n");
				exit(1);
			}
			mex[nbytes]='\0';
			read(sockfd,game,sizeof(game));

			MatrixMenu();
			if(strcmp(mex,"ostacolo")==0)printf("[+]Took an obstacle.\n");
			else if(strcmp(mex,"scontro")==0)printf("[+]Run into another user.\n");
			else if(strcmp(mex,"pacchi")==0)printf("[+]You have a package yet.\n");
			else if(strcmp(mex,"consegnato")==0)printf("[+]Package delivered.\n");
			else if(strcmp(mex,"perso")==0)printf("[+]Time out, package lost.\n");
			else if(strcmp(mex,"nopacchi")==0)printf("[+]You haven't packages to deliver.\n");
			else if(strcmp(mex,"nok")==0)printf("[+]Move not valide, retry.\n");
			else if(strcmp(mex,"ok")!=0)printf("%s",mex);

			nbytes=read(sockfd,quit,MAX);
			quit[nbytes]='\0';
			if(strcmp(quit,"noendgame")!=0)break;
		}
	}
	sleep(1);
	system("clear");
	if(strcmp(buffer,"exit\n")!=0)printf("\t\t* * *%s* * *",quit); //stampo il vincitore solo se non sono uscito dal gioco
}

/* Interface for the user with functions for the comunication with server */
void ClientMenu(){

	char buffer[MAX];
	int nbytes=0, flag=0;

	system("clear");
	PrintMenu();

	while(1){

		printf("\n>: ");
		fgets(buffer,MAX,stdin);
		write(sockfd,buffer,strlen(buffer));

		if(strcmp(buffer, "exit\n")==0){
			printf(">: [+]Client disconnected.\n");
			break;
		}

		if(strcmp(buffer,"3\n")==0){ //registrazione
			if(read(sockfd,buffer,MAX)&&strcmp(buffer,"OK")==0) Registration();
			else printf(">: %s",buffer);
		}

		else if(strcmp(buffer,"5\n")==0) Login();

		else if(strcmp(buffer,"6\n")==0){ //se ha inizio la sessione di gioco

			if(read(sockfd,buffer,MAX)&&strcmp(buffer,"OK")==0){
				Play();
				PrintMenu();
			}
			else if((strcmp(buffer,"NOK")==0))printf(">: [+]You have to login to game, retry.\n");
		}
		else{
			nbytes=read(sockfd,buffer,MAX);
			if(nbytes==0){ //se il server non invia dati, ha abortito
				printf("\n[+]Server aborted.\n");
				break;
			}
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

/* When an user executes Ctrl-C */
void ClientAbort(){

	printf("\n[+]Client aborted.\n");
	write(sockfd,"abort",8);

	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);

	close(sockfd);
	exit(1);
}

/* Main process */
int main(int argc, char **argv){

	signal(SIGINT,ClientAbort); //se un client termina in modo anomalo
	signal(SIGPIPE,ServerAbort); // se il server abortisce

	if(CheckNumberArguments(argc)){
		SocketTCP(argv);
		ClientMenu();
	}
	else printf("[!]Please enter the correct number of arguments.\n");

	return 0;
}