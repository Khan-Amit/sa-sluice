#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

#define TARGET_HOST "127.0.0.1"
#define TARGET_PORT 8080
#define BUFFER_SIZE 512

// Structure matching the telemetry signature expected by sa_sluice_core
typedef struct {
    float opex_modifier;
    float heat_factor;
    int processing_cycles;
} RawSensorPacket;

int main(int argc, char const *argv[]) {
    (void)argc; (void)argv;
    int sock_fd = 0;
    struct sockaddr_in serv_addr;
    RawSensorPacket packet = {0.0f, 25.0f, 0};

    printf("====================================================\n");
    printf("     SA-SLUICE BACKGROUND METRICS COLLECTOR         \n");
    printf("====================================================\n");
    printf("[Collector] Initializing aggregation pipelines...\n");

    // Configure loop to robustly handle reconnect safety steps
    while (1) {
        // Create streaming socket interface
        if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            fprintf(stderr, "[Collector Error] Socket initialization failed: %s\n", strerror(errno));
            sleep(2);
            continue;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(TARGET_PORT);

        if (inet_pton(AF_INET, TARGET_HOST, &serv_addr.sin_addr) <= 0) {
            fprintf(stderr, "[Collector Error] Address mapping configuration failed.\n");
            close(sock_fd);
            return -1;
        }

        printf("[Collector] Connecting to Core Telemetry Broker (%s:%d)...\n", TARGET_HOST, TARGET_PORT);
        
        if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            fprintf(stderr, "[Collector Warning] Telemetry broker offline. Re-polling infrastructure in 2s...\n");
            close(sock_fd);
            sleep(2);
            continue;
        }

        printf("[Collector] Link established! Harvesting active quantum telemetry lines.\n");

        // Continuous streaming lifecycle tracking loop
        while (1) {
            char outbound_payload[BUFFER_SIZE];
            
            // Increment dummy tracking values to simulate device polling
            packet.processing_cycles++;
            packet.opex_modifier += 0.02f;
            packet.heat_factor = 32.5f + ((float)(rand() % 50) / 10.0f);

            // Structure telemetry line precisely into JSON configuration parameters
            snprintf(outbound_payload, sizeof(outbound_payload),
                     "{\"pue\": 1.002, \"opex\": %.2f, \"exhaust\": %.2f, \"count\": %d, \"admittance\": 100}\n",
                     packet.opex_modifier, packet.heat_factor, packet.processing_cycles);

            printf("[Collector -> Core] Dispatching: %s", outbound_payload);

            // Send metric data over to sa_sluice_core
            if (send(sock_fd, outbound_payload, strlen(outbound_payload), 0) < 0) {
                fprintf(stderr, "[Collector Error] Data broadcast transmission broken: %s\n", strerror(errno));
                break; // Break nested engine loops to kick-start a full connection recovery trace
            }

            // Sleep 500ms between harvesting cycles to keep pipeline streaming fluidly
            usleep(500000); 
        }

        // Close broken socket endpoints gracefully before triggering reconnect logic
        close(sock_fd);
        printf("[Collector] Network line isolated. Attempting pipeline recovery...\n");
        sleep(1);
    }

    return 0;
}
