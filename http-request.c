#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define API_KEY "sk-a28c21f3656e411c9b7d7b2a00c635d1"  // <-- Replace with your actual key
#define API_URL "https://api.deepseek.com/v1/chat/completions"

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    strncat((char *)userp, (char *)contents, realsize);
    return realsize;
}

int main() {
    FILE *fp = fopen("ai-out.json", "w");
    CURL *curl;
    CURLcode res;

    char response[10000] = {0}; // Adjust size based on expected response

    const char *json_data =
        "{"
        "\"model\": \"deepseek-chat\","
        "\"messages\": [{\"role\": \"user\", \"content\": \"Hello, tell me a joke\"}],"
        "\"temperature\": 0.7"
        "}";

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        char auth_header[256];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", API_KEY);
        headers = curl_slist_append(headers, auth_header);

        curl_easy_setopt(curl, CURLOPT_URL, API_URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            fprintf(fp, "%s", response);
            
        } else {
            fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    fclose(fp);
    return 0;
}
