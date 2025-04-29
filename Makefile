.PHONY: clean_audio clean_all all install

CC = gcc
FLAGS_OBJECT = -Wall -c
FLAGS = -Wall -o
CURL_FLAGS_COMP = $(shell curl-config --cflags)
CURL_FLAGS_LINK = $(shell curl-config --libs)


all: text_to_audio http-request

text_to_audio: text_to_audio.o
	$(CC) $(FLAGS) $@ $<

http-request: http-request.o
	$(CC) $(FLAGS) $@ $< $(CURL_FLAGS_LINK)

text_to_audio.o: text_to_audio.c
	$(CC) $(FLAGS_OBJECT) $<

http-request.o: http-request.c
	$(CC) $(FLAGS_OBJECT) $< $(CURL_FLAGS_COMP)

install:
	brew install ffmpeg
	brew install curl  

clean_audio:
	rm -rf *.wav *.mp3 *.aiff

clean_all: clean_audio
	rm -rf *.o text_to_audio http-request
