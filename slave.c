#include "slave.h"

void solveWithMinisat(char * filename, char * outputBuff);

int main(int argc, char ** argv){
    
    char amount;
    read(STDIN_FILENO, &amount, sizeof(char)); //lee cantidad inicial que le van a pasar
    startProcessingSlave(amount, STDIN_FILENO);
    controlProcessingSlave(STDIN_FILENO);
    
    return 0;

}

void controlProcessingSlave(int fd){
    while(1){ //cuando se haga SIGKILL desde el padre, logicamente este superloop morira
        checkPipe(fd);
        char * finished = "-1";
        write(STDOUT_FILENO, finished, sizeof(finished));
    }
}

void checkPipe(int fd){
    char * path = getPath(fd);
    if(path != NULL){
        char outputBuff[OUTPUT_SIZE + strlen(path) + 1 + 2]; //1 = '\0', 2 = ": "
        solveWithMinisat(path, outputBuff);
        write(STDOUT_FILENO, outputBuff, strlen(outputBuff) + 1);
    }
}

char * getPath(int fd){
    int keepReading = 1;
    int pathSize = 0;

    //char * path = malloc(sizeof(BLOCK));
    char *path = malloc(BLOCK);
    if (path == NULL){
        perror("Not enough memory\n");
        exit(EXIT_FAILURE);
    }

    int bytesRead = 0;
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

void startProcessingSlave(char amount, int fd){
    for(int i = 0; i < amount; i++)
            checkPipe(fd);
    char * finished = "-1";
    write(STDOUT_FILENO, finished, sizeof(finished));
}

void solveWithMinisat(char * filename, char * outputBuff){
    char cmd[FN_LIMIT];
    sprintf(cmd, "minisat %s |  grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"", filename);
    
    FILE * file = popen(cmd, "w");
    if(file == NULL){
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }
    sprintf(outputBuff, "FileName: %s\nSlave ID: %d\n", filename, getpid());
    int i, character;
    character = fgetc(file);

    for(i = strlen("FileName: \nSlave ID: \n")+ strlen(filename) + sizeof(pid_t); i < OUTPUT_SIZE && character != EOF; i++){
        outputBuff[i] = character;
        character = fgetc(file);
    }

    outputBuff[i++] = '\n';
    outputBuff[i] = 0;
    pclose(file);
}
