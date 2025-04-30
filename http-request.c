#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>

#define RESPONSE_SIZE 10000
#define USER_PROMPT_SIZE 1000

#define API_URL "https://api.deepseek.com/v1/chat/completions"

const char *API_KEY;
const char *PY_PATH;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    strncat((char *)userp, (char *)contents, realsize);
    return realsize;
}

int main() {
    // Set env variables
    API_KEY = getenv("API_KEY");
    PY_PATH = getenv("PYTHON_PATH");

    char user_prompt[USER_PROMPT_SIZE];
    fprintf(stdout, "Enter prompt: \n");
    while (fgets(user_prompt, USER_PROMPT_SIZE, stdin) <= 0) {
        fprintf(stderr, "Enter prompt: \n");
    }


    FILE *fp = fopen("ai-out.json", "w");
    CURL *curl;
    CURLcode res;

    char response[RESPONSE_SIZE] = {0}; // Adjust size based on expected response
    
    /*
    const char *json_data =
        "{"
        "\"model\": \"deepseek-chat\","
        "\"messages\": [{\"role\": \"user\", \"content\": \"Hello, tell me a joke\"}],"
        "\"temperature\": 0.7"
        "}"; */
    size_t len = strlen(user_prompt);
    if (len > 0 && user_prompt[len - 1] == '\n') {
        user_prompt[len - 1] = '\0';  // Remove the newline
    }

    char json_data[1024];
    snprintf(json_data, sizeof(json_data), 
            "{"
            "\"model\": \"deepseek-chat\","
            "\"messages\": [{\"role\": \"user\", \"content\": \"%s\"}],"
            "\"temperature\": 0.7"
            "}", user_prompt);

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
            fclose(fp);
            int r = fork();
            if (r < 0) {
                perror("fork failed");
                exit(1);
            } else if (r == 0) {
                // Extract data from json file
                execl(PY_PATH, "python3", "read-json.py", NULL);
                perror("Failed extraction");
                exit(1);
            } else {
                int status;
                if (wait(&status) == -1) {
                    perror("wait");
                    exit(1);
                }

                if (!WIFEXITED(status)) {
                    fprintf(stderr, "Child never terminated");
                }
            }
            
        } else {
            fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    
    return 0;
}
