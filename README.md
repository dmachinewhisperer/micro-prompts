# Micro Prompt
An extensible LLM prompting library for embedded micros/processors written in C

## Supported Providers
1. ~~Hugging Face~~
2. Google
3. Openai
4. Groq

## Supported Frameworks
1. Bare metal
2. ESP-IDF
3. Arduino

## Rroject Structure
This is basically an ESP-IDF component. To support arduino enviroment, files in /ino provide a thin c++ encapsulation layer around the c codebase and a provides a struture the IDE understands. 

## Usage
### ESP-IDF
1. Add as an independent component to your project. Files in ino/ are irrelevant to IDF projects and may be removed. 
2. See the bundled examples 
### Arduino
1. Get: 
    1. Library manager: Search for micro-prompt. (WIP)
    2. Clone the repository. copy `micro-prompt/src/common` into `ino/src/`. Move `ino/` to the arduino library directory(usually `Documents/Arduino/libraries` on windows). Find examples in `/ino/examples/`

    3. Alternatively, run the edit the powershell script `install-ino.ps1` first line 
`$arduino_lib_path = "C:\Users\{username}\Documents\Arduino\libraries\"` to point to your Arduino lib path and run script to install. 

4. See the bundled examples and quick starter in 

## Dependencies
The only dependency is cJSON by [DaveGamble](https://github.com/DaveGamble/cJSON) bundled with the project. 
## Docs
[Here](documentation.md)
