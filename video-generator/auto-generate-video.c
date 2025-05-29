/*
 * This file is used to auto generate videos.
 * NOTE: The file containing the ideas must have different ideas on different lines, and must be a .txt file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFF_SIZE 1024


#define OUT_FILE "./IO/ai-out"
#define OUT_AUDIO "./IO/output.aiff"

#define OUT_VIDEO "./videos/output.mp4"
#define OUT_VIDEO_SUBS "./videos/output-with-subs" // Note: This is intentional, since we will append "i.mp4" for 1 <= i <= #lines in input file
#define DEFAULT_VIDEO "./videos/mine_vid.mp4"


char *IN_VIDEO;


/*
 * Function makes unique file names based on the number of files existing already/.
 */
char* make_filename(int i) {
    // Determine required length dynamically
    int needed = snprintf(NULL, 0, "%s%d.mp4", OUT_VIDEO_SUBS, i);
    
    // Allocate just enough space (+1 for null terminator)
    char* filename = malloc(needed + 1);
    if (!filename) {
        perror("malloc failed");
        exit(1);
    }

    // Build the final string
    sprintf(filename, "%s%d.mp4", OUT_VIDEO_SUBS, i);

    return filename;
}


/*
 * Function adds subtitles to produced video.
 */ 
void add_subs(char *output_with_subs_path) {
    pid_t pid_c;
    pid_c = fork();
    if (pid_c < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid_c == 0) {
        printf("Adding subtitles...");
        execl("./add-subs", "add-subs", OUT_VIDEO, output_with_subs_path, NULL);
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
        execl("./text_to_audio", "text_to_audio", OUT_FILE, NULL);
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
    if (argc > 3 || argc < 2) {
        fprintf(stderr, 
                "Error. Improper Usage!\n\n"
                "Proper Usage: ./auto-generate-video <input-file.txt> <background-video.mp4>\n"
                "Note: <background-video.mp4> is optional. If not defined, default video macro will be chosen.\n");
        exit(1);
    }
    else if (argc == 2) {
        fprintf(stdout, "Warning. Default usage requires user to define DEFAULT_VIDEO macro in current file.\n");
        IN_VIDEO = malloc(strlen(DEFAULT_VIDEO) + 1);
        if (IN_VIDEO == NULL) {
            perror("Malloc failed");
            exit(1);
        }
        strcpy(IN_VIDEO, DEFAULT_VIDEO);
    } else {
        IN_VIDEO = malloc(strlen(argv[2]) + 1);
        if (IN_VIDEO == NULL) {
            perror("Malloc failed");
            exit(1);
        }
        strcpy(IN_VIDEO, argv[1]);
    }

    // Variables for logic
    int num_lines = 0;
    char buff[BUFF_SIZE];
    FILE *file_ptr = fopen(argv[1], "r");
    

    if (file_ptr == NULL) {
        perror("fopen failed.");
        exit(1);
    }

    while (fgets(buff, BUFF_SIZE - 1, file_ptr) != NULL) {
        int fd[2];
        if (pipe(fd) == -1) {
            perror("Pipe failed");
            exit(1);
        }
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            // Child process
            if (close(fd[1]) == -1) {
                perror("Close failed");
                exit(1);
            }
            // Use dup2 to copy fd[0] read pipe file descriptor into stdin
            if (dup2(fd[0], fileno(stdin)) == -1) {
                perror("dup2 failed");
                exit(1);
            }
            // Since dup2 has been succesful, we can close read pipe
            if (close(fd[0]) == -1) {
                perror("Close failed");
                exit(1);
            }
            // Run http-request to prompt AI. Note, that this program reads from stdin, hence explaining why we used dup2
            execl("./http-request", "http-request", NULL);

            // If reached, execl failed
            perror("Execl failed");
            exit(1);
        } else {
            // Parent process
            if (close(fd[0]) == -1) {
                perror("Close failed");
                exit(1);
            }

            // Write line idea to write end of the pipe
            if (write(fd[1], buff, strlen(buff)) == -1) {
                perror("Write failed");
                exit(1);
            }
            if (close(fd[1]) == -1) {
                perror("Close failed");
                exit(1);
            }

            pid_t status;
            if (wait(&status) == -1) {
                perror("Wait failed");
                exit(1);
            }

            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 1) {
                    perror("Error http request. Please verify if correct api key in Makefile");
                    exit(1);
                }
                // Make output video file;
                char *filename = make_filename(num_lines);

                // Generate video!
                convert_prompt_to_audio();
                merge_video();
                add_subs(filename);
                
                // Make sure to free to prevent memory leaks
                free(filename);
            }
        }
        num_lines++;
    }

    free(IN_VIDEO);
    fclose(file_ptr);
    return 0;
}

