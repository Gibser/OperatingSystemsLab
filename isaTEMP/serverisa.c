#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX 2048
#define MAX_USERS 50
#define ROWS 8
#define COLS 8

int connectedClients=0;
int disconnectedClients=0;
int pack=0; //pacchi totali nella matrice: il gioco termina quando si azzerano i pacchi
int flagGame=1; //flag per dare il via ad una nuova sessione di gioco una volta terminata quella corrente
char game[ROWS][COLS]; //matrice di gioco

pthread_mutex_t ClientMutex = PTHREAD_MUTEX_INITIALIZER; //per la sincronizzazione tra i thread client
pthread_mutex_t FileUserMutex = PTHREAD_MUTEX_INITIALIZER; //per il file FileUSER.txt
pthread_mutex_t FileLogMutex = PTHREAD_MUTEX_INITIALIZER; //per il file FileLOG.txt
pthread_mutex_t MatrixMutex = PTHREAD_MUTEX_INITIALIZER; //per la matrice di gioco


struct pack{ //struttura dati associata ad ogni pacco

	int check; //assume valore 0 o 1 se il pacco è stato prelevato
	double time; //tempo di scadenza
	time_t timeDeliver; //tempo consegna
	time_t timePick; //tempo di prelevamento

};

struct pack packTime[ROWS][COLS]; //matrice di struct pack

struct user { //struttura dati associata ad ogni utente

	int sockfd; // valore della connessione
	int logged; // assume valore 0 o 1 se l'utente è loggato o no
	char username[20]; //username login
	char password[20]; //password login
	int x; //riga della matrice dove si trova l'utente
	int y; //colonna della matrice dove si trova l'utente
	int xp; //riga del pacco prelevato dall'utente nella matrice
	int yp; //colonna del pacco prelevato dall'utente nella matrice
	int packages; //assume valore 0 o 1 se il giocatore ha prelevato un pacco(1 pacco alla volta)
	int totalPack; //numero totale di pacchi prelevati in una sessione
	int inGame; //assume valore 0 o 1 se l'utente è in sessione di gioco

};

typedef struct user t_user;
t_user *OnlineUsers[MAX_USERS]; //array di puntatori a struct user


//FUNZIONI DI BASE


/* Same of LogFileUser but with ip address */
void FileLogIP(struct sockaddr_in Caddr){

	srand(time(NULL));
	int nbytes=0, fd=0;
	char buffer[MAX];
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo=localtime(&rawtime);

	if((fd=open("FileLOG.txt", O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR))<0){
		perror("[-]Error FileLOG.\n");
		exit(1);
	}
	if(!fd)perror("[-]FileLOG not exist.\n");
	else{
		nbytes=sprintf(buffer,"[IP]: %s\t[CURRENT LOCAL TIME & DATE]: %s", inet_ntoa(Caddr.sin_addr), asctime(timeinfo));
		pthread_mutex_lock(&FileLogMutex);
		write(fd,buffer,nbytes);
		pthread_mutex_unlock(&FileLogMutex);
	}

	close(fd);

}

/* Add clients to queue */
void QueueAdd(t_user *user){

	for(int i=0;i<MAX_USERS;i++){
		if(!OnlineUsers[i]){
			pthread_mutex_lock(&ClientMutex);
			OnlineUsers[i]=user;
			OnlineUsers[i]->username[0]='\0';
			OnlineUsers[i]->password[0]='\0';
			OnlineUsers[i]->logged=0;
			OnlineUsers[i]->inGame=0;
			OnlineUsers[i]->packages=0;
			OnlineUsers[i]->totalPack=0;
			pthread_mutex_unlock(&ClientMutex);
			break;
		}
	}

}

/* Remove clients from queue */
void QueueRemove(int connection){

	int index1, index2;

	for(int i=0;i<MAX_USERS;i++){
		if(OnlineUsers[i]){
			if(OnlineUsers[i]->sockfd==connection){
				pthread_mutex_lock(&ClientMutex);
				OnlineUsers[i]=NULL;
				pthread_mutex_unlock(&ClientMutex);
				break;
			}
		}
	}

}

/* Search an user with a predetermined connection */
int SearchUser(int connection){

	int ret=-1;

	for(int i=0;i<MAX_USERS; i++){
		if(OnlineUsers[i]){
			if(OnlineUsers[i]->sockfd==connection){
				ret=i;
				break;
			}
		}
	}
	return ret;

}

