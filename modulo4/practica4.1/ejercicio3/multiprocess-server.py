import socket
import pandas as pd
import multiprocessing
import io
import os

# --- Configuración ---
HOST = '0.0.0.0'
PORT = 10000
DATA_FILE = 'temperatures.csv'
VALID_CITIES = ['Madrid', 'Paris', 'Berlin', 'Rome']
BUFFER_SIZE = 1024
# --- Fin Configuración ---

# Carga los datos desde el fichero CSV y los prepara.
def load_data(filename):
    try:
        df = pd.read_csv(filename)
        df['datetime'] = pd.to_datetime(df['datetime'])
        # No imprimimos aquí, se haría en cada proceso
        return df
    except Exception as e:
        print(f"[Process {os.getpid()}] Error inesperado al cargar los datos: {e}")
        exit(1)

# Procesa una petición del cliente y devuelve la respuesta apropiada.
def process_request(request_str, df):

    request_str = request_str.strip()

    if request_str == "END":
        return "END"

    parts = request_str.split(':')
    if len(parts) != 2:
        return "Error: Formato incorrecto. Use 'Ciudad:Mes'."

    city, month_str = parts
    city = city.strip()

    if city not in VALID_CITIES:
        return f"Error: Unknown city '{city}'. Ciudades válidas: {VALID_CITIES}"

    try:
        month = int(month_str.strip())
        if not (1 <= month <= 12):
            return f"Error: Wrong month '{month}'. El mes debe estar entre 1 y 12."
    except ValueError:
        return f"Error: Mes inválido '{month_str}'. Debe ser un número."

    filtered_df = df[(df['name'] == city) & (df['datetime'].dt.month == month)]

    if filtered_df.empty:
        return f"Error: No hay datos para {city} en el mes {month}."
    else:
        mean_temp = filtered_df['temp'].mean()
        return f"{mean_temp:.1f}°C"
        
# Función ejecutada por cada PROCESO para manejar un cliente.
def handle_client(conn, addr, data_filename):

    pid = os.getpid()
    print(f"[Process {pid}] Handling connection from {addr}")

    # Cada proceso carga SU PROPIA copia de los datos ---
    try:
        print(f"[Process {pid}] Loading data from {data_filename}...")
        df = load_data(data_filename)
    except SystemExit: # Captura exit() llamado desde load_data
         print(f"[Process {pid}] Failed to load data. Closing connection.")
         conn.close() # Asegurarse de cerrar el socket si falla la carga
         return # Terminar la ejecución de este proceso hijo

    with conn:
        while True:
            try:
                data = conn.recv(BUFFER_SIZE)
                if not data:
                    print(f"[Process {pid}] Client {addr} disconnected.")
                    break

                request_str = data.decode('utf-8')
                print(f"[Process {pid}] Received from {addr}: {request_str.strip()}")

                # Procesar usando la copia local del DataFrame (df)
                response_str = process_request(request_str, df)

                if response_str == "END":
                    print(f"[Process {pid}] Client {addr} requested END. Closing connection.")
                    break

                conn.sendall(response_str.encode('utf-8'))

            except Exception as e:
                print(f"[Process {pid}] Error processing request from {addr}: {e}")

    print(f"[Process {pid}] Connection with {addr} closed.")
    # El proceso hijo termina aquí

def main():
    # El proceso principal NO carga los datos, solo verifica que el fichero existe
    if not os.path.exists(DATA_FILE):
        print(f"Error: Data file '{DATA_FILE}' not found in main process. Exiting.")
        exit(1)
    print(f"Data file '{DATA_FILE}' found.")


    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        server_socket.bind((HOST, PORT))
        server_socket.listen(10)
        print(f"Multiprocess server listening on {HOST}:{PORT}")

        active_processes = [] # Opcional: llevar registro de procesos hijos

        while True:
            # Aceptamos nueva conexión
            conn, addr = server_socket.accept()
            print(f"Accepted connection from {addr}")

            # Creamos un nuevo PROCESO para manejar esta conexión
            # Pasamos la RUTA del fichero de datos, no el DataFrame
            client_process = multiprocessing.Process(target=handle_client, args=(conn, addr, DATA_FILE))
            client_process.daemon = True
            client_process.start()

            active_processes.append(client_process)

            # IMPORTANTE: El proceso padre debe cerrar SU copia del socket de conexión.
            # El proceso hijo tiene su propia copia del descriptor de fichero.
            conn.close()

            # Limpiamos lista de procesos terminados para evitar que crezca indefinidamente
            active_processes = [p for p in active_processes if p.is_alive()]

    except Exception as e:
        print(f"An unexpected server error occurred: {e}")
    finally:
        print("Closing server socket.")
        server_socket.close()

if __name__ == "__main__":
    main()