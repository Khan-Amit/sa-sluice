#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define TELEMETRY_INTERVAL_US 500000 // Send data updates every 500ms

// Dashboard System Parameter Metrics Structures
typedef struct {
    float pue;                 // Power Usage Effectiveness
    double lifetime_opex;      // OPEX savings accumulated
    float transient_exhaust;   // Thermal sensor telemetry
    int scrutinized_count;     // Monitored data packets
    int admittance_ratio;      // Channel flow throughput capacity
} TelemetryData;

// Global engine flags
volatile int is_pipeline_active = 1;
pthread_mutex_t data_lock = PTHREAD_MUTEX_INITIALIZER;
TelemetryData system_metrics = {1.000, 0.00, 24.5, 0, 100};

// Forward Declarations
void* quantum_sluice_simulator(void* arg);
void* dashboard_ipc_server(void* arg);

// 1. Core Worker Simulator Module (Simulating background workloads)
void* quantum_sluice_simulator(void* arg) {
    (void)arg;
    unsigned int cycle = 0;
    
    printf("[Backend] Quantum Sluice Engine pipeline initialization complete.\n");
    
    while (is_pipeline_active) {
        pthread_mutex_lock(&data_lock);
        
        // Emulate fluctuations matching live hardware data loops
        system_metrics.scrutinized_count++;
        system_metrics.transient_exhaust = 38.2f + ((float)(rand() % 40) / 10.0f);
        system_metrics.admittance_ratio = 100; // Keep optimal transmission flow
        
        // Random micro-additions to active mock OPEX efficiency
        system_metrics.lifetime_opex += 0.05;
        
        // Sluice load calculations targeting perfect 1.000 efficiency baseline
        system_metrics.pue = 1.000 + ((float)(rand() % 5) / 1000.0f);
        
        pthread_mutex_unlock(&data_lock);
        
        cycle++;
        usleep(250000); // Compute state shifts every 250ms
    }
    
    printf("[Backend] Quantum Sluice Engine simulation stream safely stopped.\n");
    pthread_exit(NULL);
}

// 2. Data Transmission Server Module (Prevents UI Timeout Drops)
void* dashboard_ipc_server(void* arg) {
    (void)arg;
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // Creating master socket network descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("[Socket Error] Base descriptor initialization failed");
        pthread_exit(NULL);
    }
    
    // Forcefully attaching socket to the communication port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("[Socket Error] Reuse config mapping rejected");
        close(server_fd);
        pthread_exit(NULL);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("[Socket Error] Local port binding failed");
        close(server_fd);
        pthread_exit(NULL);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("[Socket Error] Connection queue listener failed");
        close(server_fd);
        pthread_exit(NULL);
    }
    
    printf("[IPC Server] Streaming dashboard telemetry socket active on port %d\n", PORT);
    
    while (is_pipeline_active) {
        printf("[IPC Server] Waiting for Dashboard client polling request...\n");
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        
        if (new_socket < 0) {
            if (!is_pipeline_active) break;
            perror("[IPC Server] Handshake rejection encountered");
            continue;
        }
        
        printf("[IPC Server] Dashboard handshake verified! Starting green telemetry pipelines.\n");
        
        // Loop sending live structured JSON or CSV parameters directly to UI
        while (is_pipeline_active) {
            char payload_buffer[512];
            
            pthread_mutex_lock(&data_lock);
            snprintf(payload_buffer, sizeof(payload_buffer),
                     "{\"pue\": %.3f, \"opex\": %.2f, \"exhaust\": %.1f, \"count\": %d, \"admittance\": %d}\n",
                     system_metrics.pue, system_metrics.lifetime_opex, 
                     system_metrics.transient_exhaust, system_metrics.scrutinized_count, 
                     system_metrics.admittance_ratio);
            pthread_mutex_unlock(&data_lock);
            
            // Deliver telemetry frame packet over active connection
            if (send(new_socket, payload_buffer, strlen(payload_buffer), 0) < 0) {
                printf("[IPC Server] Telemetry stream connection severed by target client receiver.\n");
                break;
            }
            
            usleep(TELEMETRY_INTERVAL_US);
        }
        close(new_socket);
    }
    
    close(server_fd);
    pthread_exit(NULL);
}

// 3. Execution Lifecycle Core Entry Point
int main(int argc, char const *argv[]) {
    (void)argc; (void)argv;
    pthread_t simulator_thread, ipc_thread;
    srand(time(NULL));
    
    printf("====================================================\n");
    printf("     INITIALIZING SA-SLUICE CORE CONTROL CORES     \n");
    printf("====================================================\n");
    
    // Spawning concurrent computing modules
    if (pthread_create(&simulator_thread, NULL, quantum_sluice_simulator, NULL) != 0) {
        fprintf(stderr, "[Fatal] Could not initiate background execution core.\n");
        return 1;
    }
    
    if (pthread_create(&ipc_thread, NULL, dashboard_ipc_server, NULL) != 0) {
        fprintf(stderr, "[Fatal] Could not instantiate network delivery socket pipelines.\n");
        is_pipeline_active = 0;
        pthread_join(simulator_thread, NULL);
        return 1;
    }
    
    printf("[Core Setup] Background listeners spinning. Core process fully operational.\n");
    printf("[Action Required] Open your UI frontend app and trigger Green Telemetry now.\n\n");
    printf("Press [ENTER] key in this terminal console frame to shut down the server stack safely.\n");
    
    // Hold program open until developer halts setup manually
    getchar();
    
    printf("\n[Teardown] Graceful termination requested. Cleaning up open network links...\n");
    is_pipeline_active = 0;
    
    pthread_join(ipc_thread, NULL);
    pthread_join(simulator_thread, NULL);
    pthread_mutex_destroy(&data_lock);
    
    printf("[Teardown] All sa-sluice resources closed. Stack isolated safely.\n");
    return 0;
}
