#!/usr/bin/env python3
import sys
import socket
import threading
import random
import time

# Lista de ciudades válidas y meses válidos
CITIES = ['Madrid', 'Rome', 'Berlin', 'Paris']
MONTHS = list(range(1, 13))


# Variables compartidas (añadir lo necesario)
finished = False
request_count = 0
# Lock para proteger el acceso a request_count
request_lock = threading.Lock()

def generate_random_request():
    city = random.choice(CITIES)
    month = random.randint(1, 12)
    return f"{city}:{month}"

def client_thread(host, port, delay_ms):
    global finished, request_count

    thread_id = threading.get_ident() # Obtener ID del hilo para depuración
    print(f"Hilo {thread_id}: Arrancado. Conectando a {host}:{port}")

    # Convertir delay de milisegundos a segundos
    delay_sec = delay_ms / 1000.0

    sock = None # Inicializar socket a None

    try:
        # Creamos el socket TCP/IP y la conexión
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, port))
        print(f"Hilo {thread_id}: Conectado.")

        while not finished:
            # Generamos una petición aleatoria
            request_str = generate_random_request()
            
            try:
                # Enviamos petición al servidor (codificada en bytes)
                print(f"Hilo {thread_id}: Enviando '{request_str}'") # Depuracion
                sock.sendall(request_str.encode('utf-8'))

                # Recibimos respuesta del servidor
                response = sock.recv(1024)

                if not response:
                    print(f"Hilo {thread_id}: Servidor cerró conexión.")
                    break 

                # Decodificamos la respuesta (depuracion)
                response_str = response.decode('utf-8')
                print(f"Hilo {thread_id}: Recibido '{response_str}'")

                # Incrementamos el contador request_count de forma segura usando el lock
                with request_lock:
                    request_count += 1

                time.sleep(delay_sec)

            except socket.error as e:
                print(f"Hilo {thread_id}: Error de socket durante comunicación: {e}")
            except Exception as e:
                print(f"Hilo {thread_id}: Error inesperado durante comunicación: {e}")

    except socket.error as e:
        print(f"Hilo {thread_id}: Error de socket al conectar: {e}")
    except Exception as e:
        print(f"Hilo {thread_id}: Error inesperado: {e}")
    finally:
        # Siempre hay que cerrar el socket al finalizar o en caso de error
        if sock:
            print(f"Hilo {thread_id}: Cerrando socket.")
            sock.close()
        print(f"Hilo {thread_id}: Terminado.")


def main():
    global finished, request_count
   
    # Validar número de argumentos
    if len(sys.argv) != 5:
        print("Uso: python3 request-generator.py <host_servidor:puerto> <T> <N> <M>")
        print("Ejemplo: python3 request-generator.py localhost:10000 20 10 100")
        sys.exit(1) 
    
    # Parsear argumentos

    try:
        host_port_str = sys.argv[1]
        if ':' not in host_port_str:
            raise ValueError("El formato debe ser host:puerto")
        host, port_str = host_port_str.split(':', 1)
        port = int(port_str)
        if not (0 < port < 65536):
             raise ValueError("Puerto inválido.")

        # Parsear T (duración), N (hilos), M (delay ms)
        T = int(sys.argv[2]) # Duración en segundos
        N = int(sys.argv[3]) # Número de hilos cliente
        M = int(sys.argv[4]) # Retardo en milisegundos

        if T <= 0 or N <= 0 or M < 0:
            raise ValueError("T y N deben ser > 0, M debe ser >= 0.")

    except ValueError as e:
        print(f"Error en los argumentos: {e}")
        sys.exit(1)
    except Exception as e:
         print(f"Error procesando argumentos: {e}")
         sys.exit(1)


    print(f"Iniciando generador de peticiones a {host}:{port}")
    print(f"Duración (T): {T} segundos")
    print(f"Número de clientes (N): {N} hilos")
    print(f"Retardo entre peticiones (M): {M} ms")

    ## Registrar tiempo inicial
    start_time = time.time()
    
    # Lanzar hilos cliente
    threads = []
    for i in range(N):
        thread = threading.Thread(target=client_thread, args=(host, port, M))
        threads.append(thread)
        thread.start()
        print(f"Hilo cliente {i+1}/{N} lanzado.")
  
    # Esperar T segundos e indicar terminacion
    print(f"Generador ejecutándose durante {T} segundos...")
    time.sleep(T)
    print("Tiempo cumplido. Señalizando finalización a los hilos...")
    finished = True

    # Esperar a que todos los hilos terminen
    print("Esperando a que los hilos terminen...")
    for i, thread in enumerate(threads):
        thread.join()
        # print(f"Hilo {i+1}/{N} finalizado.") # Descomentar para ver cuándo termina cada hilo

    print("Todos los hilos han terminado.")
    
    ## Registrar tiempo final y tiempo transcurrido
    end_time = time.time()
    duration = end_time - start_time
    
    ## Calcular tasa de peticiones por segundo ...
    if duration > 0:
        rate = request_count / duration
    else:
        rate = 0 # O manejar como indefinido si T fue 0

    # Imprimimos resultados
    print("\n--- Resultados ---")
    print(f"Peticiones totales: {request_count}")
    # Usamos :.2f para mostrar la duración y la tasa con 2 decimales
    print(f"Duración: {duration:.2f} segundos")
    print(f"Tasa de peticiones: {rate:.2f} peticiones/segundo")

if __name__ == "__main__":
    main()
