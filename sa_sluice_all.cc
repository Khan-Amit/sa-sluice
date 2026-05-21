/* =====================================================================
 * sa-sluice : Unified High-Performance Core Engine
 * Developer / Concept Builder: © Seliim Ahmed (seliim.ahmed@gmail.com)
 * Philosophy: Ingestion -> Zero-Copy Translation -> Sluice Screening -> Web Delivery
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define WEB_PORT 8082
#define BUFFER_SIZE 4096
#define MAX_ALLOWED_LENGTH 4096

// Core Ingestion tracking structures
typedef struct {
    int total_processed;
    int total_admitted;
    int last_breed;
    int last_length;
    float last_amplitude;
    int last_verdict;
} SluiceMetrics;

SluiceMetrics global_metrics = {0, 0, 1, 0, 0.0f, 0};

// Step 1 & 2: Simulating direct, raw data collection and translation internally
void crunch_real_data_event() {
    global_metrics.total_processed++;
    
    // Simulating true polymorphic source incoming variables
    int simulated_breed = (rand() % 3) + 1; // 1=Linux, 2=HTML, 3=C++
    int simulated_length = (rand() % 3500) + 600; // Real-world payload length variation
    
    global_metrics.last_breed = simulated_breed;
    global_metrics.last_length = simulated_length;
    
    // Step 3: Sluice Gate Scrutiny Logic
    if (simulated_length <= MAX_ALLOWED_LENGTH) {
        global_metrics.last_verdict = 1; // PERMITTED
        global_metrics.total_admitted++;
        global_metrics.last_amplitude = (float)(MAX_ALLOWED_LENGTH - simulated_length) / MAX_ALLOWED_LENGTH;
    } else {
        global_metrics.last_verdict = -1; // THROTTLED
        global_metrics.last_amplitude = 0.0f;
    }
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char request_buffer[BUFFER_SIZE] = {0};
    char http_response[BUFFER_SIZE * 2] = {0};
    
    // Seed randomizer for genuine system load testing
    srand(time(NULL));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(WEB_PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Unified Engine Core Bind Failure");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 20) < 0) {
        perror("Pipeline Queue Failure");
        exit(EXIT_FAILURE);
    }
    
    printf("🌊 sa-sluice Unified Engine Active on Port %d\n", WEB_PORT);
    printf("Standing by to deliver data directly to your index.html interface...\n\n");
    
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket >= 0) {
            memset(request_buffer, 0, BUFFER_SIZE);
            read(client_socket, request_buffer, BUFFER_SIZE - 1);
            
            // Generate a fresh live data crunch calculation on every browser ping pulse
            crunch_real_data_event();
            
            // Translate internal operational metrics into a clean text JSON string
            char json_payload[BUFFER_SIZE];
            snprintf(json_payload, sizeof(json_payload),
                     "{\"verdict\":%d,\"amplitude\":%.3f,\"measured_length\":%d,\"breed\":%d,\"total\":%d,\"admitted\":%d}",
                     global_metrics.last_verdict, global_metrics.last_amplitude, 
                     global_metrics.last_length, global_metrics.last_breed,
                     global_metrics.total_processed, global_metrics.total_admitted);
            
            // Build the HTTP stream with explicit CORS permission headers
            snprintf(http_response, sizeof(http_response),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: application/json\r\n"
                     "Access-Control-Allow-Origin: *\r\n"
                     "Content-Length: %d\r\n\r\n"
                     "%s", (int)strlen(json_payload), json_payload);
                     
            send(client_socket, http_response, strlen(http_response), 0);
            close(client_socket);
        }
    }
    return 0;
}
