#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int hasSpace(char* string);
int tmpCommand(char* cmd);
void removeNewLine(char* string);
int usernameCheck(char* username);
void copyStringFromFile(char* string, int fd);
int loginF(char* username, char* password);
int regF(char* username, char* password);
void loginMain();