#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv) {

    (void)argc, (void)argv;     // void para evitar problemas al compilar
    char *buffer = NULL;        // puntero al string del usuario
    size_t bufferSize = 0;      // size para asignar espacio en memoria de buffer
    ssize_t bufferRead;         // ssize para almacenar cc de caracteres leidos             
    pid_t c_pid;                // ID del hijo
    int status;                 // status del hijo
    char **array;               // puntero al array que almacena el comando y argumentos

    while (1) {
        fputs("\033[1;32mUDECshell$ \033[0m", stdout);          // prompt verde brillante "UDECshell"

        bufferRead = getline(&buffer, &bufferSize, stdin);      // almacena bufferSize y lee buffer
        if (bufferRead == -1) {                                 
            perror("Error in reading. Exiting shell.");
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

        c_pid = fork();                                 // creación proceso hijo
        if (c_pid == -1) {
            perror("Failed to create the child.");
            free(buffer);
            free(array);
            exit(1);
        }

        if (c_pid == 0) {
            if (execvp(array[0], array) == -1) {
                perror("Failed to execute");
                free(buffer);
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