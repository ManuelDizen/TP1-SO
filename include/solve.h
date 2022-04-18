#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/stat.h>      
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>           
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <regex.h>  

#include "queue.h"

#define N_SLAVES 5
#define N_INITIAL_FILES 2
#define BLOCK 50
#define MAX_EXTENSION 50
#define SLAVE_OUTPUT_SIZE 200

#define SHM_NAME "shm"
#define SEM_READ_NAME "semRead"
#define SEM_WRITE_NAME "semWrite"
#define HOLD_FOR_VIEW 5


int startPipes(int outPipes[N_SLAVES][2], int inPipes[N_SLAVES][2]);
void startSlaves(int outPipes[N_SLAVES][2], int inPipes[N_SLAVES][2], pid_t slavePIDs[N_SLAVES]);
void * createShm();
void unlinkShm(void * shmAddr);
void initSemaphores(void * shmAddr);
void closeSemaphores();
void shmWrite (char * str);
void startFileQueue(Queue * pathsToFiles, int argc, char ** argv);
void startFileQueueRec(Queue * pathsToFiles, char * path);
void startProcessing(Queue * pathsToFiles, int nfiles, int outPipes[N_SLAVES][2]);
void sendf(int fd, Queue * pathsToFiles);
void continueProcessing(int totalFiles, Queue * pathsToFiles, int outPipes[N_SLAVES][2], int inPipes[N_SLAVES][2]);
void killSlaves(int outPipes[N_SLAVES][2], int inPipes[N_SLAVES][2], int slavePIDs[N_SLAVES]);
char * readFromSlave(int fd);
int isACnf(char * path);


