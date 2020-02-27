#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "login.h"


void sendSignal(int clientsd, char *msg){
  int n;
  n = strlen(msg);
  write(clientsd, &n, sizeof(int));
  if(n>0)
    write(clientsd, msg, n);
}

int maxUsers(){ 
	pthread_t currentTid = pthread_self();
	char stringTid[20];
	char removeString[50];
	char cmd[100] = "echo $(cat logged_users | grep -c \".*\") > ";
	sprintf(stringTid, "tmp%ld", currentTid);
	strcat(cmd,stringTid);
	sprintf(removeString,"rm %s",stringTid);
	int fd = tmpCommand(cmd,stringTid);
	char buff[4];

	read(fd, buff, 4);
	close(fd);
	system(removeString);
	printf("Utenti connessi: %d\n", atoi(buff));
	if(atoi(buff) >= MAX_USERS)
		return 1;
	else
		return 0;
}




void logout(int clientsd){
	char sd[10];
	//converto clientsd in stringa
	snprintf(sd, 10, "%d", clientsd);

	char cmd[100] = "sed -i '/";
	strcat(cmd, sd);
	strcat(cmd, "/d' logged_users");
	system(cmd);

	//sed -n 's/\(.*\) 1/\1/p' -- comando per prendere solo il nome utente con clientsd = 1
}


int tmpCommand(char* cmd, char* fileName){
	int fd;
	if((fd = open(fileName, O_RDWR | O_CREAT | O_TRUNC, 0777)) < 0){
		perror("Errore creazione file tmp");
		exit(1);
	}
	system(cmd);
	
	return fd;
}

void extractUsername(char *buffer,char *username){
	int i;
	memset(username,'\0',sizeof(username));
	i=0;
	while(buffer[i]!='\n'){
		username[i]=buffer[i];
		i++;
	}
	username[i] = '\0';
}


void extractPassword(char *buffer, char *password){
	int i,j;
	memset(password,'\0',sizeof(password));
	i=0;
	j=0;
	while(buffer[i]!='\n'){
		i++;
	}
	i++;
	while(buffer[i] != '\0'){
		password[j]=buffer[i];
		i++;
		j++;
	}
	password[j] = '\0';
	//printf("Buffer: %s\n", buffer);
	//printf("%s\n", password);
}

int usernameCheck(char* username){	
	pthread_t currentTid = pthread_self();
	char stringTid[100];
	char removeString[150];
	//conto il numero di utenti con il nome inserito
	char cmd[100] = "echo $(cat users | grep -c \"";
	strcat(cmd, username);

	sprintf(stringTid, "tmp%ld", currentTid);
	strcat(cmd, " \") > ");
	strcat(cmd, stringTid);
	int fd = tmpCommand(cmd, stringTid);

	//controllo il numero restituito da grep
	int n_users;
	char buff;
	read(fd, &buff, 1);
	
	n_users = atoi(&buff);
	close(fd);
	sprintf(removeString, "rm %s", stringTid);
	system(removeString);

	if(n_users == 1)
		return 0;
	else
		return 1;

}

void copyStringFromFile(char* string, int fd){
	char tempChar;
	int i = 0;
	while(read(fd, &tempChar, 1) == 1){
		string[i] = tempChar;
		i++;
	}
	string[i-1] = '\0';
}

int loggedUser(char* username){
	pthread_t currentTid = pthread_self();
	char cmd[100] = "echo $(cat logged_users | grep -c \"";
	char stringTid[20];
	char removeString[50];
	strcat(cmd, username);
	strcat(cmd, " \") > ");
	sprintf(stringTid,"tmp%ld",currentTid);
	strcat(cmd, stringTid);
	sprintf(removeString,"rm %s",stringTid);
	int fd = tmpCommand(cmd,stringTid);

	int n_users;
	char buff;
	read(fd, &buff, 1);
	n_users = atoi(&buff);
	close(fd);
	system(removeString);

	if(n_users == 1)
		return 1;
	else
		return 0;
	
}

void logUser(char* username, int clientsd, pthread_mutex_t login){
	char sd[10];
	//converto clientsd in stringa
	snprintf(sd, 10, "%d", clientsd);

	int fd = open("logged_users", O_APPEND | O_RDWR | O_CREAT, 0666);
	if(fd < 0){
		perror("Errore apertura file utenti loggati.");
		exit(1);
	}
	char file_string[200];
	strncpy(file_string, username, strlen(username)+1);
	strcat(file_string, " ");
	strcat(file_string, sd);
	//sezione critica: scrittura logged_users
	pthread_mutex_lock(&login);
	printf("Scrivo utente...\n");
	write(fd, file_string, strlen(file_string));
	write(fd, "\n", 1);
	pthread_mutex_unlock(&login);
	close(fd);
}


