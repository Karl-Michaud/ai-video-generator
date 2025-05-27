#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/*This program merges audio file with video. It will auto-truncate the audio to ensure that the video matches the audio length.*/
int main(int argc, char *argv[]) {
    // Check for proper usage
    if (argc != 4) {
        fprintf(stderr, "Proper Usage: ./merge-audio <audio-file> <video-file> <new-video-file>\n");
        exit(1);
    }

    pid_t pid;
    pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        printf("Making video.");
        char *FFMPEG_PATH = getenv("FFMPEG_PATH");
        if (FFMPEG_PATH == NULL) {
            fprintf(stderr, "Consult Makefile for proper environmental variable setup\n");
            exit(1);
        }
        execl(FFMPEG_PATH, "ffmpeg", "-y", "-stream_loop", "-1", "-i", argv[2], "-i", argv[1], "-map", "0:v:0",
                "-map", "1:a:0", "-c:v", "libx264", "-c:a", "aac", "-shortest", argv[3], NULL);
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
    return 0;
}
