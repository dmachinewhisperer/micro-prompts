#ifndef LLM_CLIENTS_H_
#define LLM_CLIENTS_H_
#include "espidf-http-rest-api.h"

char* prompt(esp_http_client_handle_t client, LLMClientConfig *llmconfigs);

#endif