int loginF(char* username, char* password, int clientsd, pthread_mutex_t login){
	char buffer[200];
	int n;
	memset(buffer,'\0',sizeof(buffer));
	if(read(clientsd,&n,sizeof(int))>0){//read how many bytes client is going to send me
		if(n==-1){
			return 0;
		}
		read(clientsd,buffer,n);
		extractUsername(buffer,username);
		extractPassword(buffer,password);
		if(usernameCheck(username)){
			//write(clientsd, "~USRNOTEXISTS", 13); //Username non esistente!
			sendSignal(clientsd, "~USRNOTEXISTS");
			return 0;
		}

		if(loggedUser(username)){
			printf("Utente già loggato\n");
			//write(clientsd, "~USRLOGGED", 10); //Utente già loggato
			sendSignal(clientsd, "~USRLOGGED");
			return 0;
		}

		if(maxUsers()){
			printf("Il server è pieno\n");
			//write(clientsd, "~SERVERISFULL", 13); //Server pieno
			sendSignal(clientsd, "~SERVERISFULL");
			return 0;
		}

		pthread_t currentTid = pthread_self();
		char cmd[100] = "echo $(cat users | sed -n 's/";
		char stringTid[20];
		char removeString[50];
		strcat(cmd, username);
		sprintf(stringTid,"tmp%ld",currentTid);
		strcat(cmd, " \\(.*\\)/\\1/p') > ");
		strcat(cmd, stringTid);
		//pthread_mutex_lock(&login);
		int fd = tmpCommand(cmd,stringTid);
		char passwd[100];

		copyStringFromFile(passwd, fd);

		close(fd);
		sprintf(removeString,"rm %s",stringTid);
		system(removeString);
		//pthread_mutex_unlock(&login);

		//printf("\n%s %s\n", password, passwd);
		if(strcmp(password, passwd) == 0){
			//write(clientsd, "~OKLOGIN", 8); //Login effettuato!
			sendSignal(clientsd, "~OKLOGIN");
			printf("Login effettuato con successo.\n");
			logUser(username, clientsd, login);
			return 1;
		}
		else{
			//write(clientsd, "~NOVALIDPW", 10); //Password non valida
			sendSignal(clientsd, "~NOVALIDPW");
			printf("Password non valida\n");
			return 0;
		}
	}
	else{
		return -1;
	}
	
}


int regF(char* username, char* password, int clientsd, pthread_mutex_t lock){
	char buffer[200];
	int n;
	memset(buffer,'\0',sizeof(buffer));
	if(read(clientsd,&n,sizeof(int))>0){//read how many bytes client is going to send me
		if(n==-1) 
		return 0;
		read(clientsd,buffer,n);
		extractUsername(buffer,username);
		extractPassword(buffer,password);
		if(!usernameCheck(username)){
			//write(clientsd, "~USREXISTS", 10); //Username già esistente
			sendSignal(clientsd, "~USREXISTS");
			return 0;
		}
		int fd;
		if((fd = open("users", O_RDWR | O_APPEND)) < 0){
			perror("Errore apertura users");
			exit(1);
		}
		strcat(username," ");
		strcat(username,password);
		strcat(username, "\n");
		//registro l'utente, sezione critica
		pthread_mutex_lock(&lock);
		if(write(fd, username, strlen(username)) != strlen(username)){
			perror("Errore scrittura users");
			exit(1);
		}
		pthread_mutex_unlock(&lock);
		//write(clientsd, "~SIGNUPOK", 9);  //Registrazione effettuata
		sendSignal(clientsd, "~SIGNUPOK");
		return 1;
	}
	else{
		return -1;
	}
}


int loginMain(int clientsd, pthread_mutex_t lock, pthread_mutex_t login, char *username){
	char msg[30];
	char passwd[100];
	int log=0;
	int reg;
	while(1){
		printf("Waiting..\n");
		memset(msg,'\0',sizeof(msg));
		if(read(clientsd,msg,sizeof(msg))>0){
			if(strcmp(msg,"~USRLOGIN")==0){
				if((log = loginF(username, passwd, clientsd, login)) == 0)
					printf("Errore login\n");
				else if(log==-1){
					printf("Utente disconnesso durante login\n"); //Qui si dovrà gestire la disconnessione improvvisa dell'utente durante il login
					break;
				}
			}
			else if(strcmp(msg,"~USRSIGNUP")==0){
				if((reg=regF(username, passwd, clientsd, lock))==0)
					printf("Errore registrazione\n");
				else if(reg==-1){
					printf("Utente disconnesso durante registrazione\n"); //Gestione disconnessione
					break;
				}
			}
			else if(strcmp(msg,"~USREXIT")==0){
				printf("Utente disconnesso\n");//Gestione disconnessione
				break;
			}
			if(log == 1){
				break;
			}
		}
		else{
			printf("Client disconnesso.\n");
			break;
		}
		
	}

	return log;
}
