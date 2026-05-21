#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8082

int main(void) {
    int sock_fd;
    struct sockaddr_in serv_addr;

    printf("[Virtual Harvester] Spawning local test bench data stream...\n");

    while (1) {
        if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            sleep(1);
            continue;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

        if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            close(sock_fd);
            sleep(1); // Core loading, wait and loop again
            continue;
        }

        printf("[Pipeline Linked] Pumping high-density packet strings into core...\n");

        while (1) {
            // Signal a high-volume data block influx containing taxonomic indicators
            char stream_payload[] = "POST /INPUT_STREAM HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 22\r\n\r\nDOMAIN=2&CLASS=20&DATA=1\r\n";
            
            if (send(sock_fd, stream_payload, strlen(stream_payload), 0) < 0) {
                printf("[Pipeline Break] Connection reset. Re-establishing socket tracks...\n");
                break; 
            }
            usleep(100000); // Super-fast 100ms transmission loop for heavy stress testing
        }
        close(sock_fd);
    }
    return 0;
}
