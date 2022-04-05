#include "solve.h"

sem_t * semRead = NULL, * semWrite = NULL;
void * initialShmAddr, * currentShmAddr = NULL;
int shmSize = 0;

int main(int argc, char * argv[]){
    
    Queue * pathsToFiles = malloc(sizeof(*pathsToFiles));
    if (pathsToFiles == null){
        perror("Not enough memory\n");
        exit(EXIT_FAILURE);
    }
    queueInit(pathsToFiles, sizeof(char*));
    startFileQueue(pathsToFiles, argc, argv);
    int nfiles = getQueueSize(pathsToFiles);

    shmSize = nfiles * SLAVE_OUTPUT_SIZE;

    initialShmAddr = currentShmAddr = createShm();
    initSemaphores(initialShmAddr);

    write(STDOUT_FILENO, SHM_NAME, sizeof(SHM_NAME));
    
    int finished = 1;
    memcpy(initialShmAddr, &finished, sizeof(int));
    sleep(HOLD_FOR_VIEW);

    //outPipes es el arreglo de pipes de ida e inPipes de vuelta
    int outPipes[N_SLAVES][2];
    int inPipes[N_SLAVES][2];
    pid_t slavePIDs[N_SLAVES];

    if(startPipes(outPipes, inPipes) == -1){
        perror("Error initiating pipes");
        exit(EXIT_FAILURE);
    }

    startSlaves(outPipes, inPipes, slavePIDs);
    startProcessing(pathsToFiles, nfiles, outPipes);
    continueProcessing(nfiles, pathsToFiles, outPipes, inPipes);
    
    killSlaves(outPipes, inPipes, slavePIDs);

    finished = 0;
    memcpy(initialShmAddr, &finished, sizeof(int));

    unlinkShm(initialShmAddr);
    closeSemaphores();
    free(pathsToFiles);

    exit(EXIT_SUCCESS);
}

//--------------------------------------------------------

/*
-------------------------------------------
|                                         |
|          Shared memory functions        |
|                                         |
-------------------------------------------
*/

