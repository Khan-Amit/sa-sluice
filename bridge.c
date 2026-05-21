/* =====================================================================
 * sa-sluice : Language Bridge / Translator Layer
 * Architecture Flow: Step 2 (Receives from Collector -> Packs JSON -> Sends to Sluice)
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define COLLECTOR_IN_PORT 8080   /* Intercepts raw data from collector.c */
#define SLUICE_OUT_PORT 8081     /* Forwards translated data to quantum_sluice.c */
#define BUFFER_SIZE 4096

int main() {
    int server_fd, client_socket, sluice_socket;
    struct sockaddr_in address, sluice_addr;
    int addrlen = sizeof(address);
    unsigned char buffer[BUFFER_SIZE] = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(COLLECTOR_IN_PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    printf("🔗 Step 2: Language Bridge Online [Port %d]. Awaiting Collector data...\n", COLLECTOR_IN_PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) continue;

        ssize_t bytes_read = read(client_socket, buffer, BUFFER_SIZE);
        if (bytes_read >= 3) {
            // Extract information from the Collector packet
            unsigned char breed = buffer[0];
            uint16_t length = (buffer[1] << 8) | buffer[2];

            // TRANSLATION LAYER: Convert raw binary frame into a clean text JSON packet
            char json_translation[BUFFER_SIZE];
            snprintf(json_translation, sizeof(json_translation),
                     "{\"breed\":%d,\"measured_length\":%d}\n", (int)breed, length);

            // Forward the translated JSON string directly to Quantum Sluice (Port 8081)
            sluice_socket = socket(AF_INET, SOCK_STREAM, 0);
            sluice_addr.sin_family = AF_INET;
            sluice_addr.sin_port = htons(SLUICE_OUT_PORT);
            sluice_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

            if (connect(sluice_socket, (struct sockaddr *)&sluice_addr, sizeof(sluice_addr)) >= 0) {
                send(sluice_socket, json_translation, strlen(json_translation), 0);
            }
            close(sluice_socket);
        }
        close(client_socket);
    }
    return 0;
}