/* Create logfile with all activities of users */
void FileLogUser(int connection,time_t pack){

	srand(time(NULL));
	int nbytes=0, fd=0;
	char buffer[MAX];
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo=localtime(&rawtime);

	if((fd=open("FileLOG.txt", O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR))<0){
		perror("[-]Error FileLOG.\n");
		exit(1);
	}

	if(!fd)perror("[-]FileLOG not exist.\n");
	else{
		if(OnlineUsers[SearchUser(connection)]->inGame==0){
			nbytes=sprintf(buffer,"[USER]: %s\t[CURRENT LOCAL TIME & DATE]: %s", OnlineUsers[SearchUser(connection)]->username,asctime(timeinfo));
			pthread_mutex_lock(&FileLogMutex);
			write(fd,buffer,nbytes);
			pthread_mutex_unlock(&FileLogMutex);
		}
		else{
			nbytes=sprintf(buffer,"[USER]: %s\t[PACKAGE DELIVER'S TIME & DATE]: %s",OnlineUsers[SearchUser(connection)]->username,ctime(&pack));
			pthread_mutex_lock(&FileLogMutex);
			write(fd,buffer,nbytes);
			pthread_mutex_unlock(&FileLogMutex);
		}
	}

	close(fd);

}

/* Add clients to list */
void ListAdd(char buffer[], int length, int connection){

	char user[MAX], pwd[MAX];
	int j=0;

	for(int i=0; i<MAX_USERS; i++){
		if(OnlineUsers[i]){
			if(OnlineUsers[i]->sockfd==connection){
				pthread_mutex_lock(&ClientMutex);
				OnlineUsers[i]->logged=1;
				pthread_mutex_unlock(&ClientMutex);
				while(buffer[j]!=' ') j++;
				strncpy(user,&buffer[0],j);
				user[j+1]='\0';
				strncpy(pwd,&buffer[++j],length);
				pthread_mutex_lock(&ClientMutex);
				memcpy(OnlineUsers[i]->username,user,j);
				memcpy(OnlineUsers[i]->password,pwd,length-j);
				pthread_mutex_unlock(&ClientMutex);
				break;
			}
		}
	}

}

/* Verify that the user is present in the fileuser */
int CheckUser(char *buffer){

	int ret=0;
	char user[100], pwd[100];
	FILE *fd;

	if((fd=fopen("FileUSER.txt","r"))<0){
		perror("[-]Error FileUSER.\n");
		exit(1);
	}
	if(!fd) perror("[-]FileUSER not exist.\n");
	else{
		while(!feof(fd)){
			fscanf(fd,"%s %s", user, pwd);
			strcat(user," ");
			strcat(user, pwd);
			strcat(user,"\n");
			if(strcmp(buffer,user)==0){
				ret=1;
				break;
			}
		}
	}

	return ret;

}

/* Find the user with maximum number of packages */
int SearchMax(){

	int max=0, ret=0;

	for(int i=0;i<MAX_USERS;i++){
		if(OnlineUsers[i]&&OnlineUsers[i]->logged==1&&max<OnlineUsers[i]->totalPack){
				max=OnlineUsers[i]->totalPack;
				ret=OnlineUsers[i]->sockfd;
		}
	}
	return ret;

}

/* Verify that the user is logged */
int CheckLogged(int connection){

	int ret=0;

	for(int i=0;i<MAX_USERS;i++){
		if(OnlineUsers[i]){
			if(OnlineUsers[i]->sockfd==connection&&OnlineUsers[i]->logged==1){
				ret=1;
				break;
			}
		}
	}

	return ret;

}

/* Register the user */
void Registration(int connection){

	int nbytes=0, nbytes2=0, fd=0, ret=0;
	char buffer[MAX], buffer2[MAX];

	if((fd=open("FileUSER.txt", O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR))<0){
		perror("[-]Error FileUSER.\n");
		exit(1);
	}
	else if(CheckLogged(connection)==0){

		nbytes=sprintf(buffer,"OK");
		write(connection,buffer,nbytes);

		nbytes=read(connection,buffer,MAX);
		buffer[nbytes]='\0';
		nbytes2=read(connection,buffer2,MAX);
		buffer2[nbytes2]='\0';
		strcat(buffer," ");
		strcat(buffer,buffer2);
		strcat(buffer,"\n");

		if(CheckUser(buffer)==0){
			pthread_mutex_lock(&FileUserMutex);
			write(fd,buffer,strlen(buffer));
			pthread_mutex_unlock(&FileUserMutex);
			nbytes=sprintf(buffer,"[+]Registered successfully.\n");
			write(connection,buffer,nbytes);
			close(fd);
		}
		else{
			nbytes=sprintf(buffer,"[+]User with these credentials already exists, please retry.\n");
			write(connection,buffer,nbytes);
			close(fd);
		}
	}
	else{
		nbytes=sprintf(buffer,"[+]You cannot register, you already logged in.\n");
		write(connection,buffer,nbytes);
	}

}

