PARTE A


Cuando dice alternadamente ("Estos dos hilos insertarán los numeros 1..N de forma alternada en el vector compartido"),
se refiere a hilo 1, 2, 1, 2, o a que el hilo 1 puede insertar todos los pares y a la vez puede hacerlo el hilo 2 ?? Entiendo que la primera pero si entonces es de la misma forma que en la parte B, no? No entiendo la utilidad de los semaforos porque nunca van a acceder al mismo recurso al mismo tiempo, al acceder a recursos diferentes.


(*)    Por esto no se si cada hilo debe de tener una funcion propia donde se le especifique que tiene que acceder solo a los impares/pares o ambos hilos deben de tener la misma funcion que mediante semaforos permita bloquear el acceso cuando el otro se encuentra accediendo al recurso


PARTE B 

La pregunta es similar al parrafo (*) puesto que no se si se necesita solo una funcion para todos los hilos... Y tampoco si hay que llevar un contador de por que elemento vamos, o como porque no entiendo el concepto de los semaforos en este problema si no se debe acceder al mismo recurso compartido