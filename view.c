#include "view.h"

sem_t * semRead = NULL, * semWrite = NULL;

int main (int argc, char ** argv){
    char shmName[MAX_SHM_NAME] = {0};
    if (argc == 1){
        read(STDIN_FILENO, &shmName, MAX_SHM_NAME);
    }
    else if (argc == 2){
        strcpy(shmName, argv[1]);
    }
    else{
        perror("Error in number of arguments\n");
        exit(EXIT_FAILURE);
    }

    int shmFd;
    void * shmAddr = setupShm(shmName, &shmFd);

    semSetup(shmAddr, shmFd);

    printShm(shmAddr);

    unlinkShm(shmAddr, shmFd);
    
}

// Sacado de pshm_read.c de clase
void * setupShm(char * shmName, int * shmFd){
    * shmFd = shm_open(shmName, O_RDWR, S_IRWXU);

    if (*shmFd == -1){
        perror("Error opening shared memory\n");
        exit(EXIT_FAILURE);
    }

    struct stat sb;

    if (fstat(*shmFd, &sb) == -1){
        perror("Error checking shared memory status\n");
        exit(EXIT_FAILURE);
    }
        
    void * shmAddr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, *shmFd, 0);
    if (shmAddr == MAP_FAILED){
        perror("Error mapping the shared memory\n");
        exit(EXIT_FAILURE);
    }

    return shmAddr;
}


void unlinkShm(void * shmAddr, int shmFd){
    struct stat sb;

    if (fstat(shmFd, &sb) == -1){
        perror("Error checking shared memory status\n");
        exit(EXIT_FAILURE);
    }

    int i = munmap(shmAddr, sb.st_size);
    if (i == -1){
        perror("Error removing mapping of shared memory\n");
        exit(EXIT_FAILURE);
    }
}

void semSetup(void * shmAddr, int shmFd){
    semRead = sem_open(SEM_READ_NAME, O_RDWR);
    if (semRead == SEM_FAILED){
        perror("Error linking read semaphore\n");
        unlinkShm(shmAddr, shmFd);
        exit(EXIT_FAILURE);
    }
    semWrite = sem_open(SEM_WRITE_NAME, O_RDWR);
    if (semWrite == SEM_FAILED){
        perror("Error linking write semaphore\n");
        unlinkShm(shmAddr, shmFd);
        exit(EXIT_FAILURE);
    }
}

void printShm(void * shmAddr){
    void * initial_addr = shmAddr;
    int * finished = (int *) shmAddr;
    while (*finished != 0){
        //sem_wait(semRead);
        printf("%s", (char*)shmAddr);
        shmAddr += strlen((char*)shmAddr);
        sem_post(semWrite);
        finished = (int *) initial_addr;
    }
}

