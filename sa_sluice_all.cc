#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

// Live Public Mirror streaming real-world seismic/environmental event feeds
#define REMOTE_HOST "earthquake.usgs.gov"
#define REMOTE_PORT 80
#define CORE_PORT 8082

int main(void) {
    int remote_fd, core_fd;
    struct hostent *server;
    struct sockaddr_in server_addr, core_addr;
    char request[] = "GET /earthquakes/feed/v1.0/summary/all_hour.geojson HTTP/1.1\r\n"
                     "Host: earthquake.usgs.gov\r\n"
                     "User-Agent: SluiceBenchHarvester/1.0\r\n"
                     "Connection: close\r\n\r\n";

    printf("====================================================\n");
    printf("     SA-SLUICE LIVE EARTHDATA EXTRACTION BLOCK      \n");
    printf("====================================================\n");

    while (1) {
        printf("[Live Feed] Connecting to global USGS satellite/sensor node...\n");
        
        server = gethostbyname(REMOTE_HOST);
        if (server == NULL) {
            fprintf(stderr, "[Error] DNS lookup failed for live feed host.\n");
            sleep(5);
            continue;
        }

        remote_fd = socket(AF_INET, SOCK_STREAM, 0);
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        server_addr.sin_port = htons(REMOTE_PORT);

        if (connect(remote_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            printf("[Error] Live remote network feed unreachable. Retrying...\n");
            close(remote_fd);
            sleep(5);
            continue;
        }

        printf("[Live Feed] Connected! Pumping global geo-stream payload data...\n");
        send(remote_fd, request, strlen(request), 0);

        // Connect to your local Sluice Core Engine to pass the data
        core_fd = socket(AF_INET, SOCK_STREAM, 0);
        core_addr.sin_family = AF_INET;
        core_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        core_addr.sin_port = htons(CORE_PORT);

        if (connect(core_fd, (struct sockaddr *)&core_addr, sizeof(core_addr)) >= 0) {
            char chunk_buffer[4096];
            int bytes_received;

            // Stream real packets straight from the internet source into your core engine
            while ((bytes_received = recv(remote_fd, chunk_buffer, sizeof(chunk_buffer) - 1, 0)) > 0) {
                chunk_buffer[bytes_received] = '\0';
                
                // Package the live text records inside a custom Sluice framework header
                char outbound_payload[8192];
                snprintf(outbound_payload, sizeof(outbound_payload), 
                         "POST /LIVE_STREAM HTTP/1.1\r\nContent-Length: %d\r\n\r\n%s", 
                         bytes_received, chunk_buffer);
                
                send(core_fd, outbound_payload, strlen(outbound_payload), 0);
                usleep(50000); // 50ms smooth data flow rate
            }
            close(core_fd);
        } else {
            printf("[Warning] Local sa_sluice_core engine is offline. Start it on port 8082.\n");
        }

        close(remote_fd);
        printf("[Live Feed] Batch complete. Refreshing stream cycle in 10 seconds...\n");
        sleep(10);
    }
    return 0;
}
