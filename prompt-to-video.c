#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define AUTO_FLAG "-a"
#define CHECK_FLAG(flag) (strcmp((flag), AUTO_FLAG) == 0)

#define OUT_FILE "ai-out"
#define OUT_AUDIO "output.aiff"
#define IN_VIDEO "./videos/mine_vid.mp4"
#define OUT_VIDEO "./videos/output.mp4"

// Uncomment to have audio file converted to wav or mp3:
// #define AUDIO "-wav"
// #define AUDIO "-mp3"
#ifdef AUDIO
#define DEFINED 1
#else
#define DEFINED 0
#define AUDIO "Undefined" // This will not affect the code since not defined, but necessary for compilation as the macro did not exist before
#endif

#define BUFF_SIZE 1028
#define ANS_BUFF_SIZE 5000

typedef char choice_t; // Either set to y or n


/*
 * Function merges audio with produced video.
 */ 
void merge_video() {
    // ffmpeg -stream_loop -1 -i ./videos/mine_vid.mp4 -i output.aiff \
    // -map 0:v:0 -map 1:a:0 \
    // -c:v libx264 -c:a aac -shortest ./videos/output.mp4
    pid_t pid_c;
    pid_c = fork();
    if (pid_c < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid_c == 0) {
        printf("Making video.");
        char *FFMPEG_PATH = getenv("FFMPEG_PATH");
        execl(FFMPEG_PATH, "ffmpeg", "-stream_loop", "-1", "-i", IN_VIDEO, "-i", OUT_AUDIO, "-map", "0:v:0", 
                "-map", "1:a:0", "-c:v", "libx264", "-c:a", "aac", "-shortest", OUT_VIDEO, NULL);
        perror("Execl failed. Video unsuccessfully made");
        exit(1);
    } else {
        int status_c;
        if (wait(&status_c) == -1) {
            perror("Wait failed");
            exit(1);
        }
        if (WIFEXITED(status_c)) {
            if (WEXITSTATUS(status_c) == 1) {
                perror("Error. Video failed");
                exit(1);
            }
        }
        printf("Completed generating the video!");
    }

}


/*
 * Function converts OUT_FILE to audio file specified by compilation flag AUDIO.
 */
void convert_prompt_to_audio() {
    pid_t pid_c;
    pid_c = fork();
    if (pid_c < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid_c == 0) {
        if (DEFINED) {
            execl("./text_to_audio", "text_to_audio", AUDIO, OUT_FILE, NULL);
        } else {
            execl("./text_to_audio", "text_to_audio", OUT_FILE, NULL);
        }
        perror("Execl failed. Conversion to audio file unsuccessful.");
        exit(1);
    } else {
        int status_c;
        if (wait(&status_c) == -1) {
            perror("Wait failed");
            exit(1);
        }
        if (WIFEXITED(status_c)) {
            if (WEXITSTATUS(status_c) == 1) {
                perror("Error, Conversion to audio file unsuccessful.");
                exit(1);
            }
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc > 2 || (argc == 2 && !CHECK_FLAG(argv[1]))) {
        fprintf(stderr, "Proper usage:\n"
                "./prompt-to-audio [flag]\n"
                "For live feedback before converting to audio: \n"
                "./prompt-to-audio\n"
                "Otherwise, include flag -a for automatic conversion\n");    
    }

    int fd[2];
    pid_t fork_pid;
    choice_t choice = 'n'; // Default choice.
    char buff[BUFF_SIZE];
    while (choice != 'y') {
        printf("Enter video script prompt: \n");
        while (fgets(buff, sizeof(buff), stdin) == 0) {
            fprintf(stderr, "Enter valid prompt: \n");
        }

        if (pipe(fd) == -1) {
            perror("Pipe failed");
            exit(1);
        }

        fork_pid = fork();
        if (fork_pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (fork_pid == 0) {
            // Child process
            if (close(fd[1]) == -1) {
                perror("Close failed");
                exit(1);
            }
            if (dup2(fd[0], fileno(stdin)) == -1) {
                perror("Dupe2 failed");
                exit(1);
            }
            if (close(fd[0]) == -1) {
                perror("Close failed");
                exit(1);
            }
            execl("./http-request", "http-request", NULL);
            perror("Execl failed for http-request");
            exit(1);
        } else {
            if (close(fd[0]) == -1) {
                perror("Close failed");
                exit(1);
            }
            if (write(fd[1], buff, strlen(buff)) == -1) {
                perror("Write failed");
                exit(1);
            }
            if (close(fd[1]) == -1) {
                perror("Close failed");
                exit(1);
            }

            int status;
            if (wait(&status) == -1) {
                perror("Wait failed");
                exit(1);
            }
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 1) {
                    perror("Error http request. Please verify if correct api key");
                    exit(1);
                } else {
                    // Print contents of ai-out file
                    char answer[ANS_BUFF_SIZE] = {'\0'};
                    FILE *fp = fopen(OUT_FILE, "r");
                    if (fp == NULL) {
                        perror("fopen failed");
                        exit(1);
                    }
                    size_t bytes_read;
                    while ((bytes_read = fread(answer, 1, sizeof(buff), fp)) > 0) {
                        printf("\nAI generated script:\n");
                        fwrite(answer, 1, bytes_read, stdout);
                    }

                    // Check if we want to keep the answer, and convert
                    printf("\n\n");
                    printf("Convert answer to video or re-prompt? [y/n]\n");
                    while (fread(&choice, 1, 1, stdin) != 1) {
                        printf("Convert answer to video or re-prompt? [y/n]\n");
                    }
                    if (choice != 'y') {
                        // Makes sure that we only have y or n
                        choice = 'n';
                    } else {
                        convert_prompt_to_audio();
                        
                        merge_video(); 
                    }
                    
               }
            }
        }

    }
    return 0;
}
