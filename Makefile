.PHONY: clean_audio clean_all all install setup-env

CC = gcc
FLAGS_OBJECT = -Wall -c
FLAGS = -Wall -o
CURL_FLAGS_COMP = $(shell curl-config --cflags)
CURL_FLAGS_LINK = $(shell curl-config --libs)

PYTHON_PATH = $(shell which python3)
SAY_PATH = $(shell which say)
FFMPEG_PATH = $(shell which ffmpeg)

############# Enter API Key for deepseek ai #################
KEY = "PLACEHOLDER"
#############################################################

all: install setup-env text_to_audio http-request

text_to_audio: text_to_audio.o
	$(CC) $(FLAGS) $@ $<

http-request: http-request.o
	$(CC) $(FLAGS) $@ $< $(CURL_FLAGS_LINK)

text_to_audio.o: text_to_audio.c
	$(CC) $(FLAGS_OBJECT) $<

http-request.o: http-request.c
	$(CC) $(FLAGS_OBJECT) $< $(CURL_FLAGS_COMP)

setup-env:
	# Comment lines below, and uncomment, if using bash.
	# echo export API_KEY=\"$(KEY)\" >> ~/.bashrc
	# echo export FFMPEG_PATH=\"$(FFMPEG_PATH)\" >> ~/.bashrc
	# echo export PYTHON_PATH=\"$(PYTHON_PATH)\" >> ~/.bashrc
	# echo export SAY_PATH=\"$(SAY_PATH)\" >> ~/.bashrc
	# @echo "TYPE THE FOLLOWING: source ~/.bashrc"

	# For zsh terminal (Used by Macos)
	echo export API_KEY=\"$(KEY)\" >> ~/.zshrc
	echo export FFMPEG_PATH=\"$(FFMPEG_PATH)\" >> ~/.zshrc
	echo export PYTHON_PATH=\"$(PYTHON_PATH)\" >> ~/.zshrc
	echo export SAY_PATH=\"$(SAY_PATH)\" >> ~/.zshrc
	@echo "TYPE THE FOLLOWING: source ~/.zshrc"

		

install:
	brew install ffmpeg
	brew install curl  

clean_audio:
	rm -rf *.wav *.mp3 *.aiff

clean_all: clean_audio
	rm -rf *.o text_to_audio http-request
