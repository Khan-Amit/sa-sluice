#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Structure to safely handle dynamically sizing web memory payloads
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function to handle incoming data streams from the NASA server
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        printf("Out of memory error (realloc failed)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int main(void) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);  
    chunk.size = 0;           

    // Direct NASA POWER API URL for specific weather variables
    const char *nasa_url = "https://nasa.gov?"
                           "parameters=T2M,PRECTOTCORR,WS2M&"
                           "community=AG&"
                           "longitude=100.5&"
                           "latitude=13.7&"
                           "start=20260101&"
                           "end=20260105&"
                           "format=JSON";

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_init();

    if(curl_handle) {
        // Configure the URL query target
        curl_easy_setopt(curl_handle, CURLOPT_URL, nasa_url);
        
        // Define handling properties for raw response collection
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        printf("Fetching meteorological data streams directly from NASA Server...\n");
        res = curl_easy_perform(curl_handle);

        // Verify transfer status
        if(res != CURLE_OK) {
            fprintf(stderr, "NASA Request Failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("\n--- REAL-TIME METEOROLOGICAL PAYLOAD FROM NASA SECTOR ---\n");
            printf("%s\n", chunk.memory);
        }

        // Cleanup connection objects
        curl_easy_cleanup(curl_handle);
        free(chunk.memory);
    }

    curl_global_cleanup();
    return 0;
}
