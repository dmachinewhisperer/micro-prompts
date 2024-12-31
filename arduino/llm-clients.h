#pragma once

#ifdef ESP32
    #include <HTTPClient.h>
#elif defined(ESP8266)
    #include <ESP8266HTTPClient.h>
#endif
#include "types.h"

char* prompt(HTTPClient &httpclient, LLMClientConfig *llmconfigs);