/* Verify that the credentials for login have not already been used by another user */
int OneUser(char buffer[MAX], int connection, int length){

	int j=0, ret=0;
	char user[MAX], pwd[MAX];

	while(buffer[j]!=' ') j++;
	strncpy(user,&buffer[0],j);
	user[j+1]='\0';
	strncpy(pwd,&buffer[++j],length);
	for(int i=0;i<MAX_USERS;i++){
		if(OnlineUsers[i]){
			if((strcmp(OnlineUsers[i]->username,user)==0)&&(strcmp(OnlineUsers[i]->password,pwd)==0))ret=1;
		}
	}
	return ret;

}

/* Login the user */
void Login(int connection){

	char buffer[MAX], buffer2[MAX];
	int nbytes=0, nbytes2=0;
	time_t TimeNULL;

	nbytes=read(connection,buffer,MAX);
	buffer[nbytes]='\0';
	nbytes2=read(connection,buffer2,MAX);
	buffer2[nbytes2]='\0';
	strcat(buffer," ");
	strcat(buffer,buffer2);
	strcat(buffer,"\n");

	if(OneUser(buffer,connection,strlen(buffer))==0){
		if((CheckUser(buffer)==1)&&(CheckLogged(connection)==0)){
			nbytes2=sprintf(buffer2,"[+]Login done successfully!\n");
			write(connection,buffer2,nbytes2);
			ListAdd(buffer,strlen(buffer),connection); //Add user to the list
			FileLogUser(connection,TimeNULL);
		}
		else if(CheckUser(buffer)==0){
			nbytes=sprintf(buffer,"[+]You're not registered to login or bad credentials.\nPlease retry.\n");
			write(connection,buffer,nbytes);
		}
		else if(CheckLogged(connection)==1){
			nbytes=sprintf(buffer,"[+]Login already done.\n");
			write(connection,buffer,nbytes);
		}
	}
	else{
		nbytes=sprintf(buffer,"[+]Login already done by another user with these credentials.\n");
		write(connection,buffer,nbytes);
	}

}

/* List of logged in users */
void ConnectedUsers(int connection){

	char message[MAX], buffer[MAX];
	int nbytes=0;

	*message='\0';
	for(int i=0;i<MAX_USERS;i++){
		if(OnlineUsers[i]){
			memcpy(buffer,OnlineUsers[i]->username, sizeof(OnlineUsers[i]->username));
			strcat(message, buffer);
			strcat(message, "-");
		}
	}
	strcat(message,"\n");
	nbytes=strlen(message);
	write(connection,message,nbytes);

}

void ConnectedClient(int connection){

	char buffer[MAX];
	int nbytes=0;

	nbytes=sprintf(buffer,"[+]Number of connected clients: %d\n", connectedClients);
	write(connection,buffer,nbytes);

}

void DisconnectedClient(int connection){

	char buffer[MAX];
	int nbytes=0;

	nbytes=sprintf(buffer,"[+]Number of disconnected clients: %d\n", disconnectedClients);
	write(connection,buffer,nbytes);

}


//FUNZIONI PER LA MATRICE


/* Insert user into the matrix game */
void InsertUser(int connection){

	int index1=0, index2=0, count=0;

	for(int i=0;i<ROWS;i++){
		for(int j=0;j<COLS;j++){
			index1=rand()%COLS;
			index2=rand()%COLS;
			if(count==0&&game[index1][index2]==' '){
				count++;
				pthread_mutex_lock(&MatrixMutex);
				game[index1][index2]='A'+connection;
				pthread_mutex_unlock(&MatrixMutex);
				pthread_mutex_lock(&ClientMutex);
				OnlineUsers[SearchUser(connection)]->x=index1;
				OnlineUsers[SearchUser(connection)]->y=index2;
				OnlineUsers[SearchUser(connection)]->inGame=1;
				OnlineUsers[SearchUser(connection)]->packages=0;
				OnlineUsers[SearchUser(connection)]->totalPack=0;
				pthread_mutex_unlock(&ClientMutex);
			}
		}
	}

}

/* Handle that checks the validity of the time of each package(it constantly works) */
void *HandleTime(){

	time_t now;

	while(1){ //controlla costantemente
		for(int i=0;i<ROWS;i++){
			for(int j=0;j<COLS;j++){
				if(packTime[i][j].check==1){
					srand(time(NULL));
					time(&now);
					if(difftime(now,packTime[i][j].timePick)>packTime[i][j].time) packTime[i][j].time=0;
				}
			}
		}
	}

	pthread_detach(pthread_self());
	pthread_exit(0);

}

