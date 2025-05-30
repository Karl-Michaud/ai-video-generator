#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>

#define RESPONSE_SIZE 10000
#define USER_PROMPT_SIZE 5000
#define JSON_FILE "./IO/ai-out.json"

#define API_URL "https://api.deepseek.com/v1/chat/completions"

const char *API_KEY;
const char *PY_PATH;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    strncat((char *)userp, (char *)contents, realsize);
    return realsize;
}

char *escape_json_string(const char *input) {
    size_t len = strlen(input);
    // Allocate a buffer that is 6x the original size to be safe
    char *escaped = malloc(len * 6 + 1);
    if (!escaped) return NULL;

    char *dst = escaped;
    for (size_t i = 0; i < len; ++i) {
        char c = input[i];
        switch (c) {
            case '\"': *dst++ = '\\'; *dst++ = '\"'; break;
            case '\\': *dst++ = '\\'; *dst++ = '\\'; break;
            case '\b': *dst++ = '\\'; *dst++ = 'b';  break;
            case '\f': *dst++ = '\\'; *dst++ = 'f';  break;
            case '\n': *dst++ = '\\'; *dst++ = 'n';  break;
            case '\r': *dst++ = '\\'; *dst++ = 'r';  break;
            case '\t': *dst++ = '\\'; *dst++ = 't';  break;
            default:
                *dst++ = c;
        }
    }
    *dst = '\0';
    return escaped;
}


int main() {
    // Set env variables
    API_KEY = getenv("API_KEY");
    PY_PATH = getenv("PYTHON_PATH");

    if (API_KEY == NULL || PY_PATH == NULL) {
        fprintf(stderr, "Make sure to add environmental variables. Check Makefile for steps.\n");
        exit(1);
    }

    char user_prompt[USER_PROMPT_SIZE] = {'\0'};  // Guarantee null termination
    // Old code
    /*fprintf(stdout, "Enter prompt: \n");
    while (fgets(user_prompt, USER_PROMPT_SIZE, stdin) <= 0) {
        fprintf(stderr, "Enter prompt: \n");
    }*/

    if (read(fileno(stdin), user_prompt, USER_PROMPT_SIZE) == -1) {
        perror("Read from stdin");
        exit(1);
    }
    user_prompt[USER_PROMPT_SIZE - 1] = '\0';


    FILE *fp = fopen(JSON_FILE, "w");
    CURL *curl;
    CURLcode res; 
    const char* VIDEO_SCRIPT_PROMPT =
        "You create short form content. Follow these rules STRICTLY: "
        "1. Respond ONLY with the video script text. "
        "2. No questions, emojis or engagement hooks. "
        "3. Never address the user (no <Ask me more>, <would you like>, or <let me know if>). "
        "4. If the prompt relates to reddit stories, or similar, the length may exceed 60 seconds."
        "5. Total length: 30-60 seconds of spoken content. "
        "6. Be engaging. Have a sentence introducing the subject. "
        "7. Never include anything beyond the script itself. "
        "8. If I ever read, <Would you like...> or <Let me know if...>, I will implode and die. "
        "9. If rules are broken, I will die, so better not include: <Would you like more details on any of these? 😊> or similar engagement hooks or things like \"title\".\n"
        "Here is the prompt: ";

    char response[RESPONSE_SIZE] = {'\0'};// Adjust size based on expected response

    char new_prompt[USER_PROMPT_SIZE + 300] = {'\0'};
    strcpy(new_prompt, VIDEO_SCRIPT_PROMPT);
    strcat(new_prompt, user_prompt);
    new_prompt[sizeof(new_prompt) - 1] = '\0';

    char *safe_prompt = escape_json_string(new_prompt);
    
    int size_json = strlen(safe_prompt) + 100;
    char json_data[size_json];
    snprintf(json_data, sizeof(json_data), 
            "{"
            "\"model\": \"deepseek-chat\","
            "\"messages\": [{\"role\": \"user\", \"content\": \"%s\"}],"
            "\"temperature\": 0.7"
            "}", safe_prompt);


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
    free(safe_prompt);
    return 0;
}
