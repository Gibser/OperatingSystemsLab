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

int hasSpace(char* string){
	int i = 0;
	while(i < strlen(string)){
		if(string[i] == ' ')
			return 1;
		i++;
	}
	return 0;
}

int tmpCommand(char* cmd){
	int fd;
	if((fd = open("./tmp", O_RDWR | O_CREAT | O_TRUNC, 0777)) < 0){
		perror("Errore creazione file tmp");
		exit(1);
	}
	system(cmd);
	
	return fd;
}

void *extractUsername(char *buffer,char *username){
	int i;
	memset(username,'\0',sizeof(username));
	i=0;
	while(buffer[i]!='\n'){
		username[i]=buffer[i];
		i++;
	}
}


char *extractPassword(char *buffer, char *password){
	int i,j;
	memset(password,'\0',sizeof(password));
	i=0;
	j=0;
	while(buffer[i]!='\n'){
	}
	while(i<strlen(buffer)){
		password[j]=buffer[i];
		j++;
	}
}


void removeNewLine(char* string){
	int dim = strlen(string);
	string[dim] = '\0';
}

int usernameCheck(char* username){	
	//conto il numero di utenti con il nome inserito
	char cmd[100] = "echo $(cat users | grep -c \"";
	strcat(cmd, username);
	strcat(cmd, " \") > tmp");
	int fd = tmpCommand(cmd);

	//controllo il numero restituito da grep
	int n_users;
	char buff;
	read(fd, &buff, 1);
	
	n_users = atoi(&buff);
	close(fd);
	system("rm tmp");

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

int loginF(char* username, char* password, int clientsd){
	int bytes_r;

	
	//getchar();
	//scanf("%[^\n]", username);
	read(clientsd, username, 200);
	removeNewLine(username);
	if(hasSpace(username)){
		write(clientsd, "2", 1); //Il nome utente non può contenere spazi
		return 0;
	}
	else
		write(clientsd, "1", 1); //Username valido, si procede con la password

		

	//getchar();
	//scanf("%[^\n]", password);
	read(clientsd, password, 200);
	removeNewLine(password);
	if(hasSpace(password)){
		write(clientsd, "2", 1); //La password non può contenere spazi.
		return 0;
	}
	else
		write(clientsd, "1", 1); //Password valida
	

	if(usernameCheck(username)){
		write(clientsd, "3", 1); //Username non esistente!
		return 0;
	}
	else
		write(clientsd, "1", 1); //Username esistente, si procede


	char cmd[100] = "echo $(cat users | sed -n 's/";
	strcat(cmd, username);
	strcat(cmd, " \\(.*\\)/\\1/p') > tmp");
	
	int fd = tmpCommand(cmd);
	char passwd[100];

	copyStringFromFile(passwd, fd);

	close(fd);
	system("rm tmp");
	if(strcmp(password, passwd) == 0){
		write(clientsd, "1", 1); //Login effettuato!
		return 1;
	}
	else{
		write(clientsd, "2", 1); //Password non valida
		return 0;
	}
	
}


int regF(char* username, char* password, int clientsd, pthread_mutex_t lock){
	char buffer[200];
	memset(buffer,'\0',sizeof(buffer));
	read(clientsd,buffer,200);
	extractUsername(buffer,username);
	printf("username inserito: %s", username);
	extractPassword(buffer,password);
	printf("password inserita: %s", password);
	if(!usernameCheck(username)){
		write(clientsd, "~USREXISTS", 10); //Username già esistente
		return 0;
	}
	int fd;
	if((fd = open("users", O_RDWR | O_APPEND)) < 0){
		perror("Errore apertura users");
		exit(1);
	}

	char regString[200] = "";
	strcat(regString, username);
	strcat(regString, " ");
	strcat(password, "\n");
	strcat(regString, password);
	//registro l'utente, sezione critica
	pthread_mutex_lock(&lock);
	if(write(fd, regString, strlen(regString)) != strlen(regString)){
		perror("Errore scrittura users");
		exit(1);
	}
	pthread_mutex_unlock(&lock);

	write(clientsd, "~SIGNUPOK", 9);  //Registrazione effettuata
	return 1;
}


void loginMain(int clientsd, pthread_mutex_t lock){
	char msg[30];
	char nome[100], passwd[100];
	int log;
	while(1){
		memset(msg,'\0',sizeof(msg));
		read(clientsd,msg,sizeof(msg));
		if(strcmp(msg,"~USRLOGIN")==0){
			if((log = loginF(nome, passwd, clientsd)) == 0)
				printf("Errore login\n");
				//write(clientsd, "Errore login. Riprovare.\n", 25);
		}
		else if(strcmp(msg,"~USRSIGNUP")==0){
			if(!regF(nome, passwd, clientsd, lock))
				printf("Errore registrazione\n");
				//write(clientsd, "Registrazione fallita\n", 22);
		}

		if(log == 1){
			break;
		}
	}
}