/* Create the matrix game */
void MakeMatrix(){

	int index1=0, index2=0;
	pthread_t tid;
	srand(time(NULL));

	for(int i=0;i<ROWS;i++){
		for(int j=0;j<COLS;j++){
			game[i][j]=' '; //inizializzo la matrice
		}
	}

	for(int i=0;i<COLS;i++){
		index1=rand()%COLS;
		index2=rand()%COLS;
		if(game[index1][index2]==' '){
			pthread_mutex_lock(&MatrixMutex);
			game[index1][index2]='o'; //pongo un ostacolo in posizione random
			pthread_mutex_unlock(&MatrixMutex);
		}
		index1=rand()%COLS;
		index2=rand()%COLS;
		if(game[index1][index2]==' '){
			pthread_mutex_lock(&MatrixMutex);
			game[index1][index2]='+'; //pongo un pacco in posizione random
			pthread_mutex_unlock(&MatrixMutex);
			packTime[index1][index2].time=(rand()%(12-7))+7; //minimo di 7 secondi massimo 11
			packTime[index1][index2].check=0;
			pack++; //incremento numero totale di pacchi
		}
		index1=rand()%COLS;
		index2=rand()%COLS;
		if(game[index1][index2]==' '){
			pthread_mutex_lock(&MatrixMutex);
			game[index1][index2]='x'; //pongo una destinazione in posizione random
			pthread_mutex_unlock(&MatrixMutex);
		}
	}

	pthread_create(&tid,NULL,&HandleTime,NULL); //creo un thread che analizza costantemente la validità dei pacchi prelevati dagli utenti

}

/* Verify the validity of the time of every packages picked up and puts check as zero */
int FindPack(int riga, int colonna){

	int ret=0;

	for(int i=0;i<ROWS;i++){
		for(int j=0;j<COLS;j++){
			if(i==riga&&j==colonna&&packTime[i][j].check==1&&packTime[i][j].time!=0){
				packTime[i][j].check=0;
				return 1;
			}
			else if(i==riga&&j==colonna&&packTime[i][j].check==1&&packTime[i][j].time==0){
				packTime[i][j].check=0;
				return 0;
			}
		}
	}
	return ret;

}

/* Find the pack's coordinates picked up by user into the matrix game */
int SearchPack(int connection, int flag){

	int ret=0;

	for(int i=0;i<MAX_USERS;i++){
		if(OnlineUsers[i]&&OnlineUsers[i]->inGame==1&&OnlineUsers[i]->sockfd==connection&&flag==1)ret=OnlineUsers[i]->xp;
		else if(OnlineUsers[i]&&OnlineUsers[i]->inGame==1&&OnlineUsers[i]->sockfd==connection&&flag==2)ret=OnlineUsers[i]->yp;
	}
	return ret;

}


//MOVIMENTI NELLA MATRICE


void PressN(int connection){

	int nbytes=0, index1=0, index2=0, indexUser=SearchUser(connection);
	char buffer[MAX];

	index1=OnlineUsers[indexUser]->x;
	index2=OnlineUsers[indexUser]->y;

	if((index1+1)<ROWS){ // non esce dalla matrice
		if (game[index1+1][index2]==' '){ // non c'è un ostacolo
			pthread_mutex_lock(&MatrixMutex);
			game[index1][index2]=' ';
			game[index1+1][index2]='A'+connection;
			pthread_mutex_unlock(&MatrixMutex);
			OnlineUsers[indexUser]->x=index1+1;
			OnlineUsers[indexUser]->y=index2;
			nbytes=sprintf(buffer,"s");
			write(connection,buffer,MAX);
		}
		else if(game[index1+1][index2]=='-' || game[index1+1][index2]=='o'){ // ostacolo
			if(game[index1+1][index2]=='o'){
				pthread_mutex_lock(&MatrixMutex);
				game[index1+1][index2]='-';
				pthread_mutex_unlock(&MatrixMutex);
			}
			nbytes=sprintf(buffer,"-");
			write(connection,buffer,MAX);
		}
		else if(game[index1+1][index2]!='x'&&game[index1+1][index2]!='+'&&game[index1+1][index2]!='o'&&game[index1+1][index2]!='-'&& game[index1+1][index2]!=' '){ // scontro con un altro giocatore
			nbytes=sprintf(buffer,"u");
			write(connection,buffer,MAX);
		}
		else if(game[index1+1][index2]=='+'){ // pacco
			if(OnlineUsers[indexUser]->packages==0){
				pthread_mutex_lock(&MatrixMutex);
				game[index1+1][index2]=' ';
				pthread_mutex_unlock(&MatrixMutex);
				OnlineUsers[indexUser]->packages++;
				OnlineUsers[indexUser]->xp=index1+1;
				OnlineUsers[indexUser]->yp=index2;
				packTime[index1+1][index2].check=1;
				nbytes=sprintf(buffer,"[+]Package picked up->[%.2f]seconds left.\n", packTime[index1+1][index2].time);
				time(&(packTime[index1+1][index2].timePick));
				write(connection,buffer,MAX);
			}
			else{
				nbytes=sprintf(buffer,"np");
				write(connection,buffer,MAX);
			}
		}
		else if(game[index1+1][index2]=='x'){ // destinazione
			if(OnlineUsers[indexUser]->packages!=0){
				if(FindPack(SearchPack(connection,1),SearchPack(connection,2))==1){
					time(&(packTime[index1+1][index2].timeDeliver));
					FileLogUser(connection,packTime[index1+1][index2].timeDeliver);
					OnlineUsers[indexUser]->totalPack++;
					OnlineUsers[indexUser]->packages--;
					pack--;
					nbytes=sprintf(buffer,"p");
					write(connection,buffer,MAX);
				}
				else{
					OnlineUsers[indexUser]->packages--;
					pack--;
					nbytes=sprintf(buffer,"pl");
					write(connection,buffer,MAX);
				}
			}
			else{
				nbytes=sprintf(buffer,"npd");
				write(connection,buffer,MAX);
			}
		}
	}
	else {
		nbytes=sprintf(buffer,"move");
		write(connection,buffer,MAX);
	}

	write(connection,game,sizeof(game));

}

