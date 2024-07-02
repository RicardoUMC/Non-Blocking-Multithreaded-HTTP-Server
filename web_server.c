#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/time.h>
#include <errno.h>

#define PORT 8080
#define THREAD_POOL_SIZE 4
#define BUFFER_SIZE 1024
#define RESOURCE_DIR "./"
#define MAX_FD_SET_SIZE 256

// Estructura para los argumentos del hilo
typedef struct {
    int socket;
} task_t;

// Cola para almacenar tareas
task_t task_queue[256];
int queue_size = 0;
int queue_capacity = 256;
int front = 0;
int rear = -1;

// Mutex y variables de condición para sincronizar la cola
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void enqueue(int client_socket) {
    pthread_mutex_lock(&lock);
    if (queue_size < queue_capacity) {
        rear = (rear + 1) % queue_capacity;
        task_queue[rear].socket = client_socket;
        queue_size++;
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&lock);
}

task_t dequeue() {
    pthread_mutex_lock(&lock);
    while (queue_size == 0) {
        pthread_cond_wait(&cond, &lock);
    }
    task_t task = task_queue[front];
    front = (front + 1) % queue_capacity;
    queue_size--;
    pthread_mutex_unlock(&lock);
    return task;
}

void send_http_response(int client_fd, const char* path) {
    char full_path[512];
    sprintf(full_path, "%s%s", RESOURCE_DIR, path);

    int file_fd = open(full_path, O_RDONLY);
    struct stat file_stat;
    if (file_fd == -1 || fstat(file_fd, &file_stat) == -1) {
        char* response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        send(client_fd, response, strlen(response), 0);
    } else {
        char header[1024];
        sprintf(header, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", file_stat.st_size);
        send(client_fd, header, strlen(header), 0);
        sendfile(client_fd, file_fd, NULL, file_stat.st_size);
        close(file_fd);
    }
}

void *handle_request(void *arg) {
    while (1) {
        task_t task = dequeue();
        int client_socket = task.socket;

        char buffer[BUFFER_SIZE];
        int read_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (read_bytes > 0) {
            buffer[read_bytes] = '\0';
            char method[10], path[255];
            sscanf(buffer, "%s %s", method, path);

            if (strcmp(method, "GET") == 0) {
                send_http_response(client_socket, path);
            } else {
                char* response = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\n\r\n";
                send(client_socket, response, strlen(response), 0);
            }
        }
        close(client_socket);
    }
    return NULL;
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(server_fd, F_SETFL, O_NONBLOCK); // Set server socket to non-blocking
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    pthread_t threads[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&threads[i], NULL, handle_request, NULL);
    }

    printf("Servidor listo en puerto %d\n", PORT);

    fd_set read_fds;
    int max_sd, activity;
    struct timeval timeout;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        max_sd = server_fd;

        timeout.tv_sec = 1;  // Timeout after 5 seconds
        timeout.tv_usec = 0;

        activity = select(max_sd + 1, &read_fds, NULL, NULL, &timeout);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
            continue;
        }

        if (FD_ISSET(server_fd, &read_fds)) {
            client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (client_socket > 0) {
                fcntl(client_socket, F_SETFL, O_NONBLOCK); // Set client socket to non-blocking
                enqueue(client_socket);
            }
        }
    }

    return 0;
}

