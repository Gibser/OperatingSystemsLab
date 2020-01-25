#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


int tmpCommand(char* cmd){
	int fd;
	if((fd = open("./tmp", O_RDWR | O_CREAT | O_TRUNC, 0777)) < 0){
		perror("Errore creazione file tmp");
		exit(1);
	}
	system(cmd);
	
	return fd;
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
	printf("%d\n", n_users);
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

int loginF(char* username, char* password){
	printf("Nome utente: ");
	scanf("%s", username);
	printf("Password: ");
	scanf("%s", password);

	if(usernameCheck(username)){
		printf("Username non esistente!\n");
		return 0;
	}
	char cmd[100] = "echo $(cat users | sed -n 's/";
	strcat(cmd, username);
	strcat(cmd, " \\(.*\\)/\\1/p') > tmp");
	
	int fd = tmpCommand(cmd);
	char passwd[100];

	copyStringFromFile(passwd, fd);

	close(fd);
	system("rm tmp");
	if(strcmp(password, passwd) == 0){
		printf("Login effettuato!\n\n");
		return 1;
	}
	else{
		printf("Password non valida.\n\n");
		return 0;
	}

}

int regF(char* username, char* password){
	printf("Nome utente: ");
	scanf("%s", username);
	printf("Password: ");
	scanf("%s", password);
	if(!usernameCheck(username)){
		printf("Username già esistente!\n");
		return 0;
	}

	//da fare - controllo se ci sono spazi nel nome o nella password

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
	//scrivo il nome utente
	if(write(fd, regString, strlen(regString)) != strlen(regString)){
		perror("Errore scrittura users");
		exit(1);
	}

	printf("Registrazine effettuata!\n\n");
	return 1;
}


void loginMain(){
	char nome[100], passwd[100];
	int scelta = 0;
	int log;
	while(scelta != 3){
		printf("1 - Login\n2 - Registrazione\n3 - Esci\n\nScelta: ");
		scanf("%d", &scelta);
		switch(scelta){
			case 1:
				if((log = loginF(nome, passwd)) == 0)
					printf("Errore login. Riprovare.\n");
				break;
			case 2:
				if(!regF(nome, passwd))
					printf("Registrazione fallita\n");
				break;

			default:
				break;
		}

		if(log == 1)
			break;
		
	}
}




int main(){
	loginMain();
	return 0;
}