void PressS(int connection){

	int nbytes=0, index1=0, index2=0, indexUser=SearchUser(connection);
	char buffer[MAX];

	index1=OnlineUsers[indexUser]->x;
	index2=OnlineUsers[indexUser]->y;

	if((index2-1)>=0){ // non esce dalla matrice
		if (game[index1][index2-1]==' '){ // non c'è un ostacolo
			pthread_mutex_lock(&MatrixMutex);
			game[index1][index2]=' ';
			game[index1][index2-1]='A'+connection;
			pthread_mutex_unlock(&MatrixMutex);
			OnlineUsers[indexUser]->x=index1;
			OnlineUsers[indexUser]->y=index2-1;
			nbytes=sprintf(buffer,"s");
			write(connection,buffer,MAX);
		}
		else if(game[index1][index2-1]=='-' || game[index1][index2+1]=='o'){ // ostacolo
			if(game[index1][index2-1]=='o'){
				pthread_mutex_lock(&MatrixMutex);
				game[index1][index2-1]='-';
				pthread_mutex_unlock(&MatrixMutex);
			}
			nbytes=sprintf(buffer,"-");
			write(connection,buffer,MAX);
		}
		else if(game[index1][index2-1]!='x'&&game[index1][index2-1]!='+'&&game[index1][index2-1]!='o'&&game[index1][index2-1]!='-'&& game[index1][index2-1]!=' '){ // scontro con un altro giocatore
			nbytes=sprintf(buffer,"u");
			write(connection,buffer,MAX);
		}
		else if(game[index1][index2-1]=='+'){ // pacco
			if(OnlineUsers[indexUser]->packages==0){
				pthread_mutex_lock(&MatrixMutex);
				game[index1][index2-1]=' ';
				pthread_mutex_unlock(&MatrixMutex);
				OnlineUsers[indexUser]->packages++;
				OnlineUsers[indexUser]->xp=index1;
				OnlineUsers[indexUser]->yp=index2-1;
				packTime[index1][index2-1].check=1;
				nbytes=sprintf(buffer,"[+]Package picked up->[%.2f]seconds left.\n", packTime[index1][index2-1].time);
				time(&(packTime[index1][index2-1].timePick));
				write(connection,buffer,MAX);
			}
			else{
				nbytes=sprintf(buffer,"np");
				write(connection,buffer,MAX);
			}
		}
		else if(game[index1][index2-1]=='x'){ // destinazione
			if(OnlineUsers[indexUser]->packages!=0){
				if(FindPack(SearchPack(connection,1),SearchPack(connection,2))==1){
					time(&(packTime[index1][index2-1].timeDeliver));
					FileLogUser(connection,packTime[index1][index2-1].timeDeliver);
					OnlineUsers[indexUser]->totalPack++;
					OnlineUsers[indexUser]->packages--;
					pack--;
					nbytes=sprintf(buffer,"p");
					write(connection,buffer,MAX);
				}
				else{
					OnlineUsers[indexUser]->packages--;
					pack--;
					nbytes=sprintf(buffer,"pl");
					write(connection,buffer,MAX);
				}
			}
			else{
				nbytes=sprintf(buffer,"npd");
				write(connection,buffer,MAX);
			}
		}
	}
	else {
		nbytes=sprintf(buffer,"move");
		write(connection,buffer,MAX);
	}

	write(connection,game,sizeof(game));

}

