/* =====================================================================
 * sa-sluice : Quantum Sluice Screening Engine & Web Hosting Core
 * Architecture Flow: Step 3 (Screens Data -> Calculates Metrics -> Hosts for HTML)
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SLUICE_INPUT_PORT 8081  /* Listens for translated data from the Bridge */
#define HTML_OUTPUT_PORT 8082   /* Serves the finished, screened metrics to index.html */
#define BUFFER_SIZE 4096
#define MAX_ALLOWED_LENGTH 4096

// Global state indicators that accumulate based on real incoming collector traffic
int total_scrutinized = 0;
int total_admitted = 0;
char last_json_verdict[BUFFER_SIZE] = "{\"verdict\":0,\"amplitude\":0.0,\"measured_length\":0,\"breed\":1}";

void scrutinize_and_store(char *incoming_json) {
    int breed = 1, length = 0;
    // Parse the data out of the translated JSON text stream
    sscanf(incoming_json, "{\"breed\":%d,\"measured_length\":%d}", &breed, &length);

    total_scrutinized++;
    int verdict = -1;
    float amplitude = 0.0f;

    // RUN THE GATE SCREENING RULES
    if (length <= MAX_ALLOWED_LENGTH && length > 0) {
        verdict = 1; // PERMITTED
        total_admitted++;
        amplitude = (float)(MAX_ALLOWED_LENGTH - length) / MAX_ALLOWED_LENGTH;
    }

    // Save the genuine real-world calculation state to memory
    snprintf(last_json_verdict, sizeof(last_json_verdict),
             "{\"verdict\":%d,\"amplitude\":%.3f,\"measured_length\":%d,\"breed\":%d,\"total\":%d,\"admitted\":%d}",
             verdict, amplitude, length, breed, total_scrutinized, total_admitted);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Spin up the Core Sluice Data Listener
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SLUICE_INPUT_PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    printf("🌊 Step 3: Quantum Sluice Engine Live [Port %d]. Processing data and hosting for HTML...\n", SLUICE_INPUT_PORT);

    // To handle two separate network paths on a cell phone cleanly, we fork a background process to host port 8082 for HTML
    if (fork() == 0) {
        // CHILD PROCESS: Acts as a web socket server delivering metrics to index.html
        int web_fd = socket(AF_INET, SOCK_STREAM, 0);
        setsockopt(web_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        struct sockaddr_in web_addr;
        web_addr.sin_family = AF_INET;
        web_addr.sin_addr.s_addr = INADDR_ANY;
        web_addr.sin_port = htons(HTML_OUTPUT_PORT);
        bind(web_fd, (struct sockaddr *)&web_addr, sizeof(web_addr));
        listen(web_fd, 10);

        while (1) {
            int web_client = accept(web_fd, (struct sockaddr *)&web_addr, (socklen_t*)&addrlen);
            if (web_client >= 0) {
                char http_response[BUFFER_SIZE * 2];
                read(web_client, buffer, BUFFER_SIZE); // Clear request data block
                
                // Pack full HTTP response with CORS permissions so mobile browsers don't block it
                snprintf(http_response, sizeof(http_response),
                         "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %d\r\n\r\n%s",
                         (int)strlen(last_json_verdict), last_json_verdict);
                send(web_client, http_response, strlen(http_response), 0);
                close(web_client);
            }
        }
    }

    // MAIN PROCESS LOOP: Continuously receives data from the bridge translator
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket >= 0) {
            memset(buffer, 0, BUFFER_SIZE);
            read(client_socket, buffer, BUFFER_SIZE - 1);
            scrutinize_and_store(buffer);
            close(client_socket);
        }
    }
    return 0;
}
