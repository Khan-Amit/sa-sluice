#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8082

typedef struct {
    float pue;                 
    double lifetime_opex;      
    float transient_exhaust;   
    int scrutinized_count;     
    int admittance_ratio;      
} TelemetryData;

volatile int is_pipeline_active = 1;
pthread_mutex_t data_lock = PTHREAD_MUTEX_INITIALIZER;
TelemetryData system_metrics = {1.000, 0.00, 24.5, 0, 100};

void* quantum_sluice_simulator(void* arg) {
    (void)arg;
    while (is_pipeline_active) {
        pthread_mutex_lock(&data_lock);
        system_metrics.scrutinized_count++;
        system_metrics.transient_exhaust = 36.4f + ((float)(rand() % 40) / 10.0f);
        system_metrics.lifetime_opex += 0.05;
        system_metrics.pue = 1.000 + ((float)(rand() % 4) / 1000.0f);
        pthread_mutex_unlock(&data_lock);
        usleep(250000); 
    }
    pthread_exit(NULL);
}

void* dashboard_ipc_server(void* arg) {
    (void)arg;
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        pthread_exit(NULL);
    }
    
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(server_fd);
        pthread_exit(NULL);
    }
    
    if (listen(server_fd, 10) < 0) {
        close(server_fd);
        pthread_exit(NULL);
    }
    
    while (is_pipeline_active) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) continue;
        
        char request_buffer[1024] = {0};
        recv(new_socket, request_buffer, sizeof(request_buffer) - 1, 0);
        
        // Handle browser CORS preflight check requests instantly without blocking
        if (strncmp(request_buffer, "OPTIONS", 7) == 0) {
            char options_response[] = 
                "HTTP/1.1 204 No Content\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
                "Access-Control-Allow-Headers: Content-Type\r\n"
                "Connection: close\r\n\r\n";
            send(new_socket, options_response, strlen(options_response), 0);
            close(new_socket);
            continue;
        }
        
        char payload[256];
        pthread_mutex_lock(&data_lock);
        snprintf(payload, sizeof(payload),
                 "{\"pue\": %.3f, \"opex\": %.2f, \"exhaust\": %.1f, \"count\": %d, \"admittance\": %d}",
                 system_metrics.pue, system_metrics.lifetime_opex, 
                 system_metrics.transient_exhaust, system_metrics.scrutinized_count, 
                 system_metrics.admittance_ratio);
        pthread_mutex_unlock(&data_lock);
        
        char http_response[1024];
        snprintf(http_response, sizeof(http_response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: application/json\r\n"
                 "Access-Control-Allow-Origin: *\r\n"
                 "Connection: close\r\n\r\n"
                 "%s", payload);
                 
        send(new_socket, http_response, strlen(http_response), 0);
        close(new_socket); // Force clean socket tear-down to avoid page freezes
    }
    close(server_fd);
    pthread_exit(NULL);
}

int main(void) {
    pthread_t simulator_thread, ipc_thread;
    pthread_create(&simulator_thread, NULL, quantum_sluice_simulator, NULL);
    pthread_create(&ipc_thread, NULL, dashboard_ipc_server, NULL);
    
    printf("[Core Setup] Server listening on port %d without hang conditions. Press ENTER to close.\n", PORT);
    getchar();
    
    is_pipeline_active = 0;
    pthread_join(ipc_thread, NULL);
    pthread_join(simulator_thread, NULL);
    pthread_mutex_destroy(&data_lock);
    return 0;
}
