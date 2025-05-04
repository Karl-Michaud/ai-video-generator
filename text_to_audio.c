#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h> // For open sys call

#define MAX_CHARS 1000

#define OUTPUT_FILE_FLAG "--output-file=output.aiff"
#define OUTPUT_VOICE "--voice=Daniel" // default female voice --voice=Samantha
//#define OUTPUT_VOICE "--voice=Samantha"

#define NUM_FLAGS 2
#define MP3_FLAG "-mp3"
#define WAV_FLAG "-wav"

const char *FFMPEG_PATH;
const char *SAY_PATH;
const char *valid_flags[NUM_FLAGS] = {MP3_FLAG, WAV_FLAG};

/*
 * Function used to display to stderr improper usage message, and exit in case of usage error.
 */
void improper_usage_exit() {
    fprintf(stderr, "Proper Usage: ./text_to_audio [flag] input_file\n");
    fprintf(stderr, "Flags: -mp3 -wav\n");
    fprintf(stderr, "Default Usage (.aiff audio file): ./text_to_audio input_file\n");
    
    exit(1);
}

/*
 * Check if inputed flag is a valid flag. If valid, return 1. Otherwise, return 0.
 */
int check_valid_flag(char *flag) {
    for (int i = 0; i < NUM_FLAGS; i++) {
        if (strcmp(valid_flags[i], flag) == 0) {
            // strcmp returns 0 if both strings are equal
            return 1;
        }
    }
    return 0;
}

/*
 * Return string of the converted output file. Will be used as an argument for execl.
 * Precondition: specified type exists in valid_flags array.
 */
char *audio_converter(char *type) {
    char *audio = malloc(sizeof(char) * 11);
    if (strcmp("-mp3", type) == 0) {
        strcpy(audio, "output.mp3");
    } else if (strcmp("-wav", type) == 0) {
        strcpy(audio, "output.wav");
    }
    audio[10] = '\0';
    return audio;
}

int main(int argc, char *argv[]) {
    // Get environmental variable. Since getenv is not a compile-time constant, it must execute during running time.
    FFMPEG_PATH = getenv("FFMPEG_PATH");
    SAY_PATH = getenv("SAY_PATH");
    if (FFMPEG_PATH == NULL || SAY_PATH == NULL) {
        fprintf(stderr, "Make sure to add environmental variables. Check Makefile for steps.\n");
        exit(1);
    }

    if (argc < 2 || argc > 3) {
        improper_usage_exit();
    }
    if (argc == 3) {
        if (!check_valid_flag(argv[1])) {
            fprintf(stderr, "Invalid Flag \n\n");
            improper_usage_exit();
        }
    }

    // Then all the input arguments are valid
    int fd[2];
    int infile_fd;
    if (argc == 2) {
        infile_fd = open(argv[1], O_RDONLY);
    } else {
        infile_fd = open(argv[2], O_RDONLY);
    }

    if (infile_fd == -1) {
        perror("Unable to open file or file does not exist");
        exit(1);
    }

    if (pipe(fd) == -1) {
        perror("Pipe failed");
        exit(1);
    }
    
    // Fork to delegate work to child process
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid > 0) {
        // Parent Process
        if (close(fd[0]) == -1) {
            perror("Pipe close failed");
            exit(1);
        }
        char buffer[MAX_CHARS] = {'\0'};
        ssize_t bytes_read = read(infile_fd, buffer, MAX_CHARS - 1); 
        if (bytes_read == -1) {
            perror("Read input file failed");
            exit(1);
        }
        buffer[bytes_read] = '\0';
        if (write(fd[1], buffer, bytes_read + 1) == -1) {
            perror("Write to pipe failed");
            exit(1);
        }
        if (close(fd[1]) == -1) {
            perror("Close pipe failed");
            exit(1);
        }
        if (close(infile_fd) == -1) {
            perror("Close input file failed");
            exit(1);
        }
        
        int status;
        wait(&status);
        if (!WIFEXITED(status)) {
            fprintf(stderr, "Child process error\n");
            exit(1);
        }
        
        if (argc == 3) {
            // Convert audio
            pid_t pid_con = fork();
            char *out_file = audio_converter(argv[1]);
            if (pid_con == 0) {
                // Convert audio
                printf("Converting audio to %s\n", argv[1]);
                execl(FFMPEG_PATH, "ffmpeg","y", "-i", "output.aiff", out_file, NULL);

                perror("Audio Conversion failed");
                free(out_file);
                exit(1);

            } else if (pid_con > 0) {
                // Wait to convert
                int status_con;
                wait(&status_con);
                if (!WIFEXITED(status)) {
                    fprintf(stderr, "Audio conversion failed\n");
                    exit(1);
                }
                free(out_file);
                printf("Conversion to %s completed\n", argv[1]);
                
            } else {
                perror("Fork failed");
                exit(1);
            }

        }

        printf("Audio file completed!\n");

    } else {
        // Child Process
        printf("Creation of .aiff audio file\n");
        if (close(fd[1]) == -1) {
            perror("Pipe close failed");
            exit(1);
        }

        char buffer[MAX_CHARS] = {'\0'};
        ssize_t bytes_read;
        bytes_read = read(fd[0], buffer, MAX_CHARS - 1); 
        if (bytes_read == -1) {
            perror("Pipe read failed");
            exit(1);
        }
        if (bytes_read == 0) {
            fprintf(stderr, "Input a non-empty file.");
            exit(1);
        }
        buffer[bytes_read] = '\0';

        // say --output-file=test.aiff [text]
        execl(SAY_PATH, "say", OUTPUT_VOICE, OUTPUT_FILE_FLAG, buffer, NULL);

        // In case execl fails
        perror("Text to Audio conversion failed");
        exit(1);
    }
    return 0;
}

