#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8082
#define BUFFER_SIZE 2048

// System Taxonomy Framework
typedef enum { DOMAIN_DOG = 1, DOMAIN_BIRD = 2, DOMAIN_COW = 3, DOMAIN_GOAT = 4 } SluiceDomain;
typedef enum { CLASS_DOMESTIC = 10, CLASS_WILD = 20 } SluiceClass;

typedef struct {
    float pue;
    unsigned long cycle_count;
    unsigned long noise_pruned;
    int is_stalled;
} SystemMetrics;

volatile int is_engine_active = 1;
pthread_mutex_t metric_lock = PTHREAD_MUTEX_INITIALIZER;
SystemMetrics g_metrics = {1.028, 0, 0, 0};

// Taxonomic Classification Router Engine
void execute_taxonomic_route(int domain, int subclass) {
    if (domain == DOMAIN_BIRD) {
        if (subclass == CLASS_WILD) {
            // Internal routing execution step to EAGLE bank
            volatile int target = 102; 
        } else {
            // Internal routing execution step to PARROT bank
            volatile int target = 101;
        }
    }
}

void* server_broker_kernel(void* arg) {
    (void)arg;
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1, addr_len = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0 || listen(server_fd, 20) < 0) {
        printf("[Fatal] Allocation of port 8082 failed.\n");
        pthread_exit(NULL);
    }

    printf("[Sluice Bench Core] Local Virtual Engine Listening on Port %d\n", PORT);

    while (is_engine_active) {
        client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addr_len);
        if (client_socket < 0) continue;

        char rx_buf[BUFFER_SIZE] = {0};
        recv(client_socket, rx_buf, BUFFER_SIZE - 1, 0);

        // CORS Authorization Handshake
        if (strncmp(rx_buf, "OPTIONS", 7) == 0) {
            char cors[] = "HTTP/1.1 204 No Content\r\n"
                          "Access-Control-Allow-Origin: *\r\n"
                          "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                          "Access-Control-Allow-Headers: Content-Type\r\n"
                          "Connection: close\r\n\r\n";
            send(client_socket, cors, strlen(cors), 0);
            close(client_socket);
            continue;
        }

        pthread_mutex_lock(&metric_lock);
        
        // Route Evaluation Parsing
        if (strstr(rx_buf, "/stall") != NULL) {
            g_metrics.is_stalled = 1;
            printf("[Control Signal] System transitioned to STALLED/STANDBY mode.\n");
        } else if (strstr(rx_buf, "/telemetry") != NULL || strstr(rx_buf, "GET / ") != NULL) {
            g_metrics.is_stalled = 0;
        }

        // Parse Incoming Raw Telemetry Inputs from the Collector
        if (strstr(rx_buf, "INPUT_STREAM") != NULL) {
            if (!g_metrics.is_stalled) {
                g_metrics.cycle_count += 50;     // Mock packet block ingestion
                g_metrics.noise_pruned += 2450;  // 98% Noise rejection mapping
                g_metrics.pue = 1.020 + ((float)(rand() % 10) / 1000.0f);
                
                // Execute taxonomic branching logic inside the server slice
                execute_taxonomic_route(DOMAIN_BIRD, CLASS_WILD);
            }
        }

        char payload[512];
        snprintf(payload, sizeof(payload), 
                 "{\"pue\": %.3f, \"cycles\": %lu, \"pruned\": %lu, \"state\": %d}", 
                 g_metrics.pue, g_metrics.cycle_count, g_metrics.noise_pruned, g_metrics.is_stalled);
        
        pthread_mutex_unlock(&metric_lock);

        char response[1024];
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                 "Access-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n%s", payload);

        send(client_socket, response, strlen(response), 0);
        close(client_socket); // Strict Teardown to prevent network lag
    }
    close(server_fd);
    pthread_exit(NULL);
}

int main(void) {
    pthread_t net_thread;
    srand(time(NULL));
    pthread_create(&net_thread, NULL, server_broker_kernel, NULL);
    
    printf("[System Active] Press [ENTER] in this window to stop the software stack safely.\n");
    getchar();

    is_engine_active = 0;
    pthread_join(net_thread, NULL);
    pthread_mutex_destroy(&metric_lock);
    return 0;
}
