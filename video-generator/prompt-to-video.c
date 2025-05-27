#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define OUT_FILE "./IO/ai-out"
#define OUT_AUDIO "./IO/output.aiff"

#define OUT_VIDEO "./videos/output.mp4"
#define OUT_VIDEO_SUBS "./videos/output-with-subs.mp4"
#define DEFAULT_VIDEO "./videos/mine_vid.mp4"

// Uncomment to have audio file converted to wav or mp3:
// #define AUDIO "-wav"
// #define AUDIO "-mp3"
#ifdef AUDIO
#define DEFINED 1
#else
#define DEFINED 0
// This will not affect the code since not defined, but necessary for compilation as the macro did not exist before
#define AUDIO "Undefined" 
#endif

#define BUFF_SIZE 1028
#define ANS_BUFF_SIZE 5000

typedef char choice_t; // Either set to y or n

char *IN_VIDEO;

/*
 * Function adds subtitles to produced video.
 */ 
void add_subs() {
    pid_t pid_c;
    pid_c = fork();
    if (pid_c < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid_c == 0) {
        printf("Adding subtitles...");
        execl("./add-subs", "add-subs", OUT_VIDEO, OUT_VIDEO_SUBS, NULL);
        perror("Execl failed. Subtitles unsuccessfully added");
        exit(1);
    } else {
        int status_c;
        if (wait(&status_c) == -1) {
            perror("Wait failed");
            exit(1);
        }
        if (WIFEXITED(status_c)) {
            if (WEXITSTATUS(status_c) == 1) {
                perror("Error. Video subtitles failed to be added");
                exit(1);
            }
        }
        printf("Completed generating the video!");
    }
}


/*
 * Function merges audio with produced video.
 */ 
void merge_video() {
    pid_t pid_c;
    pid_c = fork();
    if (pid_c < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid_c == 0) {
        printf("Making video.");
        execl("./merge-audio", "merge-audio", OUT_AUDIO, IN_VIDEO, OUT_VIDEO, NULL);
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
    if (argc > 2) {
        fprintf(stderr, "Proper usage:\n"
                "./prompt-to-audio <background-video>\n"
                "Choose video. Default will be the minecraft parkour video \n");    
        exit(1);
    } else if (argc == 1) {
        fprintf(stdout, "Warning. Default usage requires user to define DEFAULT_VIDEO macro in current file.\n");
        IN_VIDEO = malloc(strlen(DEFAULT_VIDEO) + 1);
        if (IN_VIDEO == NULL) {
            perror("Malloc failed");
            exit(1);
        }
        strcpy(IN_VIDEO, DEFAULT_VIDEO);
    } else {
        IN_VIDEO = malloc(strlen(argv[1]) + 1);
        if (IN_VIDEO == NULL) {
            perror("Malloc failed");
            exit(1);
        }
        strcpy(IN_VIDEO, argv[1]);
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
                    printf("Convert answer to video or re-prompt or exit? [y/n/e]\n");
                    while (fread(&choice, 1, 1, stdin) != 1) {
                        printf("Convert answer to video or re-prompt or exit? [y/n/e]\n");
                    }
                    if (choice == 'e') {
                        return 0;
                    }
                    if (choice != 'y') {
                        // Makes sure that we only have y or n
                        choice = 'n';
                    } else {
                        convert_prompt_to_audio();
                        
                        merge_video();

                        add_subs();
                    }
                    
               }
            }
        }

    }
    return 0;
}
