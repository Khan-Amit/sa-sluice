/* =====================================================================
 * sa-sluice : Language Bridge & Web Translator Layer
 * Developer / Concept Builder: © Seliim Ahmed (seliim.ahmed@gmail.com)
 * Core Philosophy: Translating the polymorphic breeds into universal binary
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BRIDGE_PORT 8081       /* The port your HTML web page hits */
#define BACKEND_PORT 8080      /* The port your native C engine is on */
#define BUFFER_SIZE 4096

void handle_http_request(int client_socket) {
    int backend_socket;
    struct sockaddr_in backend_addr;
    char buffer[BUFFER_SIZE] = {0};
    char backend_response[BUFFER_SIZE] = {0};
    
    // 1. Establish a zero-overhead loop channel to the native C core
    if ((backend_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return;
    }
    
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(BACKEND_PORT);
    backend_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(backend_socket, (struct sockaddr *)&backend_addr, sizeof(backend_addr)) < 0) {
        // Core engine is offline, send error code state to HTML
        char *err_res = "HTTP/1.1 502 Bad Gateway\r\nAccess-Control-Allow-Origin: *\r\n\r\n";
        send(client_socket, err_res, strlen(err_res), 0);
        close(backend_socket);
        return;
    }
    
    // 2. Read the raw metrics line directly out of the C server registers
    // Sending a tiny 3-byte test payload header [Breed: 0x01, Length: 512]
    unsigned char mock_header[3] = {0x01, 0x02, 0x00}; 
    send(backend_socket, mock_header, 3, 0);
    
    ssize_t rec_bytes = read(backend_socket, backend_response, BUFFER_SIZE);
    close(backend_socket);
    
    if (rec_bytes <= 0) return;

    // 3. Format the Language Bridge translation headers for HTML browser validation
    char http_header[BUFFER_SIZE];
    snprintf(http_header, sizeof(http_header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: application/json\r\n"
             "Access-Control-Allow-Origin: *\r\n"  /* Prevents local browser security blocks */
             "Content-Length: %d\r\n\r\n"
             "%s", (int)strlen(backend_response), backend_response);
             
    // 4. Ship the translated binary packet payload over to the HTML interface
    send(client_socket, http_header, strlen(http_header), 0);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Bridge Failure: Socket generation error");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(BRIDGE_PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bridge Failure: Bind allocation block");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 10) < 0) {
        perror("Bridge Failure: Port listen rejected");
        exit(EXIT_FAILURE);
    }
    
    printf("🔗 sa-sluice Language Bridge Online [Port %d] -> Translating to HTML...\n", BRIDGE_PORT);
    
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket >= 0) {
            handle_http_request(client_socket);
            close(client_socket);
        }
    }
    return 0;
}
