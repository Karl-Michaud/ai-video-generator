import sys


def add_style(ass_file: str) -> None:
    """
    Function adds custom style to provided .ass file.
    """
    with open(ass_file, "r") as file:
        data = file.readlines()
    
    # Custom style for the subtitles. Will be inserted in the provide .ass file.
    custom_style = """
    [Styles]
    Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, BackColour, Bold, Italic, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding
    Style: Default,Arial,20,&H00FFFFFF,&H000000FF,&H00000000,-1,0,1,1.0,0.0,2,10,10,10,1
    """

    data.insert(data.index("[Events]") - 1, custom_style)

    with open(ass_file, "w") as file:
        file.writelines(data)

if __name__ == "__main__":
    # get command line argument
    ass_file = sys.argv[1] # since argv[0] is customize-subs.py
    add_style(ass_file)


