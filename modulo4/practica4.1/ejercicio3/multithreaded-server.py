import socket
import pandas as pd
import threading
import io

# --- Configuración ---
HOST = '0.0.0.0'
PORT = 10000
DATA_FILE = 'temperatures.csv'
VALID_CITIES = ['Madrid', 'Paris', 'Berlin', 'Rome']
BUFFER_SIZE = 1024
# --- Fin Configuración ---


def load_data(filename):
    """"""
    try:
        df = pd.read_csv(filename)
        df['datetime'] = pd.to_datetime(df['datetime'])
        print(f"Datos cargados y preparados desde {filename}")
        return df
    except Exception as e:
        print(f"Error inesperado al cargar los datos: {e}")
        exit(1)

def process_request(request_str, df):
    """Procesa una petición del cliente y devuelve la respuesta apropiada."""
    request_str = request_str.strip()
    # Log se hace en handle_client ahora para incluir el thread id/addr
    # print(f"Received: {request_str}")

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

def handle_client(conn, addr, df):
    """Función ejecutada por cada hilo para manejar un cliente."""
    print(f"[Thread {threading.get_ident()}] Handling connection from {addr}")

    with conn: # Asegura conn.close() al final
        while True:
            try:
                data = conn.recv(BUFFER_SIZE)
                if not data:
                    print(f"[Thread {threading.get_ident()}] Client {addr} disconnected.")
                    break

                request_str = data.decode('utf-8')
                print(f"[Thread {threading.get_ident()}] Received from {addr}: {request_str.strip()}")

                # El DataFrame (df) es compartido (pero solo se lee, lo cual es seguro)
                response_str = process_request(request_str, df)

                if response_str == "END":
                    print(f"[Thread {threading.get_ident()}] Client {addr} requested END. Closing connection.")
                    break

                conn.sendall(response_str.encode('utf-8'))

            except socket.error as e:
                print(f"[Thread {threading.get_ident()}] Socket error with {addr}: {e}")
                break
            except Exception as e:
                print(f"[Thread {threading.get_ident()}] Error processing request from {addr}: {e}")
                try:
                    conn.sendall(b"Error: Server error during processing.")
                except socket.error:
                    pass
                break

    print(f"[Thread {threading.get_ident()}] Connection with {addr} closed.")

def main():
    # Cargar los datos UNA VEZ al inicio (serán compartidos por los hilos)
    temperature_df = load_data(DATA_FILE)

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        server_socket.bind((HOST, PORT))
        server_socket.listen(10) # Aumentar backlog si se esperan muchas conexiones simultáneas
        print(f"Multithreaded server listening on {HOST}:{PORT}")

        while True:
            # Aceptar nueva conexión
            conn, addr = server_socket.accept()
            print(f"Accepted connection from {addr}")

            # Crear un nuevo hilo para manejar esta conexión
            # Pasamos el DataFrame cargado al hilo
            client_thread = threading.Thread(target=handle_client, args=(conn, addr, temperature_df))
            client_thread.daemon = True # Permite que el programa principal termine aunque haya hilos corriendo
            client_thread.start()
            # El bucle principal NO espera, vuelve inmediatamente a aceptar()

    except socket.error as e:
        print(f"Socket error on setup: {e}")
    except Exception as e:
        print(f"An unexpected server error occurred: {e}")
    finally:
        print("\nShutting down server.")
        server_socket.close()

if __name__ == "__main__":
    main()