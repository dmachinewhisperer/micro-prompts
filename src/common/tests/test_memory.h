#ifndef LLMCLIENT_LIB_TESTS_H
#define LLMCLIENT_LIB_TESTS_H

#include "../cJSON/cJSON.h"
#include "../llm-types.h"

cJSON *___cJSON_CreateObject();
cJSON *___cJSON_CreateArray();

void run_memory_test(int n_creates, LLMClientConfig *config, char* (*build_request)(LLMClientConfig*));
#endif