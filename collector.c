        // 1. Establish the connection exactly ONCE outside the loop
        if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            close(sock_fd);
            sleep(1); 
            continue;
        }

        printf("[Collector] Stream locked! Beginning stable data transmission...\n");

        // 2. RUN THE TELEMETRY PAYLOAD STREAM CONTINUOUSLY 
        while (is_running) {
            char outbound_payload[256];
            
            // Format data into JSON string arrays
            snprintf(outbound_payload, sizeof(outbound_payload),
                     "{\"pue\": 1.002, \"opex\": 450.25, \"exhaust\": 38.5, \"count\": 1, \"admittance\": 100}\n");

            printf("[Collector -> Core] Streaming data block...\n");

            // Send metric data over the EXISTING open socket pipeline
            if (send(sock_fd, outbound_payload, strlen(outbound_payload), 0) < 0) {
                printf("[Error] Pipeline broken. Reconnecting...\n");
                break; // Break loop to execute reconnect routine safely
            }

            // FIXED: The usleep delay happens safely inside the open connection stream!
            usleep(500000); // 500ms stable transmission interval
        }

        // 3. ONLY close the socket if the pipeline crashes or stops
        printf("[Collector] Closing socket connection cleanly.\n");
        close(sock_fd);
