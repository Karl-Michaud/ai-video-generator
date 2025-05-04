#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define OUT_SRT_FILE "output.srt"
#define OUT_ASS "ass=output.ass"
#define OUT_ASS_FILE "output.ass"

#define CUSTOM_SUBS "customize-subs.py"


/*
 * Use Whisper openai model to produce an srt file with the transcript and good time stamps. Notice, the generated srt file is always output.srt
 */
void make_srt(char *whisper_path, char *in_file) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork whisper failed");
        exit(1);
    } else if (pid == 0) {
        execl(whisper_path, "whisper", in_file, "--model", "small", "--output_format", "srt", "--language", "en", NULL);
        // execl(whisper_path, "whisper", in_file, "--model", "small", "--out_format", "srt", NULL);
        perror("whisper failed");
        exit(1);
    } else {
        int status;
        if (wait(&status) == -1) {
            perror("Wait failed");
            exit(1);
        }
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 1) {
                perror("Failed to generate subtitles");
                exit(1);
            }
        }
    }
}


/*
 * Function converts .srt to .ass
 */
void convert_to_ass(char *ffmpeg_path) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork convert to .ass failed");
        exit(1);
    } else if (pid == 0) {
        execl(ffmpeg_path, "ffmpeg", "-y", "-i", OUT_SRT_FILE, OUT_ASS_FILE, NULL);
        perror("Conversion to .ass failed");
        exit(1);
    } else {
        int status;
        if (wait(&status) == -1) {
            perror("Wait failed");
            exit(1);
        }
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 1) {
                perror("Failed to convert from .srt to .ass");
                exit(1);
            }
        }
    }
}


/*
 * Function customizes subtitles by changing the styling in .ass file
 */ 
void customize_subs(char *python_path) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork customize failed");
        exit(1);
    } else if (pid == 0) {
        execl(python_path, "python3", CUSTOM_SUBS, OUT_ASS_FILE, NULL);
        perror("Customizing subs failed");
        exit(1);
    } else {
        int status;
        if (wait(&status) == -1) {
            perror("Wait failed");
            exit(1);
        }
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 1) {
                perror("Failed to customize subtitles");
                exit(1);
            }
        }
    }
}


/*
 * Function burns the subtitles onto the video
 */
void burn_subs(char *ffmpeg_path, char *in_file, char *out_file) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork burn failed");
        exit(1);
    } else if (pid == 0) {
        execl(ffmpeg_path, "ffmpeg", "-y", "-i", in_file, "-vf", OUT_ASS, "-c:a", "copy", out_file, NULL);
        perror("Adding subs failed");
        exit(1);
    } else {
        int status;
        if (wait(&status) == -1) {
            perror("Wait failed");
            exit(1);
        }
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 1) {
                perror("Failed to generate subtitles");
                exit(1);
            }
        }
    }
}




int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Proper usage: ./add-subs [in-file.mp4] [out-file.mp4]");
        exit(1);
    }
    
    char *FFMPEG_PATH = getenv("FFMPEG_PATH");
    char *WHISPER_PATH = getenv("WHISPER_PATH");
    char *PYTHON_PATH = getenv("PYTHON_PATH");

    if (FFMPEG_PATH == NULL || WHISPER_PATH == NULL || PYTHON_PATH == NULL) {
        fprintf(stderr, "Make sure to add environmental variables. Check Makefile for steps.\n");
        exit(1);
    }
    
    make_srt(WHISPER_PATH, argv[1]);
    convert_to_ass(FFMPEG_PATH);
    // Working on fixing this feature
    // customize_subs(PYTHON_PATH);
    burn_subs(FFMPEG_PATH, argv[1], argv[2]);


    return 0;

    

}
