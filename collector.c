#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define TARGET_HOST "127.0.0.1"
#define TARGET_PORT 8082

int main(void) {
    int sock_fd;
    struct sockaddr_in serv_addr;

    printf("====================================================\n");
    printf("     SA-SLUICE LIVE PERSISTENT COLLECTOR RUNNING    \n");
    printf("====================================================\n");

    while (1) {
        if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            sleep(1);
            continue;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(TARGET_PORT);

        if (inet_pton(AF_INET, TARGET_HOST, &serv_addr.sin_addr) <= 0) {
            close(sock_fd);
            return -1;
        }
        
        // Connect once and hold it open
        if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            close(sock_fd);
            sleep(1); // Core offline, retry shortly
            continue;
        }

        printf("[Collector] Connection established. Feeding matrix stream data...\n");
        
        // Loop telemetry packets over the active socket link
        while (1) {
            char update_payload[] = "Collector Heartbeat Ack\n";
            if (send(sock_fd, update_payload, strlen(update_payload), 0) < 0) {
                printf("[Warning] Connection lost. Attempting cycle repair...\n");
                break; 
            }
            usleep(500000); // Send data cleanly every 500ms
        }

        close(sock_fd);
    }
    return 0;
}
