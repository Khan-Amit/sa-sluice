#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8082
// FIXED: Changed from any blocking 50-second macro loops to a smooth 500ms interval
#define UPDATE_INTERVAL_US 500000 

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
        system_metrics.admittance_ratio = 100;
        system_metrics.lifetime_opex += 0.05;
        system_metrics.pue = 1.000 + ((float)(rand() % 4) / 1000.0f);
        pthread_mutex_unlock(&data_lock);
        
        // FIXED: Swapped any long system stalls with low-latency micro-sleeps (250ms)
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
        perror("[Error] Socket init failed");
        pthread_exit(NULL);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("[Error] Port reuse configuration rejected");
        close(server_fd);
        pthread_exit(NULL);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("[Error] Local port binding failed");
        close(server_fd);
        pthread_exit(NULL);
    }
    
    if (listen(server_fd, 5) < 0) {
        perror("[Error] Listener initialization failed");
        close(server_fd);
        pthread_exit(NULL);
    }
    
    printf("[IPC Server] Streaming dashboard telemetry socket active on port %d\n", PORT);
    
    while (is_pipeline_active) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            if (!is_pipeline_active) break;
            continue;
        }
        
        char request_buffer[1024] = {0};
        read(new_socket, request_buffer, sizeof(request_buffer) - 1);
        
        char payload[256];
        pthread_mutex_lock(&data_lock);
        snprintf(payload, sizeof(payload),
                 "{\"pue\": %.3f, \"opex\": %.2f, \"exhaust\": %.1f, \"count\": %d, \"admittance\": %d}",
                 system_metrics.pue, system_metrics.lifetime_opex, 
                 system_metrics.transient_exhaust, system_metrics.scrutinized_count, 
                 system_metrics.admittance_ratio);
        pthread_mutex_unlock(&data_lock);
        
        char http_response[512];
        snprintf(http_response, sizeof(http_response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: application/json\r\n"
                 "Access-Control-Allow-Origin: *\r\n"
                 "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
                 "Connection: close\r\n\r\n"
                 "%s", payload);
                 
        send(new_socket, http_response, strlen(http_response), 0);
        close(new_socket);
    }
    close(server_fd);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    (void)argc; (void)argv;
    pthread_t simulator_thread, ipc_thread;
    srand(time(NULL));
    
    printf("====================================================\n");
    printf("     INITIALIZING NO-DELAY SA-SLUICE CORES          \n");
    printf("====================================================\n");
    
    pthread_create(&simulator_thread, NULL, quantum_sluice_simulator, NULL);
    pthread_create(&ipc_thread, NULL, dashboard_ipc_server, NULL);
    
    printf("[Core Setup] Server running without 50s delays. Live feeding active.\n");
    printf("Press [ENTER] in this window to shut down cleanly.\n");
    
    getchar();
    
    is_pipeline_active = 0;
    pthread_join(ipc_thread, NULL);
    pthread_join(simulator_thread, NULL);
    pthread_mutex_destroy(&data_lock);
    
    return 0;
}
