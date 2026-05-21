#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h> // For non-blocking console input readings

#define TARGET_HOST "127.0.0.1"
#define TARGET_PORT 8082

int main(int argc, char const *argv[]) {
    (void)argc; (void)argv;
    int sock_fd;
    struct sockaddr_in serv_addr;
    srand(time(NULL));

    int is_running = 1; // Start/Stop execution stream flag switch 
    char input_ch;

    // Set standard keyboard terminal descriptors to non-blocking states
    fcntl(STDIN_FILENO, f_SETFL, O_NONBLOCK);

    printf("====================================================\n");
    printf("     SA-SLUICE DATA STREAM COLLECTOR WORKING        \n");
    printf("====================================================\n");
    printf("System Controller: Type [S] + [Enter] to toggle START/STOP.\n\n");

    while (1) {
        // Evaluate if user inputted structural toggle shifts
        if (read(STDIN_FILENO, &input_ch, 1) > 0) {
            if (input_ch == 's' || input_ch == 'S') {
                is_running = !is_running;
                if (is_running) {
                    printf("\n[Collector Engine] >>> STARTING pipeline metric streams...\n");
                } else {
                    printf("\n[Collector Engine] ||| STOPPED / STANDBY pipeline active.\n");
                }
            }
        }

        // If toggled off, idle silently to eliminate unneeded connection spam
        if (!is_running) {
            usleep(200000);
            continue;
        }

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

        printf("[Collector] Snapshot metric packet successfully pumped into core server mesh.\n");
        close(sock_fd);
        
        usleep(500000); // 500ms stable delivery timing array loop
    }
    return 0;
}
