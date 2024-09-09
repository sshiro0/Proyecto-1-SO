#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <linux/limits.h>
#define clear() printf("\033[H\033[J")

// Mostrar mensaje de bienvenida
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

// Imprimir el directorio actual
void printDirectory() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\033[1;32mPROJECTshell\033[0m:\033[1;34m~%s\033[0m$ ", cwd);
    } else {
        perror("Error getting the current working directory.");
        exit(1);
    }
}

// Ejecucion de comandos con pipes
void executePipes(char *commands[]) {
    int i;
    int n = 0;
    int pipefds[2 * (1024 - 1)];
    pid_t pid;
    int fd_in = 0;

    // Conteo de comandos
    while (commands[n] != NULL) {
        n++;
    }

    // Crear pipes
    for (i = 0; i < n-1; i++) {
        if (pipe(pipefds + i*2) == -1) {
            exit(1);
        }
    }

    // Creación de procesos hijos
    for (i = 0; i < n; i++) {
        if ((pid = fork()) == -1) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            if (i > 0) {
                dup2(pipefds[(i-1) * 2], 0); // Redirige entrada estándar
            }
            if (i < n - 1) {
                dup2(pipefds[i*2 + 1], 1); // Redirige salida estándar
            }

            // Cierra descriptores de archivo en child process
            for (int j = 0; j < 2*(n-1); j++) {
                close(pipefds[j]);
            }

            // Prepara args para execvp
            char *args[1024];
            int j = 0;
            char *token = strtok(commands[i], " \n");
            while (token != NULL) {
                args[j++] = token;
                token = strtok(NULL, " \n");
            }
            args[j] = NULL;

            // Ejecución
            execvp(args[0], args);
            perror("execvp");
            exit(1);
        }
    }

    // Cierra descriptores de archivo en proceso padre
    for (i = 0; i < 2 * (n-1); i++) {
        close(pipefds[i]);
    }

    // Wait a los hijos
    for (i = 0; i < n; i++) {
        wait(NULL);
    }
}

// Sujeto a ser cambiado a un string dinámico
char mensaje[256];
void sig_handler(int sig) {
    printf("\n%s\n", mensaje);
    printDirectory();
    exit(0);
}

int main(int argc, char **argv) {
    (void)argc, (void)argv; // void para evitar problemas al compilar
    char *buffer = NULL; // puntero al string del usuario
    size_t bufferSize = 0; // size para asignar espacio en memoria de buffer
    ssize_t bufferRead; // ssize para almacenar cc de caracteres leídos
    pid_t c_pid; // ID del hijo
    int status; // status del hijo
    char **array; // puntero al array que almacena el comando y argumentos
    char *fullCommand; // puntero que almacena el comando completo

    FILE *fileTemp = fopen("misfavoritostemp.txt", "a");
    fclose(fileTemp);

    FILE *file = fopen("misfavoritos.txt", "a");
    fclose(file);

    welcome(); // mensaje de bienvenida

    while (1) {
        printDirectory(); // imprime directorio actual

        int command_executed = 0; // variable auxiliar para verificar que un comando ha sido ejecutado con exito

        bufferRead = getline(&buffer, &bufferSize, stdin); // almacena bufferSize y lee buffer
        fullCommand = strdup(buffer); // guarda copia del comando completo
        if (bufferRead == -1) {
            perror("Error reading. Exiting shell.");
            free(buffer);
            free(fullCommand);
            exit(1);
        }

        // Asignación de memoria para los arrays de comandos y argumentos
        array = malloc(sizeof(char*) * 1024);
        if (array == NULL) {
            perror("Malloc error. Exiting shell.");
            free(buffer);
            free(fullCommand);
            exit(1);
        }

        int i = 0;
        int j = 0;
        int es_pipe = 0;

        char *token = strtok(buffer, " \n"); // Divide string buffer en tokens, usa " " como delimitador y \n
        while (token) {
            if (strcmp(token, "|") == 0) {
                array[i] = NULL;
                es_pipe = 1;
                token = strtok(NULL, " \n");
                continue;
            }

            if (es_pipe) {
                array[j++] = token;
            } else {
                array[i++] = token;
            }
            token = strtok(NULL, " \n");
        }

        array[i] = NULL; 

        if (array[0] == NULL) { // Caso no se ingresa nada
            printf("Please, enter a command.\n");
            free(array);
            free(fullCommand);
            continue;
        }

        // Manejando comandos favoritos
        if (strcmp(array[0], "set") == 0) {
            if (array[1] == NULL) {
                printf("Error, se requieren más argumentos.\n");
            } else if (strcmp(array[1], "recordatorio") == 0) {
                if (array[2] != NULL) {
                    char *str = array[2];
                    int tiempo;
                    int success = 0;
                    for (int i = 0; str[i] != '\0'; i++) {
                        if (!isdigit(str[i])) {
                            printf("Error, se requiere caracter numérico.\n");
                            success = 1;
                            break;
                        }
                    }
                    if (success == 0) {
                        tiempo = atoi(array[2]);
                        if (array[3] != NULL) {
                            char str1[256]; 
                            memset(mensaje, '\0', sizeof(mensaje));
                            memset(str1, '\0', sizeof(str1));
                            for (int j = 0; array[j + 3] != NULL; j++) {
                                strcat(str1, array[j + 3]);
                                if (array[j + 4] != NULL) {
                                    strcat(str1, " ");
                                }
                            }
                            if (str1[0] == '"' && str1[strlen(str1) - 1] == '"' && strlen(str1) > 2) {
                                strncpy(mensaje, str1 + 1, strlen(str1) - 2);
                                printf("Recordatorio '%s', para %d seg.\n", mensaje, tiempo);
                                // Se crea un hijo para esperar la alarma, el padre continúa su respectivo proceso
                                if ((fork()) == 0) {
                                    signal(SIGALRM, sig_handler);
                                    alarm(tiempo);
                                    while (1) {
                                        ;
                                    }
                                }
                            } else {
                                printf("Error, no se escribió correctamente el mensaje.\n");
                                memset(mensaje, '\0', sizeof(mensaje));
                            }
                        } else {
                            printf("Error, se requiere un 'mensaje'.\n");
                        }
                    }
                } else {
                    printf("Error, se requiere un número para el tiempo.\n");
                }
            } else {
                printf("Error, argumento no reconocido.\n");
            }
            continue;
        }

        // Comandos favs
        if (strcmp(array[0], "favs") == 0) {
            if (array[1] != NULL && strcmp(array[1], "mostrar") == 0) {
                FILE *file = fopen("misfavoritostemp.txt", "r");
                char line[256]; 
                int counter = 1;
                while (fgets(line, sizeof(line), file)) { 
                    printf("%d) %s", counter, line);
                    counter++;
                }
                fclose(file);
                continue;
            } else if (array[1] != NULL && strcmp(array[1], "buscar") == 0) {
                if (array[2] == NULL) {
                    perror("Error cmd null");
                } else {
                    char line[256];
                    int counter = 1;
                    FILE *file = fopen("misfavoritos.txt", "r");
                    while (fgets(line, sizeof(line), file)) { 
                        if (strstr(line, array[2]) != NULL) {
                            printf("%d) %s", counter, line);
                        }
                        counter++;
                    }
                    fclose(file);
                    continue;
                }
            } else if (array[1] != NULL && strcmp(array[1], "guardar") == 0) {
                FILE *file = fopen("misfavoritos.txt", "a+");
                FILE *fileTemp = fopen("misfavoritostemp.txt", "r");
                char line[256];
                char line2[256];
                int write = 1;
                int counter = 0;

                while (fgets(line, sizeof(line), fileTemp)) {
                    rewind(file);
                    while (fgets(line2, sizeof(line2), file)) {
                        if (strcmp(line2, line) == 0) {
                            write = 0;
                        }
                    }
                    if (write == 1) {
                        fprintf(file, "%s", line);
                    }
                    write = 1;
                    counter++;
                }
                fclose(file);
                fclose(fileTemp);
                continue;
            } else if (array[1] != NULL && strcmp(array[1], "cargar") == 0) {
                FILE *file = fopen("misfavoritos.txt", "r");
                char line[256]; 
                int counter = 1;
                while (fgets(line, sizeof(line), file)) { 
                    printf("%d) %s", counter, line);
                    counter++;
                } 
                fclose(file);
                continue;
            } else if (array[1] != NULL && strcmp(array[1], "borrar") == 0) {
                FILE *file = fopen("misfavoritos.txt", "w");
                fclose(file);
                continue;
            } else if (array[1] != NULL && strcmp(array[1], "ejecutar") == 0) {
                if (array[2] == NULL) {
                    perror("Error, third argument needed");
                } else {
                    char **arrayExec;
                    char line[256];
                    arrayExec = malloc(sizeof(char*) * 1024);   
                    int i = 0;
                    int counter = 1;

                    FILE *file = fopen("misfavoritos.txt", "r");

                    while (fgets(line, sizeof(line), file)) {
                        if (atoi(array[2]) == counter) {
                            char *token = strtok(line, " \n");  
                            while (token) { 
                                arrayExec[i++] = token;
                                token = strtok(NULL, " \n");
                            }
                            arrayExec[i] = NULL; 
                            free(array);
                            array = arrayExec;
                            break;
                        }
                        counter++;
                    }
                    fclose(file);
                }
            }
        }

        // Comando cd
        if (strcmp(array[0], "cd") == 0) {
            if (array[1] == NULL || strcmp(array[1], "~") == 0) {
                const char *home = getenv("HOME");
                if (home == NULL) {
                    fprintf(stderr, "Error: HOME not found.\n");
                } else {
                    if (chdir(home) != 0) {
                        perror("Error in cd command.");
                    }
                }
            } else {
                if (chdir(array[1]) != 0) {
                    perror("Error in cd command");
                }
            }
            free(array);
            continue; // Skip fork() si el comando es cd
        }

        // Comando exit
        if (strcmp(array[0], "exit") == 0) {
            FILE *fileTemp = fopen("misfavoritostemp.txt", "w"); 
            fclose(fileTemp);
            free(array);
            exit(0);
        }

        // Comandos con pipes
        if (strchr(fullCommand, '|')) {
            // Dividir comandos por el pipe
            char *commands[1024];
            int command_count = 0;

            char *cmd = strtok(fullCommand, "|");
            while (cmd != NULL) {
                commands[command_count++] = cmd;
                cmd = strtok(NULL, "|");
            }
            commands[command_count] = NULL;
            executePipes(commands);
            command_executed = 1;
        }

        else {
            // Proceso sin pipes
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
                waitpid(c_pid, &status, 0);

                if (WIFEXITED(status)) {
                    int exit_status = WEXITSTATUS(status);
                    if (exit_status == 0) {
                        command_executed = 1;
                    }
                }
            }
        }

        if (command_executed){
            char line[256];
            char write = 1;

            FILE *fileTemp = fopen("misfavoritostemp.txt", "r");
            while (fgets(line, sizeof(line), fileTemp)){ 
                if(strcmp(line, fullCommand) == 0){
                    write = 0;
                }
            }

            fclose(fileTemp);

            if (write && strcmp(array[0], "favs") != 0) {
                // Escribir el comando en mis favoritos
                FILE *fileTemp = fopen("misfavoritostemp.txt", "a");
                fprintf(fileTemp, "%s", fullCommand);
                fclose(fileTemp);
            }
        }

        free(array);
    }
    free(buffer);
    return 0;
}
