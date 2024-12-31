#include "types.h"

// example default configuration
const LLMClientConfig DEFAULT_LLMCLIENT_CONFIG = {
    .llmconfig = {
        .chat = 0,                           // oneshot prompt mode
        .provider = GOOGLE_GEMINI,                  // default provider
        .base_url = "",
        .api_key = "",
        .model_name = "",
        .version = "",
        //the max_tokens specified here is just not arbitary. The http client is configured
        //with a max response payload size of 3kb. Average lenght of a token in english is around 
        //4 so 512 * 4 = 2kb reserving the remaining 1kb for approx error slack.  
        .max_tokens = 512,
        .temperature = 0.7f,
        .top_p = 1,
        .top_k = 50
    },
    .llmdata = {                           
        .prompt = "Enter your prompt here",
        .file = {.file_uri = NULL},         // initialize union with file_uri
        .mime = NULL
    },
    .user_state = NULL                      // optional user state pointer
};

//model configurations
const ProviderFeaturePool provider_google_gemini = {
    .supported_features = {
        TEXT_ONLY,
        TEXT_WITH_REMOTE_FILE,
        TEXT_WITH_LOCAL_BASE64_ENCODED_FILE,
    },
    .num_supported_features = 3,
    .provider = GOOGLE_GEMINI,
};