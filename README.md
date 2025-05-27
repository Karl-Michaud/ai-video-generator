# 🎬 AI Video Generator

This repository contains an **AI Video Generator**, a C-based tool that automates short-form content video creation using AI. It includes standalone programs for AI prompting, text-to-speech, subtitle generation, and audio/video merging; all combined in one master program for content generation.

This master program is ideal for creating quick AI-powered video content without any subscriptions or extra tools. Just write an idea, confirm the script, and let it do the rest. 🚀


## 📚 Table of Contents

- [Overview](#-overview)
- [Installation Instructions](#-installation-instructions)
- [Standalone Programs](#-standalone-programs)
- [Master Program](#-master-program)
- [Bug Reports & Feature Suggestions](#-bug-reports--feature-suggestions)
- [License](#license)


## 📖 Overview

This project includes **4 standalone programs** and **1 master program**:

1. Each **standalone program** can be used independently for its specific functionality.
2. The **master program** (`prompt-to-video.c`) ties everything together into an automated AI-powered video generation pipeline.

Originally made to avoid overpriced tools and test my C skills, this project uses:
- **C** as the primary language
- Some **Python** for JSON parsing
- System calls like `read`, `write`, `dup2`, `pipe`, `execl`, `fork`, and more


## ⚙️ Installation Instructions

### 📋 Requirements

Make sure you have the following installed:

- `python3`
- `gcc` compiler
- A terminal or shell (Zsh or Bash)
- Homebrew
> ✅ macOS is **HIGHLY** recommended. Other operating systems will require adjustments.

### Step 1: Clone the Repository

```bash
git clone <repo-url>
cd video-generator
```

### Step 2: Install Dependencies

```bash
make install
```

This installs:

- `ffmpeg` – for video/audio processing
- `curl` – for HTTP requests

> ⚠️ Requires Homebrew (for macOS)

### Step 3: Add Your DeepSeek API Key

Edit the `Makefile` and replace:

```make
KEY = "PLACEHOLDER"
```

with:

```make
KEY = "your-api-key-here"
```

### Step 4: Export Required Paths

If you're using Zsh (macOS default):

```bash
make setup-env
source ~/.zshrc
```

If you're using Bash:

Uncomment the Bash lines in the `setup-env` section of the Makefile, then run:

```bash
make setup-env
source ~/.bashrc
```

### Step 5: Build All Executables

```bash
make all
```

This compiles all the programs:

- `text_to_audio`
- `http-request`
- `add-subs`
- `merge-audio`
- `prompt-to-video`

You're now ready to use the tools individually or together through `prompt-to-video`.

---
## 🧱 Standalone Programs

This project includes **four standalone programs** that can be used independently or as part of the master program `prompt-to-video.c`:

### 1. `http-request.c`
Prompts DeepSeek AI directly from the terminal. It parses the JSON response and saves only the AI's generated text into a file called `ai-out`.

> 🔹 Usage:  
> `./http-request`

### 2. `text_to_audio.c`
Converts a text file into an audio file. You can choose between `.aiff`, `.mp3`, or `.wav` formats using flags.

> 🔹 Usage:  
> `./text_to_audio -[flag] input_text_file`  
> (Use `no flag` for `.aiff`, `-mmp3` for `.mp3`, `-wav` for `.wav`)  
> Default (AIFF):  
> `./text_to_audio input_text_file`

### 3. `add-subs.c`
Burns subtitles onto a video. It uses the Whisper model to generate a `.srt` file, converts it to `.ass`, and overlays it on the video.

> 🔹 Usage:  
> `./add-subs input_video.mp4 output_video.mp4`

### 4. `merge-audio.c`
Merges an audio file onto a video and automatically truncates the video to match the audio length.

> 🔹 Usage:  
> `./merge-audio input_audio input_video output_video`


## 🛠 Master Program

The `prompt-to-video` program is the all-in-one tool that automates the creation of short-form AI-generated videos. It combines all standalone programs:

- `http-request.c` to generate the video script via DeepSeek AI
- `text_to_audio.c` to convert the script into audio
- `merge-audio.c` to overlay the audio on a background video (and auto-truncate)
- `add-subs.c` to generate subtitles and burn them onto the video

No manual steps are required once you confirm the AI-generated script — the video is fully created with voiceover and subtitles in one shot.

### Running the Program

There are **two ways to run** `prompt-to-video`:

#### 1. Default Background Video

```bash
./prompt-to-video
```

To use this method, you **must define the `DEFAULT_VIDEO` macro** at the top of `prompt-to-video.c`:

```c
#define DEFAULT_VIDEO "path/to/background-video.mp4"
```

> Don't forget to recompile after editing:
```bash
make prompt-to-video
```

#### 2. Specify a Background Video Manually

```bash
./prompt-to-video path/to/background-video.mp4
```

This method lets you pick any video file on the fly.

### Generated Video

⚠️ The final video will be saved automatically in the `/video-generator/videos` directory.


## 🐛 Bug Reports & Feature Suggestions

If you encounter any bugs or have ideas for new features, feel free to open an issue in the [Issues](../../issues) section.

You can also start a conversation or ask questions in the [Discussions](../../discussions) tab!

## License

MIT License

Copyright (c) 2025 Karl-A. Michaud

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.







