#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

#define TARGET_HOST "127.0.0.1"
#define TARGET_PORT 8082

int main(int argc, char const *argv[]) {
    (void)argc; (void)argv;
    int sock_fd;
    struct sockaddr_in serv_addr;
    srand(time(NULL));

    printf("====================================================\n");
    printf("     SA-SLUICE DATA STREAM COLLECTOR WORKING        \n");
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
        
        if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            close(sock_fd);
            sleep(1); 
            continue;
        }

        printf("[Collector] Ping transmitted to core mesh endpoint.\n");
        close(sock_fd);
        
        // FIXED: Replaced long 50-second delays with a snappy 500ms cycle
        usleep(500000); 
    }
    return 0;
}
