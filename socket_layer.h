#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else // Unix-like systems
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif
typedef struct {
    int sockfd;
    struct sockaddr_in server_addr;
} Socket;

Socket* create_socket(void);
int connect_to_server(Socket* socket, const char* server_ip, int server_port);
int send_data(Socket* socket, const char* data);
int receive_data(Socket* socket, char* buffer, size_t buffer_size);
void close_socket(Socket* socket);
Socket* create_server_socket(int port);
int listen_for_connections(Socket* server_socket, int backlog);
Socket* accept_connection(Socket* server_socket);
void send_data_s(Socket *socket,const char *msg); 
#ifdef SOCKET_INIT
#ifdef _WIN32
void init_socket_library() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("Failed to initialize socket library");
        exit(1);
    }
}
#endif

// Function to create a new socket
Socket* create_socket(void) {
    Socket* csocket = (Socket*)malloc(sizeof(Socket));
    if (socket == NULL) {
        perror("Failed to allocate memory for socket");
        exit(1);
    }

    // Initialize the socket library (Windows-only)
#ifdef _WIN32
    init_socket_library();
#endif

    // Create a socket
    csocket->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (csocket->sockfd == -1) {
        perror("Failed to create socket");
        free(socket);
        exit(1);
    }

    return csocket;
}

// Function to connect to a remote server
int connect_to_server(Socket* socket, const char* server_ip, int server_port) {
    memset(&socket->server_addr, 0, sizeof(socket->server_addr));
    socket->server_addr.sin_family = AF_INET;
    socket->server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &socket->server_addr.sin_addr) <= 0) {
        perror("Invalid server address");
        return -1;
    }

    if (connect(socket->sockfd, (struct sockaddr*)&socket->server_addr, sizeof(socket->server_addr)) == -1) {
        perror("Failed to connect to server");
        return -1;
    }

    return 0;
}

// Function to send data over the socket
int send_data(Socket* socket, const char* data) {
    ssize_t bytes_sent = send(socket->sockfd, data, strlen(data), 0);
    if (bytes_sent == -1) {
        perror("Failed to send data");
        return -1;
    }

    return 0;
}

// Function to receive data from the socket
int receive_data(Socket* socket, char* buffer, size_t buffer_size) {
    ssize_t bytes_received = recv(socket->sockfd, buffer, buffer_size, 0);
    if (bytes_received == -1) {
        perror("Failed to receive data");
        return -1;
    }

    return bytes_received;
}

// Function to close the socket
void close_socket(Socket* socket) {
#ifdef _WIN32 // Windows-specific code
    closesocket(socket->sockfd);
    WSACleanup();
#else
    close(socket->sockfd);
#endif
    free(socket);
}

// Function to create a server socket
Socket* create_server_socket(int port) {
    Socket* socket = create_socket();

    // Allow reusing the address (useful for quick restarts of the server)
    int opt = 1;
    if (setsockopt(socket->sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) == -1) {
        perror("Failed to set SO_REUSEADDR");
        close_socket(socket);
        return NULL;
    }

    // Bind to a specific port
    socket->server_addr.sin_family = AF_INET;
    socket->server_addr.sin_addr.s_addr = INADDR_ANY;
    socket->server_addr.sin_port = htons(port);
    if (bind(socket->sockfd, (struct sockaddr*)&socket->server_addr, sizeof(socket->server_addr)) == -1) {
        perror("Failed to bind socket");
        close_socket(socket);
        return NULL;
    }

    return socket;
}

// Function to listen for incoming connections
int listen_for_connections(Socket* server_socket, int backlog) {
    if (listen(server_socket->sockfd, backlog) == -1) {
        perror("Failed to listen for connections");
        return -1;
    }

    return 0;
}

// Function to accept an incoming connection
Socket* accept_connection(Socket* server_socket) {
    Socket* client_socket = create_socket();
    socklen_t client_addr_len = sizeof(client_socket->server_addr);

    client_socket->sockfd = accept(server_socket->sockfd, (struct sockaddr*)&client_socket->server_addr, &client_addr_len);
    if (client_socket->sockfd == -1) {
        perror("Failed to accept connection");
        free(client_socket);
        return NULL;
    }

    return client_socket;
}
void send_data_s(Socket *socket,const char *msg){
if (send_data(socket, msg) == -1) {
        perror("Failed to send data");
        exit(0);
    }
}
#endif
