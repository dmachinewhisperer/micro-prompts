# Micro Prompt
An extensible LLM prompting library for embedded micros/processors written in C

## Supported Providers
~~1. Hugging Face~~
2. Google
3. Openai (WIP)
4. Grok (WIP)
5. Anthropic (WIP)

## Supported Frameworks
1. Bare metal
2. ESP-IDF
3. Arduino

## Rroject Structure
This is basically an ESP-IDF component. To support arduino enviroment, files in /ino provide a thin c++ encapsulation layer around the c codebase and a provides a struture the IDE understands. 

## Usage
ESP-IDF: Add as an independent component to your project. Files in ino/ are irrelevant to IDF projects and may be removed. 
Arduino: Clone the repository. copy micro-prompt/src/common into ino/src/. Move ino/ to the arduino library directory(usually documents/Arduino/libraries on windows). Find examples in /ino/examples/

The library is also available in the Arduino library manager. (WIP)

## Docs (WIP)

## Messsage distribution infra
/...