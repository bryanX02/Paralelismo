import socket
import pandas as pd
import io # Para manejar el string del CSV directamente

# --- Configuración ---
HOST = '0.0.0.0'  # Escuchar en todas las interfaces disponibles
PORT = 10000      # Puerto para el servicio
DATA_FILE = 'temperatures.csv' # Nombre del fichero de datos
VALID_CITIES = ['Madrid', 'Paris', 'Berlin', 'Rome']
BUFFER_SIZE = 1024 # Tamaño del buffer para recibir datos

# Metodo que carga los datos desde el fichero CSV y los prepara.
def load_data(filename):
    
    try:
        df = pd.read_csv(filename)
        # Convertir la columna 'datetime' a objetos datetime
        df['datetime'] = pd.to_datetime(df['datetime'])
        print(f"Datos cargados y preparados desde {filename}")
        return df
    except FileNotFoundError:
        print(f"Error: El fichero {filename} no se encontró.")
        # Podríamos crear un DataFrame vacío o salir
        # Por simplicidad, salimos si el fichero no existe
        exit(1)

# Metodo que procesa la petición del cliente.
def process_request(request_str, df):
    
    request_str = request_str.strip()
    print(f"Received: {request_str}") # Log petición recibida

    if request_str == "END":
        return "END" # Señal para terminar la conexión con este cliente

    parts = request_str.split(':')
    if len(parts) != 2:
        return "Error: Formato incorrecto. Use 'Ciudad:Mes'\n."

    city, month_str = parts
    city = city.strip()

    if city not in VALID_CITIES:
        return f"Error: Unknown city '{city}'. Ciudades válidas: {VALID_CITIES}\n"

    try:
        month = int(month_str.strip())
        if not (1 <= month <= 12):
            return f"Error: Wrong month '{month}'. El mes debe estar entre 1 y 12.\n"
    except ValueError:
        return f"Error: Mes inválido '{month_str}'. Debe ser un número.\n"

    # Filtramos el DataFrame para la ciudad y el mes especificados (asumiendo año 2024 implícito o único)
    # Si el CSV tuviera múltiples años, habría que añadir .dt.year == 2024
    filtered_df = df[(df['name'] == city) & (df['datetime'].dt.month == month)]

    if filtered_df.empty:
        return f"Error: No hay datos para {city} en el mes {month}.\n"
    else:
        # Calcular la temperatura media
        mean_temp = filtered_df['temp'].mean()
        return f"{mean_temp}°C\n" # Devolver con 1 decimal
        #return f"{mean_temp:.1f}°C\n" # Devolver con 1 decimal

def main():

    # Cargamos los datos UNA VEZ al inicio
    temperature_df = load_data(DATA_FILE)

    # Creamos el socket del servidor TCP/IP
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        # Vinculamos el socket al host y puerto
        server_socket.bind((HOST, PORT))

        # Empezamos a escuchar conexiones entrantes (cola de espera de 5)
        server_socket.listen(5)
        print(f"Server listening on {HOST}:{PORT}")

        # Bucle principal para aceptar conexiones
        while True:
            print("\nWaiting for a new connection...")
            # Se acepta una nueva conexión (bloqueante)
            conn, addr = server_socket.accept()
            print(f"Accepted connection from {addr}")

            with conn:
                print(f"Handling connection from {addr}")
                # Bucle para manejar peticiones del cliente conectado actualmente
                while True:
                    try:
                        data = conn.recv(BUFFER_SIZE)
                        if not data:
                            print(f"Client {addr} disconnected.")
                            break

                        request_str = data.decode('utf-8')
                        response_str = process_request(request_str, temperature_df)

                        if response_str == "END":
                             print(f"Client {addr} requested END. Closing connection.")
                             break

                        conn.sendall(response_str.encode('utf-8'))

                    except socket.error as e:
                        print(f"Socket error during communication with {addr}: {e}")
                        break 
                    except Exception as e:
                        print(f"Error processing request from {addr}: {e}")
                     
                        try:
                           conn.sendall(b"Error: Server error during processing.")
                        except socket.error:
                           pass
                        break 
                    
            print(f"Connection with {addr} closed.")
            # El servidor básico vuelve a esperar una nueva conexión (atiende una a la vez)

    except socket.error as e:
        print(f"Socket error on setup: {e}")
    except Exception as e:
        print(f"An unexpected server error occurred: {e}")
    finally:
        # Cerrar el socket del servidor al terminar (p.ej., con Ctrl+C)
        print("\nShutting down server.")
        server_socket.close()

if __name__ == "__main__":
    main()