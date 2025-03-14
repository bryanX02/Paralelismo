1
- 
Al usar la opción -x del programa, el comando indicado como argumento se pasa encerrado entre comillas dobles en el caso de que este, a su vez, acepte argumentos, como por ejemplo ls -l. ¿Qué ocurre si el argumento de -x no se pasa entrecomillado? ¿Funciona correctamente el lanzamiento del programa ls -l si se encierra entre comillas simples en lugar de dobles? Nota: Para ver las diferencias prueba a ejecutar el siguiente comando: echo $HOME


-----------
No funciona correctamente, porque optarg no lo considera como un mismo argumento, entonces se le pasa a la funcion parse_commands solo el primer elemento del comando, en el caso de ls -l se le pasa solo ls. Por tanto launch_command ejecuta ls pero sin el -l. Luego salta un error puesto que getopt no sabe identificar -l.

Con echo $HOME tampoco funciona; no printea nada porque no se le pasa nada junto al echo en el comando (no captura el $HOME)

2
-
¿Es posible utilizar execlp() en lugar de execvp() para ejecutar el comando pasado como parámetro a la función launch_command() ? En caso afirmativo, indica las posibles limitaciones derivadas del uso de execlp() en este contexto.

--------------

(Sin comprobar)

Sí que es posible pero tendría más limitaciones porque al no pasarse una lista de argumentos, habria que pasarselos todos manualmente. Como no puede saber de manera sencilla cuántos argumentos tiene el comando (hay comandos de muchas longitudes diferentes), habría que hacer una comprobación extra.


3
-
¿Qué ocurre al ejecutar el comando "echo hello > a.txt" con ./run_commands -x ? ¿y con el comando "cat run_commands.c | grep int" ? En caso de que los comandos no se ejecuten correctamente indica el motivo.

---------------

Ninguno de los dos comandos se ejecutan como se desea. 

En el caso de ./run_commands -x "echo hello > a.txt" se muestra por pantalla 

hello > a.txt

indicando que echo ha printeado ese mensaje pero no lo ha ejecutado según el propósito general que es volcar "hello" en un archivo "a.txt".

Lo mismo sucede con el caso ./run_commands -x "cat run_commands.c | grep int" que muestra por pantalla todo el codigo de run_commands.c pero no muestra solo las partes del cóigo donde coincide la expresion regular "int".

