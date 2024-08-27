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

    welcome();

    while (1) {

        printDirectory();

        bufferRead = getline(&buffer, &bufferSize, stdin);      // almacena bufferSize y lee buffer
        if (bufferRead == -1) {                                 
            perror("Error reading. Exiting shell.");
            free(buffer);
            exit(1);
        }

        array = malloc(sizeof(char*) * 1024);           // asignación de memoria
        if (array == NULL) {                               
            perror("Malloc error. Exiting shell.");
            free(buffer);
            exit(1);
        }

        int i = 0;
        char *token = strtok(buffer, " \n");            // divide string buffer en tokens, usa " " como delimitador y \n
        while (token) {
            array[i++] = token;
            token = strtok(NULL, " \n");
        }
        array[i] = NULL;                

        if (array[0] == NULL) {                         // caso no se ingresa nada, se pide algo
            printf("Please, enter a command.");
            free(array);
            continue;
        }
    
        if (strcmp(array[0], "cd") == 0) {
            if (array[1] == NULL) {
                fprintf(stderr, "Error finding the directory.\n");
            }
            else {
                if (chdir(array[1]) != 0) {
                    perror("Error in cd command");
                }
            }
            free(array);
            continue;                                   // Skipeafork() si el comando es cd
        }

        if (strcmp(array[0], "exit") == 0) {
            exit(1);
        }


        // Proceso Hijo
        c_pid = fork();
        if (c_pid == -1) {
            perror("Failed to create the child.");
            free(array);
            exit(1);
        }

        if (c_pid == 0) {
            if (execvp(array[0], array) == -1) {
                perror("Failed to execute");
                free(array);
                exit(1);
            }
        }
        else {
            wait(&status);
        }

        free(array);
    }
    
    free(buffer);
    return 0;
}