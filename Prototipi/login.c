#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

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

	write(clientsd, "Nome utente: ", 13);
	if(read(clientsd, &bytes_r, sizeof(int))!=-1){	
		//getchar();
		//scanf("%[^\n]", username);
		read(clientsd, username, bytes_r);
		removeNewLine(username);
		if(hasSpace(username)){
			write(clientsd, "Il nome utente non può contenere spazi.\n\n", 41);
			return 0;
		}
	}
	else{
		perror("Errore lettura username");
		return 0;
	}
		
	write(clientsd, "Password: ", 10);
	if(read(clientsd, &bytes_r, sizeof(int)) != -1){
		//getchar();
		//scanf("%[^\n]", password);
		read(clients, password, bytes_r);
		removeNewLine(password);
		if(hasSpace(password)){
			write(clientsd, "La password non può contenere spazi.\n\n", 38);
			return 0;
		}
	}
	else{
		perror("Errore lettura password");
		return 0;
	}

	if(usernameCheck(username)){
		write(clientsd, "Username non esistente!\n", 24);
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
		write(clientsd, "Login effettuato!\n\n", 19);
		return 1;
	}
	else{
		write(clientsd, "Password non valida.\n\n", 22);
		return 0;
	}
	
}


int regF(char* username, char* password, int clientsd, pthread_mutex_t lock){
	int bytes_r;
	write(clientsd, "Nome utente: ", 13);
	//getchar();
	//scanf("%[^\n]", username);
	if(read(clientsd, &bytes_r, sizeof(int))){
		read(clients, username, bytes_r);
		removeNewLine(username);
		if(hasSpace(username)){
			write(clientsd, "Il nome utente non può contenere spazi.\n\n", 41);
			return 0;
		}
	}

	write(clientsd, "Password: ", 10);
	//getchar();
	//scanf("%[^\n]", password);
	if(read(clientsd, &bytes_r, sizeof(int))){
		read(clientsd, password, bytes_r);
		removeNewLine(password);
		if(hasSpace(password)){
			write(clientsd, "La password non può contenere spazi.\n\n", 38);
			return 0;
		}
	}

	if(!usernameCheck(username)){
		write(clientsd, "Username già esistente!\n", 24);
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
	//registro l'utente, sezione critica
	pthread_mutex_lock(&lock);
	if(write(fd, regString, strlen(regString)) != strlen(regString)){
		perror("Errore scrittura users");
		exit(1);
	}
	pthread_mutex_unlock(&lock);

	printf("Registrazine effettuata!\n\n");
	return 1;
}


void loginMain(int clientsd, pthread_mutex_t lock){
	char nome[100], passwd[100];
	char scelta = '0';
	int log, bytes_r;
	char gameLogin[] = "\n1 - Login\n2 - Registrazione\n3 - Esci\n\nScelta: ";
	while(scelta != '3'){
		//Scrivo le opzioni sul socket, quindi visualizzate dal client
		write(clientsd, gameLogin, sizeof(gameLogin));
		if(read(clientsd, &bytes_r,sizeof(int))!=-1){     //Controllo quanti bytes invia il client
			read(clientsd, &scelta, nb);
			switch(scelta){
				case '1':
					if((log = loginF(nome, passwd, clientsd)) == 0)
						write(clientsd, "Errore login. Riprovare.\n", 25);
					break;
				case '2':
					if(!regF(nome, passwd, clientsd, lock))
						write(clientsd, "Registrazione fallita\n", 22);
					break;

				default:
					break;
			}

			if(log == 1)
				break;
		}
	}
}
