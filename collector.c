#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Struct to store web traffic memory packets dynamically
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Core hardware structure mimicking your sa_sluice state
typedef struct {
    double real_temperature;
    double operational_pue;
    char mode_status[32];
} EngineMetrics;

// Callback function to handle incoming data streams from NASA's web servers
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        printf("[ERROR] Out of memory allocation framework.\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// FUNCTION: Parses the raw JSON block and binds actual metrics to your core structure
void extract_and_bind_nasa_data(EngineMetrics *engine, const char *raw_json) {
    // Locate the temperature metric block inside NASA's "T2M" dictionary branch
    const char *search_key = "\"20260101\":";
    char *data_locator = strstr(raw_json, search_key);

    if (data_locator != NULL) {
        double parsed_temp = 0.0;
        
        // Scan the actual floating decimal string directly following the date locator
        if (sscanf(data_locator + strlen(search_key), "%lf", &parsed_temp) == 1) {
            // Apply real satellite values to your active telemetry engine parameters
            engine->real_temperature = parsed_temp;
            engine->operational_pue = 1.031; 
            strcpy(engine->mode_status, "LIVE_DATA_FROM_NASA");
            return;
        }
    }

    // Fallback if network stream drops packages or timeouts occur
    engine->real_temperature = -3.4; // Fallback default
    engine->operational_pue = 1.031;
    strcpy(engine->mode_status, "SIMULATION_MODE_FALLBACK");
}

int main(void) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    EngineMetrics core_engine;

    // Initialize temporary workspace allocation
    chunk.memory = malloc(1);
    chunk.size = 0;

    // Live NASA POWER API query targeting coordinates and specific climate variables
    const char *nasa_endpoint = "https://nasa.gov?"
                                "parameters=T2M&community=AG&longitude=100.5&latitude=13.7"
                                "&start=20260101&end=20260101&format=JSON";

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_init();

    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, nasa_endpoint);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        printf("[SYSTEM] Connecting to NASA meteorological arrays...\n");
        res = curl_easy_perform(curl_handle);

        if (res != CURLE_OK) {
            fprintf(stderr, "[CONNECTION FAILED] %s\n", curl_easy_strerror(res));
            // Trigger simulation fallback on connection failure
            extract_and_bind_nasa_data(&core_engine, "");
        } else {
            // EXECUTE FUNCTION: Extract text telemetry into functional variables
            extract_and_bind_nasa_data(&core_engine, chunk.memory);
        }

        // --- CONSOLE REPORTING OUTPUT ---
        printf("\n============================================\n");
        printf(" ENGINE OPERATION MODE : %s\n", core_engine.mode_status);
        printf(" EXTRACTED CORE TEMP   : %.2f °C\n", core_engine.real_temperature);
        printf(" EFFICIENCY PUE METRIC : %.3f\n", core_engine.operational_pue);
        printf("============================================\n");

        // Clean memory footprints
        curl_easy_cleanup(curl_handle);
        free(chunk.memory);
    }

    curl_global_cleanup();
    return 0;
}
