#!/bin/bash
OUTPUT_PROGRAM="testbench"

gcc -DBUILD_FOR_TESTING \
    -o $OUTPUT_PROGRAM \
    tests.c \
    test_memory.c \
    ../cJSON/cJSON.c \
    ../llm-constants.c \
    ../utils/base64.c \
    ../utils/debug.c \
    ../providers/google.c \
    ../providers/openai.c \
    -I ../cJSON \
    -I ../providers \
    -I ../utils \
    -I ../tests \
    -Wall -g

if [ $? -eq 0 ]; then
    echo "Compilation successful! You can now run the program with ./$OUTPUT_PROGRAM"
else
    echo "Compilation failed."
fi