#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_BUFFER_SIZE 1024
#define DEFAULT_THREADS 4
#define DEFAULT_DURATION 5 // Benchmark duration in seconds

// Mocking external system variables/functions typically present in bridge or quantum_sluice
typedef struct {
    int gate_id;
    unsigned long packets_processed;
    double latency_sum;
    int is_active;
} SluiceGateMetrics;

// Global control flag
volatile int keep_running = 1;

// Function declarations (implemented across your modules)
void* process_sluice_stream(void* arg);
void print_benchmark_report(SluiceGateMetrics* metrics, int thread_count);

// Core Benchmark Logic Worker Thread
void* process_sluice_stream(void* arg) {
    SluiceGateMetrics* metrics = (SluiceGateMetrics*)arg;
    struct timespec start, end;
    
    printf("[Core] Starting worker thread for Sluice Gate ID: %d\n", metrics->gate_id);
    
    while (keep_running) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        // Simulating data throttling processing loop ("Sluicing")
        usleep(rand() % 10000); // 0-10ms simulated workload latency
        metrics->packets_processed++;
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        // Accumulate latency metrics (in milliseconds)
        double elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + 
                         (end.tv_nsec - start.tv_nsec) / 1000000.0;
        metrics->latency_sum += elapsed;
    }
    
    pthread_exit(NULL);
}

// Generate the performance metrics summary
void print_benchmark_report(SluiceGateMetrics* metrics, int thread_count) {
    unsigned long total_packets = 0;
    double avg_latency = 0.0;
    
    printf("\n==================================================\n");
    printf("        SA-SLUICE CORE BENCHMARK REPORT           \n");
    printf("==================================================\n");
    printf("%-10s %-20s %-15s\n", "Gate ID", "Packets Processed", "Avg Latency (ms)");
    
    for (int i = 0; i < thread_count; i++) {
        double gate_avg = metrics[i].packets_processed > 0 ? 
                          (metrics[i].latency_sum / metrics[i].packets_processed) : 0;
        
        printf("%-10d %-20lu %-15.4f\n", metrics[i].gate_id, metrics[i].packets_processed, gate_avg);
        total_packets += metrics[i].packets_processed;
        avg_latency += gate_avg;
    }
    
    printf("--------------------------------------------------\n");
    printf("Total Throughput: %lu tasks/packets processed\n", total_packets);
    printf("Global Average Latency: %.4f ms\n", (avg_latency / thread_count));
    printf("==================================================\n");
}

int main(int argc, char* argv[]) {
    int thread_count = DEFAULT_THREADS;
    int duration = DEFAULT_DURATION;
    
    // Parse arguments simple override
    if (argc >= 2) thread_count = atoi(argv[1]);
    if (argc >= 3) duration = atoi(argv[2]);
    
    srand(time(NULL));
    printf("[Core] Initializing sa-sluice system benchmark...\n");
    printf("[Core] Thread workers: %d | Duration: %d seconds\n", thread_count, duration);
    
    pthread_t* threads = malloc(sizeof(pthread_t) * thread_count);
    SluiceGateMetrics* metrics = malloc(sizeof(SluiceGateMetrics) * thread_count);
    
    // Initialize structures and deploy worker threads
    for (int i = 0; i < thread_count; i++) {
        metrics[i].gate_id = 100 + i;
        metrics[i].packets_processed = 0;
        metrics[i].latency_sum = 0.0;
        metrics[i].is_active = 1;
        
        if (pthread_create(&threads[i], NULL, process_sluice_stream, (void*)&metrics[i]) != 0) {
            perror("[Core] Failed to create benchmarking thread");
            return 1;
        }
    }
    
    // Let benchmark run for designated test duration
    sleep(duration);
    
    // Stop worker loops safely
    printf("\n[Core] Testing duration complete. Signalling shutdown...\n");
    keep_running = 0;
    
    // Wait for all gates to spin down
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Print the collected metrics
    print_benchmark_report(metrics, thread_count);
    
    // Cleanup allocated buffers
    free(threads);
    free(metrics);
    printf("[Core] System context released successfully.\n");
    
    return 0;
}
