#include <fcntl.h> // Required for non-blocking keyboard input configuration

// ... inside main ...

int is_running = 1; // 1 = Active telemetry streaming, 0 = Paused/Standby
char input_ch;

// Set standard input (keyboard) to non-blocking mode so it doesn't pause the loop
fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

printf("Controls: Press [S] then [Enter] to toggle START/STOP telemetry streaming.\n\n");

while (1) {
    // Check if the developer typed something to toggle the switch
    if (read(STDIN_FILENO, &input_ch, 1) > 0) {
        if (input_ch == 's' || input_ch == 'S') {
            is_running = !is_running; // Toggle switch state
            if (is_running) {
                printf("\n[Collector Switch] >>> STARTING Telemetry Pipeline Stream...\n");
            } else {
                printf("\n[Collector Switch] ||| STOPPED / STANDBY mode activated.\n");
            }
        }
    }

    // If the switch is stopped, skip network operations and idle smoothly
    if (!is_running) {
        usleep(200000); // 200ms idle sleep to save CPU usage
        continue;
    }

    // --- YOUR INJECTED CODE LOGIC STARTS HERE ---
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        sleep(1);
        continue;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TARGET_PORT);

    if (inet_pton(AF_INET, TARGET_HOST, &serv_addr.sin_addr) <= 0) {
        close(sock_fd);
        return -1; // Critical config failure
    }
    // --- YOUR INJECTED CODE LOGIC ENDS HERE ---

    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock_fd);
        sleep(1); 
        continue;
    }

    printf("[Collector] Telemetry ping transmitted to core.\n");
    close(sock_fd);
    
    usleep(500000); // Smooth 500ms data transmission rate
}