//Sacado de pshm_create de clase
void * createShm(){
    int shmFd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRWXU);
    if (shmFd == -1){
        perror("Error creating shared memory\n");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmFd, shmSize) == -1){
        perror("Error setting shared memory size\n");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    void * addr = mmap(NULL, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (addr == MAP_FAILED){
        perror("Error mapping shared memory\n");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    int finished = 0;
    memcpy(addr, &finished, sizeof(int));
    return addr;
}

void unlinkShm(void * shmAddr){
    int i = munmap(shmAddr, shmSize);
    if (i == -1){
        perror("Error removing mapping of shared memory\n");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }
    i = shm_unlink(SHM_NAME);
    if (i == -1){
        perror("Error unlinking shared memory\n");
        exit(EXIT_FAILURE);
    }
}

void shmWrite (char * str){
    sprintf(currentShmAddr, "%s\n", str);
    currentShmAddr += strlen(str);
    sem_post(semRead);
}

// ---------------------------------------------------------

/*
-------------------------------------------
|                                         |
|          Semaphore management           |
|                                         |
-------------------------------------------
*/

void initSemaphores(void * shmAddr){
    semRead = sem_open(SEM_READ_NAME, O_CREAT | O_RDWR, 0666, 1);
    if (semRead == SEM_FAILED){
        perror("Error opening read semaphore\n");
        unlinkShm(shmAddr);
        exit(EXIT_FAILURE);
    }
    semWrite = sem_open(SEM_WRITE_NAME, O_CREAT | O_RDWR, 0666, 0);
    if (semWrite == SEM_FAILED){
        perror("Error opening write semaphore\n");
        unlinkShm(shmAddr);
        exit(EXIT_FAILURE);
    }
}

void closeSemaphores(){
    int i = sem_close(semRead);
    if (i == -1){
        perror("Error closing read semaphore\n");
        exit(EXIT_FAILURE);
    }
    i = sem_close(semWrite);
    if (i == -1){
        perror("Error closing write semaphore\n");
        exit(EXIT_FAILURE);
    }
}
// -------------------------------------------------


/*
-------------------------------------------
|                                         |
|          Slave management               |
|                                         |
-------------------------------------------
*/

int startPipes(int outPipes[N_SLAVES][2], int inPipes[N_SLAVES][2]){
    for(int i = 0; i < N_SLAVES; i++){
        if(pipe(outPipes[i]) == -1 || pipe(inPipes[i]) == -1){
            return -1;
        }
    }
    return 0;
}

void startSlaves(int outPipes[N_SLAVES][2], int inPipes[N_SLAVES][2], pid_t slavePIDs[N_SLAVES]){
    int i = 0;
    while(i < N_SLAVES){
        slavePIDs[i] = fork();
        if(slavePIDs[i] > 0){ //proceso padre --> tenemos que cerrar: Escritura en vuelta, lectura en ida
            close(outPipes[i][0]);
            close(inPipes[i][1]);
        }
        else if(slavePIDs[i] == 0){ //proceso hijo --> tenemos que cerrar: Escritura en ida, lectura en vuelta
            close(outPipes[i][1]);
            close(inPipes[i][0]);

            /* Redireccionamos entrada estandar al pipe 
            (https://stackoverflow.com/questions/34911850/c-redirecting-stdin-stdout-to-pipes)
            Recordando que al utilizar dup, tomo el fichero mas chico disponible (man dup)
            y lo reemplaza por el nuevo fd en el call, si yo cierro STD_IN o STD_OUT, el primer
            fichero disponible sera el que cierre.  Entonces combinando --> close(STD_IN) --> dup(miFD) --> miFD.
            */

            //stdin --> puntero a FILE
            //STDIN_FILENO --> File descriptor estandar del stdin
            //https://stackoverflow.com/questions/15102992/what-is-the-difference-between-stdin-and-stdin-fileno
            close(STDIN_FILENO);
            dup(outPipes[i][0]);
            close(STDOUT_FILENO);
            dup(inPipes[i][1]);

            // Por ultimo, execv y que corra el esclavo
            char ** aux = {0};
            execv("./slave", aux);

        }
        else{
            perror("Error creating slave processes\n");
            exit(EXIT_FAILURE);
        }
        i++;
    }
}

void killSlaves(int outPipes[N_SLAVES][2], int inPipes[N_SLAVES][2], int slavePIDs[N_SLAVES]){
    for(int i = 0; i < N_SLAVES; i++){
        close(outPipes[i][1]);
        close(inPipes[i][0]);
        kill(slavePIDs[i], SIGKILL); // https://stackoverflow.com/questions/6501522/how-to-kill-a-child-process-by-the-parent-process
    }
}

// -----------------------------------------------------------------------------

/*
-------------------------------------------
|                                         |
|          File processing                |
|                                         |
-------------------------------------------
*/

void startFileQueue(Queue * pathsToFiles, int argc, char ** argv){
    for(int i = 1; i < argc; i++){
        char * auxStr = malloc(strlen(argv[i]) + 1);
        if (auxStr == null){
            perror("Not enough memory\n");
            exit(EXIT_FAILURE);
        }
        strcpy(auxStr, argv[i]); //null terminated incluido en el cpy (man strcpy)
        startFileQueueRec(pathsToFiles, auxStr);
    }
}

void startFileQueueRec(Queue * pathsToFiles, char * path){
    struct stat auxStat;
    stat(path, &auxStat);
    if(S_ISREG(auxStat.st_mode)){
        if(isACnf(path) == 0){
            enqueue(pathsToFiles, &path);
        }
    }
    else if(S_ISDIR(auxStat.st_mode)){ 
        struct dirent * entry;
        DIR * auxDir = opendir(path);
        while((entry = readdir(auxDir)) != NULL){
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
                continue;
            }
            //no es ni "." ni "..", vamos al paso recursivo
            int pathLen = strlen(path);
            char * auxStr = malloc(pathLen + strlen(entry->d_name) + 2);
            if (auxStr == null){
                perror("Not enough memory\n");
                exit(EXIT_FAILURE);
            }
            //chequeo si el path sigue con /
            if(path[pathLen] == '/'){
                sprintf(auxStr, "%s%s", path, entry->d_name); //quedaria <path>/<ent->d_name>
            }
            else{
                sprintf(auxStr, "%s/%s", path, entry->d_name); //le agrego yo el /
            }
            startFileQueueRec(pathsToFiles, auxStr);
        }
        closedir(auxDir);
    }
}

int isACnf(char * path){
    char * auxp = strrchr(path, '.'); // https://stackoverflow.com/questions/5309471/getting-file-extension-in-c
    char * ext = malloc(sizeof(MAX_EXTENSION));
    if (ext == null){
        perror("Not enough memory\n");
        exit(EXIT_FAILURE);
    }
    int i = 0;
    char c;
    while((c = *auxp) != '\0'){
        ext[i++] = c;
        auxp += 1;
    }
    ext[i] = '\0';
    int a = strcmp(".cnf", ext);
    return a;
}

void startProcessing(Queue * pathsToFiles, int nfiles, int outPipes[N_SLAVES][2]){
    int evaluate = N_SLAVES*N_INITIAL_FILES;
    int i = 0;
    int j = 0;
    char amount;
    if(evaluate > nfiles){ //le mandamos 1 a cada uno
        int auxVar = nfiles;
        amount = 1;
        for(i = 0; i < N_SLAVES && auxVar > 0; i++){
            write(outPipes[i][1], &amount, sizeof(char));
            sendf(outPipes[i][1], pathsToFiles);
            auxVar--;
        }
    }
    else{ //le mandamos N_INITIAL_FILES a cada uno
        for(i = 0; i < N_SLAVES; i++){ 
            amount = N_INITIAL_FILES;
            write(outPipes[i][1], &amount, sizeof(char));
            for(j = 0; j < N_INITIAL_FILES; j++){
                sendf(outPipes[i][1], pathsToFiles);
            }
            j = 0;
        }
    }
}

void continueProcessing(int totalFiles, Queue * pathsToFiles, int outPipes[N_SLAVES][2], int inPipes[N_SLAVES][2]){
    struct timeval time_interval;
    time_interval.tv_sec = 5;
    time_interval.tv_usec = 0;
    int max, e;

    fd_set slave_fd_set;
    FD_ZERO(&slave_fd_set);
    int i = 0;
    FILE * file_ptr = fopen("results.txt", "r+");
    if (file_ptr == NULL){
        perror("Error creating results file\n");
        exit(EXIT_FAILURE);
    }
    while(totalFiles > 0){
        //Necesito colocar todos los FD de lectura en el set, para la invocación de select
        FD_ZERO(&slave_fd_set);
        for(i = 0; i < N_SLAVES; i++){
            FD_SET(inPipes[i][0], &slave_fd_set);
            if(i == 0 || inPipes[i][1] > max){
                max = inPipes[i][1];
            }
        }
        
        max += 1; //para llamado de select
        time_interval.tv_sec = 5;
        time_interval.tv_usec = 0;

        if(select(max, &slave_fd_set, NULL, NULL, &time_interval) < 0){
            perror("Error in select call\n");
            exit(EXIT_FAILURE);
        }

        for(i = 0; i < N_SLAVES && totalFiles > 0; i++){
            char * output = NULL;
            if(FD_ISSET(inPipes[i][0], &slave_fd_set)){
                output = readFromSlave(inPipes[i][0]);
                /*
                Aca hay 3 opciones:
                    1) Lee un "-1" (termino de procesar) y esta idle esperando que le manden mas
                        1.1) Mandamos mas archivos (si quedan)
                        1.2) No hacemos nada si no quedan, lo dejamos hasta que termine
                        (podriamos killearlo, dado que no tiene mas utilidad)
                        ((ver que es mas limpio, si matar 1x1 o todos juntos))
                    2) Lee algo distinto a "-1" --> Me devolvio algo
                */
                if(output != NULL && (e = strcmp(output, "-1")) == 0 && totalFiles > 0){
                    sendf(outPipes[i][1], pathsToFiles);
                }
                else if(output != NULL && e != 0){
                    totalFiles--;
                    shmWrite(output);
                    fputs(output, file_ptr);
                }
            }
            free(output);
        }
    }
    fclose(file_ptr);
}

void sendf(int fd, Queue * pathsToFiles){
    char * auxStr;
    dequeue(pathsToFiles, &auxStr);
    int bytesToSend = strlen(auxStr) + 1;
    if (strcmp("-1", auxStr) != 0)
        write(fd, auxStr, bytesToSend);
}
// ----------------------------------------------------------------

char * readFromSlave(int fd){
    int keepReading = 1;
    int pathSize = 0;
    int bytesRead = 0;

    char * path = malloc(sizeof(BLOCK));
    if (path == null){
        perror("Not enough memory\n");
        exit(EXIT_FAILURE);
    }
    while(keepReading){
        bytesRead = read(fd, &keepReading, 1);
        if(bytesRead <= 0){
            perror("Failed to read pipe\n");
            exit(EXIT_FAILURE);
        }
        if(pathSize % BLOCK == 0){
            path = realloc(path, BLOCK*2 + pathSize);
            if (path == NULL){
                perror("Not enough memory\n");
                exit(EXIT_FAILURE);
            }
        }
        path[pathSize++] = keepReading;
    }
    if(pathSize % BLOCK == 0){
        path = realloc(path, BLOCK*2 + pathSize);
        if (path == NULL){
            perror("Not enough memory\n");
            exit(EXIT_FAILURE);
        }
    }
    path[pathSize] = '\0';
    return path;
}


