import json
import os
import sys

JSON_FILE = "./IO/ai-out.json"
OUTPUT_FILE = "./IO/ai-out"


def _setup_json(file_name: str) -> dict | None:
    """
    Function opens, and read contents of given json file. Returns a dict of contents in json file.
    """
    try:
        with open(file_name, "r") as json_file:
            return json.load(json_file)
    except FileNotFoundError as error:
        print("Error opening file", file=sys.stderr)
        sys.exit(1)


def _setup_write(file_name: str, data: str) -> None:
    """
    Function opens <file_name>, and writes data to file. Returns nothing
    """
    try:
        with open(file_name, "w") as file:
            file.write(data)
    except FileNotFoundError as error:
        print("Error opening file", file=sys.stderr)
        sys.exit(1)


def extract_message(json_file: str, out_file: str) -> None:
    """
    Function extracts the message sent back from the deepseek API which is stored in json_file. It then writes
    data to specified out_file.
    """
    data = _setup_json(json_file)
    try:
        _setup_write(out_file, data["choices"][0]["message"]["content"])
    except KeyError as error:
        print("API key is invalid", file=sys.stderr)
        sys.exit(1);
    return



if __name__ == "__main__":
    extract_message(JSON_FILE, OUTPUT_FILE)
    
