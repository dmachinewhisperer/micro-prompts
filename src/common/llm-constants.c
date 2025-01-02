#include <stddef.h>
#include "llm-types.h"

// example default configuration
const LLMClientConfig DEFAULT_LLMCLIENT_CONFIG = {
    .llmconfig = {
        .chat = 0,                           // oneshot prompt mode
        .provider = GOOGLE_GEMINI,                  // default provider
        .base_url = NULL,
        .api_key = NULL,
        .model_name = NULL,
        .version = NULL,
        .max_tokens = MAX_LLM_OUTPUT_TOKENS,
        .temperature = 0.7f,
        .json_response_schema = NULL,
        .top_p = 1,
        .top_k = 50
    },
    .llmdata = {                           
        .prompt = NULL,
        .file = {.file_uri = NULL},         // initialize union with file_uri
        .mime = NULL
    },
    .user_state = NULL                      // optional user state pointer
};

//model configurations
const ProviderFeaturePool provider_google_gemini = {
    .supported_features = {
        TEXT_INPUT,
        TEXT_INPUT_WITH_REMOTE_FILE_URL,
        TEXT_INPUT_WITH_LOCAL_BASE64_FILE,
        TEXT_INPUT_WITH_STRUCTURED_OUTPUT,
    },
    .num_supported_features = 4,
    .provider = GOOGLE_GEMINI,
};