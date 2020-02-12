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
#define ROWS 8
#define COLS 8

int sockfd;
char game[ROWS][COLS]; //matrice che possiede il puntatore alla matrice di gioco creata nel server
char player[MAX];

struct package{ //struct per la stampa delle informazioni riguardante i pacchi

	int riga;
	int colonna;
	struct package *next; //lista di pacchi

};

char *ip(char **argv){

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
	serverAddr=ip(argv);

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
	printf("\n\n");

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
			if(game[i][j]=='+'){ 
				printf("Trovato pacco\n");
				top=MakePackage(top,i,j);
			}
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

void Registration(){

	char *buf, buffer[MAX];
	int nbytes=0;

	printf(">: [!]Insert username:\n>: ");
	fgets(buffer,MAX,stdin);
	write(sockfd,buffer,strlen(buffer)-1);
	printf(">: [!]Insert password:\n");
	buf=getpass(">: ");
	nbytes=sprintf(buffer,"%s",buf);
	write(sockfd,buffer,strlen(buffer));
	nbytes=read(sockfd,buffer,MAX);
	buffer[nbytes]='\0';
	printf(">: %s",buffer);
}

void Login(){

	char *buf, buffer[MAX];
	int nbytes=0;

	printf(">: [!]Insert username:\n>: ");
	fgets(buffer,MAX,stdin);
	write(sockfd,buffer,strlen(buffer)-1);
	printf(">: [!]Insert password:\n");
	buf=getpass(">: ");
	nbytes=sprintf(buffer,"%s",buf);
	write(sockfd,buffer,strlen(buffer));
	nbytes=read(sockfd,buffer,MAX);
	buffer[nbytes]='\0';
	printf(">: %s",buffer);
}

void printMenu(){

	printf("\n\n\t\t\t* * *C  L  I  E  N  T* * *\n\n");
	printf("TYPE:\n [1]Number of connected clients.\n [2]Number of disconnected clients.\n [3]Registration.\n");
	printf(" [4]Connected users.\n [5]Login.\n [6]Start Game.\n *To exit write [exit]*\n");

}

void MatrixMenu(){

	//system("clear");
	printf("\n\t\t\t* * *G A M E* * *\n\n");
	printf(">: %s",player);
	PrintMatrix();
}

/* Interface for the user with functions for the comunication with server */
void ClientMenu(){

	char buffer[MAX], message[MAX];
	int nbytes=0, flag=0;
	int i,j;
	system("clear");
	printMenu();

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
				for(i=0;i<ROWS;i++){
					for(j=0;j<COLS;j++){
						read(sockfd,&game[i][j],1);
					}
				}
				//nbytes=read(sockfd,game,sizeof(game));//puntatore alla matrice di gioco inizializzata nel server
				nbytes=read(sockfd,message,MAX);
				message[nbytes]='\0';
				strcpy(player,message); 
				MatrixMenu();
				
				while(1){ //menu di gioco
				
					printf("\n[S]left\n[N]down\n[O]right\n[E]up\n*To exit write [exit]*\n");
					printf(">: ");
					fgets(buffer,MAX,stdin);
					write(sockfd,buffer,strlen(buffer));

					if(strcmp(buffer, "exit\n")==0){
						printf(">: [+]Disconnected from game.\n");
						break;
					}
					else if((strcmp(buffer,"\n")==0)||(strcmp(buffer,"s\n")!=0&&strcmp(buffer,"o\n")!=0&&strcmp(buffer,"e\n")!=0 &&strcmp(buffer,"n\n")!=0)) printf(">: [!]Command not valid, retry.\n");

					else{
						nbytes=read(sockfd,buffer,MAX);
						buffer[nbytes]='\0';
						nbytes=read(sockfd,game,sizeof(game));

						MatrixMenu();
						if(strcmp(buffer,"-")==0)printf("ostacolo\n");
						else if(strcmp(buffer,"u")==0)printf("scontro\n");
						else if(strcmp(buffer,"np")==0)printf("hai gia un pacchi\n");
						else if(strcmp(buffer,"p")==0)printf("consegnato\n");
						else if(strcmp(buffer,"pl")==0)printf("scaduto\n");
						else if(strcmp(buffer,"npd")==0)printf("non hai pacchi\n");
						else if(strcmp(buffer,"move")==0)printf("riprova\n");
						else if(strcmp(buffer,"s")!=0)printf("%s",buffer);

						if(read(sockfd,buffer,MAX)&&strcmp(buffer,"noendgame")!=0)break;
					}
				}

				sleep(1);
				system("clear");
				if(strcmp(buffer,"exit\n")!=0)printf("\t\t* * *%s* * *",buffer); //stampo il vincitore solo se non sono uscito dal gioco
				printMenu();
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

	if(CheckNumberArguments(argc)){
		SocketTCP(argv);
		ClientMenu();
	}
	else printf("[!]Please enter the correct number of arguments.\n");

	return 0;

}
