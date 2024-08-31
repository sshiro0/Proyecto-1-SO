#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <linux/limits.h>

#define clear() printf("\033[H\033[J")

void welcome() {
    clear();
    printf("\n\n");
    printf("                               ██     ██ ███████ ██       ██████  ██████  ███    ███ ███████ ██ \n");
    printf("                               ██     ██ ██      ██      ██      ██    ██ ████  ████ ██      ██ \n");
    printf("                               ██  █  ██ █████   ██      ██      ██    ██ ██ ████ ██ █████   ██ \n");
    printf("                               ██ ███ ██ ██      ██      ██      ██    ██ ██  ██  ██ ██         \n");
    printf("                                ███ ███  ███████ ███████  ██████  ██████  ██      ██ ███████ ██ \n");
    char* username = getenv("USER");
    printf("\nUSER: @%s\n\n", username);
}


void printDirectory() {

    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\033[1;32mPROJECTshell\033[0m:\033[1;34m~%s\033[0m$ ", cwd);
    } 
    else {
        perror("Error getting the current working directory.");
        exit(1);
    }
}

int main(int argc, char **argv) {
    (void)argc, (void)argv;     // void para evitar problemas al compilar
    char *buffer = NULL;        // puntero al string del usuario
    size_t bufferSize = 0;      // size para asignar espacio en memoria de buffer
    ssize_t bufferRead;         // ssize para almacenar cc de caracteres leidos             
    pid_t c_pid;                // ID del hijo
    int status;                 // status del hijo
    char **array;               // puntero al array que almacena el comando y argumentos
    char **array2;
    char *fullCommand;
    
    FILE *file = fopen("misfavoritos.txt", "a");
    fclose(file);

    welcome();

    while (1) {
        printDirectory();

        bufferRead = getline(&buffer, &bufferSize, stdin);// almacena bufferSize y lee buffer
        fullCommand = strdup(buffer);
        if (bufferRead == -1) {                                 
            perror("Error reading. Exiting shell.");
            free(buffer);
            free(fullCommand);
            exit(1);
        }
        


        array = malloc(sizeof(char*) * 1024);           // asignación de memoria
        if (array == NULL) {                               
            perror("Malloc error. Exiting shell.");
            free(buffer);
            exit(1);
        }

        array2 = malloc(sizeof(char*) * 1024);           // asignación de memoria
        if (array2 == NULL) {                               
            perror("Malloc error. Exiting shell.");
            free(buffer);
            exit(1);
        }

        int i = 0;
        int j = 0;
        int es_pipe = 0;

        char *token = strtok(buffer, " \n");            // divide string buffer en tokens, usa " " como delimitador y \n
        while (token) { 
            if (strcmp(token, "|") == 0) {
                array[i] = NULL;
                es_pipe = 1;
                token = strtok(NULL, " \n");
                continue;
            }

            if (es_pipe) {
                array2[j++] = token;
            } else {
                array[i++] = token;
            }
            token = strtok(NULL, " \n");
        }

        array[i] = NULL;   
        array2[j] = NULL;   

        if (array[0] == NULL) {                         // caso no se ingresa nada, se pide algo
            printf("Please, enter a command.\n");
            free(array);
            free(array2);
            free(fullCommand);
            continue;
        }

        if (strcmp(array[0], "favs") == 0){
            // Mostrar los comandos favoritos
            if (array[1] != NULL && strcmp(array[1], "mostrar") == 0){
                FILE *file = fopen("misfavoritos.txt", "r");

                char line[256]; 
                
                int counter = 1;
                while (fgets(line, sizeof(line), file)){ 
                    printf("%d) %s",counter ,line);
                    counter++;
                }

                fclose(file);
                continue;
            }

            else if (array[1] != NULL && strcmp(array[1], "buscar") == 0){
                if (array[2] == NULL)
                    perror("Error cmd null");

                else{
                    char line[256];
                    int counter = 1;
                    FILE *file = fopen("misfavoritos.txt", "r");
                    while (fgets(line, sizeof(line), file)){ 
                        if (strstr(line, array[2]) != NULL){
                            printf("%d) %s",counter ,line);
                        }
                        counter++;
                    }
                    fclose(file);
                    continue;
                }
            }
        }
        
        if (strcmp(array[0], "cd") == 0) {
            if (array[1] == NULL) {
                fprintf(stderr, "Error finding the directory.\n");
            } else {
                if (chdir(array[1]) != 0) {
                    perror("Error in cd command");
                }
            }
            free(array);
            free(array2);
            continue;                                   // Skip fork() si el comando es cd
        }

        if (strcmp(array[0], "exit") == 0) {
            free(array);
            free(array2);
            exit(0);
        }

        if (es_pipe) {
            int p[2];
            pipe(p);

            pid_t pid1 = fork();
            if (pid1 == 0) {
                // Hijo 1 (ejecuta el primer comando)
                close(p[0]); // cierra lectura
                dup2(p[1], STDOUT_FILENO); // redirige stdout
                close(p[1]);
                execvp(array[0], array);
                perror("Failed to execute first command");
                exit(1);
            } else {
                pid_t pid2 = fork();
                if (pid2 == 0) {
                    // Hijo 2 (ejecuta el segundo comando)
                    close(p[1]); // cierra escritura
                    dup2(p[0], STDIN_FILENO); // redirige stdin
                    close(p[0]);
                    execvp(array2[0], array2);
                    perror("Failed to execute second command");
                    exit(1);
                } else {
                    // Proceso padre
                    close(p[0]);
                    close(p[1]);
                    waitpid(pid1, &status, 0);
                    waitpid(pid2, &status, 0);
                }
            }
        } else {
            // Proceso sin pipes
            c_pid = fork();
            if (c_pid == -1) {
                perror("Failed to create the child.");
                free(array);
                free(array2);
                exit(1);
            }

            if (c_pid == 0) {
                if (execvp(array[0], array) == -1) {
                    perror("Failed to execute");
                    free(array);
                    free(array2);
                    exit(1);
                }
            } else {
                waitpid(c_pid, &status, 0);

                // chequear que el comando no haya sido escrito antes

                if (WIFEXITED(status)){
                    int exit_status = WEXITSTATUS(status);
                    char line[256];
                    char write = 1;

                    FILE *file = fopen("misfavoritos.txt", "r");
                    while (fgets(line, sizeof(line), file)){ 
                        if(strcmp(line, fullCommand) == 0){
                            write = 0;
                        }
                    }

                    fclose(file);

                    if (write && strcmp(array[0], "favs") != 0) {
                        // Escribir el comando en mis favoritos
                        FILE *file = fopen("misfavoritos.txt", "a");
                        fprintf(file, "%s", fullCommand);
                        fclose(file);
                    }

                }    

            }
        }
        
        free(array);
        free(array2);
    }
    free(buffer);
    return 0;
}