#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

#define PORT 8080
#define THREAD_POOL_SIZE 4
#define BUFFER_SIZE 1024
#define RESOURCE_DIR "resources/"

// Estructura para los argumentos del hilo
typedef struct {
    int socket;
} thread_args;

// Función para extraer el método y la ruta de la solicitud HTTP
void parse_http_request(const char* request, char* method, char* path) {
    sscanf(request, "%s %s", method, path);
}

// Función para enviar respuestas HTTP con contenido de archivo
void send_http_response(int client_fd, const char* path, const char* method) {
    char full_path[512];
    sprintf(full_path, "%s%s", RESOURCE_DIR, path);

    int file_fd = open(full_path, O_RDONLY);
    struct stat file_stat;
    if (file_fd == -1 || fstat(file_fd, &file_stat) == -1) {
        char* header = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        send(client_fd, header, strlen(header), 0);
    } else {
        char header[1024];
        sprintf(header, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", file_stat.st_size);
        send(client_fd, header, strlen(header), 0);

        if (strcmp(method, "HEAD") != 0) { // No enviar el cuerpo para HEAD
            sendfile(client_fd, file_fd, NULL, file_stat.st_size);
        }
    }
    close(file_fd);
}

// Función para manejar las peticiones HTTP
void* handle_request(void* args) {
    thread_args* t_args = (thread_args*) args;
    int client_socket = t_args->socket;
    free(t_args);

    char buffer[BUFFER_SIZE];
    int read_bytes = read(client_socket, buffer, BUFFER_SIZE - 1);
    buffer[read_bytes] = '\0';

    char method[10], path[255];
    parse_http_request(buffer, method, path);

    if (strcmp(method, "GET") == 0 || strcmp(method, "HEAD") == 0) {
        send_http_response(client_socket, path, method);
    } else {
        char* response = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
    return NULL;
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t threads[THREAD_POOL_SIZE];

    // Setup server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket creation");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    while(1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        thread_args *t_args = malloc(sizeof(thread_args));
        t_args->socket = client_socket;

        pthread_create(&threads[client_socket % THREAD_POOL_SIZE], NULL, handle_request, (void*) t_args);
    }

    return 0;
}