void PressO(int connection){

	int nbytes=0, index1=0, index2=0, indexUser=SearchUser(connection);
	char buffer[MAX];

	index1=OnlineUsers[indexUser]->x;
	index2=OnlineUsers[indexUser]->y;

	if((index2+1)<COLS){ // non esce dalla matrice
		if (game[index1][index2+1]==' '){ // non c'è un ostacolo
			pthread_mutex_lock(&MatrixMutex);
			game[index1][index2]=' ';
			game[index1][index2+1]='A'+connection;
			pthread_mutex_unlock(&MatrixMutex);
			OnlineUsers[indexUser]->x=index1;
			OnlineUsers[indexUser]->y=index2+1;
			nbytes=sprintf(buffer,"s");
			write(connection,buffer,MAX);
		}
		else if(game[index1][index2+1]=='-' || game[index1][index2+1]=='o'){ // ostacolo
			if(game[index1][index2+1]=='o'){
				pthread_mutex_lock(&MatrixMutex);
				game[index1][index2+1]='-';
				pthread_mutex_unlock(&MatrixMutex);
			}
			nbytes=sprintf(buffer,"-");
			write(connection,buffer,MAX);
		}
		else if(game[index1][index2+1]!='x'&&game[index1][index2+1]!='+'&&game[index1][index2+1]!='o'&&game[index1][index2+1]!='-'&& game[index1][index2+1]!=' '){ // scontro con un altro giocatore
			nbytes=sprintf(buffer,"u");
			write(connection,buffer,MAX);
		}
		else if(game[index1][index2+1]=='+'){ // pacco
			if(OnlineUsers[indexUser]->packages==0){
				pthread_mutex_lock(&MatrixMutex);
				game[index1][index2+1]=' ';
				pthread_mutex_unlock(&MatrixMutex);
				OnlineUsers[indexUser]->packages++;
				OnlineUsers[indexUser]->xp=index1;
				OnlineUsers[indexUser]->yp=index2+1;
				packTime[index1][index2+1].check=1;
				nbytes=sprintf(buffer,"[+]Package picked up->[%.2f]seconds left.\n", packTime[index1][index2+1].time);
				time(&(packTime[index1][index2+1].timePick));
				write(connection,buffer,MAX);
			}
			else{
				nbytes=sprintf(buffer,"np");
				write(connection,buffer,MAX);
			}
		}
		else if(game[index1][index2+1]=='x'){ // destinazione
			if(OnlineUsers[indexUser]->packages!=0){
				if(FindPack(SearchPack(connection,1),SearchPack(connection,2))==1){
					time(&(packTime[index1][index2+1].timeDeliver));
					FileLogUser(connection,packTime[index1][index2+1].timeDeliver);
					OnlineUsers[indexUser]->totalPack++;
					OnlineUsers[indexUser]->packages--;
					pack--;
					nbytes=sprintf(buffer,"p");
					write(connection,buffer,MAX);
				}
				else{
					OnlineUsers[indexUser]->packages--;
					pack--;
					nbytes=sprintf(buffer,"pl");
					write(connection,buffer,MAX);
				}
			}
			else{
				nbytes=sprintf(buffer,"npd");
				write(connection,buffer,MAX);
			}
		}
	}
	else {
		nbytes=sprintf(buffer,"move");
		write(connection,buffer,MAX);
	}

	write(connection,game,sizeof(game));

}

void PressE(int connection){

	int nbytes=0, index1=0, index2=0, indexUser=SearchUser(connection);
	char buffer[MAX];

	index1=OnlineUsers[indexUser]->x;
	index2=OnlineUsers[indexUser]->y;

	if((index1-1)>=0){ // non esce dalla matrice
		if (game[index1-1][index2]==' '){ // non c'è un ostacolo
			pthread_mutex_lock(&MatrixMutex);
			game[index1][index2]=' ';
			game[index1-1][index2]='A'+connection;
			pthread_mutex_unlock(&MatrixMutex);
			OnlineUsers[indexUser]->x=index1-1;
			OnlineUsers[indexUser]->y=index2;
			nbytes=sprintf(buffer,"s");
			write(connection,buffer,MAX);
		}
		else if(game[index1-1][index2]=='-' || game[index1-1][index2]=='o'){ // ostacolo
			if(game[index1-1][index2]=='o'){
				pthread_mutex_lock(&MatrixMutex);
				game[index1-1][index2]='-';
				pthread_mutex_unlock(&MatrixMutex);
			}
			nbytes=sprintf(buffer,"-");
			write(connection,buffer,MAX);
		}
		else if(game[index1-1][index2]!='x'&&game[index1-1][index2]!='+'&&game[index1-1][index2]!='o'&&game[index1-1][index2]!='-'&& game[index1-1][index2]!=' '){ // scontro con un altro giocatore
			nbytes=sprintf(buffer,"u");
			write(connection,buffer,MAX);
		}
		else if(game[index1-1][index2]=='+'){ // pacco
			if(OnlineUsers[indexUser]->packages==0){
				pthread_mutex_lock(&MatrixMutex);
				game[index1-1][index2]=' ';
				pthread_mutex_unlock(&MatrixMutex);
				OnlineUsers[indexUser]->packages++;
				OnlineUsers[indexUser]->xp=index1-1;
				OnlineUsers[indexUser]->yp=index2;
				packTime[index1-1][index2].check=1;
				nbytes=sprintf(buffer,"[+]Package picked up->[%.2f]seconds left.\n", packTime[index1-1][index2].time);
				time(&(packTime[index1-1][index2].timePick));
				write(connection,buffer,MAX);
			}
			else{
				nbytes=sprintf(buffer,"np");
				write(connection,buffer,MAX);
			}
		}
		else if(game[index1-1][index2]=='x'){ // destinazione
			if(OnlineUsers[indexUser]->packages!=0){
				if(FindPack(SearchPack(connection,1),SearchPack(connection,2))==1){
					time(&(packTime[index1-1][index2].timeDeliver));
					FileLogUser(connection,packTime[index1-1][index2].timeDeliver);
					OnlineUsers[indexUser]->totalPack++;
					OnlineUsers[indexUser]->packages--;
					pack--;
					nbytes=sprintf(buffer,"p");
					write(connection,buffer,MAX);
				}
				else{
					OnlineUsers[indexUser]->packages--;
					pack--;
					nbytes=sprintf(buffer,"pl");
					write(connection,buffer,MAX);
				}
			}
			else{
				nbytes=sprintf(buffer,"npd");
				write(connection,buffer,MAX);
			}
		}
	}
	else {
		nbytes=sprintf(buffer,"move");
		write(connection,buffer,MAX);
	}

	write(connection,game,sizeof(game));

}

