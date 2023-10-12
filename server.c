#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdbool.h>
#define STRING_IMP
#include "string/string_t.h"
#define SOCKET_INIT
#include "socket_layer.h"
#define PORT 12345
#define MAX_CLIENTS 5

typedef struct {
    Socket* socket;
    String* name;
} Client;

Client clients[MAX_CLIENTS];
int num_clients = 0;

// Broadcast a message from the sender to all other clients
void broadcast_message(Client* sender, const char* message) {
    String* formatted_message = allocStr(256 * 2);
    snprintf((char*)formatted_message->value, formatted_message->size, "%s: %s\n", c_str(sender->name), message);

    for (int i = 0; i < num_clients; i++) {
        if (clients[i].socket != sender->socket) {
            send_data_s(clients[i].socket, c_str(formatted_message));
        }
    }
}

void client_side(Client* client) {
    String* name = allocStr(256 * 2);
    receive_data(client->socket, (char*)name->value, name->size);
    client->name = name;

    // Notify the client that they have successfully connected
    send_data_s(client->socket, "You are now connected to the chat server.\n");

    while (true) {
        String* buffer = allocStr(256 * 2);
        int err = receive_data(client->socket, (char*)buffer->value, buffer->size);
        if (err == -1) {
            break;
        }

        broadcast_message(client, c_str(buffer));
    }

    printf("Client disconnected: %s\n", c_str(client->name));

    // Remove the client from the list of connected clients
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].socket == client->socket) {
            close_socket(client->socket);
            for (int j = i; j < num_clients - 1; j++) {
                clients[j] = clients[j + 1];
            }
            num_clients--;
            break;
        }
    }

    free(client->name);
}

void* client_thread(void* arg) {
    Socket* client_socket = (Socket*)arg;

    if (num_clients >= MAX_CLIENTS) {
        send_data_s(client_socket, "Chat room is full. Connection rejected.\n");
        close_socket(client_socket);
        return NULL;
    }

    // Add the new client to the list of connected clients
    clients[num_clients].socket = client_socket;
    num_clients++;

    client_side(&clients[num_clients - 1]);
    close_socket(client_socket);
    return NULL;
}

int main(void) {
    Socket* server_socket = create_server_socket(PORT);
    if (server_socket == NULL) {
        fprintf(stderr, "Error creating server socket\n");
        return 1;
    }
    if (listen_for_connections(server_socket, MAX_CLIENTS) == -1) {
        fprintf(stderr, "Error listening for connections\n");
        close_socket(server_socket);
        return 1;
    }

    while (1) {
        Socket* client_socket = accept_connection(server_socket);
        if (client_socket == NULL) {
            fprintf(stderr, "Error accepting connection\n");
            continue;
        }

        // Create a new thread to handle the client
        pthread_t thread;
        if (pthread_create(&thread, NULL, client_thread, (void*)client_socket) != 0) {
            perror("Error creating thread");
            close_socket(client_socket);
        }
    }

    close_socket(server_socket);
    return 0;
}
