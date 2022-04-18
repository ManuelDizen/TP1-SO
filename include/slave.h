#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "queue.h"

#define FN_LIMIT 256
#define BLOCK (ROW_SIZE*COL_SIZE)
#define MAX_OUTPUT_BUFF 3000
#define ROW_SIZE 80
#define COL_SIZE 19
#define OUTPUT_SIZE 200

// Llama a minisat con popen, y ejecuta el comando "minisat <filename>"
void solveWithMinisat(char * filename, char * outputBuff);
void startProcessingSlave(char amount, int fd);
void controlProcessingSlave(int fd);
void checkPipe(int fd);
char * getPath(int fd);