/* If an user quit the game, he's removed from the matrix game */
void RemoveFromGame(int connection){

	int nbytes=0, index1=0, index2=0, indexUser=SearchUser(connection);

	pthread_mutex_lock(&ClientMutex);
	OnlineUsers[indexUser]->inGame=0;
	OnlineUsers[indexUser]->totalPack=0;
	OnlineUsers[indexUser]->packages=0;
	pthread_mutex_unlock(&ClientMutex);
	index1=OnlineUsers[indexUser]->x;
	index2=OnlineUsers[indexUser]->y;
	pthread_mutex_lock(&MatrixMutex);
	game[index1][index2]=' ';
	pthread_mutex_unlock(&MatrixMutex);

}


//SWITCH DELLA MATRICE E SWITCH DI BASE


/* Switch that manages the movement into the matrix game */
void SwitchMatrix(int scelta,int connection){

	int nbytes=0;
	char buffer[MAX];

	srand(time(NULL));

	switch(scelta){

		case 101: //E: up
			PressE(connection);
			break;

		case 110: //N: down
			PressN(connection);
			break;

		case 111: //O: right
			PressO(connection);
			break;

		case 115: //S: left
			PressS(connection);
			break;
	}

	if(pack==0){ //fine della sessione di gioco
		nbytes=sprintf(buffer,"THE WINNER IS: PLAYER [%c] !", 'A'+SearchMax());
		write(connection,buffer,MAX);
	}
	else{
		nbytes=sprintf(buffer,"noendgame");
		write(connection,buffer,MAX);
	}

}

/* Handle that generates new session game when number of packages is zero */
void *HandleMatrix(){

	while(1){
		if(pack==0&&flagGame==1) {
			sleep(3);
			flagGame=0;
			MakeMatrix();
		}
	}

	pthread_detach(pthread_self());
	pthread_exit(0);

}
			
/* Base switch for every clients */
void SwitchMenu(int scelta, int connection){

	char buffer[MAX];
	int nbytes=0, flag=0, var=0, index1=0, index2=0;
	int i,j;

	switch(scelta){

		case 1:
			ConnectedClient(connection);
			break;

		case 2:
			DisconnectedClient(connection);
			break;

		case 3:
			Registration(connection);
			break;

		case 4:
			ConnectedUsers(connection);
			break;

		case 5:
			Login(connection);
			break;

		case 6:
			if(CheckLogged(connection)!=0){ //utente loggato

				nbytes=sprintf(buffer,"OK");
				write(connection,buffer,MAX);
				InsertUser(connection);
				nbytes=sprintf(buffer,"[+]YOU'RE THE PLAYER: [%c]\n\n", 'A'+connection);
				for(i=0;i<ROWS;i++){
					for(j=0;j<COLS;j++){
						printf("%c ",game[i][j]);
					}
					printf("\n");
				}
				write(connection,game,sizeof(game));
				write(connection,buffer,nbytes);

				while(pack>0){

					if(flag) break;
					nbytes=read(connection,buffer,MAX);
					buffer[nbytes]='\0';

					if(strcmp(buffer,"exit\n")==0){
						flag=1;
						RemoveFromGame(connection);
					}
					else if((strcmp(buffer,"\n")==0)||(strcmp(buffer,"s\n")!=0&&strcmp(buffer,"o\n")!=0&&strcmp(buffer,"e\n")!=0 &&strcmp(buffer,"n\n")!=0))continue;

					else{
						if(strcmp(buffer,"s\n")==0) var=115;
						else if(strcmp(buffer,"e\n")==0) var=101;
						else if(strcmp(buffer,"o\n")==0) var=111;
						else if(strcmp(buffer,"n\n")==0) var=110;

						SwitchMatrix(var,connection);
					}
				}
				flagGame=1;

			}
			else{ //utente non loggato
				nbytes=sprintf(buffer,"NOK");
				write(connection,buffer,MAX);
			}
			break;

		default:
			nbytes=sprintf(buffer,"[!]Command not valid, retry.\n");
			write(connection,buffer,nbytes);
			break;

	}

}


