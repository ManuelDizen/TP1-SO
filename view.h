#ifndef _VIEW_H_
#define _VIEW_H_

#define MAX_SHM_NAME 100

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h> 
#include <fcntl.h>
#include <string.h>
#include "solve.h"

//Conecta la shared memory 
void * setupShm(char * shmName, int * shmFd);
void unlinkShm(void * shmAddr, int shmFd);
void printShm(void * shmAddr);
void semSetup(void * shmAddr, int shmFd);


#endif
