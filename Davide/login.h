#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#define MAX_USERS 8


int tmpCommand(char* cmd);
void extractUsername(char *buffer,char* username);
void extractPassword(char *buffer, char* password);
int usernameCheck(char* username);
void copyStringFromFile(char* string, int fd);
int loginF(char* username, char* password, int clientsd, pthread_mutex_t login);
int regF(char* username, char* password, int clientsd, pthread_mutex_t lock);
int loginMain(int clientsd, pthread_mutex_t lock, pthread_mutex_t login, char *username);
void logUser(char* username, int clientsd, pthread_mutex_t login);
int loggedUser(char* username);
void logout(int clientsd);
void sendMessage(int clientsd, char *msg);