//FUNZIONI DI MAIN


/* Disconnect client thread and delete its connection */
void DisconnectThreads(int connection){

	pthread_mutex_lock(&ClientMutex);
	disconnectedClients++;
	connectedClients--;
	pthread_mutex_unlock(&ClientMutex);

	close(connection);
	pthread_detach(pthread_self());
	pthread_exit(0);

}

/* Handle all comunication with user */
void *HandleClient(void *arg){

	int nbytes=0, flag=0, var=0, connection=0;
	char buffer[MAX];

	connection=*((int*) arg);
	pthread_mutex_lock(&ClientMutex);
	connectedClients++;
	pthread_mutex_unlock(&ClientMutex);

	while(1){

		if(flag) break;
		nbytes=read(connection,buffer,MAX); // read client's message from socket
		buffer[nbytes]='\0';
		if(strcmp(buffer,"exit\n")==0){
			printf("[+]Client [%d] disconnected.\n",connection);
			QueueRemove(connection);
			flag=1;
		}
		else{
			var=atoi(buffer);
			SwitchMenu(var,connection);
		}
	}

	DisconnectThreads(connection);

}

/* Base function that creates client threads */
void MainThread(int sockfd){

	while(1){

		int connection=0;
		int *thread_sd;
		pthread_t tid;

		if(listen(sockfd,10)==0) printf("[+]Listening...\n\n");
		else printf("[-]Error Listening...\n");
		connection=accept(sockfd,NULL,NULL); //connessione del client

		struct sockaddr_in Caddr;
		socklen_t client_len=sizeof(Caddr);
		if(getpeername(connection,(struct sockaddr *)&Caddr,&client_len)<0) perror("[-]Error peername.\n");

		if((connectedClients+1)==MAX_USERS){ //verifica se il numero massimo di client è stato raggiunto
			printf("[-]Server: max clients reached.\n");
			close(connection);
			continue;
		}

		t_user *user=(t_user *)malloc(sizeof(t_user)); //alloca memoria per il client
		user->sockfd=connection;

		QueueAdd(user); //aggiunge il client alla coda
		FileLogIP(Caddr);

		/* Thread */
		thread_sd=(int *)malloc(sizeof(int));
		*thread_sd=connection;
		printf("[+]Server: new connection from [%d]\n", connection);
		pthread_create(&tid,NULL,&HandleClient,(void*)thread_sd);
	}

	close(sockfd);

}

void SocketTCP(int PORT){

	int sockfd, bind_var, connection;
	struct sockaddr_in Saddr;
	pthread_t tidm;

	Saddr.sin_family=AF_INET;
	Saddr.sin_port=htons(PORT);
	Saddr.sin_addr.s_addr=htonl(INADDR_ANY);

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		printf("[-]Error socket.\n");
		exit(1);
	}
	else printf("[+]ServerSocket created.\n");

	if((setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int))<0)){
		printf("[-]Error sockopt.\n");
		exit(1);
	}

	signal(SIGPIPE,SIG_IGN); //se un client termina in modo anomalo gestisco SIGPIPE e non faccio terminare il server

	if((bind_var=bind(sockfd,(struct sockaddr*)&Saddr, sizeof(Saddr)))<0){
		printf("[-]Error binding.\n");
		exit(1);
	}
	else printf("[+]Bing to port_number[%d]\n", PORT);

	pthread_create(&tidm,NULL,&HandleMatrix,NULL); //thread che valuta lo stato dei giocatori per creare una nuova sessione

	MainThread(sockfd);

}

/* Check that number of argument inserted by user on command line is correct */
int CheckNumberArguments(int argc){

	if(argc==2) return 1;
	else return 0;

}

int main(int argc, char **argv){

	if(CheckNumberArguments(argc)) SocketTCP(atoi(argv[1]));
	else printf("[!]Please enter the correct number of arguments.\n");

	return 0;

}
