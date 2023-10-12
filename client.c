#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#define STRING_IMP
#include "string/string_t.h"
#define SOCKET_INIT
#include "socket_layer.h"
#define SERVER_IP "127.0.0.1"
#define PORT 12345

String* receive_server_msgs(Socket* client) {
    String* msg = allocStr(256 * 2);
    receive_data(client, (char*)msg->value, msg->size);
    return msg;
}

int send_to_server(Socket* client) {
    String* msg = allocStr(256 * 2);
    printf("Say your message: ");
    scanf(" %[^\n]", (char*)msg->value); // Read a line of text (including spaces)
    int sent = send_data(client, c_str(msg));
    return sent;
}

int main() {
    Socket* client_socket = create_socket();
    if (connect_to_server(client_socket, SERVER_IP, PORT) == -1) {
        fprintf(stderr, "Error connecting to the server\n");
        close_socket(client_socket);
        return 1;
    }

    char name[256 * 2];
    printf("Say your name: ");
    scanf(" %[^\n]", name); // Read a line of text (including spaces)

    // Send the user's name to the server
    send_data(client_socket, name);

    while (true) {
        send_to_server(client_socket);
        String* serv_msg = receive_server_msgs(client_socket);
        printf("%s\n", c_str(serv_msg));
    }

    close_socket(client_socket);
    return 0;
}
