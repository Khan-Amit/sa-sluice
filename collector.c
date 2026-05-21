#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

// CHANGED: Points directly to the shared 8082 data line configuration
#define TARGET_HOST "127.0.0.1"
#define TARGET_PORT 8082

int main(int argc, char const *argv[]) {
    (void)argc; (void)argv;
    int sock_fd;
    struct sockaddr_in serv_addr;
    srand(time(NULL));

    printf("====================================================\n");
    printf("     SA-SLUICE BG METRICS COLLECTOR ACTIVE          \n");
    printf("====================================================\n");

    while (1) {
        if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            sleep(2);
            continue;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(TARGET_PORT);

        if (inet_pton(AF_INET, TARGET_HOST, &serv_addr.sin_addr) <= 0) {
            close(sock_fd);
            return -1;
        }
        
        // Attempt linking up to the core server
        if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            close(sock_fd);
            sleep(2); // If core is recycling, wait and loop again safely
            continue;
        }

        printf("[Collector] Data stream pipeline locked. Synchronizing...\n");
        close(sock_fd);
        usleep(500000); 
    }
    return 0;
}
