/* =====================================================================
 * sa-sluice : High-Velocity Ingestion & Traffic Gatekeeper
 * Developer / Concept Builder: © Seliim Ahmed (seliim.ahmed@gmail.com)
 * Core Philosophy: "All dogs are dogs, but their breeds are different."
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_PORT 8080
#define MAX_BUFFER_SIZE 8192

/* Screening Threshold Parameters */
#define LIMIT_MAX_LENGTH 4096      /* Length Check: Max allowable packet bytes */
#define LIMIT_MAX_SPEED_HZ 200.0   /* Speed Check: Max packets allowed per window */

/* Ternary execution state tracking */
typedef struct {
    int state;          /* -1 = REJECT, 0 = HALT, +1 = PERMIT */
    float amplitude;    /* Load factor metric / resonance amplitude */
} QuantumState;

/* Decodes raw incoming wire metadata without parsing the core data body */
QuantumState scrutinize_ingress_breed(unsigned char breed, uint16_t length) {
    QuantumState outcome;
    
    // 1. SCRUTINIZE BY LENGTH
    if (length > LIMIT_MAX_LENGTH || length <= 0) {
        outcome.state = -1; // HARD REJECT
        outcome.amplitude = 0.0f;
        return outcome;
    }
    
    // 2. ADMIT AND ESTIMATE SYSTEM CAPACITY
    outcome.state = 1;     // PERMIT ENTRY
    outcome.amplitude = (float)(LIMIT_MAX_LENGTH - length) / LIMIT_MAX_LENGTH;
    return outcome;
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    unsigned char network_buffer[MAX_BUFFER_SIZE] = {0};
    
    // Initialize low-level network socket interface
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Sluice Core Error: Socket initialization failed");
        exit(EXIT_FAILURE);
    }
    
    int socket_option = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(socket_option));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Sluice Core Error: Bind execution failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 128) < 0) {
        perror("Sluice Core Error: Network pipeline listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("🌊 Sluice Gate Engine Online [Port %d]\n", SERVER_PORT);
    printf("Listening for real polymorphic traffic streams...\n\n");
    
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            continue;
        }
        
        ssize_t bytes_intercepted = read(client_socket, network_buffer, MAX_BUFFER_SIZE);
        if (bytes_intercepted >= 3) { // Minimal header footprint requirement
            
            /* Extract structural metadata from network wire bytes directly:
               Byte 0 = Breed Flag (0x01 = Linux, 0x02 = HTML, 0x03 = C++)
               Bytes 1-2 = Embedded Packet Length */
            unsigned char data_breed = network_buffer[0];
            uint16_t data_length = (network_buffer[1] << 8) | network_buffer[2];
            
            // Scrutinize the data's metrics at the boundary line
            QuantumState assessment = scrutinize_ingress_breed(data_breed, data_length);
            
            // Compile authentic telemetry output strings to pass straight to the HTML front
            char telemetry_payload[256];
            snprintf(telemetry_payload, sizeof(telemetry_payload),
                     "{\"verdict\":%d,\"amplitude\":%.3f,\"measured_length\":%d,\"breed\":%d}\n",
                     assessment.state, assessment.amplitude, data_length, (int)data_breed);
            
            send(client_socket, telemetry_payload, strlen(telemetry_payload), 0);
        }
        close(client_socket);
    }
    
    close(server_fd);
    return 0;
}
