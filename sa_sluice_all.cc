            // Translate internal operational metrics into a clean text JSON string
            char json_payload[BUFFER_SIZE];
            snprintf(json_payload, sizeof(json_payload),
                     "{\"verdict\":%d,\"amplitude\":%.3f,\"measured_length\":%d,\"breed\":%d,\"total\":%d,\"admitted\":%d}",
                     , global_metrics.last_amplitude, 
                     global_metrics.last_length, global_metrics.last_breed,
                     global_metrics.total_processed, global_metrics.total_admitted);
            
            // Build the HTTP  with explicit CORS permission headers
            snprintf(http_response, sizeof(http_response),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: application/json\r\n"
                     "Access-Control-Allow-Origin: *\r\n"
                     "Content-Length: %d\r\n\r\n"
                     "%s", (int)strlen(json_payload), json_payload);
                     
            send(client_socket, http_response, strlen(http_response), 0);
            close(client_socket);
        }
    }
    return 0;
}
