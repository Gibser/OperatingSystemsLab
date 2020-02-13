#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

int hasSpace(char* string);
int tmpCommand(char* cmd);
void removeNewLine(char* string);
char * extractUsername(char *buffer);
char *extractPassword(char *buffer);
int usernameCheck(char* username);
void copyStringFromFile(char* string, int fd);
int loginF(char* username, char* password, int clientsd);
int regF(char* username, char* password, int clientsd, pthread_mutex_t lock);
void loginMain(int clientsd, pthread_mutex_t lock);