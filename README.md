# Proyecto-1-SistemasOperativos

## Instrucciones de uso:

Para inicializar la shell en sí, compilamos con la siguiente línea de comando: *“gcc main.c -o shell && ./shell”*, tomando el archivo main.c como fuente, “shell” como nombre del archivo de salida ejecutable, y “./shell” para inicializar. Así, al ejecutarse, la shell imprimirá un mensaje de bienvenida junto al nombre del usuario.

Ya la shell iniciada proceda a escribir el comando deseado y presione “enter” para que se ejecute, una vez realizado la shell volverá a esperar a instrucciones, además la terminal soporta el uso de pipes, por lo que podrás usar el resultado de un comando como argumento para otros, si se desea salir de la shell  el escriba el comando “exit”. 


Comandos soportados :
int representa un argumento de tipo número  entero.
string representa un argumento de tipo string.

Los comandos aceptados en una terminal por default.
 - set recordatorio int “string”: Permite poner un recordatorio en int segundos que al salir mostrará el mensaje string.
 - favs mostrar: Muestra los comandos que han sido ejecutados en la sección activa de la Shell a forma de lista indexada. 
 - favs cargar: Muestra los comandos que han sido ejecutados a lo largo de todas las secciones de la Shell en forma de lista indexada. 
 - favs borrar: Borra todos los comandos de “misfavoritos.txt” qué es el archivo con los comandos que se mostrarían al ejecutar “favs cargar”. 
 - favs guardar: Guarda los comandos ejecutados en la sección actual en el archivo “misfavoritos.txt” a menos que estos ya estuvieran previamente. 
 - favs buscar string: Comando para buscar comandos guardados en favoritos que posean un substring dado. Si se encuentra se imprime en la terminal el comando junto al número de identificación respectivo a este.
 - favs eliminar num: Elimina de “misfavoritos.txt” al comando en el índice int. Se pueden poner varios int separados por un espacio. 
 - favs ejecutar num: Se ejecuta el comando en el indice int de “misfavoritos.txt”. 


